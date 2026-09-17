# Character sets and explicit text encoding

`ESC t n` chooses a table in printer firmware. It does not convert UTF-8 into
that table. The older `MUNBYN_CODEPAGE_*` enum and `getCodepageName()` retain their
legacy numbers for source compatibility; their names are not verified firmware
identification. Numeric selectors 0–255 are accepted by all three APIs.

## Find the correct selector

The bundled [manual](ITPP047%20Program%20Manual-1.00.pdf), §2.40, p. 30, specifies
these single-byte tables:

| Host encoding | Manual selector | Retained legacy selector |
| --- | --- | --- |
| CP437 | 0 | 0 |
| CP850 | 2 | 2 |
| Windows-1252 | 16 | 11 |
| CP866 | 17 | 12 |
| CP852 | 18 | 13 |
| CP858 | 19 | 14 |
| Windows-1250 | Not specified | 50 |
| Windows-1251 | Not specified | 51 |

These are distinct mapping sources, not interchangeable profiles. The printer
may have another firmware table. MUNBYN's [font-selection instructions](https://support.munbyn.com/hc/en-us/articles/4619828093587-ITPP047-How-to-change-printing-font-according-to-my-language)
describe printing the self-test by holding FEED while powering on, then pressing
FEED again for the code-page sheet. Record model, main firmware, Wi-Fi firmware,
and the actual table list before choosing a selector. Those sheets have not yet
been supplied for the tested unit.

C exposes `MUNBYN_MANUAL_CODEPAGE_*` constants for the six documented mappings.
Node/browser export `codepageProfiles.manual` and `codepageProfiles.legacy`.
Neither detects the connected printer or asserts its installed tables.

## Strict conversion

C `munbyn_print_encoded(handle, utf8, encoding, selector)` and JavaScript
`printEncoded(text, encoding, selector)` convert a UTF-8 string to a selected
single-byte encoding. Supported encodings are ASCII, CP437, CP850, CP852, CP858,
CP866, Windows-1250, Windows-1251, and Windows-1252. JavaScript names are `ascii`,
`cp437`, `cp850`, `cp852`, `cp858`, `cp866`, `windows1250`, `windows1251`, and
`windows1252`.

Conversion validates the entire input before writing. Unrepresentable characters,
malformed UTF-8, and control characters other than tab, CR, and LF are rejected.
JavaScript rejects embedded NUL; the C string ends at its first NUL. There is no
silent substitution. The complete write exits Kanji mode (`FS .`), selects the
USA international set (`ESC R 0`), selects the explicit code page (`ESC t n`),
and then sends the converted bytes. These settings persist for later text.

```c
// Only use selector 18 if the printer's sheet identifies it as CP852.
munbyn_error_t result = munbyn_print_encoded(
    printer, "Čćšžđ\n", MUNBYN_ENCODING_CP852, 18);
if (result != MUNBYN_OK) {
    // Handle the error; no bytes were sent for invalid text.
}
```

```js
// Node is synchronous; await the equivalent browser method.
printer.printEncoded('Čćšžđ\n', 'cp852', 18);
// Cyrillic needs a different encoding and its own confirmed firmware selector:
// printer.printEncoded('ЉЊЋЂ Ј\n', 'windows1251', confirmedSelector);
```

`print()` and C raw writes keep their existing behavior. For already-encoded
bytes, use `writeData(Buffer/Uint8Array)` or `munbyn_write_data()` after choosing
the matching table. Raw writes do not validate text. International character
sets (`ESC R`) substitute certain ASCII positions; they are separate from the
high-byte table and are not a UTF-8 conversion mechanism.

The Node terminal starts with ASCII. `encoding cp852 18` selects the encoder and
table for subsequent `print` / `println`. Select a number from the unit's sheet.
`hex` remains available for explicitly constructed bytes.

## Character-table examples

`examples/charset_example.c` demonstrates conversion with the manual's CP437/850
selectors. `examples/codepage_table_example.c` prints a raw byte grid for an
explicit selector (0–255), with legacy names clearly labeled. Both open
`/dev/usb/lp0` and print immediately; adjust the device path for your system.

```sh
make
./build/examples/codepage_table_example 18 true 7
```

A printed grid helps confirm a table; successful transmission alone does not.
Host-side Serbian Latin/Cyrillic conversion is covered by fixed byte-vector
tests. Non-English text has not yet been physically verified on this unit.
Regenerate the shared C/browser tables and Node mapping IDs with
`python3 tools/generate-encodings.py` (Python standard-library codecs).
