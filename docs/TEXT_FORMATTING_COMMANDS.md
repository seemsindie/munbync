# Text Formatting Commands

This document describes the comprehensive text formatting, justification, and spacing commands available in the Munbyn printer library.

## Overview

The library now supports a complete set of ESC/POS text formatting commands including:
- Text justification (left, center, right)
- Font selection (Font A and Font B)
- Text emphasis and styles (bold, underline, double-strike)
- Combined text modes (double height/width)
- Advanced text scaling (1x to 8x width/height)
- Inverted text (white on black background)
- Line and character spacing control
- Character pitch control (10-20 CPI)
- Text rotation and inversion
- Print direction control
- Horizontal positioning (absolute and relative)
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
  - `MUNBYN_JUSTIFY_LEFT_ALT` (48): Left alignment (alternative)
  - `MUNBYN_JUSTIFY_CENTER_ALT` (49): Center alignment (alternative)
  - `MUNBYN_JUSTIFY_RIGHT_ALT` (50): Right alignment (alternative)

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
  - `MUNBYN_FONT_A_ALT` (48): Font A (12×24 dots) alternative
  - `MUNBYN_FONT_B_ALT` (49): Font B (9×17 dots) alternative

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
Sets underline mode for regular characters using ESC/POS command `ESC - n`.

```c
munbyn_error_t munbyn_set_underline(munbyn_handle_t handle, uint8_t mode);
```

**Parameters:**
- `mode`: Underline thickness
  - `0` or `48`: No underline
  - `1` or `49`: 1-dot thick underline
  - `2` or `50`: 2-dot thick underline

### `munbyn_set_underline_kanji(handle, mode)`
Sets underline mode for Kanji characters using ESC/POS command `FS - n`.

```c
munbyn_error_t munbyn_set_underline_kanji(munbyn_handle_t handle, uint8_t mode);
```

**Parameters:**
- `mode`: Underline thickness
  - `0` or `48`: No underline for Kanji characters
  - `1` or `49`: 1-dot thick underline for Kanji characters
  - `2` or `50`: 2-dot thick underline for Kanji characters

**Note:** This command specifically affects Kanji characters and does not change the underline setting for regular characters.

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
- `spacing`: Line spacing in vertical motion units (0-255)
  - With default motion units: n/180 inch
  - ESC 3 30 = 1/6 inch (same as ESC 2 default)
  - Motion units can be changed with `munbyn_set_motion_units()`

### `munbyn_set_motion_units(handle, horizontal, vertical)`
Sets horizontal and vertical motion units using ESC/POS command `GS P x y`.

```c
munbyn_error_t munbyn_set_motion_units(munbyn_handle_t handle, uint8_t horizontal, uint8_t vertical);
```

**Parameters:**
- `horizontal`: Horizontal motion unit (0-255), 0 = use default (180)
- `vertical`: Vertical motion unit (0-255), 0 = use default (180)

**Motion unit calculation:**
- Actual unit = 1/n inches = 25.4/n mm
- Default horizontal: 1/180 inch ≈ 0.141mm  
- Default vertical: 1/180 inch ≈ 0.141mm
- Note: Manual shows y=360 default, but standard ESC/POS uses 180

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

**Parameters:**
- `margin`: Left margin in horizontal motion units (0-65535)
  - With default motion units: margin/180 inches
  - Actual margin = margin × horizontal_motion_unit
  - Example: margin=90 → 90/180 = 0.5 inches

**Note:** Only effective at beginning of line in standard mode. In page mode, performs only internal flag operations.

### `munbyn_set_print_area_width(handle, width)`
Sets print area width using ESC/POS command `GS W nL nH`.

```c
munbyn_error_t munbyn_set_print_area_width(munbyn_handle_t handle, uint16_t width);
```

**Parameters:**
- `width`: Print area width in horizontal motion units (0-65535)
  - With default motion units: width/180 inches
  - Actual width = width × horizontal_motion_unit
  - Default: 512 units (nL=0, nH=2) for standard paper
  - For 58mm paper: 360 units (nL=104, nH=1)

