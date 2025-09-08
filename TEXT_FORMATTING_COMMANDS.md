# Text Formatting Commands

This document describes the comprehensive text formatting, justification, and spacing commands available in the Munbyn printer library.

## Overview

The library now supports a complete set of ESC/POS text formatting commands including:
- Text justification (left, center, right)
- Font selection (Font A and Font B)
- Text emphasis and styles (bold, underline, double-strike)
- Combined text modes (double height/width)
- Line and character spacing control
- Text rotation and inversion
- Margin and print area control

## Text Justification Commands

### `munbyn_set_justification(handle, justify)`
Sets the text justification using ESC/POS command `ESC a n`.

```c
munbyn_error_t munbyn_set_justification(munbyn_handle_t handle, munbyn_justify_t justify);
```

**Parameters:**
- `justify`: Justification mode
  - `MUNBYN_JUSTIFY_LEFT` (0): Left alignment
  - `MUNBYN_JUSTIFY_CENTER` (1): Center alignment
  - `MUNBYN_JUSTIFY_RIGHT` (2): Right alignment

**Example:**
```c
munbyn_set_justification(printer, MUNBYN_JUSTIFY_CENTER);
munbyn_write_data(printer, "Centered Text", 13);
```

## Font Selection Commands

### `munbyn_set_font(handle, font)`
Selects the character font using ESC/POS command `ESC M n`.

```c
munbyn_error_t munbyn_set_font(munbyn_handle_t handle, munbyn_font_t font);
```

**Parameters:**
- `font`: Font type
  - `MUNBYN_FONT_A` (0): Font A (12×24 dots)
  - `MUNBYN_FONT_B` (1): Font B (9×17 dots)

**Example:**
```c
munbyn_set_font(printer, MUNBYN_FONT_B);  // Smaller font
munbyn_write_data(printer, "Small text", 10);
```

## Text Emphasis Commands

### `munbyn_set_emphasis(handle, enabled)`
Enables or disables emphasized (bold) text using ESC/POS command `ESC E n`.

```c
munbyn_error_t munbyn_set_emphasis(munbyn_handle_t handle, bool enabled);
```

### `munbyn_set_double_strike(handle, enabled)`
Enables or disables double-strike text using ESC/POS command `ESC G n`.

```c
munbyn_error_t munbyn_set_double_strike(munbyn_handle_t handle, bool enabled);
```

### `munbyn_set_underline(handle, mode)`
Sets underline mode using ESC/POS command `ESC - n`.

```c
munbyn_error_t munbyn_set_underline(munbyn_handle_t handle, uint8_t mode);
```

**Parameters:**
- `mode`: Underline thickness
  - `0`: No underline
  - `1`: 1-dot thick underline
  - `2`: 2-dot thick underline

## Combined Text Modes

### `munbyn_set_text_mode(handle, modes)`
Sets multiple text modes simultaneously using ESC/POS command `ESC ! n`.

```c
munbyn_error_t munbyn_set_text_mode(munbyn_handle_t handle, uint8_t modes);
```

**Mode Flags:**
- `MUNBYN_MODE_NORMAL` (0x00): Normal text
- `MUNBYN_MODE_EMPHASIZED` (0x08): Bold text
- `MUNBYN_MODE_DOUBLE_HEIGHT` (0x10): Double height
- `MUNBYN_MODE_DOUBLE_WIDTH` (0x20): Double width
- `MUNBYN_MODE_UNDERLINE` (0x80): Underlined text

**Example:**
```c
// Big bold text
uint8_t big_bold = MUNBYN_MODE_EMPHASIZED | MUNBYN_MODE_DOUBLE_HEIGHT | MUNBYN_MODE_DOUBLE_WIDTH;
munbyn_set_text_mode(printer, big_bold);
munbyn_write_data(printer, "BIG BOLD", 8);
```

## Line Spacing Commands

### `munbyn_set_line_spacing_default(handle)`
Sets default line spacing using ESC/POS command `ESC 2`.

```c
munbyn_error_t munbyn_set_line_spacing_default(munbyn_handle_t handle);
```

### `munbyn_set_line_spacing(handle, spacing)`
Sets custom line spacing using ESC/POS command `ESC 3 n`.

```c
munbyn_error_t munbyn_set_line_spacing(munbyn_handle_t handle, uint8_t spacing);
```

**Parameters:**
- `spacing`: Line spacing in 1/180 inch units (0-255)

## Character Spacing Commands

### `munbyn_set_character_spacing(handle, spacing)`
Sets right-side character spacing using ESC/POS command `ESC SP n`.

```c
munbyn_error_t munbyn_set_character_spacing(munbyn_handle_t handle, uint8_t spacing);
```

**Parameters:**
- `spacing`: Character spacing in 1/120 inch units (0-255)

