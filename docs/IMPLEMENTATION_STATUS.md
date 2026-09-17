# ITPP047 implementation status

Updated 2026-09-18. This records fixes following the
[baseline survey](ITPP047_CAPABILITY_REPORT.md) of commit `9156e6d`.

## Completed software work

| Task | Result |
| --- | --- |
| Manual API coverage | Dedicated C, Node, and browser access to all 73 numbered families; added `FS 2` custom Kanji glyph definition and public browser Kanji underline. See the [command matrix](ITPP047_COMMAND_MATRIX.csv). |
| Text encoding | Strict UTF-8 to ASCII or eight single-byte encodings, explicit table selector, validation before sending, separate manual/legacy mappings. Supports Serbian Latin/Cyrillic host conversion. |
| Barcode validation | Shared valid/invalid fixtures across C/Node/browser; all nine manual symbologies, selectors 65–71, alphabet and length constraints, CODE128 code sets/escapes, GS1 wire constraints. |
| Binary barcode data | C length-based API, Node Buffer, browser Uint8Array; embedded NUL supported where the selected symbology permits it. |
| Numeric validation | Bindings and terminal reject fractional values and overflow before narrowing. Unsupported native serial baud rates return an error. Empty tab lists clear stops; nonempty stops must ascend. |
| Terminal behavior | Explicit text encoding selection; accurate status and barcode-width help; partial/one-point cutter names; feed-and-cut distances labeled as motion units. Existing seven-line receipt footer retained. |
| Device policy | Node/browser default to `itpp047-tested`, blocking native PDF417. Explicit generic mode retains native access for independently verified firmware. C provides an opt-in equivalent policy. |
| PDF417 fallback | Node/browser encode locally with pinned `bwip-js` 4.11.4 and print raster; configurable columns, error correction, module size, and width; quiet zones and packed width validated. |
| Examples and docs | Correct CODE128 prefixes, explicit encoded-text example, table-selector diagnostics, API migration notes, current matrix, and physical verification record. |

All 73 counts describe API availability, not proof that every parameter or
command works on this firmware. GS1 checks cover wire syntax and basic ranges;
applications still need correct AI values, lengths, dates, and check digits.

## API usage and migration

```js
const { MunbynPrinter, encodePdf417, codepageProfiles } = require('munbync');
const printer = new MunbynPrinter(); // itpp047-tested policy
// Once connected:
printer.printPdf417('MUNBYNC-OK'); // now raster, not native GS ( k
printer.printPdf417Raster('Čćšžđ / ЉЊЋЂ', {
  moduleSize: 3, ecLevel: 3, maxWidth: 512,
});
// Only if the unit's sheet confirms this selector:
printer.printEncoded('Čćšžđ\n', 'cp852', codepageProfiles.manual.cp852);
const image = encodePdf417(Buffer.from([65, 0, 66]));
// image = { bitmap: Uint8Array, width, height }
```

Browser methods use the same names and require `await`. Node/browser
`printPdf417(data, columns = 0, ecLevel = 1)` preserves the positional arguments
but changes output to raster. `printPdf417Raster(data, options)` and
`encodePdf417(data, options)` default to error correction 2. Both default to
two-dot modules and a maximum raster width of 512 dots, including row padding.
Specify the actual active print width if you change margins or print-area width.
Wider modules/error correction may require more paper or reject larger payloads.

Text PDF417 data uses UTF-8 with an ECI marker; byte arrays preserve binary data.
The 2710-byte input ceiling is a preliminary guard; actual capacity depends on
content, columns, width, and error correction. Encoding fails before writing if
the symbol cannot fit. `bwip-js` is a local MIT-licensed dependency; browser
users need a bundler or the example import map. ZXing is a development-only
independent test decoder.

Native PDF417 remains `printPdf417Native()` in Node/browser, gated by an explicit
`generic` profile. Generic means unknown, not supported. Profiles are local
policy and persist across initialization; they do not query firmware. Native
QR remains available and has been verified on the test unit.

C retains `munbyn_print_pdf417()` as a native command for compatibility. Call
`munbyn_set_profile(printer, MUNBYN_PROFILE_ITPP047_TESTED)` to block it. C does
not embed a PDF417 encoder; pass an externally encoded bitmap to
`munbyn_print_raster_image()` with sufficient storage for
`((width + 7) / 8) * height` bytes. The JavaScript encoder can produce that bitmap.

Raw text printing does not transcode; see [text encoding](CHARSET_COMMANDS.md).
Raw writes also bypass local state tracking, including browser glyph font
tracking. Initialize and use the typed setters before defining glyphs.

## Verification

- C protocol tests cover exact framing, rejection without output, binary
  barcodes, UTF-8 validation/conversion, custom Kanji, and native PDF417 policy;
  shared/static builds and relocated CMake consumer checks pass.
- Node and browser tests cover native/builder boundaries, transport/status
  behavior, the shared barcode cases, text conversion, custom glyphs, profiles,
  and independent PDF417 decoding for ASCII, Serbian UTF-8, and binary NUL.
- Standalone package archives install outside the checkout; the native addon
  builds from bundled C sources, browser imports and encoder output agree, and
  strict TypeScript consumer checks pass. Compatible patches to five existing
  Node build dependencies leave npm audit with zero reported vulnerabilities.
- On the printer at `192.168.1.70:9100`, the new raster PDF417 decoded from the
  supplied receipt photo as `MUNBYNC-OK` (crop/deskew/enlargement required).
  CODE128 decoded as `TEST-1234`, QR as `MUNBYNC-OK`; checkerboard and footer
  remained clear. Live status before/after reported no errors. See the
  [hardware record](COMMAND_COVERAGE.md#raster-pdf417-verification-2026-09-18-local-time).

## Remaining hardware evidence and optional work

The unit's exact SKU, main firmware, Wi-Fi firmware, and installed character
tables still require its self-test/code-page sheets. No table selector is
claimed as verified for Serbian text on this unit. Once those are available,
record their mappings and print/inspect a short Latin and Cyrillic receipt.
This is the outstanding item from the hardware-identification plan; it does
not prevent using the tested ASCII/barcode/raster receipt path.

Custom Kanji, other barcode symbologies, page mode, NV images, cash drawer,
sound/light, and fault states have not been physically tested here. Broader ASB
stream decoding, vendor configuration commands, native Windows transport,
Bluetooth, and hardware coverage of those features remain optional follow-up
projects, outside this completion pass.
