# munbync-web

Browser ESC/POS command builders and WebSerial/WebUSB transports for MUNBYN
ITPP047 printers. Build with `npm ci && npm run build`; run hardware-free
regressions with `npm test`.

```js
import { MunbynPrinter, WebSerialTransport } from 'munbync-web';
const printer = new MunbynPrinter(new WebSerialTransport({
  baudRate: 9600,
  readTimeoutMs: 1000,
}));
await printer.connect(); // Call from a click handler.
try {
  await printer.initialize();
  await printer.print('Hello!\n');
} finally {
  await printer.disconnect();
}
```

Use HTTPS or localhost and a browser that provides the selected hardware API.
`WebUSBTransport` accepts `filters` and `readTimeoutMs` options. These transports
do not provide raw TCP printing; use the native Node package for port 9100.

Reads return up to the requested byte count and have a timeout. Timeouts close
the transport, cancelling outstanding reads before reconnecting. `getStatus()`
requires all four valid one-byte replies. A one-way printer interface can print
without returning status. Keep automatic status back (ASB) disabled when querying
status synchronously, and await operations sequentially.

Command builders validate numeric arguments, barcode content, and frame sizes
before writing. `print()` is UTF-8; selecting a legacy printer codepage does not
transcode JavaScript strings. Use `printEncoded(text, encoding, selector)` for
strict single-byte conversion with the table number from the printer's sheet.
QR/PDF417, GS1, Wi-Fi, and the proprietary self-test are firmware-dependent
extensions. `executeTestPrint()` enters hex-dump mode (`n=0`, `m=1`) according to
the ITPP047 manual; it is not a generic test pattern command.

`printPdf417()` now prints a locally encoded raster. `printPdf417Raster(data,
options)` and exported `encodePdf417` accept text or `Uint8Array`; options are
`columns` (0 = automatic), `ecLevel` (default 2), `moduleSize` (default 2), and
`maxWidth` (default 512 dots). Strings carry a UTF-8 ECI marker. Encoding uses
`bwip-js/browser` without canvas or a remote service. Bundle the dependency, or
use the import map in `examples/index.html` to serve the built modules directly.

The default `itpp047-tested` profile blocks native PDF417 because it printed
command text on the tested unit. Other firmware can opt into
`new MunbynPrinter(transport, { profile: 'generic' })` and
`printPdf417Native()` after independent verification. `profile`/`capabilities`
describe local policy; no firmware detection occurs.

`printBarcode()` accepts binary `Uint8Array`, including CODE128 set A/C and
CODE93 NUL. `defineKanjiChar(0xFE, c2, bitmap)` accepts a 72-byte glyph for
`c2=0xA1..0xFE`; `setUnderlineKanji()` is exposed publicly. ASCII custom glyphs
respect the font selected through printer methods (A width ≤12, B width ≤9).
