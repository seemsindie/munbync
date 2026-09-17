#!/usr/bin/env node
'use strict';

/*
 * printer-term — an interactive terminal (REPL) for the MUNBYN ITPP047.
 *
 * Two ways to drive the printer from one prompt:
 *   1. High-level verbs   — `align center`, `bold on`, `print Hello`, `cut`, `status`
 *   2. Raw escape hatch    — `hex 1B 40`, `raw "Hi\n"` for bytes the API doesn't wrap
 *
 * Features: TAB completion of verbs (and their first argument), command
 * history, `help` / `help <verb>`, and a prompt that shows connection state.
 *
 * Run:   node tools/printer-term.js [transport] [address] [port]
 *   node tools/printer-term.js                 # start disconnected (network default)
 *   node tools/printer-term.js 192.168.1.50    # auto-connect network :9100
 *   node tools/printer-term.js 192.168.1.50 9100
 *   node tools/printer-term.js usb /dev/usb/lp0
 *   node tools/printer-term.js serial /dev/ttyUSB0 19200
 */

const readline = require('readline');
const { MunbynPrinter, constants } = require('../lib');

const printer = new MunbynPrinter();
let connection = null; // human-readable description of the active link

// ---------------------------------------------------------------------------
// Small helpers
// ---------------------------------------------------------------------------

const out = (s) => process.stdout.write(s + '\n');
const err = (s) => process.stdout.write('  ! ' + s + '\n');

function onOff(token, dflt = true) {
  if (token === undefined) return dflt;
  const t = String(token).toLowerCase();
  if (['on', 'true', '1', 'yes', 'y'].includes(t)) return true;
  if (['off', 'false', '0', 'no', 'n'].includes(t)) return false;
  throw new Error(`expected on/off, got "${token}"`);
}

function int(token, name) {
  const n = Number(token);
  if (!Number.isFinite(n)) throw new Error(`${name || 'value'} must be a number, got "${token}"`);
  return n | 0;
}

function requireOpen() {
  if (!printer.isOpen) throw new Error('not connected — use `connect <usb|serial|network> <addr>` first');
}

// Parse a string of hex into a Buffer. Accepts "1B 40", "1b40", "0x1B 0x40".
function parseHex(rest) {
  const cleaned = rest.replace(/0x/gi, '').replace(/[\s,]+/g, '');
  if (cleaned.length === 0) throw new Error('no hex bytes given');
  if (cleaned.length % 2 !== 0) throw new Error('hex must have an even number of digits');
  if (!/^[0-9a-fA-F]+$/.test(cleaned)) throw new Error('hex contains non-hex characters');
  return Buffer.from(cleaned, 'hex');
}

// Process C-style escapes in raw text: \n \r \t \0 \\ \xHH
function unescape(s) {
  return s.replace(/\\x([0-9a-fA-F]{2})|\\n|\\r|\\t|\\0|\\\\/g, (m, hex) => {
    if (hex !== undefined) return String.fromCharCode(parseInt(hex, 16));
    return { '\\n': '\n', '\\r': '\r', '\\t': '\t', '\\0': '\0', '\\\\': '\\' }[m];
  });
}

// Tokenize a raw argument string, respecting "double" and 'single' quotes so
// values may contain spaces. Returns an array of unquoted tokens.
function splitArgs(rest) {
  const out = [];
  const re = /"([^"]*)"|'([^']*)'|(\S+)/g;
  let m;
  while ((m = re.exec(rest)) !== null) {
    out.push(m[1] !== undefined ? m[1] : m[2] !== undefined ? m[2] : m[3]);
  }
  return out;
}

// Strip one layer of surrounding quotes if present.
function dequote(s) {
  if (s.length >= 2 && ((s[0] === '"' && s.endsWith('"')) || (s[0] === "'" && s.endsWith("'")))) {
    return s.slice(1, -1);
  }
  return s;
}

// ---------------------------------------------------------------------------
// Command table
// ---------------------------------------------------------------------------
// Each entry: { usage, help, run(args, rest), complete?(args) }
//   args = whitespace-split tokens after the verb
//   rest = the raw remainder of the line after the verb (for print/raw/hex)

