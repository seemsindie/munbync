import { ESC, FS, GS } from './constants.js';

const encoder = new TextEncoder();

function integer(value: number, min: number, max: number, name: string): void {
  if (!Number.isInteger(value) || value < min || value > max) {
    throw new RangeError(`${name} must be an integer from ${min} to ${max}`);
  }
}

function oneOf(value: number, allowed: number[], name: string): void {
  if (!allowed.includes(value)) throw new RangeError(`Invalid ${name}`);
}

function cString(value: string, min: number, max: number, name: string): Uint8Array {
  const encoded = encoder.encode(value);
  if (value.includes('\0') || encoded.length < min || encoded.length > max) {
    throw new RangeError(`${name} must contain ${min}..${max} bytes without NUL`);
  }
  return encoded;
}

function wifi(ssid: string, password: string, keyType: number): void {
  cString(ssid, 1, 32, 'SSID');
  cString(password, 0, 64, 'password');
  integer(keyType, 0, 8, 'keyType');
}


function bytes(...values: number[]): Uint8Array {
  return new Uint8Array(values);
}

function concat(...parts: Uint8Array[]): Uint8Array {
  let len = 0;
  for (const p of parts) len += p.length;
  const out = new Uint8Array(len);
  let offset = 0;
  for (const p of parts) {
    out.set(p, offset);
    offset += p.length;
  }
  return out;
}

// ESC @ - Initialize printer
export function initialize(): Uint8Array {
  return bytes(ESC, 0x40);
}

// GS V m - Cut paper
export function cutPaper(mode: number): Uint8Array {
  return bytes(GS, 0x56, mode);
}

// GS V 66 n - Feed and cut
export function feedAndCut(feedAmount: number): Uint8Array {
  return bytes(GS, 0x56, 66, feedAmount & 0xff);
}

// ESC d n - Feed n lines
export function feedLines(lines: number): Uint8Array {
  return bytes(ESC, 0x64, lines & 0xff);
}

// ESC p m t1 t2 - Open cash drawer
export function openDrawer(pin: number, onTime: number, offTime: number): Uint8Array {
  return bytes(ESC, 0x70, pin, onTime & 0xff, offTime & 0xff);
}

// Self-test command
export function selfTest(): Uint8Array {
  return bytes(0x1f, 0x1b, 0x1f, 0x67);
}

// LF
export function lineFeed(): Uint8Array {
  return bytes(0x0a);
}

// CR
export function carriageReturn(): Uint8Array {
  return bytes(0x0d);
}

// HT
export function horizontalTab(): Uint8Array {
  return bytes(0x09);
}

// ESC D n1...nk NUL - Set horizontal tab positions
export function setHorizontalTabPositions(positions: number[]): Uint8Array {
  const cmd = new Uint8Array(2 + positions.length + 1);
  cmd[0] = ESC;
  cmd[1] = 0x44;
  for (let i = 0; i < positions.length; i++) {
    cmd[2 + i] = positions[i];
  }
  cmd[2 + positions.length] = 0x00;
  return cmd;
}

// ESC D NUL - Clear tab positions
export function clearHorizontalTabPositions(): Uint8Array {
  return bytes(ESC, 0x44, 0x00);
}

// ESC R n - Select international character set
export function setInternationalCharset(charset: number): Uint8Array {
  return bytes(ESC, 0x52, charset);
}

// ESC t n - Select code page
export function setCodepage(codepage: number): Uint8Array {
  return bytes(ESC, 0x74, codepage);
}

// ESC a n - Set justification
export function setJustification(justify: number): Uint8Array {
  return bytes(ESC, 0x61, justify);
}

// ESC M n - Select font
export function setFont(font: number): Uint8Array {
  return bytes(ESC, 0x4d, font);
}

// ESC ! n - Select print mode(s)
export function setTextMode(modes: number): Uint8Array {
  return bytes(ESC, 0x21, modes);
}

// ESC E n - Emphasis on/off
export function setEmphasis(enabled: boolean): Uint8Array {
  return bytes(ESC, 0x45, enabled ? 1 : 0);
}

