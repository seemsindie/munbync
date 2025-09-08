
# Character Set and Text Control Commands

This document describes the new character set and text control commands added to the Munbyn printer library.

## New Functions Added

### Text Control Commands

#### `munbyn_line_feed(handle)`
Sends a Line Feed (LF) command to move the cursor down one line.
```c
munbyn_error_t munbyn_line_feed(munbyn_handle_t handle);
```

#### `munbyn_carriage_return(handle)`
Sends a Carriage Return (CR) command to move the cursor to the beginning of the current line.
```c
munbyn_error_t munbyn_carriage_return(munbyn_handle_t handle);
```

### Character Set Commands

#### `munbyn_set_codepage(handle, codepage)`
Sets the character code page using ESC/POS command `ESC t n`.
```c
munbyn_error_t munbyn_set_codepage(munbyn_handle_t handle, munbyn_codepage_t codepage);
```

#### `munbyn_set_international_charset(handle, charset)`
Sets the international character set using ESC/POS command `ESC R n`.
```c
munbyn_error_t munbyn_set_international_charset(munbyn_handle_t handle, munbyn_international_charset_t charset);
```

#### `munbyn_get_codepage_name(codepage)`
Returns a human-readable name for the given codepage.
```c
const char* munbyn_get_codepage_name(munbyn_codepage_t codepage);
```

## Available Code Pages

The library supports 68 different code pages based on ESC/POS standards:

| Code Page | Description |
|-----------|-------------|
| `MUNBYN_CODEPAGE_PC437` | PC437(Std.Europe) |
| `MUNBYN_CODEPAGE_KATAKANA` | Katakana |
| `MUNBYN_CODEPAGE_PC850` | PC850(Multilingual) |
| `MUNBYN_CODEPAGE_PC860` | PC860(Portugal) |
| `MUNBYN_CODEPAGE_PC863` | PC863(Canadian) |
| `MUNBYN_CODEPAGE_PC865` | PC865(Nordic) |
| `MUNBYN_CODEPAGE_WEST_EUROPE` | West Europe |
| `MUNBYN_CODEPAGE_GREEK` | Greek |
| `MUNBYN_CODEPAGE_HEBREW` | Hebrew |
| `MUNBYN_CODEPAGE_ARABIC` | Arabic |
| `MUNBYN_CODEPAGE_THAI` | Thai |
| `MUNBYN_CODEPAGE_WPC1252` | WCP1252 |
| ... and many more | (See header file for complete list) |

## Available International Character Sets

| Character Set | Description |
|---------------|-------------|
| `MUNBYN_INTL_USA` | USA |
| `MUNBYN_INTL_FRANCE` | France |
| `MUNBYN_INTL_GERMANY` | Germany |
| `MUNBYN_INTL_UK` | United Kingdom |
| `MUNBYN_INTL_DENMARK_I` | Denmark I |
| `MUNBYN_INTL_SWEDEN` | Sweden |
| `MUNBYN_INTL_ITALY` | Italy |
| `MUNBYN_INTL_SPAIN_I` | Spain I |
| `MUNBYN_INTL_JAPAN` | Japan |
| `MUNBYN_INTL_NORWAY` | Norway |
| ... and more | (See header file for complete list) |

## Usage Examples

### Basic Text Control
```c
// Move to next line
munbyn_line_feed(printer);

// Return to beginning of current line
munbyn_carriage_return(printer);

// Overwrite text on current line
munbyn_write_data(printer, "Original text", 13);
munbyn_carriage_return(printer);
munbyn_write_data(printer, "Overwritten!", 12);
```

### Character Set Selection
```c
// Set codepage for European characters
munbyn_set_codepage(printer, MUNBYN_CODEPAGE_PC850);

// Set international character set for German
munbyn_set_international_charset(printer, MUNBYN_INTL_GERMANY);

// Print text with special characters
munbyn_write_data(printer, "Spëçiål çhåråçtërs", 18);
```

### Complete Example
```c
#include "munbyn_printer.h"

int main() {
    munbyn_handle_t printer;
    
    // Open and initialize printer
    munbyn_open_usb("/dev/usb/lp0", &printer);
    munbyn_initialize(printer);
    
    // Set up for multilingual text
    munbyn_set_codepage(printer, MUNBYN_CODEPAGE_PC850);
    munbyn_set_international_charset(printer, MUNBYN_INTL_GERMANY);
    
    // Print text with special characters
    munbyn_write_data(printer, "Möchten Sie Kaffee?", 19);
    munbyn_line_feed(printer);
    
    // Switch to different character set
    munbyn_set_international_charset(printer, MUNBYN_INTL_FRANCE);
    munbyn_write_data(printer, "Voulez-vous du café?", 20);
    munbyn_line_feed(printer);
    
    // Clean up
    munbyn_close(printer);
    return 0;
}
```

## Building and Running

1. Build the library and examples:
   ```bash
   make
   ```

2. Run the character set example:
   ```bash
   ./charset_example
   ```

3. Run the codepage table generator:
   ```bash
   ./codepage_table_example [codepage_number] [show_reset_message] [feed_lines]
   ```
   
   Examples:
   ```bash
   ./codepage_table_example           # Print PC437 table with help
   ./codepage_table_example 2         # Print PC850 table  
   ./codepage_table_example 7 true 5  # Print Greek table with reset message, 5 feed lines
   ```

## Codepage Table Generator

The `codepage_table_example` program generates a visual character table for any supported codepage, similar to ASCII tables. This is extremely useful for:

- Testing which characters are available in each codepage
- Debugging character encoding issues
- Visual reference for international characters
- Verifying printer support for specific codepages

### Output Format

The generated table shows:
- **Header**: Explanation of the table format
- **Codepage Info**: Name and number of the selected codepage  
- **Grid**: 16x16 character table where:
  - Rows represent the first hex digit (0-F)
  - Columns represent the second hex digit (0-F)
  - Each cell shows the character at that hex position
  - Control characters (0x00-0x20) are displayed as spaces

### Usage Examples

```bash
# Show all available codepages
./codepage_table_example

# Print standard European characters (PC437)
./codepage_table_example 0

# Print multilingual characters (PC850) 
./codepage_table_example 2

# Print Greek characters with reset message
./codepage_table_example 7 true

# Print Hebrew characters with custom feed lines
./codepage_table_example 8 false 10
```

## Notes

- The printer must be initialized before using these commands
- Not all printers support all character sets - refer to your printer's manual
- Character set changes affect subsequent text printing
- The library maintains internal state for current codepage and character set
- Default settings: PC437 codepage and USA international character set
