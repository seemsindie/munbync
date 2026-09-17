const assert = require('node:assert/strict');
const test = require('node:test');
const fs = require('node:fs');
const os = require('node:os');
const path = require('node:path');
const { spawnSync } = require('node:child_process');
const { MunbynPrinter, constants, codepageProfiles, encodePdf417 } = require('../lib');
const zxing = require('@zxing/library');

test('terminal rejects fractional and wrapping selectors without changing encoding', () => {
  const result = spawnSync(process.execPath, [path.join(__dirname, '../tools/printer-term.js')], {
    input: 'encoding cp852 4294967296\nencoding cp852 18.5\nencoding cp852 18\nexit\n',
    encoding: 'utf8', timeout: 5000,
  });
  assert.ifError(result.error);
  assert.equal(result.status, 0, result.stderr);
  assert.match(result.stdout, /Codepage must be 0\.\.255/);
  assert.match(result.stdout, /codepage must be an integer/);
  assert.match(result.stdout, /Text encoding cp852, printer selector 18/);
  assert.equal((result.stdout.match(/Text encoding/g) || []).length, 1);
});

function capture(action, initial = Buffer.alloc(0)) {
  const dir = fs.mkdtempSync(path.join(os.tmpdir(), 'munbyn-node-'));
  const output = path.join(dir, 'capture');
  fs.writeFileSync(output, initial);
  const printer = new MunbynPrinter().openUsb(output);
  try {
    const result = action(printer);
    return { result, data: fs.readFileSync(output) };
  } finally {
    printer.close();
    fs.rmSync(dir, { recursive: true });
  }
}

test('native addon exposes a closed printer and documented command defaults', () => {
  const printer = new MunbynPrinter();
  assert.equal(printer.isOpen, false);
  printer.close();
  const { data } = capture(p => p.realtimeDrawerPulse().executeTestPrint());
  assert.equal(data.toString('hex'), '10140100011d284102000001');
});

test('short raster buffers and narrowing overflow are rejected before writing', () => {
  for (const action of [
    p => p.printRasterImage(0, Buffer.alloc(1), 16, 16),
    p => p.printRasterImage(0, Buffer.alloc(1), 65537, 1),
    p => p.printBitImage(0, 65537, Buffer.alloc(1)),
    p => p.printQr('hello', 262),
    p => p.printPdf417('hello', 256),
    p => p.printPdf417('hello', 0, 1.5),
    p => p.realtimeDrawerPulse(256, 1),
    p => p.buzzerAlarm(1, 21, 3),
    p => p.readData(2 ** 32 + 1),
    p => p.setWifiStatic('test', 'pass', 8, [256, 1, 1, 1], [0, 0, 0, 0], [0, 0, 0, 0]),
    p => p.setWifi('test\0hidden', 'pass'),
  ]) {
    const { data } = capture(p => assert.throws(() => action(p)));
    assert.equal(data.length, 0);
  }
});

test('PDF417 boundary passes through the native binding without length overflow', () => {
  const { data } = capture(p => p.setProfile('generic').printPdf417Native('x'.repeat(65532)));
  assert.equal(data.readUInt16LE(20), 65535);
  assert.equal(data.length, 65532 + 33);
  const rejected = capture(p => assert.throws(() => p.setProfile('generic').printPdf417Native('x'.repeat(65533))));
  assert.equal(rejected.data.length, 0);
});

test('status uses all four groups and rejects partial replies', () => {
  const initial = Buffer.from('00000016000000120000001200000012', 'hex');
  const { result, data } = capture(p => p.getStatus(), initial);
  assert.equal(result.online, true);
  assert.equal(result.paperPresent, true);
  assert.equal(result.coverClosed, true);
  assert.equal(result.errorOccurred, false);
  assert.equal(data.toString('hex'), '10040116100402121004031210040412');
  capture(p => assert.throws(() => p.getStatus(), /Timeout/), Buffer.from('00000012', 'hex'));
});

test('GS1 extension and bitmap wrappers emit complete frames', () => {
  const { data } = capture(p => p.printBarcode(constants.BARCODE_GS1_128, '1234'));
  assert.equal(data.toString('hex'), '1d6b4a0431323334');
  const image = capture(p => p.printRasterImage(0, Buffer.from([128, 64]), 8, 2));
  assert.equal(image.data.toString('hex'), '1d763000010002008040');
});