// ESC G n - Double-strike on/off
export function setDoubleStrike(enabled: boolean): Uint8Array {
  return bytes(ESC, 0x47, enabled ? 1 : 0);
}

// ESC - n - Underline mode
export function setUnderline(mode: number): Uint8Array {
  return bytes(ESC, 0x2d, mode);
}

// FS - n - Kanji underline mode
export function setUnderlineKanji(mode: number): Uint8Array {
  return bytes(FS, 0x2d, mode);
}

// ESC 2 - Default line spacing
export function setLineSpacingDefault(): Uint8Array {
  return bytes(ESC, 0x32);
}

// ESC 3 n - Set line spacing
export function setLineSpacing(spacing: number): Uint8Array {
  return bytes(ESC, 0x33, spacing & 0xff);
}

// GS P x y - Set motion units
export function setMotionUnits(horizontal: number, vertical: number): Uint8Array {
  return bytes(GS, 0x50, horizontal & 0xff, vertical & 0xff);
}

// ESC SP n - Set character spacing
export function setCharacterSpacing(spacing: number): Uint8Array {
  return bytes(ESC, 0x20, spacing & 0xff);
}

// GS L nL nH - Set left margin
export function setLeftMargin(margin: number): Uint8Array {
  return bytes(GS, 0x4c, margin & 0xff, (margin >> 8) & 0xff);
}

// GS W nL nH - Set print area width
export function setPrintAreaWidth(width: number): Uint8Array {
  return bytes(GS, 0x57, width & 0xff, (width >> 8) & 0xff);
}

// ESC V n - Rotate 90 degrees
export function setRotate90(enabled: boolean): Uint8Array {
  return bytes(ESC, 0x56, enabled ? 1 : 0);
}

// ESC { n - Upside-down mode
export function setUpsideDown(enabled: boolean): Uint8Array {
  return bytes(ESC, 0x7b, enabled ? 1 : 0);
}

// GS B n - White/black reverse
export function setInvertedText(enabled: boolean): Uint8Array {
  return bytes(GS, 0x42, enabled ? 1 : 0);
}

// GS ! n - Character size (scale)
export function setTextScale(widthScale: number, heightScale: number): Uint8Array {
  integer(widthScale, 1, 8, 'widthScale'); integer(heightScale, 1, 8, 'heightScale');
  const value = ((widthScale - 1) << 4) | (heightScale - 1);
  return bytes(GS, 0x21, value);
}

// ESC T n - Print direction in page mode
export function setPrintDirection(direction: number): Uint8Array {
  return bytes(ESC, 0x54, direction);
}

// ESC \ nL nH - Relative horizontal position
export function setRelativeHorizontalPosition(position: number): Uint8Array {
  let value: number;
  if (position >= 0) {
    value = position;
  } else {
    value = 65536 + position;
  }
  return bytes(ESC, 0x5c, value & 0xff, (value >> 8) & 0xff);
}

// ESC $ nL nH - Absolute horizontal position
export function setAbsoluteHorizontalPosition(position: number): Uint8Array {
  return bytes(ESC, 0x24, position & 0xff, (position >> 8) & 0xff);
}

// GS h n - Barcode height
export function setBarcodeHeight(height: number): Uint8Array {
  integer(height, 1, 255, 'height');
  return bytes(GS, 0x68, height & 0xff);
}

// GS w n - Barcode width
export function setBarcodeWidth(width: number): Uint8Array {
  integer(width, 2, 6, 'width');
  return bytes(GS, 0x77, width & 0xff);
}

// GS H n - HRI position
export function setHriPosition(position: number): Uint8Array {
  return bytes(GS, 0x48, position);
}

// GS f n - HRI font
export function setHriFont(font: number): Uint8Array {
  return bytes(GS, 0x66, font);
}

