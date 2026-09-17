#!/usr/bin/env node
// Run explicitly against a printer; never used by automated tests.
const { MunbynPrinter, constants: C } = require('../lib');
const [host, ...options] = process.argv.slice(2);
if (!host || options.some(option => !['--print', '--native-pdf417'].includes(option))) {
  console.error('Usage: node tools/check-printer.js <IPv4-address> [--print] [--native-pdf417]');
  process.exit(2);
}
const printer = new MunbynPrinter();
try {
  printer.openNetwork(host, 9100, 2000);
  const before = printer.getStatus();
  console.log('Before:', JSON.stringify(before));
  if (options.includes('--print')) {
    if (!before.online || !before.paperPresent || !before.coverClosed || before.errorOccurred) {
      throw new Error('Printer is not ready to print');
    }
    const bitmap = Buffer.from(Array.from({ length: 16 * 64 }, (_, i) =>
      (Math.floor(i / 128) + i % 16) % 2 === 0 ? 0xff : 0x00));
    printer.initialize()
      .align('center').bold().print('MUNBYNC VERIFICATION\n').bold(false)
      .print('Text / CODE128 / QR / Raster\n')
      .print(new Date().toISOString() + '\n')
      .setBarcodeHeight(48).setBarcodeWidth(2).setHriPosition(C.HRI_BELOW)
      .printBarcode(C.BARCODE_CODE128, '{BTEST-1234').lineFeed()
      .align('center').print('QR: MUNBYNC-OK\n').printQr('MUNBYNC-OK', 4).lineFeed();
    if (options.includes('--native-pdf417')) {
      printer.print('EXPERIMENTAL native PDF417\n').printPdf417('MUNBYNC-OK').lineFeed();
    }
    printer.align('center').print('Raster: 128 x 64 checkerboard\n')
      .printRasterImage(0, bitmap, 128, 64).lineFeed()
      .printAndCut('END OF TEST\n');
    console.log('Test receipt sent. Check the complete footer, visible checkerboard, and scan the QR/CODE128 symbols.');
  }
  console.log('After:', JSON.stringify(printer.getStatus()));
} catch (error) {
  console.error(error.message);
  process.exitCode = 1;
} finally {
  printer.close();
}
