import type { Transport, PrinterStatus } from './types.js';
import * as cmd from './commands.js';
import * as C from './constants.js';

export class MunbynPrinter {
  private transport: Transport;

  constructor(transport: Transport) {
    this.transport = transport;
  }

  get connected(): boolean {
    return this.transport.connected;
  }

  // --- Connection ---

  async connect(): Promise<this> {
    await this.transport.connect();
    return this;
  }

  async disconnect(): Promise<this> {
    await this.transport.disconnect();
    return this;
  }

  // --- Internal: send command ---

  private async send(data: Uint8Array): Promise<void> {
    if (!this.transport.connected) {
      throw new Error('Printer not connected');
    }
    await this.transport.write(data);
  }

  // --- Core ---

  async initialize(): Promise<this> {
    await this.send(cmd.initialize());
    return this;
  }

  async writeData(data: Uint8Array | string): Promise<this> {
    if (typeof data === 'string') {
      await this.send(cmd.text(data));
    } else {
      await this.send(data);
    }
    return this;
  }

  async print(text: string): Promise<this> {
    await this.send(cmd.text(text));
    return this;
  }

  async getStatus(): Promise<PrinterStatus> {
    // Query each real-time status group separately. Paper status comes from
    // DLE EOT 4 (not DLE EOT 1, which only carries the online bit).
    const query = async (n: number): Promise<number> => {
      await this.send(cmd.requestStatus(n));
      const response = await this.transport.read(1);
      if (response.length !== 1 || (response[0] & 0x93) !== 0x12) {
        throw new Error(`Missing or invalid printer status response for group ${n}`);
      }
      return response[0];
    };

    const b1 = await query(1); // printer/transmission (online)
    const b2 = await query(2); // off-line cause (cover)
    const b3 = await query(3); // error (cutter / recoverable / unrecoverable)
    const b4 = await query(4); // paper roll sensor

    const online = (b1 & 0x08) === 0;
    const coverClosed = (b2 & 0x04) === 0;
    const cutError = (b3 & 0x08) !== 0;
    const unrecoverableError = (b3 & 0x20) !== 0;
    const recoverableError = (b3 & 0x40) !== 0;
    const paperPresent = (b4 & 0x60) === 0;
    const errorOccurred = (b2 & 0x40) !== 0 ||
      cutError || unrecoverableError || recoverableError || !paperPresent || !coverClosed;

    return {
      online,
      paperPresent,
      coverClosed,
      errorOccurred,
      cutError,
      recoverableError,
      unrecoverableError,
    };
  }

  // --- Basic operations ---

  async cutPaper(mode: number = C.CUT_PARTIAL): Promise<this> {
    await this.send(cmd.cutPaper(mode));
    return this;
  }

  async feedAndCut(feedAmount: number = C.OPTIMAL_FEED_LINES): Promise<this> {
    await this.send(cmd.feedAndCut(feedAmount));
    return this;
  }

  async feedLines(lines: number): Promise<this> {
    await this.send(cmd.feedLines(lines));
    return this;
  }

  async printAndCut(text: string, cutMode: number = C.CUT_PARTIAL): Promise<this> {
    await this.send(cmd.text(text));
    await this.send(cmd.feedLines(C.OPTIMAL_FEED_LINES));
    await this.send(cmd.cutPaper(cutMode));
    return this;
  }

  async openDrawer(pin: number, onTime: number, offTime: number): Promise<this> {
    await this.send(cmd.openDrawer(pin, onTime, offTime));
    return this;
  }

  async openDrawerDefault(pin: number = C.DRAWER_PIN_2): Promise<this> {
    await this.send(cmd.openDrawer(pin, 25, 250));
    return this;
  }

  async selfTest(): Promise<this> {
    await this.send(cmd.selfTest());
    return this;
  }

  // --- Text control ---

  async lineFeed(): Promise<this> {
    await this.send(cmd.lineFeed());
    return this;
  }

  async carriageReturn(): Promise<this> {
    await this.send(cmd.carriageReturn());
    return this;
  }

  async horizontalTab(): Promise<this> {
    await this.send(cmd.horizontalTab());
    return this;
  }

  async setHorizontalTabPositions(positions: number[]): Promise<this> {
    await this.send(cmd.setHorizontalTabPositions(positions));
    return this;
  }

