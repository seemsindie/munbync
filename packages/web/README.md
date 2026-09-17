# munbync-web

Browser ESC/POS command builders and WebSerial/WebUSB transports for MUNBYN
ITPP047 printers. Build with `npm ci && npm run build`; run hardware-free
regressions with `npm test`.

```js
import { MunbynPrinter, WebSerialTransport } from 'munbync-web';
const printer = new MunbynPrinter(new WebSerialTransport({
  baudRate: 9600,
  readTimeoutMs: 1000,
}));
await printer.connect(); // Call from a click handler.
try {
  await printer.initialize();
  await printer.print('Hello!\n');
} finally {
  await printer.disconnect();
}
```

Use HTTPS or localhost and a browser that provides the selected hardware API.
`WebUSBTransport` accepts `filters` and `readTimeoutMs` options. These transports
do not provide raw TCP printing; use the native Node package for port 9100.

Reads return up to the requested byte count and have a timeout. Timeouts close
the transport, cancelling outstanding reads before reconnecting. `getStatus()`
requires all four valid one-byte replies. A one-way printer interface can print
without returning status. Keep automatic status back (ASB) disabled when querying
status synchronously, and await operations sequentially.

Command builders check variable-length frame sizes before writing. Text is UTF-8;
selecting a legacy printer codepage does not transcode JavaScript strings.
QR/PDF417, GS1, Wi-Fi, and the proprietary self-test are firmware-dependent
extensions. `executeTestPrint()` enters hex-dump mode (`n=0`, `m=1`) according to
the ITPP047 manual; it is not a generic test pattern command.

Native PDF417 did not render a barcode on the tested ITPP047 firmware; it printed
command text. The native PDF417 API is experimental and should only be used with
firmware known to support it. For that printer, encode PDF417 externally and send
it using the raster-image API. The live receipt tool skips native PDF417 unless
`--native-pdf417` is explicitly requested.
