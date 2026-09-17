# Munbyn Printer Barcode Commands

This document describes the barcode functionality available in the Munbyn printer library.

## Overview

The library supports printing various barcode types with configurable height, width, and HRI (Human Readable Interpretation) positioning.

## Supported Barcode Types

- **UPC-A** - Universal Product Code (11-12 digits)
- **UPC-E** - Universal Product Code (11-12 digits) 
- **EAN13/JAN13** - European Article Number (12-13 digits)
- **EAN8/JAN8** - European Article Number (7-8 digits)
- **CODE39** - Alphanumeric barcode (1-255 characters)
- **ITF** - Interleaved 2 of 5 (even number of digits, 1-255 characters)
- **CODABAR** - Used in libraries and blood banks (2-255 bytes including A-D start/stop)
- **CODE93** - More compact than CODE39 (1-255 characters)
- **CODE128** - High-density barcode (3-255 command bytes including a code-set prefix)

## Basic Usage

### 1. Set Barcode Configuration

```c
// Set barcode height (default: 162 dots, range: 1-255)
munbyn_set_barcode_height(handle, 100);

// Set barcode width (default: 3 dots, range: 2-6)
munbyn_set_barcode_width(handle, 2);

// Set HRI position
munbyn_set_hri_position(handle, MUNBYN_HRI_BELOW);
```

### 2. Print Barcode

```c
// Print a CODE39 barcode
munbyn_print_barcode(handle, MUNBYN_BARCODE_CODE39, "HELLO123");

// Print an EAN13 barcode (with valid check digit)
munbyn_print_barcode(handle, MUNBYN_BARCODE_JAN13, "1234567890128");

// Print a CODE128 barcode
munbyn_print_barcode(handle, MUNBYN_BARCODE_CODE128, "{BMunbynPrinter2024");
```

## HRI Position and Font Options

HRI (Human Readable Interpretation) is the text representation of the barcode data:

- `MUNBYN_HRI_NONE` - No HRI text printed
- `MUNBYN_HRI_ABOVE` - HRI text above the barcode
- `MUNBYN_HRI_BELOW` - HRI text below the barcode
- `MUNBYN_HRI_BOTH` - HRI text both above and below

### HRI Font

Select the typeface used for HRI characters when printing barcodes (GS f n):

- `MUNBYN_HRI_FONT_STANDARD` (n = 0 or 48): Standard ASCII characters (12 × 24)
- `MUNBYN_HRI_FONT_COMPRESSED` (n = 1 or 49): Compressed ASCII characters (9 × 17)

Example:

```c
// Use compressed HRI font to save space under barcodes
munbyn_set_hri_font(handle, MUNBYN_HRI_FONT_COMPRESSED);
```

## Data Requirements

Each barcode type has specific data requirements:

### UPC-A / UPC-E
- **Length**: 11 or 12 digits
- **Characters**: 0-9 only
- **Note**: Check digit automatically calculated if 11 digits provided

### EAN13 (JAN13)
- **Length**: 12 or 13 digits
- **Characters**: 0-9 only
- **Note**: Check digit automatically calculated if 12 digits provided

### EAN8 (JAN8)
- **Length**: 7 or 8 digits
- **Characters**: 0-9 only
- **Note**: Check digit automatically calculated if 7 digits provided

### CODE39
- **Length**: 1-255 characters
- **Characters**: 0-9, A-Z, space, $, %, +, -, ., /
- **Note**: Start/stop characters (*) automatically added

### ITF (Interleaved 2 of 5)
- **Length**: Even number of digits, 1-255 characters
- **Characters**: 0-9 only
- **Note**: Must have even number of digits

### CODABAR
- **Length**: 2-255 bytes including start/stop
- **Characters**: 0-9, A-D, $, +, -, ., /, :
- **Note**: Start/stop characters (A, B, C, D) required

### CODE93
- **Length**: 1-255 characters
- **Characters**: Full ASCII character set
- **Note**: More compact than CODE39

### CODE128
- **Length**: 3-255 command bytes; prefix alone is rejected
- Begin with `{A`, `{B`, or `{C`. Set A accepts bytes 0-95; set B accepts 32-127;
  set C uses binary values 0-99, each representing a digit pair.
- Supported escapes include code-set switches, `{S` shift in A/B, `{{` for a
  literal brace in B, and FNC1-4 (`{1`..`{4`, only FNC1 in C).
- Use the binary API for embedded NUL; string data includes its prefix, e.g.
  `{BTEST-1234`. Invalid or incomplete sequences are rejected before sending.

### Binary data and alternate selectors

Selectors 65-71 are the length-prefixed counterparts of 0-6. C constants end in
`_B` (`MUNBYN_BARCODE_CODABAR_B` for selector 71); Node/browser export corresponding
`BARCODE_*_B` names. CODE93 (72) and CODE128 (73) are also length-prefixed.

```c
const uint8_t data[] = {'{', 'A', 0, 'A'};
munbyn_print_barcode_bytes(handle, MUNBYN_BARCODE_CODE128, data, sizeof(data));
```