  async clearHorizontalTabPositions(): Promise<this> {
    await this.send(cmd.clearHorizontalTabPositions());
    return this;
  }

  // --- Character set ---

  async setInternationalCharset(charset: number): Promise<this> {
    await this.send(cmd.setInternationalCharset(charset));
    return this;
  }

  async setCodepage(codepage: number): Promise<this> {
    await this.send(cmd.setCodepage(codepage));
    return this;
  }

  // --- Text formatting ---

  async setJustification(justify: number): Promise<this> {
    await this.send(cmd.setJustification(justify));
    return this;
  }

  async align(alignment: 'left' | 'center' | 'right'): Promise<this> {
    const map: Record<string, number> = {
      left: C.JUSTIFY_LEFT,
      center: C.JUSTIFY_CENTER,
      right: C.JUSTIFY_RIGHT,
    };
    const val = map[alignment];
    if (val === undefined) {
      throw new Error(`Invalid alignment "${alignment}" — use "left", "center", or "right"`);
    }
    await this.send(cmd.setJustification(val));
    return this;
  }

  async setFont(font: number): Promise<this> {
    await this.send(cmd.setFont(font));
    return this;
  }

  async setTextMode(modes: number): Promise<this> {
    await this.send(cmd.setTextMode(modes));
    return this;
  }

  async setEmphasis(enabled: boolean): Promise<this> {
    await this.send(cmd.setEmphasis(enabled));
    return this;
  }

  async bold(enabled: boolean = true): Promise<this> {
    await this.send(cmd.setEmphasis(enabled));
    return this;
  }

  async setDoubleStrike(enabled: boolean): Promise<this> {
    await this.send(cmd.setDoubleStrike(enabled));
    return this;
  }

  async setUnderline(mode: number): Promise<this> {
    await this.send(cmd.setUnderline(mode));
    return this;
  }

  async underline(enabled: boolean = true): Promise<this> {
    await this.send(cmd.setUnderline(enabled ? 1 : 0));
    return this;
  }

  async setLineSpacingDefault(): Promise<this> {
    await this.send(cmd.setLineSpacingDefault());
    return this;
  }

  async setLineSpacing(spacing: number): Promise<this> {
    await this.send(cmd.setLineSpacing(spacing));
    return this;
  }

  async setMotionUnits(horizontal: number, vertical: number): Promise<this> {
    await this.send(cmd.setMotionUnits(horizontal, vertical));
    return this;
  }

  async setCharacterSpacing(spacing: number): Promise<this> {
    await this.send(cmd.setCharacterSpacing(spacing));
    return this;
  }

  async setLeftMargin(margin: number): Promise<this> {
    await this.send(cmd.setLeftMargin(margin));
    return this;
  }

  async setPrintAreaWidth(width: number): Promise<this> {
    await this.send(cmd.setPrintAreaWidth(width));
    return this;
  }

  // --- Rotation / inversion ---

  async setRotate90(enabled: boolean): Promise<this> {
    await this.send(cmd.setRotate90(enabled));
    return this;
  }

  async setUpsideDown(enabled: boolean): Promise<this> {
    await this.send(cmd.setUpsideDown(enabled));
    return this;
  }

  // --- Advanced text ---

  async setInvertedText(enabled: boolean): Promise<this> {
    await this.send(cmd.setInvertedText(enabled));
    return this;
  }

  async invert(enabled: boolean = true): Promise<this> {
    await this.send(cmd.setInvertedText(enabled));
    return this;
  }

  async setTextScale(widthScale: number, heightScale: number): Promise<this> {
    await this.send(cmd.setTextScale(widthScale, heightScale));
    return this;
  }

  async cancelAllFormatting(): Promise<this> {
    await this.send(cmd.setEmphasis(false));
    await this.send(cmd.setDoubleStrike(false));
    await this.send(cmd.setUnderline(0));
    await this.send(cmd.setInvertedText(false));
    await this.send(cmd.setTextMode(C.MODE_NORMAL));
    await this.send(cmd.setTextScale(1, 1));
    await this.send(cmd.setJustification(C.JUSTIFY_LEFT));
    await this.send(cmd.setFont(C.FONT_A));
    await this.send(cmd.setCharacterSpacing(0));
    await this.send(cmd.setLineSpacingDefault());
    await this.send(cmd.setRotate90(false));
    await this.send(cmd.setUpsideDown(false));
    return this;
  }

