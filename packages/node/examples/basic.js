const { MunbynPrinter, constants } = require('../lib');

// Connect via USB
const printer = new MunbynPrinter();
printer.openUsb('/dev/usb/lp0');
printer.initialize();

// Print a receipt header
printer
  .align('center')
  .bold()
  .setTextScale(2, 2)
  .print('MY STORE\n')
  .setTextScale(1, 1)
  .bold(false)
  .print('123 Main Street\n')
  .print('Tel: 555-0123\n')
  .print('================================\n')
  .align('left');

// Print items
printer
  .print('Coffee              $3.50\n')
  .print('Muffin              $2.75\n')
  .print('================================\n')
  .bold()
  .print('TOTAL               $6.25\n')
  .bold(false);

// Print barcode
printer
  .lineFeed()
  .align('center')
  .setBarcodeHeight(80)
  .setBarcodeWidth(3)
  .setHriPosition(constants.HRI_BELOW)
  .printBarcode(constants.BARCODE_CODE128, '{A12345678')
  .align('left');

// Feed and cut
printer
  .feedLines(4)
  .cutPaper();

// Check status
const status = printer.getStatus();
console.log('Printer status:', status);

printer.close();
