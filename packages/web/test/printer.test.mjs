import assert from 'node:assert/strict';
import test from 'node:test';
import fs from 'node:fs';
import { createRequire } from 'node:module';
import { encodePdf417 } from '../dist/pdf417.js';
import { codepageProfiles } from '../dist/encoding.js';
const zxing = createRequire(import.meta.url)('@zxing/library');
import * as cmd from '../dist/commands.js';
import { MunbynPrinter } from '../dist/printer.js';
import { WebSerialTransport } from '../dist/transports/webserial.js';
import { WebUSBTransport } from '../dist/transports/webusb.js';

const hex = (s) => Uint8Array.from(Buffer.from(s.replaceAll(' ', ''), 'hex'));

test('barcode syntax and binary code sets match the manual fixtures', () => {
  const cases = JSON.parse(fs.readFileSync(new URL('../../../tests/barcode_cases.json', import.meta.url), 'utf8'));
  for (const item of cases) {
    const payload = item.hex ? hex(item.hex) : item.text;
    if (item.valid) assert.doesNotThrow(() => cmd.printBarcode(item.type, payload), JSON.stringify(item));
    else assert.throws(() => cmd.printBarcode(item.type, payload), RangeError, JSON.stringify(item));
  }
  assert.deepEqual(cmd.printBarcode(73, hex('7b410041')), hex('1d6b49047b410041'));
});

test('old setters reject invalid numbers before writing, including multi-command receipts', async () => {
  for (const action of [p => p.feedLines(256), p => p.feedLines(1.5), p => p.feedLines(NaN),
    p => p.feedLines('3'), p => p.setLeftMargin(65536), p => p.setCodepage(2 ** 32),
    p => p.setHorizontalTabPositions([8, 8]), p => p.setHorizontalTabPositions([8.5]),
    p => p.setHorizontalTabPositions(new Array(2)), p => p.setHorizontalTabPositions([, 8]),
    p => p.defineKanjiChar(0xfe, 0xa1, Array(72).fill(256)),
    p => p.setRelativeHorizontalPosition(-32769), p => p.printAndCut('must not print', 99)]) {
    const transport = statusTransport([]);
    await assert.rejects(action(new MunbynPrinter(transport)));
    assert.deepEqual(transport.writes, []);
  }
  assert.deepEqual(cmd.setHorizontalTabPositions([]), hex('1b4400'));
});

test('encoded text uses explicit code pages and rejects unrepresentable characters before output', async () => {
  assert.equal(codepageProfiles.manual.cp852, 18);
  assert.equal(codepageProfiles.legacy.cp852, 13);
  assert.deepEqual(cmd.printEncoded('Čćšžđ', 'cp852', 18), hex('1c2e1b52001b7412ac86e7a7d0'));
  assert.deepEqual(cmd.printEncoded('ЉЊЋЂ Ј', 'windows1251', 51), hex('1c2e1b52001b74338a8c8e8020a3'));
  for (const text of ['😀', 'a\0b', '\x1b@', '\ud800']) {
    const transport = statusTransport([]);
    await assert.rejects(new MunbynPrinter(transport).printEncoded(text, 'cp852', 18));
    assert.deepEqual(transport.writes, []);
  }
});

test('public Kanji APIs and font B glyph limits work through the printer class', async () => {
  const transport = statusTransport([]), printer = new MunbynPrinter(transport);
  await printer.setUnderlineKanji(2);
  await printer.defineKanjiChar(0xfe, 0xa1, new Uint8Array(72));
  assert.deepEqual(transport.writes[0], [0x1c, 0x2d, 2]);
  assert.equal(transport.writes[1].length, 76);
  await assert.rejects(printer.defineKanjiChar(0xfe, 0xa1, new Uint8Array(71)));
  await printer.setFont(1);
  await assert.rejects(printer.defineUserDefinedChars(3, 65, 65, new Uint8Array([10, ...Array(30).fill(0)])));
  await printer.cancelAllFormatting();
  await printer.defineUserDefinedChars(3, 65, 65, new Uint8Array([10, ...Array(30).fill(0)]));
});