// GS k - Print barcode
export function printBarcode(type: number, data: string): Uint8Array {
  oneOf(type, [0, 1, 2, 3, 4, 5, 6, 72, 73, 74, 75, 76, 77, 78], 'barcode type');
  const payload = cString(data, type === 73 ? 2 : 1, 255, 'barcode data');
  const lengths: Record<number, number[]> = {0: [11, 12], 1: [11, 12], 2: [12, 13], 3: [7, 8]};
  if (lengths[type]) oneOf(payload.length, lengths[type], 'barcode length');
  if (type === 5 && payload.length % 2 !== 0) throw new RangeError('ITF requires an even length');
  const encoded = encoder.encode(data);

  // CODE93/CODE128 and GS1 family (72-78) use method 2: GS k m n d1...dn
  if (type >= 72) {
    const cmd = new Uint8Array(4 + encoded.length);
    cmd[0] = GS;
    cmd[1] = 0x6b;
    cmd[2] = type;
    cmd[3] = encoded.length;
    cmd.set(encoded, 4);
    return cmd;
  }

  // Others use method 1: GS k m d1...dk NUL
  const cmd = new Uint8Array(3 + encoded.length + 1);
  cmd[0] = GS;
  cmd[1] = 0x6b;
  cmd[2] = type;
  cmd.set(encoded, 3);
  cmd[3 + encoded.length] = 0x00;
  return cmd;
}

// GS ( k - QR code (model 2). moduleSize 1-16, ecLevel 48-51 (L/M/Q/H).
export function printQr(data: string, moduleSize: number = 6, ecLevel: number = 49): Uint8Array {
  cString(data, 1, 7089, 'QR data');
  integer(moduleSize, 1, 16, 'moduleSize');
  integer(ecLevel, 48, 51, 'ecLevel');
  const encoded = encoder.encode(data);
  const storeLen = encoded.length + 3;
  return concat(
    bytes(GS, 0x28, 0x6b, 0x04, 0x00, 0x31, 0x41, 0x32, 0x00), // model 2
    bytes(GS, 0x28, 0x6b, 0x03, 0x00, 0x31, 0x43, moduleSize & 0xff), // module size
    bytes(GS, 0x28, 0x6b, 0x03, 0x00, 0x31, 0x45, ecLevel & 0xff), // error correction
    bytes(GS, 0x28, 0x6b, storeLen & 0xff, (storeLen >> 8) & 0xff, 0x31, 0x50, 0x30), // store header
    encoded,
    bytes(GS, 0x28, 0x6b, 0x03, 0x00, 0x31, 0x51, 0x30) // print
  );
}

// GS ( k - PDF417. columns 0=auto (else 1-30), ecLevel 0-8.
export function printPdf417(data: string, columns: number = 0, ecLevel: number = 1): Uint8Array {
  cString(data, 1, 65532, 'PDF417 data');
  integer(columns, 0, 30, 'columns');
  integer(ecLevel, 0, 8, 'ecLevel');
  const encoded = encoder.encode(data);
  const storeLen = encoded.length + 3;
  return concat(
    bytes(GS, 0x28, 0x6b, 0x03, 0x00, 0x30, 0x41, columns & 0xff), // columns
    bytes(GS, 0x28, 0x6b, 0x04, 0x00, 0x30, 0x45, 0x30, (0x30 + ecLevel) & 0xff), // EC level
    bytes(GS, 0x28, 0x6b, storeLen & 0xff, (storeLen >> 8) & 0xff, 0x30, 0x50, 0x30), // store header
    encoded,
    bytes(GS, 0x28, 0x6b, 0x03, 0x00, 0x30, 0x51, 0x30) // print
  );
}

// GS v 0 m xL xH yL yH d1..dk - Raster bit image
export function printRasterImage(
  mode: number,
  bitmap: Uint8Array,
  widthPixels: number,
  heightPixels: number
): Uint8Array {
  oneOf(mode, [0, 1, 2, 3, 48, 49, 50, 51], 'image mode');
  integer(widthPixels, 1, 65535, 'widthPixels');
  integer(heightPixels, 1, 65535, 'heightPixels');
  if (bitmap.length !== Math.ceil(widthPixels / 8) * heightPixels) {
    throw new RangeError('Raster bitmap length does not match dimensions');
  }
  const widthBytes = Math.ceil(widthPixels / 8);
  const header = bytes(
    GS,
    0x76,
    0x30,
    mode,
    widthBytes & 0xff,
    (widthBytes >> 8) & 0xff,
    heightPixels & 0xff,
    (heightPixels >> 8) & 0xff
  );

  const cmd = new Uint8Array(header.length + bitmap.length);
  cmd.set(header, 0);
  cmd.set(bitmap, header.length);
  return cmd;
}