  // --- Print direction / position ---

  async setPrintDirection(direction: number): Promise<this> {
    await this.send(cmd.setPrintDirection(direction));
    return this;
  }

  async setRelativeHorizontalPosition(position: number): Promise<this> {
    await this.send(cmd.setRelativeHorizontalPosition(position));
    return this;
  }

  async setAbsoluteHorizontalPosition(position: number): Promise<this> {
    await this.send(cmd.setAbsoluteHorizontalPosition(position));
    return this;
  }

  // --- Barcode ---

  async setBarcodeHeight(height: number): Promise<this> {
    await this.send(cmd.setBarcodeHeight(height));
    return this;
  }

  async setBarcodeWidth(width: number): Promise<this> {
    await this.send(cmd.setBarcodeWidth(width));
    return this;
  }

  async setHriPosition(position: number): Promise<this> {
    await this.send(cmd.setHriPosition(position));
    return this;
  }

  async setHriFont(font: number): Promise<this> {
    await this.send(cmd.setHriFont(font));
    return this;
  }

  async printBarcode(type: number, data: string): Promise<this> {
    await this.send(cmd.printBarcode(type, data));
    return this;
  }

  // --- 2D barcodes ---

  async printQr(
    data: string,
    moduleSize: number = 6,
    ecLevel: number = C.QR_EC_M
  ): Promise<this> {
    await this.send(cmd.printQr(data, moduleSize, ecLevel));
    return this;
  }

  async printPdf417(data: string, columns: number = 0, ecLevel: number = 1): Promise<this> {
    await this.send(cmd.printPdf417(data, columns, ecLevel));
    return this;
  }

  // --- Image ---

  async printRasterImage(
    mode: number,
    bitmap: Uint8Array,
    width: number,
    height: number
  ): Promise<this> {
    await this.send(cmd.printRasterImage(mode, bitmap, width, height));
    return this;
  }

  async printBitImage(mode: number, widthDots: number, data: Uint8Array): Promise<this> {
    await this.send(cmd.printBitImage(mode, widthDots, data));
    return this;
  }

  async defineDownloadedBitImage(x: number, y: number, data: Uint8Array): Promise<this> {
    await this.send(cmd.defineDownloadedBitImage(x, y, data));
    return this;
  }

  async printDownloadedBitImage(mode: number = 0): Promise<this> {
    await this.send(cmd.printDownloadedBitImage(mode));
    return this;
  }

  async printNvBitImage(n: number, mode: number = 0): Promise<this> {
    await this.send(cmd.printNvBitImage(n, mode));
    return this;
  }

  async defineNvBitImage(numImages: number, imageData: Uint8Array): Promise<this> {
    await this.send(cmd.defineNvBitImage(numImages, imageData));
    return this;
  }

  // --- Page mode ---

  async selectPageMode(): Promise<this> {
    await this.send(cmd.selectPageMode());
    return this;
  }

  async selectStandardMode(): Promise<this> {
    await this.send(cmd.selectStandardMode());
    return this;
  }

  async printPageMode(): Promise<this> {
    await this.send(cmd.printPageMode());
    return this;
  }

  async formFeed(): Promise<this> {
    await this.send(cmd.formFeed());
    return this;
  }

  async cancelPageData(): Promise<this> {
    await this.send(cmd.cancelPageData());
    return this;
  }

  async setPageArea(x: number, y: number, dx: number, dy: number): Promise<this> {
    await this.send(cmd.setPageArea(x, y, dx, dy));
    return this;
  }

  async setAbsoluteVerticalPosition(position: number): Promise<this> {
    await this.send(cmd.setAbsoluteVerticalPosition(position));
    return this;
  }

  async setRelativeVerticalPosition(position: number): Promise<this> {
    await this.send(cmd.setRelativeVerticalPosition(position));
    return this;
  }

  // --- Misc text / user-defined characters ---

  async printAndFeedUnits(units: number): Promise<this> {
    await this.send(cmd.printAndFeedUnits(units));
    return this;
  }