test('PDF417 raster decodes independently; default printer never sends native PDF417', async () => {
  for (const [input, expected] of [['MUNBYNC-OK', 'MUNBYNC-OK'], ['Čćšžđ / ЉЊЋЂ / €', 'Čćšžđ / ЉЊЋЂ / €'], [new Uint8Array([65, 0, 66]), 'A\0B']]) {
    const { bitmap, width, height } = encodePdf417(input);
    const stride = Math.ceil(width / 8), gray = new Uint8ClampedArray(width * height);
    assert.ok(width <= 512);
    assert.ok(bitmap.subarray(0, stride * 4).every(b => b === 0));
    for (let y = 0; y < height; y++) for (let x = 0; x < width; x++)
      gray[y * width + x] = bitmap[y * stride + (x >> 3)] & (128 >> (x & 7)) ? 0 : 255;
    const binary = new zxing.BinaryBitmap(new zxing.HybridBinarizer(new zxing.RGBLuminanceSource(gray, width, height)));
    assert.equal(new zxing.PDF417Reader().decode(binary).getText(), expected);
  }
  const transport = statusTransport([]), printer = new MunbynPrinter(transport);
  await assert.rejects(printer.printPdf417Native('test'), /unsupported/);
  assert.deepEqual(transport.writes, []);
  await printer.printPdf417('MUNBYNC-OK');
  assert.deepEqual(transport.writes[0].slice(0, 4), [0x1d, 0x76, 0x30, 0]);
});

test('documented command bytes and limits', () => {
  const cases = [
    [cmd.realtimeDrawerPulse(1, 8), '10 14 01 01 08'],
    [cmd.executeTestPrint(0, 1), '1d 28 41 02 00 00 01'],
    [cmd.buzzerAlarm(20, 20, 3), '1b 43 14 14 03'],
    [cmd.printBarcode(73, '{B123'), '1d 6b 49 05 7b 42 31 32 33'],
    [cmd.printBitImage(33, 1, hex('80 40 20')), '1b 2a 21 01 00 80 40 20'],
    [cmd.setRelativeVerticalPosition(-2), '1d 5c fe ff'],
    [cmd.setPageArea(1, 256, 512, 1024), '1b 57 01 00 00 01 00 02 00 04'],
  ];
  for (const [actual, expected] of cases) assert.deepEqual(actual, hex(expected));
  for (const invalid of [
    () => cmd.realtimeDrawerPulse(0, 9), () => cmd.buzzerAlarm(21, 1, 3),
    () => cmd.executeTestPrint(0, 2), () => cmd.printBarcode(73, 'x'.repeat(256)),
    () => cmd.printBarcode(5, '123'), () => cmd.printBarcode(4, 'x\0y'),
    () => cmd.printBitImage(0, 1024, new Uint8Array(1024)),
    () => cmd.defineDownloadedBitImage(20, 48, new Uint8Array(7680)),
    () => cmd.printRasterImage(0, new Uint8Array(1), 16, 16),
    () => cmd.defineUserDefinedChars(2, 65, 65, new Uint8Array([1, 0, 0])),
    () => cmd.setWifi('x'.repeat(33), 'password'),
    () => cmd.setWifiStatic('test', 'password', 8, [256, 1, 1, 1], [0, 0, 0, 0], [0, 0, 0, 0]),
  ]) assert.throws(invalid, RangeError);
});

test('2D data length checks use UTF-8 byte counts and reject length wrap', () => {
  const maximum = cmd.printPdf417('x'.repeat(65532));
  assert.deepEqual(maximum.slice(20, 22), hex('ff ff'));
  assert.equal(maximum.length, 65532 + 33);
  assert.throws(() => cmd.printPdf417('x'.repeat(65533)), RangeError);
  assert.throws(() => cmd.printPdf417('é'.repeat(32767)), RangeError);
  assert.throws(() => cmd.printQr(''), RangeError);
  assert.throws(() => cmd.printQr('x'.repeat(7090)), RangeError);
  assert.throws(() => cmd.printQr('x', 0), RangeError);
  assert.deepEqual(cmd.printPdf417('abc'), hex(
    '1d 28 6b 03 00 30 41 00 1d 28 6b 04 00 30 45 30 31 ' +
    '1d 28 6b 06 00 30 50 30 61 62 63 1d 28 6b 03 00 30 51 30'));
});

test('NV images require complete headers and exact data lengths', () => {
  const image = new Uint8Array([1, 0, 1, 0, ...Array(8).fill(128)]);
  assert.deepEqual(cmd.defineNvBitImage(1, image), new Uint8Array([0x1c, 0x71, 1, ...image]));
  assert.throws(() => cmd.defineNvBitImage(2, image), RangeError);
  assert.throws(() => cmd.defineNvBitImage(1, image.slice(0, -1)), RangeError);
  assert.throws(() => cmd.defineNvBitImage(1, new Uint8Array([...image, 0])), RangeError);
  assert.throws(() => cmd.defineNvBitImage(1, new Uint8Array(65537)), RangeError);
});

function statusTransport(replies) {
  const writes = [];
  return {
    connected: true, writes,
    async connect() {}, async disconnect() { this.connected = false; },
    async write(data) { writes.push([...data]); },
    async read(length) {
      assert.equal(length, 1, 'each status query returns one byte');
      const reply = replies.shift();
      if (reply instanceof Error) throw reply;
      return reply === undefined ? new Uint8Array() : new Uint8Array([reply]);
    },
  };
}

