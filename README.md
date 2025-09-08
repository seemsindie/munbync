# Munbyn Printer Library

A C library for controlling Munbyn thermal printers, providing comprehensive support for text formatting, barcode printing, and character set management.

## Features

- **Text Formatting**: Font sizes, styles, alignment, rotation
- **Barcode Support**: Multiple barcode types (Code128, EAN, QR codes, etc.)
- **Character Sets**: Support for multiple codepages and character encodings
- **Cross-Platform**: Works on Linux, macOS, and Windows
- **Simple API**: Easy-to-use functions for common printing tasks

## Project Structure

```
munbync/
├── src/                          # Source code
│   ├── munbyn_printer.c         # Main library implementation
│   └── munbyn_printer.h         # Library header file
├── examples/                     # Example programs
│   ├── example.c                # Basic usage example
│   ├── barcode_example.c        # Barcode printing examples
│   ├── text_formatting_example.c # Text formatting examples
│   └── ...                      # Other examples
├── docs/                         # Documentation
│   ├── BARCODE_COMMANDS.md      # Barcode command reference
│   ├── TEXT_FORMATTING_COMMANDS.md # Text formatting reference
│   └── ...                      # Other documentation
├── build/                        # Build outputs (auto-generated)
│   ├── lib/                     # Libraries (.a, .so files)
│   ├── examples/                # Example executables
│   └── obj/                     # Object files
├── Makefile                      # Build configuration
└── README.md                     # This file
```

## Building

### Prerequisites

- GCC compiler
- Make
- Standard C library

### Build Commands

```bash
# Build everything (library + examples)
make all

# Build only the static library
make library

# Build only the shared library
make shared

# Build only example programs
make examples

# Clean build files
make clean

# Show all available targets
make help
```

### Build Outputs

- **Libraries**: `build/lib/libmunbyn.a` (static), `build/lib/libmunbyn.so` (shared)
- **Examples**: `build/examples/` directory contains all example executables

## Installation (Linux/macOS)

```bash
# Install system-wide
sudo make install

# Uninstall
sudo make uninstall
```

## Usage

### Basic Example

```c
#include "munbyn_printer.h"

int main() {
    // Open printer connection
    MunbynPrinter printer = munbyn_open("/dev/ttyUSB0", 9600);
    if (!printer) {
        printf("Failed to open printer\n");
        return 1;
    }
    
    // Print some text
    munbyn_print_text(printer, "Hello, World!\n");
    
    // Print a barcode
    munbyn_print_barcode_code128(printer, "1234567890", 50, 2, 1);
    
    // Feed and cut
    munbyn_feed_lines(printer, 3);
    munbyn_cut_paper(printer);
    
    // Close connection
    munbyn_close(printer);
    return 0;
}
```

### Compiling Your Programs

After installation:
```bash
gcc -o myprogram myprogram.c -lmunbyn
```

Or if using the library locally:
```bash
gcc -Isrc -o myprogram myprogram.c -Lbuild/lib -lmunbyn
```

## Examples

The `examples/` directory contains several demonstration programs:

- **`example.c`**: Basic printing and barcode operations
- **`text_formatting_example.c`**: Text styling, fonts, and alignment
- **`barcode_example.c`**: Various barcode types and configurations
- **`charset_example.c`**: Character set and codepage examples
- **`feed_test.c`**: Paper feeding and cutting operations

Run the examples:
```bash
make examples
./build/examples/example
./build/examples/barcode_example
# etc.
```

## Documentation

Detailed command references are available in the `docs/` directory:

- [`docs/TEXT_FORMATTING_COMMANDS.md`](docs/TEXT_FORMATTING_COMMANDS.md) - Text styling and formatting
- [`docs/BARCODE_COMMANDS.md`](docs/BARCODE_COMMANDS.md) - Barcode printing commands
- [`docs/CHARSET_COMMANDS.md`](docs/CHARSET_COMMANDS.md) - Character set handling

## License

This project is open source. Please refer to the LICENSE file for details.

## Contributing

Contributions are welcome! Please feel free to submit issues and pull requests.