**Important Notes:**
- Only effective when processed at beginning of line in standard mode
- In page mode, performs only internal flag operations
- If [left margin + print area width] exceeds printable area, the printable area is automatically adjusted
- The printer automatically extends/reduces the area to accommodate characters and bit images

**Examples:**
```c
// Standard paper width (default)
munbyn_set_print_area_width(handle, 512);   // 512/180 = 2.84 inches

// 58mm paper width  
munbyn_set_print_area_width(handle, 360);   // 360/180 = 2.0 inches

// Custom widths
munbyn_set_print_area_width(handle, 180);   // 180/180 = 1.0 inch
munbyn_set_print_area_width(handle, 270);   // 270/180 = 1.5 inches
```

## Text Rotation and Inversion Commands

### `munbyn_set_rotate_90(handle, enabled)`
Enables 90° clockwise text rotation using ESC/POS command `ESC V n`.

```c
munbyn_error_t munbyn_set_rotate_90(munbyn_handle_t handle, bool enabled);
```

**Parameters:**
- `enabled`: true = enable 90° rotation, false = disable
- Internally uses n=0,1 (alternative values n=48,49 are functionally identical)

**Important Notes:**
- Affects printing in standard mode (setting always effective)
- When underline mode is on, 90°-rotated text is not underlined
- Double-width/height commands work oppositely in rotation mode vs normal mode

### `munbyn_set_upside_down(handle, enabled)`
Enables upside-down text printing using ESC/POS command `ESC { n`.

```c
munbyn_error_t munbyn_set_upside_down(munbyn_handle_t handle, bool enabled);
```

**Parameters:**
- `enabled`: true = enable upside-down printing, false = disable
- Uses LSB logic: any even n = off, any odd n = on (only lowest bit matters)
- Range: 0-255, but implementation uses 0,1 for clarity

## Advanced Text Effects

### `munbyn_set_inverted_text(handle, enabled)`
Enables white/black reverse printing mode using ESC/POS command `GS B n`.

```c
munbyn_error_t munbyn_set_inverted_text(munbyn_handle_t handle, bool enabled);
```

**Description:**
- Creates white text on black background when enabled
- Uses the GS B n command specific to Munbyn printers
- Only the lowest bit of the parameter is valid
- Higher priority than underline mode
- Useful for highlighting important information
- Can be combined with other formatting effects

**Example:**
```c
munbyn_set_inverted_text(printer, true);
munbyn_write_data(printer, "IMPORTANT!", 10);
munbyn_set_inverted_text(printer, false);
```

### `munbyn_set_text_scale(handle, width_scale, height_scale)`
Sets advanced text scaling using ESC/POS command `GS ! n`.

```c
munbyn_error_t munbyn_set_text_scale(munbyn_handle_t handle, uint8_t width_scale, uint8_t height_scale);
```

**Parameters:**
- `width_scale`: Width scaling factor (1-8 times)
- `height_scale`: Height scaling factor (1-8 times)

**Example:**
```c
// Create 3x2 scaled text
munbyn_set_text_scale(printer, 3, 2);
munbyn_write_data(printer, "BIG TEXT", 8);
munbyn_set_text_scale(printer, 1, 1); // Reset
```

### `munbyn_cancel_all_formatting(handle)`
Resets all text formatting to defaults.

```c
munbyn_error_t munbyn_cancel_all_formatting(munbyn_handle_t handle);
```

**Description:**
- Cancels all active formatting modes
- Resets to normal text, left alignment, Font A
- Useful for ensuring clean state

## Print Direction and Positioning

### `munbyn_set_print_direction(handle, direction)`
Selects print direction and starting position in page mode using ESC/POS command `ESC T n`.

```c
munbyn_error_t munbyn_set_print_direction(munbyn_handle_t handle, uint8_t direction);
```

**Parameters:**
- `direction`: Print direction and starting position (page mode only)
  - `0` or `48`: Left to right, Upper left starting position
  - `1` or `49`: Bottom to top, Lower left starting position
  - `2` or `50`: Right to left, Lower right starting position
  - `3` or `51`: Top to bottom, Upper right starting position

**Note:** This command only affects printing in page mode, not standard mode.