test('barcode wire syntax and binary payloads follow the shared manual cases', () => {
  const cases = require('../../../tests/barcode_cases.json');
  for (const item of cases) {
    const payload = item.hex ? Buffer.from(item.hex, 'hex') : item.text;
    const output = capture(p => {
      if (item.valid) p.printBarcode(item.type, payload);
      else assert.throws(() => p.printBarcode(item.type, payload), undefined, JSON.stringify(item));
    });
    if (!item.valid) assert.equal(output.data.length, 0);
  }
  assert.equal(capture(p => p.printBarcode(73, Buffer.from('7b410041', 'hex'))).data.toString('hex'), '1d6b49047b410041');
});

test('numeric coercion cannot wrap or truncate before native validation', () => {
  for (const action of [p => p.feedLines(256), p => p.feedLines(1.5), p => p.feedLines(NaN),
    p => p.feedLines('3'), p => p.setLeftMargin(65536), p => p.setCodepage(2 ** 32),
    p => p.setHorizontalTabPositions([8, 8]), p => p.setHorizontalTabPositions([8.5]),
    p => p.setRelativeHorizontalPosition(-32769), p => p.printAndCut('must not print', 99)]) {
    assert.equal(capture(p => assert.throws(() => action(p))).data.length, 0);
  }
  assert.equal(capture(p => p.setHorizontalTabPositions([])).data.toString('hex'), '1b4400');
});

test('explicit text encoding emits complete single-byte configuration and rejects unsupported text', () => {
  assert.equal(codepageProfiles.manual.cp852, 18);
  assert.equal(codepageProfiles.legacy.cp852, 13);
  assert.equal(capture(p => p.printEncoded('Čćšžđ', 'cp852', 18)).data.toString('hex'), '1c2e1b52001b7412ac86e7a7d0');
  assert.equal(capture(p => p.printEncoded('ЉЊЋЂ Ј', 'windows1251', 51)).data.toString('hex'), '1c2e1b52001b74338a8c8e8020a3');
  for (const text of ['😀', 'a\0b', '\x1b@', '\ud800']) {
    assert.equal(capture(p => assert.throws(() => p.printEncoded(text, 'cp852', 18))).data.length, 0);
  }
  assert.equal(capture(p => p.setCodepage(69)).data.toString('hex'), '1b7445');
});

test('Kanji glyphs are exact length and native PDF417 stays blocked after initialization', () => {
  const glyph = Buffer.alloc(72, 128);
  assert.deepEqual(capture(p => p.defineKanjiChar(0xfe, 0xa1, glyph)).data, Buffer.concat([Buffer.from('1c32fea1', 'hex'), glyph]));
  assert.equal(capture(p => assert.throws(() => p.defineKanjiChar(0xfe, 0xa1, glyph.subarray(1)))).data.length, 0);
  const result = capture(p => {
    assert.equal(p.capabilities.nativePdf417, false);
    p.initialize();
    assert.throws(() => p.printPdf417Native('MUNBYNC-OK'), /unsupported/);
  });
  assert.equal(result.data.toString('hex'), '1b40');
});

test('raster PDF417 independently decodes ASCII, UTF-8 and binary data', () => {
  for (const [input, expected] of [['MUNBYNC-OK', 'MUNBYNC-OK'], ['Čćšžđ / ЉЊЋЂ / €', 'Čćšžđ / ЉЊЋЂ / €'], [Buffer.from([65, 0, 66]), 'A\0B']]) {
    const { bitmap, width, height } = encodePdf417(input);
    assert.ok(width <= 512);
    const stride = Math.ceil(width / 8);
    assert.ok(bitmap.subarray(0, stride * 4).every(b => b === 0), 'top quiet zone');
    assert.ok(bitmap.subarray(-stride * 4).every(b => b === 0), 'bottom quiet zone');
    const gray = new Uint8ClampedArray(width * height);
    for (let y = 0; y < height; y++) for (let x = 0; x < width; x++) {
      gray[y * width + x] = bitmap[y * stride + (x >> 3)] & (128 >> (x & 7)) ? 0 : 255;
    }
    const binary = new zxing.BinaryBitmap(new zxing.HybridBinarizer(new zxing.RGBLuminanceSource(gray, width, height)));
    assert.equal(new zxing.PDF417Reader().decode(binary).getText(), expected);
  }
  assert.throws(() => encodePdf417('test', { maxWidth: 80 }), /width/);
  assert.throws(() => encodePdf417('test', { columns: 30 }), /width/);
  assert.throws(() => encodePdf417('x'.repeat(2711)), /input/);
  const frame = capture(p => p.printPdf417('MUNBYNC-OK')).data;
  assert.equal(frame.subarray(0, 4).toString('hex'), '1d763000');
  assert.equal(frame.length, 8 + frame.readUInt16LE(4) * frame.readUInt16LE(6));
});
