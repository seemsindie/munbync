# MUNBYN ITPP047 printer library

C library, Node.js bindings, and browser ESC/POS command builders for the MUNBYN
ITPP047 thermal receipt printer. The bundled [programming manual, version
1.00](docs/ITPP047%20Program%20Manual-1.00.pdf) is the reference for documented
commands and parameter limits.

## Repository

- `src/`: C API and USB device, serial, and IPv4 TCP transports.
- `packages/node/`: native Node.js bindings, TypeScript declarations, and terminal.
- `packages/web/`: browser command builders, WebSerial, and WebUSB transports.
- `examples/`: C examples; several print immediately when run.
- `tests/`: protocol, timeout, and installed-package regression checks.
- `docs/COMMAND_COVERAGE.md`: manual reconciliation, extensions, and limitations.
- `docs/IMPLEMENTATION_STATUS.md`: completed fixes, migration examples, and remaining hardware checks.
- `docs/ITPP047_COMMAND_MATRIX.csv`: all 73 manual command families mapped to the three APIs.

## Build and test the C library

Requires a C99 compiler, CMake 3.16+, and Python 3 for tests. Linux is the validated
native platform. macOS uses the POSIX implementation but has not been validated
on hardware. Windows native transports and Bluetooth are not implemented;
opening them returns an error.

```sh
./build.sh --test
# A build directory outside the repository also works:
./build.sh --build-dir /tmp/munbyn-build --test
# Static library and checks:
./build.sh --static --no-examples --test
```

The traditional `make all shared` builds the library and examples. CMake also
supports installation and `find_package`; see [CMAKE.md](CMAKE.md).

```c
#include "munbyn_printer.h"

int main(void) {
    munbyn_handle_t printer = NULL;
    if (munbyn_open_network("192.0.2.10", 9100, 2000, &printer) != MUNBYN_OK)
        return 1;
    munbyn_error_t result = munbyn_initialize(printer);
    if (result == MUNBYN_OK)
        result = munbyn_print_and_cut(printer, "Hello, world!\n", MUNBYN_CUT_PARTIAL);
    munbyn_close(printer);
    return result == MUNBYN_OK ? 0 : 1;
}
```

Replace the example address with the printer's IPv4 address. For USB use
`munbyn_open_usb("/dev/usb/lp0", &printer)`; for serial use
`munbyn_open_serial("/dev/ttyUSB0", 9600, &printer)`.

## Node.js

Requires Node.js 18+, a C/C++ compiler, and Python for node-gyp.

```sh
npm --prefix packages/node ci
npm --prefix packages/node test
node packages/node/tools/printer-term.js <printer-ip>
# Read-only live check; add --print to produce one verification receipt:
node packages/node/tools/check-printer.js <printer-ip>
```

```js
const { MunbynPrinter } = require('./packages/node');
const printer = new MunbynPrinter();
try {
  printer.openNetwork('192.0.2.10');
  console.log(printer.getStatus());
  printer.initialize().print('Hello, world!\n').feedLines(7).cutPaper();
} finally {
  printer.close();
}
```

The binding is synchronous; reads block the calling thread up to the transport
read timeout. Use a worker thread when necessary. Rebuilding synchronizes the C
sources into an ignored `core/` directory. `npm pack` includes those sources,
so the package can build independently of this repository.

## Browser

```sh
npm --prefix packages/web ci
npm --prefix packages/web test
```

```js
import { MunbynPrinter, WebSerialTransport } from './packages/web/dist/index.js';
const printer = new MunbynPrinter(new WebSerialTransport({ readTimeoutMs: 1000 }));
await printer.connect(); // Invoke from a user gesture to show the port chooser.
try {
  await printer.initialize();
  await printer.print('Hello, world!\n');
} finally {
  await printer.disconnect();
}
```

Serve the browser example from HTTPS or localhost. WebSerial/WebUSB availability
and device permissions depend on the browser and OS. Browsers do not connect to
raw printer TCP port 9100 through these transports; use the Node package for
network printing.

The browser package uses `bwip-js/browser` for local PDF417 encoding. Use a
bundler, or the import map in `packages/web/examples/index.html` when serving
the built modules directly.

## Command behavior

Status requires all four one-byte `DLE EOT` replies. Missing or malformed replies
are errors, never healthy defaults. USB devices with no return channel may still
print successfully while status reads fail. A browser read timeout closes the
transport to prevent late responses from contaminating the next request.
Reconnect after a timeout, and await operations sequentially on each printer.

Native QR/PDF417, GS1, proprietary self-test, and Wi-Fi configuration are
firmware-dependent extensions absent from the bundled manual. Successful writes
confirm transmission only; inspect printed output to establish firmware support.
See [command coverage](docs/COMMAND_COVERAGE.md) before using these extensions.

The C raster API requires the caller to supply a buffer of
`((width + 7) / 8) * height` bytes. Node/browser wrappers check its length.
Barcode string APIs reject embedded NUL in the bindings. For binary barcode
payloads, use `munbyn_print_barcode_bytes`, Node `Buffer`, or browser `Uint8Array`.
Payload alphabets, lengths, CODE128 sequences, and GS1 wire syntax are checked
before sending. Applications remain responsible for GS1 application-identifier
semantics and firmware support.

`print()` retains its raw UTF-8 behavior. For printer code pages, use
`printEncoded(text, encoding, selector)` with an explicit encoding and the
selector from your printer's code-page sheet. Conversion rejects unrepresentable
characters before sending. Legacy enum names do not establish the mapping on
your firmware. See [text encoding](docs/CHARSET_COMMANDS.md).

Node/browser `printPdf417()` now encodes locally and prints a raster image.
Their default `itpp047-tested` profile blocks native PDF417, which printed command
text on the tested unit. `printPdf417Native()` is available with an explicit
`generic` profile for other, independently verified firmware. The C native API
retains its behavior; opt into `MUNBYN_PROFILE_ITPP047_TESTED` to block it there.
The live receipt includes raster PDF417; `--native-pdf417` adds the experimental
native diagnostic. See [implementation status](docs/IMPLEMENTATION_STATUS.md)
for migration and verification limits.

## License

[MIT](LICENSE).
