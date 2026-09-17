export interface PrinterStatus {
  paperPresent: boolean;
  coverClosed: boolean;
  online: boolean;
  errorOccurred: boolean;
  cutError: boolean;
  recoverableError: boolean;
  unrecoverableError: boolean;
}

export declare const constants: {
  // Justification
  JUSTIFY_LEFT: number;
  JUSTIFY_CENTER: number;
  JUSTIFY_RIGHT: number;

  // Fonts
  FONT_A: number;
  FONT_B: number;

  // Print modes (bitmask)
  MODE_NORMAL: number;
  MODE_EMPHASIZED: number;
  MODE_DOUBLE_HEIGHT: number;
  MODE_DOUBLE_WIDTH: number;
  MODE_UNDERLINE: number;

  // Cut modes
  CUT_ONE_POINT_UNCUT: number;
  CUT_PARTIAL: number;

  // Drawer pins
  DRAWER_PIN_2: number;
  DRAWER_PIN_5: number;

  // Barcode types
  BARCODE_UPC_A: number;
  BARCODE_UPC_E: number;
  BARCODE_JAN13: number;
  BARCODE_JAN8: number;
  BARCODE_CODE39: number;
  BARCODE_ITF: number;
  BARCODE_CODEBAR: number;
  BARCODE_CODE93: number;
  BARCODE_CODE128: number;
  BARCODE_GS1_128: number;
  BARCODE_GS1_DATABAR_OMNI: number;
  BARCODE_GS1_DATABAR_TRUNCATED: number;
  BARCODE_GS1_DATABAR_LIMITED: number;
  BARCODE_GS1_DATABAR_EXPANDED: number;

  // HRI position
  HRI_NONE: number;
  HRI_ABOVE: number;
  HRI_BELOW: number;
  HRI_BOTH: number;

  // HRI font
  HRI_FONT_STANDARD: number;
  HRI_FONT_COMPRESSED: number;

  // WiFi key types
  WIFI_WEP64: number;
  WIFI_WEP128: number;
  WIFI_WPA_AES_PSK: number;
  WIFI_WPA_TKIP_PSK: number;
  WIFI_WPA_TKIP_AES_PSK: number;
  WIFI_WPA2_AES_PSK: number;
  WIFI_WPA2_TKIP: number;
  WIFI_WPA2_TKIP_AES_PSK: number;
  WIFI_WPA_WPA2_MIXED: number;

  // QR error-correction levels
  QR_EC_L: number;
  QR_EC_M: number;
  QR_EC_Q: number;
  QR_EC_H: number;

  // Image modes
  IMAGE_NORMAL: number;
  IMAGE_DOUBLE_WIDTH: number;
  IMAGE_DOUBLE_HEIGHT: number;
  IMAGE_QUADRUPLE: number;
};

export declare class MunbynPrinter {
  constructor();

  /** Whether the printer connection is open */
  readonly isOpen: boolean;

  // Connection
  openUsb(devicePath: string): this;
  openSerial(portName: string, baudRate?: number): this;
  openNetwork(ip: string, port?: number, timeoutMs?: number): this;
  close(): this;

  // Core
  initialize(): this;
  writeData(data: Buffer | string): this;
  print(text: string): this;
  /** Read up to `length` bytes from the printer. Returns the bytes actually read. */
  readData(length: number): Buffer;
  getStatus(): PrinterStatus;

  // Basic operations
  cutPaper(mode?: number): this;
  feedAndCut(feedAmount?: number): this;
  feedLines(lines: number): this;
  printAndCut(text: string, cutMode?: number): this;
  openDrawer(pin: number, onTime: number, offTime: number): this;
  openDrawerDefault(pin?: number): this;
  selfTest(): this;

  // Text control
  lineFeed(): this;
  carriageReturn(): this;
  horizontalTab(): this;
  setHorizontalTabPositions(positions: number[]): this;
  clearHorizontalTabPositions(): this;

  // Character set
  setInternationalCharset(charset: number): this;
  setCodepage(codepage: number): this;
  /** Look up the human-readable name of a code page constant. */
  getCodepageName(codepage: number): string;

  // Text formatting
  setJustification(justify: number): this;
  align(alignment: 'left' | 'center' | 'right'): this;
  setFont(font: number): this;
  setTextMode(modes: number): this;
  setEmphasis(enabled: boolean): this;
  bold(enabled?: boolean): this;
  setDoubleStrike(enabled: boolean): this;
  setUnderline(mode: number): this;
  underline(enabled?: boolean): this;
  setUnderlineKanji(mode: number): this;
  setLineSpacingDefault(): this;
  setLineSpacing(spacing: number): this;
  setMotionUnits(horizontal: number, vertical: number): this;
  setCharacterSpacing(spacing: number): this;
  setLeftMargin(margin: number): this;
  setPrintAreaWidth(width: number): this;