// --- Network / WiFi (vendor; send over USB/serial, then power-cycle) ---
// 1F 1B 1F B3 <keyt> <ssid> 00 <password> 00
export function setWifi(ssid: string, password: string, keyType: number = 8): Uint8Array {
  wifi(ssid, password, keyType);
  return concat(
    bytes(0x1f, 0x1b, 0x1f, 0xb3, keyType & 0xff),
    encoder.encode(ssid),
    bytes(0x00),
    encoder.encode(password),
    bytes(0x00)
  );
}
// 1F 1B 1F B4 <ip4> <mask4> <gateway4> <keyt> <ssid> 00 <password> 00 - WiFi + static IP
export function setWifiStatic(
  ssid: string,
  password: string,
  keyType: number,
  ip: number[],
  mask: number[],
  gateway: number[]
): Uint8Array {
  wifi(ssid, password, keyType);
  for (const address of [ip, mask, gateway]) {
    if (address.length !== 4) throw new RangeError('Expected four IPv4 octets');
    for (const octet of address) integer(octet, 0, 255, 'IPv4 octet');
  }
  const oct = (a: number[]) => Uint8Array.from([a[0] & 0xff, a[1] & 0xff, a[2] & 0xff, a[3] & 0xff]);
  return concat(
    bytes(0x1f, 0x1b, 0x1f, 0xb4),
    oct(ip),
    oct(mask),
    oct(gateway),
    bytes(keyType & 0xff),
    encoder.encode(ssid),
    bytes(0x00),
    encoder.encode(password),
    bytes(0x00)
  );
}
// 1F 1B 1F 28 13 14 04 n  (n=0 DHCP on, 1 off)
export function setDhcp(enabled: boolean): Uint8Array {
  return bytes(0x1f, 0x1b, 0x1f, 0x28, 0x13, 0x14, 0x04, enabled ? 0 : 1);
}

// --- Kanji ---
// FS ! n
export function setKanjiMode(modes: number): Uint8Array {
  return bytes(FS, 0x21, modes & 0xff);
}
// FS &
export function selectKanji(): Uint8Array {
  return bytes(FS, 0x26);
}
// FS .
export function cancelKanji(): Uint8Array {
  return bytes(FS, 0x2e);
}
// FS S n1 n2
export function setKanjiSpacing(left: number, right: number): Uint8Array {
  return bytes(FS, 0x53, left & 0xff, right & 0xff);
}
// FS W n
export function setKanjiQuadSize(enabled: boolean): Uint8Array {
  return bytes(FS, 0x57, enabled ? 1 : 0);
}

// --- Mechanism / sound / macros ---
// ESC c 5 n - enable/disable panel buttons (LSB 0=enable, 1=disable)
export function setPanelButtons(enabled: boolean): Uint8Array {
  return bytes(ESC, 0x63, 0x35, enabled ? 0 : 1);
}
// ESC B n t - buzzer (MUNBYN-specific)
export function buzzer(count: number, duration: number): Uint8Array {
  integer(count, 1, 9, 'count'); integer(duration, 1, 9, 'duration');
  return bytes(ESC, 0x42, count & 0xff, duration & 0xff);
}
// ESC C m t n - buzzer + alarm light (MUNBYN-specific)
export function buzzerAlarm(count: number, interval: number, mode: number): Uint8Array {
  integer(count, 1, 20, 'count'); integer(interval, 1, 20, 'interval'); integer(mode, 0, 3, 'mode');
  return bytes(ESC, 0x43, count & 0xff, interval & 0xff, mode & 0xff);
}
// GS : - start/end macro definition
export function macroDefineToggle(): Uint8Array {
  return bytes(GS, 0x3a);
}
// GS ^ r t m - execute macro
export function executeMacro(times: number, wait: number, mode: number): Uint8Array {
  integer(times, 0, 255, 'times'); integer(wait, 0, 255, 'wait'); oneOf(mode, [0, 1], 'mode');
  return bytes(GS, 0x5e, times & 0xff, wait & 0xff, mode & 0xff);
}