Node accepts `printBarcode(type, Buffer)`; browser accepts
`printBarcode(type, Uint8Array)`. Other alphabet and size constraints still apply.

GS1 selectors 74-78 are extensions outside the bundled manual. Validation checks
wire syntax, allowed characters, and basic ranges (75-77 require 13 digits; 77
starts with 0/1). It does not validate every GS1 application identifier, semantic
field length, date, or check digit. Applications must validate those and confirm
firmware support.

Node/browser PDF417 uses a local raster encoder by default. See
[implementation status](IMPLEMENTATION_STATUS.md) for native-command migration,
width options, and verified hardware behavior.

## Complete Example

```c
#include "munbyn_printer.h"

int main() {
    munbyn_handle_t handle;
    
    // Open printer connection
    munbyn_open_usb("/dev/usb/lp0", &handle);
    munbyn_initialize(handle);
    
    // Configure barcode appearance
    munbyn_set_barcode_height(handle, 100);
    munbyn_set_barcode_width(handle, 3);
    munbyn_set_hri_position(handle, MUNBYN_HRI_BELOW);
    
    // Print text label
    munbyn_write_data(handle, (uint8_t*)"Product Code:", 13);
    munbyn_line_feed(handle);
    
    // Print barcode
    munbyn_print_barcode(handle, MUNBYN_BARCODE_CODE39, "PRODUCT123");
    
    // Feed seven text lines before cutting (feed_and_cut uses motion units).
    munbyn_feed_lines(handle, MUNBYN_OPTIMAL_FEED_LINES);
    munbyn_cut_paper(handle, MUNBYN_CUT_PARTIAL);
    
    // Close connection
    munbyn_close(handle);
    return 0;
}
```

## Error Handling

All barcode functions return `munbyn_error_t` values:

- `MUNBYN_OK` - Success
- `MUNBYN_ERROR_INVALID_PARAMETER` - Invalid barcode type, data, or configuration
- `MUNBYN_ERROR_BUFFER_OVERFLOW` - Memory allocation failed
- `MUNBYN_ERROR_COMMUNICATION` - Communication error with printer

Always check return values:

```c
munbyn_error_t result = munbyn_print_barcode(handle, MUNBYN_BARCODE_CODE39, "TEST");
if (result != MUNBYN_OK) {
    fprintf(stderr, "Barcode printing failed: %d\n", result);
    return -1;
}
```

## Building and Running the Example

```bash
# Build the barcode example
make barcode_example

# Run the example (adjust device path as needed)
./barcode_example /dev/usb/lp0
```

The example program demonstrates:
- Different barcode types
- HRI positioning options
- Size variations
- Complete workflow from setup to printing

## Technical Notes

- Barcode height is specified in dots (printer resolution dependent)
- Barcode width affects the narrow bar width in dots
- Commands are based on ESC/POS GS h, GS w, GS H, and GS k commands
- The library automatically validates data format for each barcode type
- Linear barcode frames are fully validated and assembled before sending
- Two barcode printing methods are supported:
  - Method 1 (GS k m d1...dk NUL) - Used for barcode types 0-6
  - Method 2 (GS k m n d1...dn) - Types 65-73 and GS1 extensions 74-78
- HRI supports both standard (0-3) and alternative (48-51) positioning values
- Multi-level barcodes (UPC, EAN, CODE93, CODE128) and binary-level barcodes (CODE39, ITF, CODABAR) have different width specifications

### Important Notes on Check Digits and Valid Data

- **EAN13**: Check digit calculation: Sum (odd positions × 1) + (even positions × 3), then (10 - (sum mod 10)) mod 10
- **EAN8**: Check digit calculation: Sum (odd positions × 3) + (even positions × 1), then (10 - (sum mod 10)) mod 10  
- **UPC-A**: Uses a standard 12-digit format with proper check digit (e.g., "012345678905")
- **UPC-E**: Must be a valid UPC-A that can be compressed to UPC-E format (e.g., "042100005264" compresses to UPC-E)
- **CODABAR**: Requires proper start/stop characters (A, B, C, D) and industry-standard patterns for reliable scanning
- **Other barcode types**: CODE39, ITF, CODE93, CODE128 may have their own validation requirements depending on the specific implementation

### Common Valid Test Examples

```c
// UPC-A with valid check digit
munbyn_print_barcode(handle, MUNBYN_BARCODE_UPC_A, "012345678905");

// UPC-E with valid compressible UPC-A pattern  
munbyn_print_barcode(handle, MUNBYN_BARCODE_UPC_E, "042100005264");

// EAN13 with valid check digit
munbyn_print_barcode(handle, MUNBYN_BARCODE_JAN13, "1234567890128");

// EAN8 with valid check digit (calculated: 0)
munbyn_print_barcode(handle, MUNBYN_BARCODE_JAN8, "12345670");

// CODABAR with industry-standard library pattern
munbyn_print_barcode(handle, MUNBYN_BARCODE_CODEBAR, "A123456789B");
```