### `munbyn_set_absolute_horizontal_position(handle, position)`
Sets absolute print position from beginning of line using ESC/POS command `ESC $ nL nH`.

```c
munbyn_error_t munbyn_set_absolute_horizontal_position(munbyn_handle_t handle, uint16_t position);
```

**Description:**
- Sets distance from beginning of line to print position
- Distance = `(nL + nH × 256) × (motion unit)` inches
- Position value: `0-65535` (nL + nH × 256)
- Settings outside printable area are ignored

**Motion Units:**
- Units specified by `GS P` command (default: 180 DPI)
- Standard mode: Uses horizontal motion unit (x)
- Page mode: Unit type depends on starting position set by `munbyn_set_print_direction`
  - Upper left/lower right: horizontal motion unit (x)
  - Upper right/lower left: vertical motion unit (y)

### `munbyn_set_relative_horizontal_position(handle, position)`
Sets relative print position from current position using ESC/POS command `ESC \ nL nH`.

```c
munbyn_error_t munbyn_set_relative_horizontal_position(munbyn_handle_t handle, int16_t position);
```

**Description:**
- Moves print position relative to current position
- Positive values: rightward movement (`N = nL + nH × 256`)
- Negative values: leftward movement using 65536 complement (`nL + nH × 256 = 65536 - N`)
- Distance calculated in horizontal or vertical motion units
- Motion unit type depends on print direction (set by `munbyn_set_print_direction`)
- Any setting that exceeds printable area is ignored

**Motion Units:**
- Standard mode: Uses horizontal motion unit
- Page mode: Unit type depends on starting position set by ESC T

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
    munbyn_set_line_spacing(printer, 45);  // 45/180 = 1/4 inch (with default motion units)
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
| `munbyn_set_justification` | `ESC a n` | Set text justification (n=0-2, 48-50) |
| `munbyn_set_font` | `ESC M n` | Select character font |
| `munbyn_set_text_mode` | `ESC ! n` | Select print mode(s) |
| `munbyn_set_emphasis` | `ESC E n` | Emphasized mode on/off |
| `munbyn_set_double_strike` | `ESC G n` | Double-strike mode on/off |
| `munbyn_set_underline` | `ESC - n` | Underline mode on/off (regular characters) |
| `munbyn_set_underline_kanji` | `FS - n` | Underline mode on/off (Kanji characters) |
| `munbyn_set_line_spacing_default` | `ESC 2` | Select default line spacing |
| `munbyn_set_line_spacing` | `ESC 3 n` | Set line spacing (n × vertical motion unit) |
| `munbyn_set_motion_units` | `GS P x y` | Set horizontal/vertical motion units |
| `munbyn_set_character_spacing` | `ESC SP n` | Set character spacing |
| `munbyn_set_left_margin` | `GS L nL nH` | Set left margin (horizontal motion units) |
| `munbyn_set_print_area_width` | `GS W nL nH` | Set print area width (horizontal motion units) |
| `munbyn_set_rotate_90` | `ESC V n` | 90° rotation on/off |
| `munbyn_set_upside_down` | `ESC { n` | Upside-down printing on/off |
| `munbyn_set_inverted_text` | `GS B n` | White/black reverse printing on/off |
| `munbyn_set_text_scale` | `GS ! n` | Advanced text scaling (1-8x) |
| `munbyn_set_print_direction` | `ESC T n` | Print direction control |
| `munbyn_set_absolute_horizontal_position` | `ESC $ nL nH` | Absolute horizontal position |
| `munbyn_set_relative_horizontal_position` | `ESC \ nL nH` | Relative horizontal position |
| `munbyn_cancel_all_formatting` | Multiple | Reset all formatting |

## Notes

- All formatting commands maintain state until explicitly changed or printer is reset
- The `munbyn_initialize()` function resets all formatting to defaults
- Text modes can be combined using bitwise OR operations
- Line spacing uses vertical motion units (default: 1/180 inch), character spacing uses 1/120 inch
- Motion units can be customized with `munbyn_set_motion_units()` (default: horizontal=180, vertical=180)
- Some advanced features may not be supported on all printer models