  // Rotation / inversion
  setRotate90(enabled: boolean): this;
  setUpsideDown(enabled: boolean): this;

  // Advanced text
  setInvertedText(enabled: boolean): this;
  invert(enabled?: boolean): this;
  setTextScale(widthScale: number, heightScale: number): this;
  cancelAllFormatting(): this;

  // Print direction / position
  setPrintDirection(direction: number): this;
  setRelativeHorizontalPosition(position: number): this;
  setAbsoluteHorizontalPosition(position: number): this;

  // Barcode
  setBarcodeHeight(height: number): this;
  setBarcodeWidth(width: number): this;
  setHriPosition(position: number): this;
  setHriFont(font: number): this;
  printBarcode(type: number, data: string): this;

  // 2D barcodes
  /** Print a QR code (GS ( k, model 2). moduleSize 1-16, ecLevel = constants.QR_EC_*. */
  printQr(data: string, moduleSize?: number, ecLevel?: number): this;
  /** Print a PDF417 2D barcode. columns 0=auto (else 1-30), ecLevel 0-8. */
  /** Experimental native command; unsupported on the tested ITPP047. Use a raster encoder. */
  printPdf417(data: string, columns?: number, ecLevel?: number): this;

  // Image
  printRasterImage(mode: number, bitmap: Buffer, width: number, height: number): this;
  /** ESC * column bit image. mode 0/1 (8-dot) or 32/33 (24-dot). */
  printBitImage(mode: number, widthDots: number, data: Buffer): this;
  /** GS * define downloaded bit image. data length must be x*y*8. */
  defineDownloadedBitImage(x: number, y: number, data: Buffer): this;
  /** GS / print the downloaded bit image. */
  printDownloadedBitImage(mode?: number): this;
  /** FS p print NV bit image n at scaling mode. */
  printNvBitImage(n: number, mode?: number): this;
  /** FS q define NV bit image(s). imageData = consecutive [xL xH yL yH d...] blocks. */
  defineNvBitImage(numImages: number, imageData: Buffer): this;

  // Page mode
  selectPageMode(): this;
  selectStandardMode(): this;
  printPageMode(): this;
  formFeed(): this;
  cancelPageData(): this;
  setPageArea(x: number, y: number, dx: number, dy: number): this;
  setAbsoluteVerticalPosition(position: number): this;
  setRelativeVerticalPosition(position: number): this;

  // Misc text / user-defined characters
  printAndFeedUnits(units: number): this;
  setPeripheralDevice(n: number): this;
  selectUserDefinedCharset(enabled: boolean): this;
  /** ESC & define user-defined chars for codes c1..c2. data = per-char (width + columns) blocks. */
  defineUserDefinedChars(y: number, c1: number, c2: number, data: Buffer): this;
  cancelUserDefinedChar(code: number): this;

  // Status & real-time
  realtimeRequest(n: number): this;
  /** Pin 0/1; onTime 1..8 in 100 ms units (default 1). */
  realtimeDrawerPulse(pin?: number, onTime?: number): this;
  /** GS r transmit status; returns the status byte read back from the printer. */
  transmitStatus(n: number): number;
  setAsb(n: number): this;
  setPaperEndSensors(n: number): this;
  setStopPrintSensors(n: number): this;
  /** Enter hex-dump mode: n=0/48, m=1/49 (defaults 0,1). */
  executeTestPrint(n?: number, m?: number): this;

  // Mechanism / sound / macros
  setPanelButtons(enabled: boolean): this;
  /** ESC B - sound the buzzer count times, duration x 50ms each (MUNBYN). */
  buzzer(count?: number, duration?: number): this;
  /** ESC C - beeper + alarm light. mode 0=none 1=buzzer 2=light 3=both (MUNBYN). */
  buzzerAlarm(count?: number, interval?: number, mode?: number): this;
  macroDefineToggle(): this;
  executeMacro(times?: number, wait?: number, mode?: number): this;

  // Kanji
  setKanjiMode(modes: number): this;
  selectKanji(): this;
  cancelKanji(): this;
  setKanjiSpacing(left: number, right: number): this;
  setKanjiQuadSize(enabled: boolean): this;

  // Network / WiFi (vendor; send over USB, then power-cycle)
  /** Set WiFi SSID + password (DHCP). keyType = constants.WIFI_* (default WPA/WPA2 mixed). */
  setWifi(ssid: string, password: string, keyType?: number): this;
  /** Set WiFi + static IP. ip/mask/gateway are 4-octet arrays e.g. [192,168,1,50]. */
  setWifiStatic(ssid: string, password: string, keyType: number, ip: number[], mask: number[], gateway: number[]): this;
  /** Enable/disable DHCP (1F 1B 1F 28 13 14 04 n). */
  setDhcp(enabled: boolean): this;
}
