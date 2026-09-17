# ITPP047 repository and printer capability survey

Survey date: 2026-09-17. Repository baseline: `9156e6dfd4a84d8a01c7f20cdf6c3d63dbf296a0` (`master`).

**Historical baseline survey.** The gaps below describe that commit. Subsequent
fixes are recorded in [implementation status](IMPLEMENTATION_STATUS.md): all 73
families now have APIs in all three packages, strict encoding and validation are
implemented, and raster PDF417 has printed and decoded successfully on this unit.
The accompanying [CSV matrix](ITPP047_COMMAND_MATRIX.csv) tracks the current
implementation; the inventory and findings in this report retain the baseline.

Reference device: the printer tested earlier in this session at `192.168.1.70:9100`. Its exact SKU, main firmware revision, Wi-Fi module revision, and installed character tables have not been recorded. Conclusions about physical behavior apply to that unit in its tested configuration.

## Assessment

The repository provides broad ESC/POS command coverage and a working network receipt path. It is usable for the receipt features already verified: ASCII text, CODE128, QR, raster images, feeding, and partial cutting. It is not yet a fully validated implementation of every printer feature.

| Comparison | Result |
| --- | --- |
| Bundled programming manual | 73 numbered command families, §2.1–§2.73 |
| C API | Dedicated access to 72/73 families |
| Node.js API | Dedicated access to the same 72/73 families |
| Browser public `MunbynPrinter` API | Dedicated access to 71/73 families |
| Missing everywhere | `FS 2`: custom Kanji glyph definition |
| Additional browser public API gap | `FS -`: Kanji underline; a builder exists internally |
| Most consequential compatibility issue found | Code-page names/numbers conflict with the PDF; JS text is not transcoded |
| Known unsuccessful physical feature | Native PDF417, which printed command fragments instead of a barcode |

These counts measure **API presence per manual section**, not full parameter coverage, test coverage, or successful physical execution. For example, `GS k` counts once despite supporting multiple barcode types; its alternate selectors are not all exposed. Code-page selection counts as present even though its mapping needs reconciliation. Raw byte access is available in every package but does not count as a dedicated command implementation.

The complete section-by-section inventory appears at the end. The accompanying [CSV matrix](ITPP047_COMMAND_MATRIX.csv) includes C symbols, Node/browser methods, wire prefixes, manual pages, implementation locations, and verification notes.

## What is verified on this printer