const ALIGNMENTS = ['left', 'center', 'right'];
const BARCODE_TYPES = {
  'upc-a': constants.BARCODE_UPC_A, 'upc-e': constants.BARCODE_UPC_E,
  jan13: constants.BARCODE_JAN13, jan8: constants.BARCODE_JAN8,
  code39: constants.BARCODE_CODE39, itf: constants.BARCODE_ITF,
  codebar: constants.BARCODE_CODEBAR, code93: constants.BARCODE_CODE93,
  code128: constants.BARCODE_CODE128,
  'gs1-128': constants.BARCODE_GS1_128,
  'gs1-omni': constants.BARCODE_GS1_DATABAR_OMNI,
  'gs1-truncated': constants.BARCODE_GS1_DATABAR_TRUNCATED,
  'gs1-limited': constants.BARCODE_GS1_DATABAR_LIMITED,
  'gs1-expanded': constants.BARCODE_GS1_DATABAR_EXPANDED,
};
const HRI_POS = {
  none: constants.HRI_NONE, above: constants.HRI_ABOVE,
  below: constants.HRI_BELOW, both: constants.HRI_BOTH,
};

const commands = {
  // --- connection ---
  connect: {
    usage: 'connect <usb|serial|network> <address> [port|baud]',
    help: 'Open a connection. Defaults: network port 9100, serial baud 9600.',
    complete: (args) => (args.length <= 1 ? ['usb', 'serial', 'network'] : []),
    run: (args) => {
      const [kind, addr, extra] = args;
      if (!kind || !addr) throw new Error('usage: connect <usb|serial|network> <address> [port|baud]');
      if (printer.isOpen) printer.close();
      switch (kind) {
        case 'usb':
          printer.openUsb(addr); connection = `usb ${addr}`; break;
        case 'serial':
          printer.openSerial(addr, extra ? int(extra, 'baud') : 9600);
          connection = `serial ${addr}@${extra || 9600}`; break;
        case 'network':
          printer.openNetwork(addr, extra ? int(extra, 'port') : 9100);
          connection = `net ${addr}:${extra || 9100}`; break;
        default:
          throw new Error(`unknown transport "${kind}"`);
      }
      out(`  connected (${connection})`);
    },
  },
  close: { usage: 'close', help: 'Close the current connection.',
    run: () => { printer.close(); connection = null; out('  closed'); } },
  status: { usage: 'status', help: 'Query and print printer status (NOTE: parsing is known-buggy in the lib).',
    run: () => { requireOpen(); out('  ' + JSON.stringify(printer.getStatus())); } },

  // --- core text ---
  init: { usage: 'init', help: 'Initialize the printer (ESC @).', run: () => { requireOpen(); printer.initialize(); } },
  print: { usage: 'print <text>', help: 'Send text verbatim (no newline added).',
    run: (a, rest) => { requireOpen(); printer.print(unescape(dequote(rest))); } },
  println: { usage: 'println <text>', help: 'Send text followed by a newline.',
    run: (a, rest) => { requireOpen(); printer.print(unescape(dequote(rest)) + '\n'); } },
  lf: { usage: 'lf', help: 'Line feed.', run: () => { requireOpen(); printer.lineFeed(); } },
  cr: { usage: 'cr', help: 'Carriage return.', run: () => { requireOpen(); printer.carriageReturn(); } },
  tab: { usage: 'tab', help: 'Horizontal tab.', run: () => { requireOpen(); printer.horizontalTab(); } },
  feed: { usage: 'feed <lines>', help: 'Feed N lines.', run: (a) => { requireOpen(); printer.feedLines(int(a[0], 'lines')); } },

  // --- formatting ---
  align: { usage: 'align <left|center|right>', help: 'Set justification.',
    complete: (args) => (args.length <= 1 ? ALIGNMENTS : []),
    run: (a) => { requireOpen(); printer.align(a[0]); } },
  bold: { usage: 'bold [on|off]', help: 'Toggle emphasis (default on).',
    complete: (args) => (args.length <= 1 ? ['on', 'off'] : []),
    run: (a) => { requireOpen(); printer.bold(onOff(a[0])); } },
  underline: { usage: 'underline [on|off]', help: 'Toggle underline.',
    complete: (args) => (args.length <= 1 ? ['on', 'off'] : []),
    run: (a) => { requireOpen(); printer.underline(onOff(a[0])); } },
  invert: { usage: 'invert [on|off]', help: 'Toggle white-on-black.',
    complete: (args) => (args.length <= 1 ? ['on', 'off'] : []),
    run: (a) => { requireOpen(); printer.invert(onOff(a[0])); } },
  doublestrike: { usage: 'doublestrike [on|off]', help: 'Toggle double-strike.',
    complete: (args) => (args.length <= 1 ? ['on', 'off'] : []),
    run: (a) => { requireOpen(); printer.setDoubleStrike(onOff(a[0])); } },
  rotate90: { usage: 'rotate90 [on|off]', help: 'Toggle 90° rotation.',
    complete: (args) => (args.length <= 1 ? ['on', 'off'] : []),
    run: (a) => { requireOpen(); printer.setRotate90(onOff(a[0])); } },
  upsidedown: { usage: 'upsidedown [on|off]', help: 'Toggle upside-down.',
    complete: (args) => (args.length <= 1 ? ['on', 'off'] : []),
    run: (a) => { requireOpen(); printer.setUpsideDown(onOff(a[0])); } },
  font: { usage: 'font <a|b>', help: 'Select font A or B.',
    complete: (args) => (args.length <= 1 ? ['a', 'b'] : []),
    run: (a) => { requireOpen(); printer.setFont(String(a[0]).toLowerCase() === 'b' ? constants.FONT_B : constants.FONT_A); } },
  scale: { usage: 'scale <width 1-8> <height 1-8>', help: 'Set text size multipliers.',
    run: (a) => { requireOpen(); printer.setTextScale(int(a[0], 'width'), int(a[1], 'height')); } },
  charspacing: { usage: 'charspacing <n>', help: 'Set right-side character spacing.',
    run: (a) => { requireOpen(); printer.setCharacterSpacing(int(a[0], 'n')); } },
  linespacing: { usage: 'linespacing <n|default>', help: 'Set line spacing (or reset).',
    complete: (args) => (args.length <= 1 ? ['default'] : []),
    run: (a) => { requireOpen(); a[0] === 'default' ? printer.setLineSpacingDefault() : printer.setLineSpacing(int(a[0], 'n')); } },
  leftmargin: { usage: 'leftmargin <n>', help: 'Set left margin.',
    run: (a) => { requireOpen(); printer.setLeftMargin(int(a[0], 'n')); } },
  areawidth: { usage: 'areawidth <n>', help: 'Set print area width.',
    run: (a) => { requireOpen(); printer.setPrintAreaWidth(int(a[0], 'n')); } },
  charset: { usage: 'charset <n>', help: 'Set international charset (numeric).',
    run: (a) => { requireOpen(); printer.setInternationalCharset(int(a[0], 'n')); } },
  codepage: { usage: 'codepage <n>', help: 'Set code page (numeric).',
    run: (a) => { requireOpen(); printer.setCodepage(int(a[0], 'n')); } },
  reset: { usage: 'reset', help: 'Cancel all formatting.', run: () => { requireOpen(); printer.cancelAllFormatting(); } },

  // --- barcode ---
  barcode: { usage: 'barcode <type> <data>', help: `Print a 1D barcode. Types: ${Object.keys(BARCODE_TYPES).join(', ')}`,
    complete: (args) => (args.length <= 1 ? Object.keys(BARCODE_TYPES) : []),
    run: (a, rest) => {
      requireOpen();
      const type = BARCODE_TYPES[String(a[0]).toLowerCase()];
      if (type === undefined) throw new Error(`unknown barcode type "${a[0]}"`);
      const data = rest.slice(a[0].length).trim();
      if (!data) throw new Error('barcode data is empty');
      printer.printBarcode(type, dequote(data));
    } },
  bcheight: { usage: 'bcheight <dots>', help: 'Set barcode height.', run: (a) => { requireOpen(); printer.setBarcodeHeight(int(a[0], 'height')); } },
  bcwidth: { usage: 'bcwidth <1-6>', help: 'Set barcode module width.', run: (a) => { requireOpen(); printer.setBarcodeWidth(int(a[0], 'width')); } },
  hri: { usage: 'hri <none|above|below|both>', help: 'Set HRI text position.',
    complete: (args) => (args.length <= 1 ? Object.keys(HRI_POS) : []),
    run: (a) => { requireOpen(); const p = HRI_POS[String(a[0]).toLowerCase()]; if (p === undefined) throw new Error(`bad HRI position "${a[0]}"`); printer.setHriPosition(p); } },
  qr: { usage: 'qr <data> [moduleSize]', help: 'Print a QR code (native GS(k, lib API printQr).',
    run: (a, rest) => {
      requireOpen();
      const tokens = rest.split(/\s+/);
      let size = 6, data = rest;
      const last = tokens[tokens.length - 1];
      if (tokens.length > 1 && /^\d+$/.test(last)) { size = int(last, 'moduleSize'); data = rest.slice(0, rest.lastIndexOf(last)).trim(); }
      data = dequote(data);
      if (!data) throw new Error('qr data is empty');
      printer.printQr(data, size);
    } },
  pdf417: { usage: 'pdf417 <data> [columns]', help: 'Print a PDF417 2D barcode (columns 0=auto).',
    run: (a, rest) => {
      requireOpen();
      const tokens = rest.split(/\s+/);
      let columns = 0, data = rest;
      const last = tokens[tokens.length - 1];
      if (tokens.length > 1 && /^\d+$/.test(last)) { columns = int(last, 'columns'); data = rest.slice(0, rest.lastIndexOf(last)).trim(); }
      data = dequote(data);
      if (!data) throw new Error('pdf417 data is empty');
      printer.printPdf417(data, columns);
    } },

  // --- cut / drawer / hardware ---
  cut: { usage: 'cut [partial|full]', help: 'Cut paper (default partial).',
    complete: (args) => (args.length <= 1 ? ['partial', 'full'] : []),
    run: (a) => { requireOpen(); printer.cutPaper(a[0] === 'full' ? constants.CUT_ONE_POINT_UNCUT : constants.CUT_PARTIAL); } },
  feedcut: { usage: 'feedcut [lines]', help: 'Feed then cut.', run: (a) => { requireOpen(); printer.feedAndCut(a[0] ? int(a[0], 'lines') : 7); } },
  drawer: { usage: 'drawer [pin 2|5]', help: 'Pulse the cash drawer.', run: (a) => { requireOpen(); printer.openDrawerDefault(a[0] === '5' ? constants.DRAWER_PIN_5 : constants.DRAWER_PIN_2); } },
  selftest: { usage: 'selftest', help: 'Run the printer self-test.', run: () => { requireOpen(); printer.selfTest(); } },
  nvlogo: { usage: 'nvlogo [n] [mode]', help: 'Print NV-stored logo n (FS p).',
    run: (a) => { requireOpen(); printer.printNvBitImage(a[0] ? int(a[0], 'n') : 1, a[1] ? int(a[1], 'mode') : 0); } },
  testprint: { usage: 'testprint [n] [m]', help: 'Enter hex-dump mode (GS ( A), n=0/48, m=1/49; power-cycle to exit.',
    run: (a) => { requireOpen(); printer.executeTestPrint(a[0] ? int(a[0], 'n') : 0, a[1] ? int(a[1], 'm') : 1); } },
  buzzer: { usage: 'buzzer [count] [duration]', help: 'Sound the buzzer (ESC B, MUNBYN). count/duration 1-9.',
    run: (a) => { requireOpen(); printer.buzzer(a[0] ? int(a[0], 'count') : 3, a[1] ? int(a[1], 'duration') : 2); } },
  panel: { usage: 'panel <on|off>', help: 'Enable/disable the FEED panel button (ESC c 5).',
    complete: (args) => (args.length <= 1 ? ['on', 'off'] : []),
    run: (a) => { requireOpen(); printer.setPanelButtons(onOff(a[0])); } },
  wifi: { usage: 'wifi <ssid> <password> [keyType]', help: 'Set WiFi creds (USB only!). Quote values with spaces. keyType 0-8, default 8=WPA/WPA2 mixed.',
    run: (a, rest) => {
      requireOpen();
      const parts = splitArgs(rest);
      if (parts.length < 2) throw new Error('usage: wifi <ssid> <password> [keyType] — quote values containing spaces');
      const keyType = parts[2] !== undefined ? int(parts[2], 'keyType') : constants.WIFI_WPA_WPA2_MIXED;
      printer.setWifi(parts[0], parts[1], keyType);
      out(`  WiFi creds sent (ssid="${parts[0]}", keyType=${keyType}) — power-cycle the printer to apply.`);
    } },
  dhcp: { usage: 'dhcp <on|off>', help: 'Enable/disable DHCP (USB only!).',
    complete: (args) => (args.length <= 1 ? ['on', 'off'] : []),
    run: (a) => { requireOpen(); printer.setDhcp(onOff(a[0])); } },
  wifistatic: { usage: 'wifistatic <ssid> <password> <ip> <mask> <gateway> [keyType]', help: 'Set WiFi + static IP (USB only!). IPs dotted, e.g. 192.168.1.50.',
    run: (a, rest) => {
      requireOpen();
      const parts = splitArgs(rest);
      if (parts.length < 5) throw new Error('usage: wifistatic <ssid> <password> <ip> <mask> <gateway> [keyType] — quote values with spaces');
      const ip4 = (s) => { const o = String(s).split('.').map((x) => int(x, 'octet')); if (o.length !== 4) throw new Error(`bad IPv4 "${s}"`); return o; };
      const keyType = parts[5] !== undefined ? int(parts[5], 'keyType') : constants.WIFI_WPA_WPA2_MIXED;
      printer.setWifiStatic(parts[0], parts[1], keyType, ip4(parts[2]), ip4(parts[3]), ip4(parts[4]));
      out('  WiFi+static-IP sent — power-cycle the printer to apply.');
    } },

  // --- raw escape hatch ---
  hex: { usage: 'hex <bytes>', help: 'Send raw bytes, e.g. `hex 1B 40` or `hex 1f1b1f67`.',
    run: (a, rest) => { requireOpen(); const buf = parseHex(rest); printer.writeData(buf); out(`  sent ${buf.length} byte(s)`); } },
  raw: { usage: 'raw <text>', help: 'Send literal text with \\n \\r \\t \\xHH escapes.',
    run: (a, rest) => { requireOpen(); printer.writeData(Buffer.from(unescape(dequote(rest)), 'binary')); } },

  // --- meta ---
  help: { usage: 'help [command]', help: 'List commands, or show help for one.',
    complete: (args) => (args.length <= 1 ? Object.keys(commands) : []),
    run: (a) => printHelp(a[0]) },
  exit: { usage: 'exit', help: 'Quit (also: quit, Ctrl-D).', run: () => quit() },
  quit: { usage: 'quit', help: 'Quit.', run: () => quit() },
};

