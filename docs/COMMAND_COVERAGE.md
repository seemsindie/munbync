# ITPP047 manual reconciliation

Reference: [ITPP047 Program Manual 1.00](ITPP047%20Program%20Manual-1.00.pdf),
59 pages. Page numbers below are the printed/PDF page numbers. This records the
new command work and its limits; it does not claim support for every firmware.

| Area | Manual reference | Implementation and validation |
| --- | --- | --- |
| Real-time status | §2.6, pp. 7–9 | Query groups 1–4 separately; exactly one byte per query; validate fixed bits; reject missing replies. Online comes from group 1, cover/general error from group 2, cutter/recoverable/unrecoverable errors from group 3, paper end from group 4. |
| Recovery and real-time drawer | §2.7–2.8, pp. 10–11 | Recovery n=1/2; drawer pin 0/1, duration 1–8 × 100 ms. |
| Column images | §2.15, pp. 16–17 | Modes 0/1/32/33; up to 1023 columns; exactly 1 or 3 bytes per column. |
| Downloaded images | §2.46–2.47, pp. 35–37 | x=1–255, y=1–48, x×y≤912; exactly x×y×8 data bytes. |
| NV images | §2.42–2.43, pp. 31–34 | Validate every header and bitmap before sending; x=1–1023, y=1–288; all blocks together ≤65536 bytes. Replaces all stored images and triggers a reset. |
| User-defined ASCII glyphs | §2.14, pp. 14–15 | y=3; one width+bitmap block per character; font A width≤12, font B width≤9 in native API. Stateless browser builders enforce width≤12; callers selecting font B must keep width≤9. |
| Page mode | §2.3, 2.5, 2.9, 2.26, 2.29–2.33, 2.45, 2.56 | Commands and little-endian position fields are exposed; signed relative positions use 16-bit two's complement. |
| Hex-dump mode | §2.48, p. 37 | `executeTestPrint` is the existing API name for GS ( A. Valid values are n=0/48, m=1/49; the binding default is now 0,1. This is not a self-test pattern. |
| Macros | §2.49, 2.57, pp. 37, 42 | Definition toggle and execution; m=0/1, delay in 100 ms units. |
| Buffered status / ASB | §2.58, 2.62, pp. 42–43, 47–48 | GS r selectors 1/2/49/50; ASB bitmask exposed. ASB frames are not decoded. Disable ASB before synchronous status queries; invalid real-time replies fail instead of being misparsed. |
| Linear barcodes | §2.61, pp. 44–46 | Existing types 0–6, 72–73; length-prefixed framing for 72/73 and extension types. Payload bounds checked before sending. C string API cannot carry embedded NUL; raw writes remain available. |
| Raster images | §2.63, pp. 48–49 | Exact row-major bitmap length checked by Node/web; C callers must supply the required buffer size. |
| Kanji | §2.65–2.71, pp. 49–52 | Mode, underline, spacing, selection/cancel, and quadruple size exposed. FS 2 custom Kanji glyph definition remains available through raw writes, without a dedicated wrapper. |
| Sound/light | §2.72–2.73, p. 53 | Buzzer count/duration 1–9; alarm count/interval 1–20, mode 0–3; timing uses 50 ms units. |

## Manual inconsistencies

- §2.14 gives an upper code of 127 in its range, but 0x7E in the details.
  The glyph wrapper uses the conservative 32–126 range.
- §2.62's summary table conflicts with both its range and detailed status tables.
  The API follows the range/detailed tables: 1/49 paper, 2/50 drawer.
- The manual labels several commands with `(*)` without establishing universal
  firmware support. Command encoding and successful transmission alone cannot
  establish the printed result.

## Firmware-dependent extensions

These are absent from this manual and are kept as explicitly conditional APIs:

- QR model 2 and PDF417 use Epson-style `GS ( k` frames. PDF417 data is bounded
  to 65532 bytes, leaving three function bytes in the 16-bit length field, per
  [Epson function 080](https://download4.epson.biz/sec_pubs/pos/reference_en/escpos/gs_lparen_lk_fn080.html).
  Error correction uses the level form documented by
  [Epson function 069](https://download4.epson.biz/sec_pubs/pos/reference_en/escpos/gs_lparen_lk_fn069.html).
  QR input is capped at 7089 bytes. These are API/framing limits, not guarantees
  that all inputs fit a symbol at every error-correction setting or print width.
- GS1 type values 74–78 are extension values beyond the manual's 65–73 range.
  Applications must provide the required GS1 content and verify firmware support.
- `1F 1B 1F 67` self-test and Wi-Fi/DHCP commands retain their existing
  reverse-engineered encodings. They are not validated by this PDF. Wi-Fi
  changes can interrupt network communication and require a power cycle.

## Transport behavior and verification

C USB/serial reads have a 1000 ms readiness timeout. Network reads use the
configured timeout, falling back to 1000 ms for nonpositive values. Missing
responses return an error. A status result is populated only after all four
queries succeed. Close/reopen after a timeout to discard late replies.

Browser reads default to 1000 ms; timeout closes the connection and cancels the
pending read. Serial reads retain excess bytes for the next read, and USB reads
respect the returned DataView's bounds and transfer status. Await operations
sequentially; printer handles/transports are not designed for concurrent jobs.

Automated checks use capture files, a silent serial pseudo-terminal, and mocked
browser devices. They verify packet bytes, boundary rejection, complete status,
read timeouts, native buffer lengths, and relocated CMake installation. They do
not prove physical output, device-driver compatibility, or undocumented firmware
support. Use `node packages/node/tools/check-printer.js <ip> --print` for a live
receipt, then inspect/scan the printed results.

## Live ITPP047 check (2026-09-17)

A network-connected printer returned status bytes `16 12 12 12`: online, cover
closed, paper present, and no reported errors. A user-supplied receipt photo
confirmed native CODE128 and QR output. Native PDF417 instead printed fragments
of its command data, so support for that extension is **not present on the tested
firmware**. The small raster sample was only 16×8 dots; the verification tool now
uses a visible 128×64-dot checkerboard. Its footer uses `printAndCut()` with the
existing seven-line feed because the first three-line feed clipped the footer.

Native PDF417 is an opt-in diagnostic in the receipt tool (`--native-pdf417`),
not part of the default test. Applications targeting this firmware should render
PDF417 with an external encoder and print the resulting raster bitmap instead.
A successful send or healthy status cannot establish barcode rendering support.

The second receipt photo and user confirmation verified the larger checkerboard
and intact footer with a clear margin before the cut. ZBar decoded the photo's
QR as `MUNBYNC-OK` and CODE128 as `TEST-1234`. Printer status remained healthy
before and after both receipts. This validates those features on this unit; it
does not extend support claims to other firmware or the untested commands.
