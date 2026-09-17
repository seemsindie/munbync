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

`printPdf417(data, columns = 0, ecLevel = 1)` now prints a locally encoded raster
using `bwip-js`. `printPdf417Raster(data, options)` and exported `encodePdf417`
also accept binary `Uint8Array`/`Buffer`; options include `columns`, `ecLevel`
(default 2), `moduleSize` (default 2), and `maxWidth` (default 512 dots).
Strings use UTF-8 with an explicit ECI marker. No remote encoder is used.

The default `itpp047-tested` profile blocks native PDF417 because the tested
printer printed command text. Native output requires an explicit
`new MunbynPrinter({ profile: 'generic' })` and `printPdf417Native()`; generic
means unverified, not guaranteed support. `profile` and `capabilities` describe
the selected local policy, not detected firmware. The live receipt includes
raster PDF417; `--native-pdf417` adds the native diagnostic.

For non-ASCII text, use `printEncoded('Čćšžđ\n', 'cp852', 18)` **only if the
printer's code-page sheet identifies selector 18 as CP852**. `print()` retains
raw UTF-8 behavior. `printEncoded()` rejects invalid/unrepresentable text, exits
Kanji mode, selects the USA international set, and selects the explicit table
before sending encoded bytes. Exported `codepageProfiles` distinguishes manual
selectors from retained legacy numbers; neither detects your firmware.

`printBarcode(type, Buffer)` supports binary payloads, including CODE128 set A/C
and CODE93 NUL. String CODE128 needs its code-set prefix, e.g. `{BTEST-1234`.
Numeric arguments reject fractions and overflow. `defineKanjiChar(c1, c2, data)`
defines a 72-byte glyph (`c1=0xFE`, `c2=0xA1..0xFE`).

The terminal uses ASCII initially. `encoding cp852 18` selects an explicit
encoding/table for `print` and `println`; choose the selector from the unit's
sheet. `cut partial` and `cut one-point` describe the manual's cutter behavior;
`feedcut` distances are motion units, not lines.