// --- Status & real-time ---
// DLE ENQ n
export function realtimeRequest(n: number): Uint8Array {
  oneOf(n, [1, 2], 'request');
  return bytes(0x10, 0x05, n & 0xff);
}
// DLE DC4 1 m t
export function realtimeDrawerPulse(pin: number, onTime: number): Uint8Array {
  oneOf(pin, [0, 1], 'pin'); integer(onTime, 1, 8, 'onTime');
  return bytes(0x10, 0x14, 0x01, pin & 0xff, onTime & 0xff);
}
// GS r n
export function transmitStatus(n: number): Uint8Array {
  oneOf(n, [1, 2, 49, 50], 'status group');
  return bytes(GS, 0x72, n & 0xff);
}
// GS a n
export function setAsb(n: number): Uint8Array {
  return bytes(GS, 0x61, n & 0xff);
}
// ESC c 3 n
export function setPaperEndSensors(n: number): Uint8Array {
  return bytes(ESC, 0x63, 0x33, n & 0xff);
}
// ESC c 4 n
export function setStopPrintSensors(n: number): Uint8Array {
  return bytes(ESC, 0x63, 0x34, n & 0xff);
}
// GS ( A pL pH n m - enter hex-dump mode (manual p. 37)
export function executeTestPrint(n: number, m: number): Uint8Array {
  oneOf(n, [0, 48], 'hex dump target'); oneOf(m, [1, 49], 'hex dump mode');
  return bytes(GS, 0x28, 0x41, 0x02, 0x00, n & 0xff, m & 0xff);
}

// --- Misc text / position / user-defined characters ---
// ESC J n - print and feed n motion units
export function printAndFeedUnits(units: number): Uint8Array {
  return bytes(ESC, 0x4a, units & 0xff);
}
// ESC = n - select peripheral device
export function setPeripheralDevice(n: number): Uint8Array {
  return bytes(ESC, 0x3d, n & 0xff);
}
// ESC % n - select/cancel user-defined character set
export function selectUserDefinedCharset(enabled: boolean): Uint8Array {
  return bytes(ESC, 0x25, enabled ? 1 : 0);
}
// ESC & y c1 c2 d... - define user-defined characters
export function defineUserDefinedChars(
  y: number,
  c1: number,
  c2: number,
  data: Uint8Array
): Uint8Array {
  oneOf(y, [3], 'character height');
  integer(c1, 32, 126, 'c1');
  integer(c2, c1, 126, 'c2');
  let offset = 0;
  for (let c = c1; c <= c2; c++) {
    if (offset >= data.length) throw new RangeError('Missing character width');
    const width = data[offset++];
    integer(width, 0, 12, 'character width');
    offset += width * y;
    if (offset > data.length) throw new RangeError('Incomplete character data');
  }
  if (offset !== data.length) throw new RangeError('Unexpected character data');
  return concat(bytes(ESC, 0x26, y & 0xff, c1 & 0xff, c2 & 0xff), data);
}
// ESC ? n - cancel user-defined character
export function cancelUserDefinedChar(code: number): Uint8Array {
  integer(code, 32, 126, 'code');
  return bytes(ESC, 0x3f, code & 0xff);
}