  async setPeripheralDevice(n: number): Promise<this> {
    await this.send(cmd.setPeripheralDevice(n));
    return this;
  }

  async selectUserDefinedCharset(enabled: boolean): Promise<this> {
    await this.send(cmd.selectUserDefinedCharset(enabled));
    return this;
  }

  async defineUserDefinedChars(
    y: number,
    c1: number,
    c2: number,
    data: Uint8Array
  ): Promise<this> {
    await this.send(cmd.defineUserDefinedChars(y, c1, c2, data));
    return this;
  }

  async cancelUserDefinedChar(code: number): Promise<this> {
    await this.send(cmd.cancelUserDefinedChar(code));
    return this;
  }

  // --- Status & real-time ---

  async realtimeRequest(n: number): Promise<this> {
    await this.send(cmd.realtimeRequest(n));
    return this;
  }

  async realtimeDrawerPulse(pin: number = 0, onTime: number = 1): Promise<this> {
    await this.send(cmd.realtimeDrawerPulse(pin, onTime));
    return this;
  }

  /** GS r n - transmit status; returns the status byte, or null on timeout. */
  async transmitStatus(n: number): Promise<number | null> {
    await this.send(cmd.transmitStatus(n));
    await new Promise((r) => setTimeout(r, 100));
    try {
      const response = await this.transport.read(1);
      if (response.length > 0) return response[0];
    } catch {
      // fall through
    }
    return null;
  }

  async setAsb(n: number): Promise<this> {
    await this.send(cmd.setAsb(n));
    return this;
  }

  async setPaperEndSensors(n: number): Promise<this> {
    await this.send(cmd.setPaperEndSensors(n));
    return this;
  }

  async setStopPrintSensors(n: number): Promise<this> {
    await this.send(cmd.setStopPrintSensors(n));
    return this;
  }

  async executeTestPrint(n: number = 0, m: number = 1): Promise<this> {
    await this.send(cmd.executeTestPrint(n, m));
    return this;
  }

  // --- Mechanism / sound / macros ---

  async setPanelButtons(enabled: boolean): Promise<this> {
    await this.send(cmd.setPanelButtons(enabled));
    return this;
  }

  async buzzer(count: number = 1, duration: number = 2): Promise<this> {
    await this.send(cmd.buzzer(count, duration));
    return this;
  }

  async buzzerAlarm(count: number = 1, interval: number = 2, mode: number = 3): Promise<this> {
    await this.send(cmd.buzzerAlarm(count, interval, mode));
    return this;
  }

  async macroDefineToggle(): Promise<this> {
    await this.send(cmd.macroDefineToggle());
    return this;
  }

  async executeMacro(times: number = 1, wait: number = 0, mode: number = 0): Promise<this> {
    await this.send(cmd.executeMacro(times, wait, mode));
    return this;
  }

  // --- Kanji ---

  async setKanjiMode(modes: number): Promise<this> {
    await this.send(cmd.setKanjiMode(modes));
    return this;
  }

  async selectKanji(): Promise<this> {
    await this.send(cmd.selectKanji());
    return this;
  }

  async cancelKanji(): Promise<this> {
    await this.send(cmd.cancelKanji());
    return this;
  }

  async setKanjiSpacing(left: number, right: number): Promise<this> {
    await this.send(cmd.setKanjiSpacing(left, right));
    return this;
  }

  async setKanjiQuadSize(enabled: boolean): Promise<this> {
    await this.send(cmd.setKanjiQuadSize(enabled));
    return this;
  }

  // --- Network / WiFi (vendor; send over USB/serial, then power-cycle) ---

  async setWifi(
    ssid: string,
    password: string,
    keyType: number = C.WIFI_WPA_WPA2_MIXED
  ): Promise<this> {
    await this.send(cmd.setWifi(ssid, password, keyType));
    return this;
  }

  async setWifiStatic(
    ssid: string,
    password: string,
    keyType: number,
    ip: number[],
    mask: number[],
    gateway: number[]
  ): Promise<this> {
    await this.send(cmd.setWifiStatic(ssid, password, keyType, ip, mask, gateway));
    return this;
  }

  async setDhcp(enabled: boolean): Promise<this> {
    await this.send(cmd.setDhcp(enabled));
    return this;
  }
}
