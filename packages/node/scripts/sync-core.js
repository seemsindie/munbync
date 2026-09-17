// Keep the native package buildable both in the repo and from an npm tarball.
const fs = require('node:fs');
const path = require('node:path');
const source = path.resolve(__dirname, '../../../src');
const destination = path.resolve(__dirname, '../core');
fs.mkdirSync(destination, { recursive: true });
for (const file of ['munbyn_printer.c', 'munbyn_printer.h', 'munbyn_barcode.inc', 'munbyn_text.inc', 'munbyn_encoding_tables.inc']) {
  if (fs.existsSync(path.join(source, file))) {
    fs.copyFileSync(path.join(source, file), path.join(destination, file));
  } else if (!fs.existsSync(path.join(destination, file))) {
    throw new Error(`Missing bundled core source: ${file}`);
  }
}