// --- Page mode ---
export function selectPageMode(): Uint8Array {
  return bytes(ESC, 0x4c); // ESC L
}
export function selectStandardMode(): Uint8Array {
  return bytes(ESC, 0x53); // ESC S
}
export function printPageMode(): Uint8Array {
  return bytes(ESC, 0x0c); // ESC FF
}
export function formFeed(): Uint8Array {
  return bytes(0x0c); // FF
}
export function cancelPageData(): Uint8Array {
  return bytes(0x18); // CAN
}
// ESC W xL xH yL yH dxL dxH dyL dyH
export function setPageArea(x: number, y: number, dx: number, dy: number): Uint8Array {
  for (const value of [x, y, dx, dy]) integer(value, 0, 65535, 'page coordinate');
  return bytes(
    ESC, 0x57,
    x & 0xff, (x >> 8) & 0xff,
    y & 0xff, (y >> 8) & 0xff,
    dx & 0xff, (dx >> 8) & 0xff,
    dy & 0xff, (dy >> 8) & 0xff
  );
}
// GS $ nL nH
export function setAbsoluteVerticalPosition(position: number): Uint8Array {
  integer(position, 0, 65535, 'position');
  return bytes(GS, 0x24, position & 0xff, (position >> 8) & 0xff);
}
// GS \ nL nH
export function setRelativeVerticalPosition(position: number): Uint8Array {
  integer(position, -32768, 32767, 'position');
  const value = position >= 0 ? position : 65536 + position;
  return bytes(GS, 0x5c, value & 0xff, (value >> 8) & 0xff);
}

// ESC * m nL nH d... - Column bit image
export function printBitImage(mode: number, widthDots: number, data: Uint8Array): Uint8Array {
  oneOf(mode, [0, 1, 32, 33], 'bit image mode');
  integer(widthDots, 1, 1023, 'widthDots');
  if (data.length !== widthDots * (mode < 32 ? 1 : 3)) {
    throw new RangeError('Column bitmap length does not match dimensions');
  }
  return concat(bytes(ESC, 0x2a, mode & 0xff, widthDots & 0xff, (widthDots >> 8) & 0xff), data);
}

// GS * x y d... - Define downloaded bit image
export function defineDownloadedBitImage(x: number, y: number, data: Uint8Array): Uint8Array {
  integer(x, 1, 255, 'x');
  integer(y, 1, 48, 'y');
  if (x * y > 912 || data.length !== x * y * 8) {
    throw new RangeError('Downloaded bitmap exceeds capacity or has the wrong length');
  }
  return concat(bytes(GS, 0x2a, x & 0xff, y & 0xff), data);
}

// GS / m - Print downloaded bit image
export function printDownloadedBitImage(mode: number = 0): Uint8Array {
  oneOf(mode, [0, 1, 2, 3, 48, 49, 50, 51], 'image mode');
  return bytes(GS, 0x2f, mode & 0xff);
}

// FS p n m - Print NV bit image
export function printNvBitImage(n: number, mode: number = 0): Uint8Array {
  integer(n, 1, 255, 'n'); oneOf(mode, [0, 1, 2, 3, 48, 49, 50, 51], 'image mode');
  return bytes(FS, 0x70, n & 0xff, mode & 0xff);
}

// FS q n [xL xH yL yH d...] - Define NV bit image(s)
export function defineNvBitImage(numImages: number, imageData: Uint8Array): Uint8Array {
  integer(numImages, 1, 255, 'numImages');
  if (imageData.length > 65536) throw new RangeError('NV images exceed 64 KiB');
  let offset = 0;
  for (let i = 0; i < numImages; i++) {
    if (imageData.length - offset < 4) throw new RangeError('Missing NV image header');
    const x = imageData[offset] | (imageData[offset + 1] << 8);
    const y = imageData[offset + 2] | (imageData[offset + 3] << 8);
    integer(x, 1, 1023, 'NV width');
    integer(y, 1, 288, 'NV height');
    offset += 4 + x * y * 8;
    if (offset > imageData.length) throw new RangeError('Incomplete NV image');
  }
  if (offset !== imageData.length) throw new RangeError('Unexpected NV image data');
  return concat(bytes(FS, 0x71, numImages & 0xff), imageData);
}

// DLE EOT n - Request real-time status.
// n: 1=printer/transmission, 2=off-line cause, 3=error, 4=paper roll sensor.
export function requestStatus(n: number = 1): Uint8Array {
  integer(n, 1, 4, 'status group');
  return bytes(0x10, 0x04, n & 0xff);
}

// Encode text to bytes
export function text(str: string): Uint8Array {
  return encoder.encode(str);
}
