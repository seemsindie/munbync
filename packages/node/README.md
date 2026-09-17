# munbync Node.js bindings

Synchronous native bindings for the MUNBYN ITPP047 C library. Requires Node.js 18+,
a C/C++ compiler, and Python for node-gyp. Linux native transports are validated;
Windows transport I/O is not implemented.

```js
const { MunbynPrinter } = require('munbync');
const printer = new MunbynPrinter();
try {
  printer.openNetwork('192.0.2.10', 9100, 2000);
  console.log(printer.getStatus());
  printer.initialize().print('Hello!\n').feedLines(7).cutPaper();
} finally {
  printer.close();
}
```

Replace the address with your printer. The `munbyn-term` executable exposes the
interactive terminal. `node tools/check-printer.js <ip>` only queries status;
add `--print` to print a verification receipt.

From the repository, `npm ci` builds the addon and `npm test` runs regressions
without physical hardware. `npm run rebuild` refreshes the bundled C source and
rebuilds. `npm pack` includes the C source, terminal, types, and license so a
standalone installation does not depend on a sibling checkout.

`getStatus()` requires all four real-time status replies and throws on missing or
malformed data. USB devices can support printing without supporting reads. Keep
ASB disabled during synchronous status queries; reconnect after a timeout.

QR/PDF417, GS1, self-test, and Wi-Fi commands depend on printer firmware and are
not documented in the bundled ITPP047 manual. `executeTestPrint()` is a retained
API name for the manual's **hex-dump mode** command (`n=0`, `m=1`); it is not the
proprietary `selfTest()`. `realtimeDrawerPulse()` defaults to one 100 ms unit.
Avoid configuring Wi-Fi through the connection being reconfigured.

Native PDF417 did not render a barcode on the tested ITPP047 firmware; it printed
command text. The native PDF417 API is experimental and should only be used with
firmware known to support it. For that printer, encode PDF417 externally and send
it using the raster-image API. The live receipt tool skips native PDF417 unless
`--native-pdf417` is explicitly requested.