function printHelp(name) {
  if (name && commands[name]) {
    const c = commands[name];
    out(`  ${c.usage}`);
    out(`    ${c.help}`);
    return;
  }
  out('Commands (type `help <cmd>` for details):');
  const names = Object.keys(commands).sort();
  const width = Math.max(...names.map((n) => n.length));
  for (const n of names) out(`  ${n.padEnd(width)}  ${commands[n].help}`);
  out('');
  out('Modes: high-level verbs (align/bold/print/cut/barcode/qr) OR raw bytes (hex/raw).');
  out('Quoting: wrap text in quotes to keep trailing spaces, e.g. print "hi  "');
}

// ---------------------------------------------------------------------------
// REPL
// ---------------------------------------------------------------------------

function execLine(line) {
  const trimmed = line.trim();
  if (!trimmed) return;
  if (trimmed.startsWith('#')) return; // comment
  const sp = trimmed.indexOf(' ');
  const verb = (sp === -1 ? trimmed : trimmed.slice(0, sp)).toLowerCase();
  const rest = sp === -1 ? '' : trimmed.slice(sp + 1).trim();
  const args = rest.length ? rest.split(/\s+/) : [];
  const cmd = commands[verb];
  if (!cmd) { err(`unknown command "${verb}" — type \`help\``); return; }
  try {
    cmd.run(args, rest);
  } catch (e) {
    err(e.message || String(e));
  }
}