## Margin and Print Area Commands

### `munbyn_set_left_margin(handle, margin)`
Sets left margin using ESC/POS command `GS L nL nH`.

```c
munbyn_error_t munbyn_set_left_margin(munbyn_handle_t handle, uint16_t margin);
```

### `munbyn_set_print_area_width(handle, width)`
Sets print area width using ESC/POS command `GS W nL nH`.

```c
munbyn_error_t munbyn_set_print_area_width(munbyn_handle_t handle, uint16_t width);
```

## Text Rotation and Inversion Commands

### `munbyn_set_rotate_90(handle, enabled)`
Enables 90° clockwise text rotation using ESC/POS command `ESC V n`.

```c
munbyn_error_t munbyn_set_rotate_90(munbyn_handle_t handle, bool enabled);
```

### `munbyn_set_upside_down(handle, enabled)`
Enables upside-down text printing using ESC/POS command `ESC { n`.

```c
munbyn_error_t munbyn_set_upside_down(munbyn_handle_t handle, bool enabled);
```

### `munbyn_set_character_smoothing(handle, enabled)`
Enables character smoothing using ESC/POS command `GS b n`.

```c
munbyn_error_t munbyn_set_character_smoothing(munbyn_handle_t handle, bool enabled);
```

## Complete Example

See `text_formatting_example.c` for a comprehensive demonstration of all text formatting features.

```c
#include "munbyn_printer.h"

int main() {
    munbyn_handle_t printer;
    
    // Open and initialize
    munbyn_open_usb("/dev/usb/lp0", &printer);
    munbyn_initialize(printer);
    
    // Center-aligned header
    munbyn_set_justification(printer, MUNBYN_JUSTIFY_CENTER);
    munbyn_set_emphasis(printer, true);
    munbyn_write_data(printer, "RECEIPT HEADER", 14);
    munbyn_line_feed(printer);
    
    // Reset formatting
    munbyn_set_justification(printer, MUNBYN_JUSTIFY_LEFT);
    munbyn_set_emphasis(printer, false);
    
    // Body text with custom spacing
    munbyn_set_line_spacing(printer, 45);  // 45/180 = 1/4 inch
    munbyn_write_data(printer, "Item 1: $10.00", 14);
    munbyn_line_feed(printer);
    munbyn_write_data(printer, "Item 2: $15.00", 14);
    munbyn_line_feed(printer);
    
    // Big total
    munbyn_set_text_mode(printer, MUNBYN_MODE_DOUBLE_HEIGHT | MUNBYN_MODE_EMPHASIZED);
    munbyn_write_data(printer, "TOTAL: $25.00", 13);
    munbyn_line_feed(printer);
    
    // Clean up and cut
    munbyn_feed_lines(printer, 3);
    munbyn_cut_paper(printer, MUNBYN_CUT_PARTIAL);
    munbyn_close(printer);
    
    return 0;
}
```

## Building and Running

1. Build the library and examples:
   ```bash
   make clean && make
   ```

2. Run the formatting demo:
   ```bash
   ./text_formatting_example [device_path]
   ```

3. Test specific formatting features:
   ```bash
   ./text_formatting_example /dev/usb/lp0
   ```

## ESC/POS Command Reference

| Function | ESC/POS Command | Description |
|----------|----------------|-------------|
| `munbyn_set_justification` | `ESC a n` | Set text justification |
| `munbyn_set_font` | `ESC M n` | Select character font |
| `munbyn_set_text_mode` | `ESC ! n` | Select print mode(s) |
| `munbyn_set_emphasis` | `ESC E n` | Emphasized mode on/off |
| `munbyn_set_double_strike` | `ESC G n` | Double-strike mode on/off |
| `munbyn_set_underline` | `ESC - n` | Underline mode on/off |
| `munbyn_set_line_spacing_default` | `ESC 2` | Select default line spacing |
| `munbyn_set_line_spacing` | `ESC 3 n` | Set line spacing |
| `munbyn_set_character_spacing` | `ESC SP n` | Set character spacing |
| `munbyn_set_left_margin` | `GS L nL nH` | Set left margin |
| `munbyn_set_print_area_width` | `GS W nL nH` | Set print area width |
| `munbyn_set_rotate_90` | `ESC V n` | 90° rotation on/off |
| `munbyn_set_upside_down` | `ESC { n` | Upside-down printing on/off |
| `munbyn_set_character_smoothing` | `GS b n` | Character smoothing on/off |

## Notes

- All formatting commands maintain state until explicitly changed or printer is reset
- The `munbyn_initialize()` function resets all formatting to defaults
- Text modes can be combined using bitwise OR operations
- Spacing values are specified in printer-specific units (1/180 inch for line spacing, 1/120 inch for character spacing)
- Some advanced features may not be supported on all printer models
