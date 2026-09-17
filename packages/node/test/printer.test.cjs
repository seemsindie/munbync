const assert = require('node:assert/strict');
const test = require('node:test');
const fs = require('node:fs');
const os = require('node:os');
const path = require('node:path');
const { MunbynPrinter, constants } = require('../lib');

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
  const { data } = capture(p => p.printPdf417('x'.repeat(65532)));
  assert.equal(data.readUInt16LE(20), 65535);
  assert.equal(data.length, 65532 + 33);
  const rejected = capture(p => assert.throws(() => p.printPdf417('x'.repeat(65533))));
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