function completer(line) {
  const sp = line.indexOf(' ');
  if (sp === -1) {
    // completing the verb
    const hits = Object.keys(commands).filter((c) => c.startsWith(line));
    return [hits.length ? hits : Object.keys(commands), line];
  }
  // completing an argument
  const verb = line.slice(0, sp).toLowerCase();
  const cmd = commands[verb];
  const rest = line.slice(sp + 1);
  const args = rest.length ? rest.split(/\s+/) : [];
  const partial = rest.endsWith(' ') ? '' : args[args.length - 1] || '';
  if (cmd && cmd.complete) {
    const candidates = cmd.complete(args).filter((c) => c.startsWith(partial));
    if (candidates.length) {
      const prefix = line.slice(0, line.length - partial.length);
      return [candidates.map((c) => prefix + c), line];
    }
  }
  return [[], line];
}

function prompt() {
  rl.setPrompt(`munbyn${connection ? '(' + connection + ')' : ''}> `);
  rl.prompt();
}

function quit() {
  try { if (printer.isOpen) printer.close(); } catch (_) {}
  out('bye');
  process.exit(0);
}

const rl = readline.createInterface({ input: process.stdin, output: process.stdout, completer });

// Optional auto-connect from argv: [transport] [address] [port]
(function autoConnect() {
  const a = process.argv.slice(2);
  if (a.length === 0) return;
  try {
    if (['usb', 'serial', 'network'].includes(a[0])) commands.connect.run(a);
    else commands.connect.run(['network', a[0], a[1]]); // bare IP → network
  } catch (e) {
    err(`auto-connect failed: ${e.message}`);
  }
})();

out('MUNBYN ITPP047 terminal. Type `help` for commands, `exit` to quit.');
if (!printer.isOpen) out('Not connected — `connect network <ip>` or `connect usb <path>`.');
prompt();

rl.on('line', (line) => { execLine(line); prompt(); });
rl.on('close', () => quit());