Evidence comes from the two earlier receipt photographs, user confirmation, barcode decoding, and the recorded live status check; this survey did not send another print job or change printer settings. See the existing [hardware verification record](COMMAND_COVERAGE.md#live-itpp047-check-2026-09-17).

| Feature | Evidence and practical conclusion |
| --- | --- |
| Network printing | The C core through the Node binding successfully sent both receipts over TCP port 9100. |
| Text and alignment | Headings, centered text, emphasis, labels, and the final footer rendered visibly. Other fonts, languages, and formatting combinations remain untested. |
| CODE128 | ZBar decoded `TEST-1234` from the corrected receipt photo. The API payload included the code-set prefix: `{BTEST-1234`. |
| Native QR | ZBar decoded `MUNBYNC-OK`; the receipt used QR module size 4. |
| Raster images | The corrected 128×64-dot checkerboard rendered clearly. This confirms the raster path, not every bitmap mode or maximum size. |
| Feed and cut | `printAndCut('END OF TEST\n')` with its seven-line feed produced a complete footer and margin. Three lines had clipped the earlier footer. |
| Real-time status | Recorded replies were `16 12 12 12` in hexadecimal: online, paper present, cover closed, no reported errors. Fault states have automated coverage, not a physical fault-injection check. |
| Native PDF417 | The first receipt printed fragments of command data. Treat `printPdf417()` as unsupported for this unit/configuration. Healthy status did not detect the rendering failure. |

The default verification receipt now skips native PDF417. A PDF417 raster fallback is feasible through the verified image path, but **no PDF417 encoder/fallback is implemented or physically verified in this repository**.

## Repository interfaces and tools

| Component | Actual scope |
| --- | --- |
| [C core](../src/munbyn_printer.h) | ESC/POS encoding, status, connection management, raw reads/writes, and convenience helpers. Native transport implementations use POSIX facilities. |
| [Node package](../packages/node/lib/index.js) | Synchronous native binding; USB device paths, serial ports, and IPv4 TCP. Includes TypeScript declarations and fluent helpers. Reads block the calling thread. |
| [Browser package](../packages/web/src/printer.ts) | Asynchronous methods over WebSerial/WebUSB or a caller-provided `Transport`. No built-in TCP connection to `192.168.1.70`. |
| [Terminal](../packages/node/tools/printer-term.js) | Interactive subset of the APIs, plus raw text and hexadecimal writes. It does not expose every library method. |
| [Verification tool](../packages/node/tools/check-printer.js) | Status-only by default; `--print` adds the corrected receipt; `--native-pdf417` additionally enables the experimental native command. |

Native Linux is the validated platform. Native Windows transports and Bluetooth transport are unimplemented; their presence in enums does not provide working connections. macOS uses the POSIX code but has no recorded hardware validation. Network addresses must be IPv4 literals; DNS names and IPv6 are not supported by the native connection routine. Serial supports 9600/19200/38400/57600/115200, 8N1; other baud values silently fall back to 9600. Sources: [transport implementation](../src/munbyn_printer.c), [README](../README.md).

The browser package exports the printer class, two transports, constants, and types. Its command builders are not exported through the package entry point. WebUSB/WebSerial access also depends on browser/device availability and permissions; mock transport tests do not establish USB compatibility with this physical unit. See [package exports](../packages/web/src/index.ts) and [package manifest](../packages/web/package.json).

The terminal has 48 verbs, including aliases:

| Area | Terminal commands |
| --- | --- |
| Connection/status | `connect`, `close`, `status` |
| Text/feed | `init`, `print`, `println`, `lf`, `cr`, `tab`, `feed` |
| Formatting | `align`, `bold`, `underline`, `invert`, `doublestrike`, `rotate90`, `upsidedown`, `font`, `scale`, `charspacing`, `linespacing`, `leftmargin`, `areawidth`, `charset`, `codepage`, `reset` |
| Barcodes | `barcode`, `bcheight`, `bcwidth`, `hri`, `qr`, `pdf417` |
| Mechanism/configuration | `cut`, `feedcut`, `drawer`, `selftest`, `nvlogo`, `testprint`, `buzzer`, `panel`, `wifi`, `dhcp`, `wifistatic` |
| Raw/help | `hex`, `raw`, `help`, `exit`, `quit` |

Examples, shown for reference rather than executed by this survey:

```sh
# Status only
node packages/node/tools/check-printer.js 192.168.1.70
# Print the verified receipt layout
node packages/node/tools/check-printer.js 192.168.1.70 --print
# Interactive terminal
node packages/node/tools/printer-term.js 192.168.1.70
```

## Important gaps and inconsistencies

### 1. Character-table mapping needs a firmware-specific decision

The PDF's §2.40, pp. 30–31, documents selectors 0–10 and 16–21. The C header and browser constants instead provide a different 0–67 table. The disagreement is substantive:

| Intended encoding | PDF selector | Repo's primary named selector | Repo name at the PDF selector |
| --- | --- | --- | --- |
| Windows-1252 | 16 | 11 (`WCP1252`) | Latvian |
| PC866 | 17 | 12 | Arabic |
| PC852 | 18 | 13 | PT151125 |
| PC858 | 19 | 14 | PC747 |
| Iran II | 20 | 15 | WPC1257 |
| Latvian | 21 | 16 | Thai |

Sources: [bundled PDF](ITPP047%20Program%20Manual-1.00.pdf#page=31), [C constants](../src/munbyn_printer.h), [browser constants](../packages/web/src/constants.ts). Node accepts numeric selectors and exposes `getCodepageName()`, but does not export the code-page constants.

This is a confirmed disagreement with the PDF, not proof that every extra table is absent from newer printers. MUNBYN's newer guidance mentions selectors including 69, which the C/Node implementation rejects because it caps selectors at 67. The manufacturer recommends printing the device's own code-page sheet after its self-test. That is the appropriate next evidence before choosing a mapping for this unit. [MUNBYN language/code-page guidance](https://support.munbyn.com/hc/en-us/articles/4619828093587-ITPP047-How-to-change-printing-font-according-to-my-language).

There is a second, independent issue: Node converts JS strings to UTF-8 and the browser uses `TextEncoder`. `setCodepage()` only sends a selector; it does not transcode text. An offline browser capture of `setCodepage(50); print('č')` produced selector bytes `1B 74 32` followed by UTF-8 `C4 8D`. A legacy single-byte table needs appropriately encoded bytes instead. Serbian Latin/Cyrillic letters and currency symbols need explicit encoding or host-rendered raster text, with a physical sample. The successful ASCII receipt does not validate these characters.

### 2. Two dedicated command/API gaps

- `FS 2` (§2.69, p. 51) has no wrapper in C, Node, or web. The PDF specifies first character byte `FE`, second `A1`–`FE`, and exactly 72 bitmap bytes. Raw writes can carry it; execution is untested.
- `setUnderlineKanji()` exists in C/Node and in [browser builders](../packages/web/src/commands.ts), but is absent from the browser printer class and package exports. An offline check confirmed the public method is `undefined`.

### 3. Barcode and numeric validation is incomplete

All nine linear symbologies in the PDF are represented: UPC-A, UPC-E, EAN13/JAN13, EAN8/JAN8, CODE39, ITF, CODABAR (named `CODEBAR` in the API), CODE93, and CODE128. Selectors 0–6 and 72–73 are implemented; length-prefixed equivalents 65–71 are rejected. Thus symbology coverage is broader than exact selector coverage.

Length checks exist, including fixed UPC/EAN lengths and even ITF lengths, but character alphabets and CODE128 code-set syntax are not fully validated. Offline browser captures accepted both a UPC-A containing twelve letters and CODE128 `AB` without a code-set prefix. The native implementation has the same broad validation gap. The PDF explains that invalid barcode data may be printed as ordinary text or discarded; acceptance by the API is insufficient.

GS1 extension modes 74–78 likewise use broad length checks. For comparison, Epson requires exactly 13 digits for modes 75–77 and a restricted first digit for mode 77. This establishes missing validation against the chosen Epson extension, not GS1 support in this MUNBYN firmware. [Epson GS k reference](https://download4.epson.biz/sec_pubs/pos/reference_en/escpos/gs_lk.html).

Many older browser numeric setters coerce values into bytes instead of rejecting invalid input: `feedLines(256)` emitted `1B 64 00` in the offline probe. Browser tab stops also lack the manual's count/order checks. Newer image and 2D builders validate more rigorously; validation is not uniform across APIs. See [browser command builders](../packages/web/src/commands.ts) and [native barcode implementation](../src/munbyn_printer.c).

### 4. Cutting works, but terminology needs correction

The PDF's §2.54, p. 40, specifies partial cutting only, including mode 0. The terminal's `cut full` maps to `CUT_ONE_POINT_UNCUT`, so the label promises a full cut that this command does not provide. `feedcut [lines]` is also mislabeled: `GS V 66 n` uses vertical motion units beyond the cutter position, not text lines.

`printAndCut()` explicitly feeds seven text lines and then cuts. Its observed margin is valid for the tested line-spacing configuration; changing line spacing changes the margin. `feedAndCut(7)` and `feedLines(7).cutPaper()` are different operations.

Other stale terminal help: `status` still calls parsing “known-buggy” despite the status repair; `bcwidth` advertises 1–6 while the implementation/manual require 2–6; `pdf417` lacks the unsupported-on-this-unit qualification present in the README. `testprint` correctly describes hex-dump mode now. Sources: [terminal](../packages/node/tools/printer-term.js), [cut/feed implementations](../src/munbyn_printer.c).

### 5. Status support covers a useful subset

`getStatus()` requires all four valid one-byte `DLE EOT` replies and returns seven booleans: online, paper present, cover closed, general error, cutter error, recoverable error, and unrecoverable error. It does not expose the near-end paper sensor, drawer state, or every offline cause carried by the manual's status bytes.

`setAsb()` enables automatic status transmission but there is no ASB stream decoder. Leave ASB disabled when using the current synchronous status path. `transmitStatus()` returns a raw `GS r` byte; it is not a richer interpreted status object. The PDF describes `GS r` as serial-only and `ESC c 3` as parallel-interface-only, so those wrappers must not be assumed effective over the tested network connection.

Read timeouts and incomplete-response rejection are implemented. Native callers should reconnect after a timeout; bundled browser transports close on read timeout. Jobs need sequential use: there is no printer job queue or synchronization protecting multi-command transactions. Sources: PDF §2.6, §2.35, §2.58, §2.62; [status types](../src/munbyn_printer.h), [browser status logic](../packages/web/src/printer.ts), [coverage notes](COMMAND_COVERAGE.md).

### 6. Image and advanced-mode support is mostly encoded, not exercised

| Family | Implementation boundaries / qualification |
| --- | --- |
| Raster `GS v 0` | Packed 1-bit rows, MSB first, `ceil(width/8) × height` bytes. Node/web enforce exact length; C callers must provide a sufficiently large buffer. No integrated PNG/JPEG/PDF-to-raster pipeline in the library API. |
| Column image `ESC *` | Modes 0/1/32/33, up to 1023 columns; exact 1 or 3 bytes per column. Not interchangeable with raster layout. |
| Downloaded image `GS *`, `GS /` | `x=1..255`, `y=1..48`, `x*y<=912`; exact `x*y*8` data bytes. Untested physically. |
| NV images `FS q`, `FS p` | Up to 64 KiB including block headers under the PDF rules. Definition replaces all NV images and resets the printer. Wrapper requires at least one image although the PDF range includes zero. Untested physically. |
| ASCII user glyphs `ESC &` | Codes 32–126, height 3 bytes, width up to 12 for font A or 9 for font B in native code. Browser builder permits width 12 without tracking font B. |
| Page mode / macros / Kanji / sound / drawer | Dedicated encoders exist as mapped below; their physical behavior and optional hardware have not been verified in this session. |

`executeTestPrint()` is the historical API name for `GS ( A`, which enters hex-dump mode. It is distinct from the proprietary `selfTest()` command. The PDF contains starred commands, ambiguous parameter tables, and typographical errors; it is not evidence that every optional mechanism is installed. See [manual reconciliation](COMMAND_COVERAGE.md).

## What official online sources add

### Printer family context

MUNBYN's product table lists 230 mm/s, 576 or 512 dots per line, a 72 mm printing width, fonts A 12×24 and B 9×17, the same nine 1D barcode families, and QR as its listed 2D format. Bluetooth and Wi-Fi depend on variant. This supports the QR finding but makes no native PDF417 promise. [Official ITPP047 series specifications](https://pos.munbyn.com/munbyn-itpp047-series-thermal-receipt-printer/).

The same page also uses 80 mm in its broader marketing and lists 256 KiB NV flash. These should not override the PDF's 64 KiB `FS q` image-area limit or be treated as confirmation of this unit's active print width. Paper width, active printable dots, total flash, and the NV-image allocation are different quantities. At 576 printable dots, unscaled font A would fit 48 characters and font B 64, calculated from those font widths; this has not been measured on the unit. [Official specification table](https://pos.munbyn.com/munbyn-itpp047-series-thermal-receipt-printer/).

### Implemented extensions outside the PDF

| Extension | Repo API / bytes | Evidence |
| --- | --- | --- |
| QR model 2 | `printQr()`, `GS ( k` | Physically verified; also listed by MUNBYN. API permits module sizes 1–16, EC levels 48–51, and up to 7089 input bytes. These are API limits, not a guaranteed symbol capacity. |
| PDF417 | `printPdf417()`, `GS ( k` | Native output failed on this unit. Columns 0–30, EC 0–8, payload up to 65532 bytes. Frame length is not physical symbol capacity. |
| GS1-128 / DataBar | `printBarcode()` types 74–78 | Implemented Epson-style extensions; neither PDF support nor physical validation for this printer. |
| Proprietary self-test | `selfTest()`, `1F 1B 1F 67` | Existing implementation; exact opcode not corroborated by the official pages inspected. |
| Wi-Fi credentials/static IP | `setWifi()` / `setWifiStatic()`, vendor `B3` / `B4` commands | Existing reverse-engineered implementation; exact frames not corroborated by the official pages inspected. |
| Wi-Fi DHCP | `setDhcp()`, `1F 1B 1F 28 13 14 04 n` | Manufacturer publishes this encoding, with `n=0` on and `n=1` off. Not physically exercised here. |

The DHCP finding strengthens the older coverage note, which groups Wi-Fi/DHCP under reverse-engineered commands. [MUNBYN HEX command reference](https://support.munbyn.com/hc/en-us/articles/11503298932243-ITPP047-esc-pos-HEX-command). PDF417 framing can be compared with [Epson store-data function 080](https://download4.epson.biz/sec_pubs/pos/reference_en/escpos/gs_lparen_lk_fn080.html); it does not establish MUNBYN compatibility.

### Published vendor commands with no dedicated repo wrapper

The following are documented by [MUNBYN's HEX reference](https://support.munbyn.com/hc/en-us/articles/11503298932243-ITPP047-esc-pos-HEX-command), but absent as named APIs:

| Function | Published hexadecimal bytes |
| --- | --- |
| Startup Wi-Fi configuration receipt | `1F 1B 1F 28 13 14 00 n`; 1 disables, 0 enables |
| Factory reset | `1F 1B 1F 11 11 00` |
| Persistent DHCP option | `1F 1B 10 13 14 00` |
| Reset Wi-Fi module | `1F 1B 1F 27 13 14 52 00` |
| Hardware print-content width | `1F 1B 1F E1 13 14 n` |

The width table repeats selector 5 for two widths; its mapping needs clarification before implementation. Hardware width configuration is distinct from the repo's `GS W` layout command. These commands were researched, not sent.

MUNBYN separately publishes a currency-symbol command, `1F 1B 10 12 12 n`, with selectors 0/1/2 for dollar/euro/pound; there is no dedicated wrapper. Its exact behavior on this unit remains untested. [Official currency-symbol article](https://support.munbyn.com/hc/en-us/articles/4619733099795-How-to-print-out-the-currency-symbols-USD-EUR-GBP).

DHCP also needs a precise scope: MUNBYN distinguishes Wi-Fi DHCP from LAN/NET DHCP, and its persistent-DHCP guidance describes firmware-dependent behavior after obtaining an address. The generic `setDhcp()` name does not prove that wired Ethernet DHCP or persistence is configured on every variant. [Wi-Fi versus LAN DHCP](https://support.munbyn.com/hc/en-us/articles/4619846244243-ITPP047-How-to-enable-disable-the-DHCP), [persistent-DHCP guidance](https://support.munbyn.com/hc/en-us/articles/30961237004051-How-to-set-the-printer-s-DHCP-to-remain-enabled).

### Configuration available through manufacturer tools, absent from this API

| Feature | Official evidence / boundary |
| --- | --- |
| Print density | Levels 1–8 through the setup tool; no repo density setter. [MUNBYN density guidance](https://support.munbyn.com/hc/en-us/articles/11430553600659-ITPP047-ITPP098-How-to-adjust-Print-Density) |
| Printer-side serial baud rate | Setup-tool configuration exists. `openSerial(..., baud)` only configures the host port. [MUNBYN baud-rate guidance](https://support.munbyn.com/hc/en-us/articles/9659329648659-How-to-Modify-the-Baudrate-of-ITPP047) |
| Wired LAN address configuration | Manufacturer provides a setup-tool flow; `openNetwork()` only connects to an existing address. Wi-Fi static-IP commands do not establish wired-LAN configuration support. [MUNBYN LAN IP guidance](https://support.munbyn.com/hc/en-us/articles/17779280104211-How-to-manually-set-a-new-NET-LAN-IP-address-for-ITPP047P) |
| USB Epson compatibility mode | Manufacturer exposes a setting and reports its state on the self-test page. No repo setter. This does not establish support for all Epson commands. [Enable Epson mode](https://support.munbyn.com/hc/en-us/articles/30531813601683-ITPP047-turns-on-Epson-mode), [disable Epson mode](https://support.munbyn.com/hc/en-us/articles/47321760373907-ITPP047-turns-off-Epson-mode) |
| Firmware update | Manufacturer supplies firmware based on the self-test and device label, then uses its tool for installation. The repo has no updater. [MUNBYN firmware guidance](https://support.munbyn.com/hc/en-us/articles/9794921137171-How-to-upgrade-the-firmware-of-ITPP047-printer) |

The manufacturer also distributes platform drivers and an ITPP047/ITPP102 SDK. Those offerings are separate from this repository's transport support. The download index was inspected; the binary SDK archive was not audited. [Official drivers and SDK index](https://support.munbyn.com/hc/en-us/articles/6092502480787-Printer-Drivers-SDK-Download).

## Verification quality and suggested next work

The existing [CI workflow](../.github/workflows/test.yml) builds shared/static C libraries, Make examples, Node and browser packages, and package archives. Existing tests cover wire bytes, parameter boundaries, missing/malformed status replies, silent serial timeouts, USB transfer errors, and relocated CMake installation. They are not exhaustive per-command hardware tests. The baseline has a [successful CI run](https://github.com/seemsindie/munbync/actions/runs/35275246745).

For this survey, the browser TypeScript build passed. Offline captures reproduced the missing public method, numeric wrapping, invalid barcode acceptance, and unchanged UTF-8 text encoding. No device changes were needed to establish these findings.

Recommended order:

1. **Identify this unit's character tables and firmware.** Reconcile code-page profiles and add explicit text encoding, then verify a small Serbian Latin/Cyrillic and currency sample. This matters more for real receipts than increasing the command count.
2. **Correct validation and API/help inconsistencies.** Close the browser Kanji-underline gap, validate barcode alphabets/code sets and numeric fields, and fix the terminal cut/feed/PDF417 labels.
3. **Add custom Kanji glyph support if needed.** This closes the single completely missing PDF command family, but it is not required for the tested receipt workflow.
4. **Add a printer capability profile and PDF417 raster option if needed.** Record native QR as verified and native PDF417 as unsupported for this unit; do not equate successful transmission with feature support.
5. **Expand hardware checks by use case.** Other linear barcodes, alternate fonts, page mode, drawer, buzzer/light, NV logos, and status fault conditions still need intentional physical verification. Add ASB decoding or transport work only if the application needs it.

## Complete bundled-manual inventory

“C/N/W” means dedicated C, Node, and public web access exists. It does not mean physically verified or fully conformant. API names below are the JS methods; the CSV contains exact C names and source locations. `*` markers from the manual are not treated as proof of support. Wire notation abbreviates parameters; all numbers in the CSV wire-prefix column are hexadecimal.

| PDF section / start page | Command | JS API | Availability / note |
| --- | --- | --- | --- |
| §2.1 / 6 | `HT` | `horizontalTab` | C/N/W. Next configured tab stop. |
| §2.2 / 6 | `LF` | `lineFeed` | C/N/W. Used in verified receipt. |
| §2.3 / 6 | `FF` | `formFeed` | C/N/W. Page print, clear, return to standard. |
| §2.4 / 7 | `CR` | `carriageReturn` | C/N/W. Effect depends on mode/settings. |
| §2.5 / 7 | `CAN` | `cancelPageData` | C/N/W. Cancel page-mode buffer. |
| §2.6 / 7 | `DLE EOT n` | `getStatus` | C/N/W. Four groups queried; seven fields exposed; healthy replies verified. |
| §2.7 / 10 | `DLE ENQ n` | `realtimeRequest` | C/N/W. Recovery selectors 1/2. |
| §2.8 / 10 | `DLE DC4 1 m t` | `realtimeDrawerPulse` | C/N/W. Pin 0/1; t=1..8 in 100 ms units. |
| §2.9 / 11 | `ESC FF` | `printPageMode` | C/N/W. Print page buffer, stay in page mode. |
| §2.10 / 11 | `ESC SP n` | `setCharacterSpacing` | C/N/W. Right-side character spacing. |
| §2.11 / 12 | `ESC ! n` | `setTextMode` | C/N/W. Combined text-mode bitmask. |
| §2.12 / 13 | `ESC $ nL nH` | `setAbsoluteHorizontalPosition` | C/N/W. Absolute horizontal position. |
| §2.13 / 13 | `ESC % n` | `selectUserDefinedCharset` | C/N/W. Select/cancel downloaded ASCII glyphs. |
| §2.14 / 14 | `ESC & ...` | `defineUserDefinedChars` | C/N/W. Conservative codes 32..126; browser font-B width caveat. |
| §2.15 / 16 | `ESC * ...` | `printBitImage` | C/N/W. Column data; modes 0/1/32/33. |
| §2.16 / 17 | `ESC - n` | `setUnderline` | C/N/W. ASCII underline. |
| §2.17 / 18 | `ESC 2` | `setLineSpacingDefault` | C/N/W. Restore default line spacing. |
| §2.18 / 18 | `ESC 3 n` | `setLineSpacing` | C/N/W. Custom line spacing. |
| §2.19 / 19 | `ESC = n` | `setPeripheralDevice` | C/N/W. Peripheral selection bitmask. |
| §2.20 / 19 | `ESC ? n` | `cancelUserDefinedChar` | C/N/W. Cancel one ASCII user glyph. |
| §2.21 / 19 | `ESC @` | `initialize` | C/N/W. Used in verified receipt. |
| §2.22 / 20 | `ESC D ... NUL` | `setHorizontalTabPositions`, `clearHorizontalTabPositions` | C/N/W. Max 32 ascending stops per PDF; browser lacks validation. |
| §2.23 / 20 | `ESC E n` | `setEmphasis` | C/N/W. Emphasis visibly verified. |
| §2.24 / 21 | `ESC G n` | `setDoubleStrike` | C/N/W. Double-strike mode. |
| §2.25 / 21 | `ESC J n` | `printAndFeedUnits` | C/N/W. Feed vertical motion units. |
| §2.26 / 22 | `ESC L` | `selectPageMode` | C/N/W. Enter page mode. |
| §2.27 / 22 | `ESC M n` | `setFont` | C/N/W. Font A/B. |
| §2.28 / 22 | `ESC R n` | `setInternationalCharset` | C/N/W. 0..15; PDF restricts 14/15 to Simplified Chinese models. |
| §2.29 / 23 | `ESC S` | `selectStandardMode` | C/N/W. Return to standard mode. |
| §2.30 / 24 | `ESC T n` | `setPrintDirection` | C/N/W. Page-mode print direction. |
| §2.31 / 24 | `ESC V n` | `setRotate90` | C/N/W. 90-degree text rotation. |
| §2.32 / 25 | `ESC W ...` | `setPageArea` | C/N/W. Page origin and dimensions. |
| §2.33 / 26 | `ESC \ nL nH` | `setRelativeHorizontalPosition` | C/N/W. Signed relative horizontal position. |
| §2.34 / 27 | `ESC a n` | `setJustification` | C/N/W. Centering visibly verified. |
| §2.35 / 27 | `ESC c 3 n` | `setPaperEndSensors` | C/N/W. PDF says parallel interface only. |
| §2.36 / 28 | `ESC c 4 n` | `setStopPrintSensors` | C/N/W. Select stop-print sensors. |
| §2.37 / 29 | `ESC c 5 n` | `setPanelButtons` | C/N/W. Panel enable/disable; PDF heading uses uppercase C erroneously. |
| §2.38 / 29 | `ESC d n` | `feedLines` | C/N/W. Seven-line receipt feed verified; depends on line spacing. |
| §2.39 / 30 | `ESC p m t1 t2` | `openDrawer`, `openDrawerDefault` | C/N/W. Drawer pulse; t1/t2 in 2 ms units. |
| §2.40 / 30 | `ESC t n` | `setCodepage` | C/N/W. Present, but named table conflicts with PDF; no text transcoding. |
| §2.41 / 31 | `ESC { n` | `setUpsideDown` | C/N/W. Upside-down text. |
| §2.42 / 31 | `FS p n m` | `printNvBitImage` | C/N/W. Print stored NV image. |
| §2.43 / 32 | `FS q ...` | `defineNvBitImage` | C/N/W. Replaces NV images and resets; wrapper rejects n=0. |
| §2.44 / 34 | `GS ! n` | `setTextScale` | C/N/W. Width/height factors 1..8. |
| §2.45 / 35 | `GS $ nL nH` | `setAbsoluteVerticalPosition` | C/N/W. Page-mode absolute vertical position. |
| §2.46 / 35 | `GS * x y ...` | `defineDownloadedBitImage` | C/N/W. x*y<=912; exact x*y*8 bitmap bytes. |
| §2.47 / 36 | `GS / m` | `printDownloadedBitImage` | C/N/W. Print volatile downloaded image. |
| §2.48 / 37 | `GS ( A ...` | `executeTestPrint` | C/N/W. Hex-dump mode, not a self-test receipt. |
| §2.49 / 37 | `GS :` | `macroDefineToggle` | C/N/W. Start/end macro definition. |
| §2.50 / 37 | `GS B n` | `setInvertedText` | C/N/W. White/black reverse text. |
| §2.51 / 38 | `GS H n` | `setHriPosition` | C/N/W. HRI below CODE128 visibly verified. |
| §2.52 / 38 | `GS L nL nH` | `setLeftMargin` | C/N/W. Left layout margin. |
| §2.53 / 39 | `GS P x y` | `setMotionUnits` | C/N/W. Motion-unit scale. |
| §2.54 / 40 | `GS V m [n]` | `cutPaper`, `feedAndCut` | C/N/W. Partial cut verified; PDF specifies no full cut. |
| §2.55 / 40 | `GS W nL nH` | `setPrintAreaWidth` | C/N/W. Layout width, not hardware width configuration. |
| §2.56 / 41 | `GS \ nL nH` | `setRelativeVerticalPosition` | C/N/W. Page-mode signed relative vertical position. |
| §2.57 / 42 | `GS ^ r t m` | `executeMacro` | C/N/W. Mode 0/1; delay in 100 ms units. |
| §2.58 / 42 | `GS a n` | `setAsb` | C/N/W. ASB enabled/disabled; no stream decoder. |
| §2.59 / 44 | `GS f n` | `setHriFont` | C/N/W. HRI font A/B. |
| §2.60 / 44 | `GS h n` | `setBarcodeHeight` | C/N/W. 1..255 dots; 48-dot setting used in verified receipt. |
| §2.61 / 44 | `GS k ...` | `printBarcode` | C/N/W. Nine PDF symbologies; no selectors 65..71; only CODE128 physically verified. |
| §2.62 / 47 | `GS r n` | `transmitStatus` | C/N/W. Raw byte; selectors 1/2/49/50; PDF says serial only. |
| §2.63 / 48 | `GS v 0 ...` | `printRasterImage` | C/N/W. 128x64-dot raster visibly verified; C has no buffer-length argument. |
| §2.64 / 49 | `GS w n` | `setBarcodeWidth` | C/N/W. 2..6; width 2 used in verified receipt. |
| §2.65 / 49 | `FS ! n` | `setKanjiMode` | C/N/W. Kanji mode bitmask. |
| §2.66 / 50 | `FS &` | `selectKanji` | C/N/W. Enter Kanji mode. |
| §2.67 / 50 | `FS - n` | `setUnderlineKanji` | C/N; web builder only. C/Node exposed; browser builder only, absent from public API. |
| §2.68 / 51 | `FS .` | `cancelKanji` | C/N/W. Exit Kanji mode. |
| §2.69 / 51 | `FS 2 ...` | — | Missing. No dedicated wrapper; raw writes only; 72-byte glyph data. |
| §2.70 / 52 | `FS S n1 n2` | `setKanjiSpacing` | C/N/W. Left/right Kanji spacing. |
| §2.71 / 52 | `FS W n` | `setKanjiQuadSize` | C/N/W. Quadruple-size Kanji. |
| §2.72 / 53 | `ESC B n t` | `buzzer` | C/N/W. Count/duration 1..9; duration in 50 ms units. |
| §2.73 / 53 | `ESC C m t n` | `buzzerAlarm` | C/N/W. Count/interval 1..20; mode 0..3; interval in 50 ms units. |