test('all four status groups are required and mapped according to pp. 8-9', async () => {
  const transport = statusTransport([0x16, 0x12, 0x12, 0x12]);
  const status = await new MunbynPrinter(transport).getStatus();
  assert.deepEqual(status, {
    online: true, paperPresent: true, coverClosed: true, errorOccurred: false,
    cutError: false, recoverableError: false, unrecoverableError: false,
  });
  assert.deepEqual(transport.writes, [1, 2, 3, 4].map(n => [0x10, 4, n]));
  const errors = await new MunbynPrinter(statusTransport([0x1a, 0x56, 0x7a, 0x72])).getStatus();
  assert.equal(errors.online, false);
  assert.equal(errors.paperPresent, false);
  assert.equal(errors.coverClosed, false);
  assert.equal(errors.cutError, true);
  assert.equal(errors.recoverableError, true);
  assert.equal(errors.unrecoverableError, true);
  const general = await new MunbynPrinter(statusTransport([0x12, 0x52, 0x12, 0x12])).getStatus();
  assert.equal(general.errorOccurred, true);
});

test('partial, silent, invalid and failed status replies never return healthy defaults', async () => {
  for (const replies of [[], [0x12], [0x12, 0x12], [0x12, 0x12, 0x12], [0], [new Error('read failed')]]) {
    await assert.rejects(new MunbynPrinter(statusTransport(replies)).getStatus());
  }
});

function serialPort(chunk) {
  let closed = false;
  return {
    get closed() { return closed; },
    readable: new ReadableStream({ start(controller) { if (chunk) controller.enqueue(chunk); } }),
    writable: new WritableStream(),
    async open() {}, async close() { closed = true; },
  };
}

test('serial reads return one status byte and preserve surplus bytes', async () => {
  const port = serialPort(new Uint8Array([0x12, 0x16]));
  const transport = new WebSerialTransport({ readTimeoutMs: 50 });
  await transport.connectToPort(port);
  assert.deepEqual(await transport.read(1), hex('12'));
  assert.deepEqual(await transport.read(1), hex('16'));
  await transport.disconnect();
  assert.equal(port.closed, true);
});

test('silent serial reads time out and close the port without orphaning readers', async () => {
  const port = serialPort();
  const transport = new WebSerialTransport({ readTimeoutMs: 20 });
  await transport.connectToPort(port);
  await assert.rejects(transport.read(1), /timed out/);
  assert.equal(transport.connected, false);
  assert.equal(port.closed, true);
  assert.equal(port.readable.locked, false);
});

function usbDevice() {
  return {
    opened: false,
    configuration: { interfaces: [{ interfaceNumber: 2, alternates: [
      { alternateSetting: 0, interfaceClass: 3, endpoints: [] },
      { alternateSetting: 1, interfaceClass: 7, endpoints: [
        { type: 'bulk', direction: 'out', endpointNumber: 3 },
        { type: 'bulk', direction: 'in', endpointNumber: 4 },
      ] },
    ] }] },
    async open() { this.opened = true; }, async close() { this.opened = false; },
    async claimInterface(n) { assert.equal(n, 2); },
    async selectAlternateInterface(n, alt) { assert.equal(n, 2); assert.equal(alt, 1); },
    async transferOut(endpoint, data) { assert.equal(endpoint, 3); return { status: 'ok', bytesWritten: data.length }; },
    async transferIn(endpoint, length) {
      assert.equal(endpoint, 4); assert.equal(length, 1);
      return { status: 'ok', data: new DataView(new Uint8Array([0xff, 0x12, 0xff]).buffer, 1, 1) };
    },
  };
}

test('USB selects the matching alternate and respects DataView offsets', async () => {
  const device = usbDevice();
  const transport = new WebUSBTransport();
  await transport.connectToDevice(device);
  await transport.write(hex('10 04 01'));
  assert.deepEqual(await transport.read(1), hex('12'));
  await transport.disconnect();
  assert.equal(transport.connected, false);
});

test('USB rejects incomplete writes and stalled reads', async () => {
  const device = usbDevice();
  device.transferOut = async () => ({ status: 'ok', bytesWritten: 0 });
  device.transferIn = async () => ({ status: 'stall' });
  const transport = new WebUSBTransport();
  await transport.connectToDevice(device);
  await assert.rejects(transport.write(hex('10 04 01')), /incomplete/);
  await assert.rejects(transport.read(1), /failed/);
  await transport.disconnect();
});

test('USB timeout closes the device before a subsequent request can consume stale data', async () => {
  const device = usbDevice();
  device.transferIn = () => new Promise(() => {});
  const transport = new WebUSBTransport({ readTimeoutMs: 20 });
  await transport.connectToDevice(device);
  await assert.rejects(transport.read(1), /timed out/);
  assert.equal(device.opened, false);
  assert.equal(transport.connected, false);
});
