const path = require('path');
const native = require(path.join(__dirname, '..', 'build', 'Release', 'munbync.node'));

const { MunbynPrinter: NativePrinter, constants } = native;
const { encodings, codepageProfiles } = require('./encoding');
const { encodePdf417 } = require('./pdf417');
const printerProfiles = Object.freeze({
  'itpp047-tested': Object.freeze({ nativeQr: true, nativePdf417: false, raster: true }),
  generic: Object.freeze({ nativeQr: null, nativePdf417: null, raster: null }),
});

class MunbynPrinter {
  constructor(options = {}) {
    this._printer = new NativePrinter();
    this.setProfile(options.profile ?? 'itpp047-tested');
  }

  setProfile(profile) {
    if (!Object.prototype.hasOwnProperty.call(printerProfiles, profile)) throw new RangeError('Unknown printer profile');
    if (this.isOpen) this._printer.setProfile(profile === 'generic' ? 0 : 1);
    this._profile = profile;
    return this;
  }

  get profile() { return this._profile; }
  get capabilities() { return printerProfiles[this._profile]; }

  // --- Connection ---

  openUsb(devicePath) {
    this._printer.openUsb(devicePath);
    this.setProfile(this._profile);
    return this;
  }

  openSerial(portName, baudRate = 9600) {
    this._printer.openSerial(portName, baudRate);
    this.setProfile(this._profile);
    return this;
  }

  openNetwork(ip, port = 9100, timeoutMs = 5000) {
    this._printer.openNetwork(ip, port, timeoutMs);
    this.setProfile(this._profile);
    return this;
  }

  close() {
    this._printer.close();
    return this;
  }

  get isOpen() {
    return this._printer.isOpen;
  }

  // --- Core ---

  initialize() {
    this._printer.initialize();
    return this;
  }

  writeData(data) {
    this._printer.writeData(data);
    return this;
  }

  print(text) {
    this._printer.writeData(text);
    return this;
  }

  printEncoded(text, encoding, codepage) {
    if (!Object.prototype.hasOwnProperty.call(encodings, encoding)) throw new RangeError('Unsupported text encoding');
    this._printer.printEncoded(text, encodings[encoding], codepage);
    return this;
  }

  readData(length) {
    return this._printer.readData(length);
  }

  getStatus() {
    return this._printer.getStatus();
  }

  // --- Basic operations ---

  cutPaper(mode = constants.CUT_PARTIAL) {
    this._printer.cutPaper(mode);
    return this;
  }

  feedAndCut(feedAmount = 7) {
    this._printer.feedAndCut(feedAmount);
    return this;
  }

  feedLines(lines) {
    this._printer.feedLines(lines);
    return this;
  }

  printAndCut(text, cutMode = constants.CUT_PARTIAL) {
    this._printer.printAndCut(text, cutMode);
    return this;
  }

  openDrawer(pin, onTime, offTime) {
    this._printer.openDrawer(pin, onTime, offTime);
    return this;
  }

  openDrawerDefault(pin = constants.DRAWER_PIN_2) {
    this._printer.openDrawerDefault(pin);
    return this;
  }

  selfTest() {
    this._printer.selfTest();
    return this;
  }

  // --- Text control ---

  lineFeed() {
    this._printer.lineFeed();
    return this;
  }

  carriageReturn() {
    this._printer.carriageReturn();
    return this;
  }

  horizontalTab() {
    this._printer.horizontalTab();
    return this;
  }

  setHorizontalTabPositions(positions) {
    this._printer.setHorizontalTabPositions(positions);
    return this;
  }

  clearHorizontalTabPositions() {
    this._printer.clearHorizontalTabPositions();
    return this;
  }

  // --- Character set ---

  setInternationalCharset(charset) {
    this._printer.setInternationalCharset(charset);
    return this;
  }

  setCodepage(codepage) {
    this._printer.setCodepage(codepage);
    return this;
  }

  getCodepageName(codepage) {
    return this._printer.getCodepageName(codepage);
  }

  // --- Text formatting ---

  setJustification(justify) {
    this._printer.setJustification(justify);
    return this;
  }

  align(alignment) {
    const map = {
      left: constants.JUSTIFY_LEFT,
      center: constants.JUSTIFY_CENTER,
      right: constants.JUSTIFY_RIGHT,
    };
    const val = map[alignment];
    if (val === undefined) {
      throw new Error(`Invalid alignment "${alignment}" — use "left", "center", or "right"`);
    }
    this._printer.setJustification(val);
    return this;
  }

  setFont(font) {
    this._printer.setFont(font);
    return this;
  }

  setTextMode(modes) {
    this._printer.setTextMode(modes);
    return this;
  }

  setEmphasis(enabled) {
    this._printer.setEmphasis(enabled);
    return this;
  }

  bold(enabled = true) {
    this._printer.setEmphasis(enabled);
    return this;
  }

  setDoubleStrike(enabled) {
    this._printer.setDoubleStrike(enabled);
    return this;
  }

  setUnderline(mode) {
    this._printer.setUnderline(mode);
    return this;
  }

  underline(enabled = true) {
    this._printer.setUnderline(enabled ? 1 : 0);
    return this;
  }

  setUnderlineKanji(mode) {
    this._printer.setUnderlineKanji(mode);
    return this;
  }

  setLineSpacingDefault() {
    this._printer.setLineSpacingDefault();
    return this;
  }

  setLineSpacing(spacing) {
    this._printer.setLineSpacing(spacing);
    return this;
  }

  setMotionUnits(horizontal, vertical) {
    this._printer.setMotionUnits(horizontal, vertical);
    return this;
  }

  setCharacterSpacing(spacing) {
    this._printer.setCharacterSpacing(spacing);
    return this;
  }

  setLeftMargin(margin) {
    this._printer.setLeftMargin(margin);
    return this;
  }

  setPrintAreaWidth(width) {
    this._printer.setPrintAreaWidth(width);
    return this;
  }

  // --- Rotation / inversion ---

  setRotate90(enabled) {
    this._printer.setRotate90(enabled);
    return this;
  }

  setUpsideDown(enabled) {
    this._printer.setUpsideDown(enabled);
    return this;
  }

  // --- Advanced text ---

  setInvertedText(enabled) {
    this._printer.setInvertedText(enabled);
    return this;
  }

  invert(enabled = true) {
    this._printer.setInvertedText(enabled);
    return this;
  }

  setTextScale(widthScale, heightScale) {
    this._printer.setTextScale(widthScale, heightScale);
    return this;
  }

  cancelAllFormatting() {
    this._printer.cancelAllFormatting();
    return this;
  }

  // --- Print direction / position ---

  setPrintDirection(direction) {
    this._printer.setPrintDirection(direction);
    return this;
  }

  setRelativeHorizontalPosition(position) {
    this._printer.setRelativeHorizontalPosition(position);
    return this;
  }

  setAbsoluteHorizontalPosition(position) {
    this._printer.setAbsoluteHorizontalPosition(position);
    return this;
  }

  // --- Barcode ---

  setBarcodeHeight(height) {
    this._printer.setBarcodeHeight(height);
    return this;
  }

  setBarcodeWidth(width) {
    this._printer.setBarcodeWidth(width);
    return this;
  }

  setHriPosition(position) {
    this._printer.setHriPosition(position);
    return this;
  }

  setHriFont(font) {
    this._printer.setHriFont(font);
    return this;
  }

  printBarcode(type, data) {
    this._printer.printBarcode(type, data);
    return this;
  }

  // --- 2D barcodes ---

  printQr(data, moduleSize = 6, ecLevel = constants.QR_EC_M) {
    this._printer.printQr(data, moduleSize, ecLevel);
    return this;
  }

  printPdf417(data, columns = 0, ecLevel = 1) {
    return this.printPdf417Raster(data, { columns, ecLevel });
  }

  printPdf417Raster(data, options = {}) {
    const { bitmap, width, height } = encodePdf417(data, options);
    return this.printRasterImage(0, Buffer.from(bitmap), width, height);
  }

  printPdf417Native(data, columns = 0, ecLevel = 1) {
    this._printer.printPdf417(data, columns, ecLevel);
    return this;
  }

  // --- Image ---

  printRasterImage(mode, bitmap, width, height) {
    this._printer.printRasterImage(mode, bitmap, width, height);
    return this;
  }

  printBitImage(mode, widthDots, data) {
    this._printer.printBitImage(mode, widthDots, data);
    return this;
  }

  defineDownloadedBitImage(x, y, data) {
    this._printer.defineDownloadedBitImage(x, y, data);
    return this;
  }

  printDownloadedBitImage(mode = 0) {
    this._printer.printDownloadedBitImage(mode);
    return this;
  }

  printNvBitImage(n, mode = 0) {
    this._printer.printNvBitImage(n, mode);
    return this;
  }

  defineNvBitImage(numImages, imageData) {
    this._printer.defineNvBitImage(numImages, imageData);
    return this;
  }

  // --- Page mode ---

  selectPageMode() {
    this._printer.selectPageMode();
    return this;
  }

  selectStandardMode() {
    this._printer.selectStandardMode();
    return this;
  }

  printPageMode() {
    this._printer.printPageMode();
    return this;
  }

  formFeed() {
    this._printer.formFeed();
    return this;
  }

  cancelPageData() {
    this._printer.cancelPageData();
    return this;
  }

  setPageArea(x, y, dx, dy) {
    this._printer.setPageArea(x, y, dx, dy);
    return this;
  }

  setAbsoluteVerticalPosition(position) {
    this._printer.setAbsoluteVerticalPosition(position);
    return this;
  }

  setRelativeVerticalPosition(position) {
    this._printer.setRelativeVerticalPosition(position);
    return this;
  }

  // --- Misc text / user-defined characters ---

  printAndFeedUnits(units) {
    this._printer.printAndFeedUnits(units);
    return this;
  }

  setPeripheralDevice(n) {
    this._printer.setPeripheralDevice(n);
    return this;
  }

  selectUserDefinedCharset(enabled) {
    this._printer.selectUserDefinedCharset(enabled);
    return this;
  }

  defineUserDefinedChars(y, c1, c2, data) {
    this._printer.defineUserDefinedChars(y, c1, c2, data);
    return this;
  }

  cancelUserDefinedChar(code) {
    this._printer.cancelUserDefinedChar(code);
    return this;
  }

  // --- Status & real-time ---

  realtimeRequest(n) {
    this._printer.realtimeRequest(n);
    return this;
  }

  realtimeDrawerPulse(pin = 0, onTime = 1) {
    this._printer.realtimeDrawerPulse(pin, onTime);
    return this;
  }

  transmitStatus(n) {
    return this._printer.transmitStatus(n);
  }

  setAsb(n) {
    this._printer.setAsb(n);
    return this;
  }

  setPaperEndSensors(n) {
    this._printer.setPaperEndSensors(n);
    return this;
  }

  setStopPrintSensors(n) {
    this._printer.setStopPrintSensors(n);
    return this;
  }

  executeTestPrint(n = 0, m = 1) {
    this._printer.executeTestPrint(n, m);
    return this;
  }

  // --- Mechanism / sound / macros ---

  setPanelButtons(enabled) {
    this._printer.setPanelButtons(enabled);
    return this;
  }

  buzzer(count = 1, duration = 2) {
    this._printer.buzzer(count, duration);
    return this;
  }

  buzzerAlarm(count = 1, interval = 2, mode = 3) {
    this._printer.buzzerAlarm(count, interval, mode);
    return this;
  }

  macroDefineToggle() {
    this._printer.macroDefineToggle();
    return this;
  }

  executeMacro(times = 1, wait = 0, mode = 0) {
    this._printer.executeMacro(times, wait, mode);
    return this;
  }

  // --- Kanji ---

  setKanjiMode(modes) {
    this._printer.setKanjiMode(modes);
    return this;
  }

  selectKanji() {
    this._printer.selectKanji();
    return this;
  }

  cancelKanji() {
    this._printer.cancelKanji();
    return this;
  }

  setKanjiSpacing(left, right) {
    this._printer.setKanjiSpacing(left, right);
    return this;
  }

  setKanjiQuadSize(enabled) {
    this._printer.setKanjiQuadSize(enabled);
    return this;
  }

  defineKanjiChar(c1, c2, data) {
    this._printer.defineKanjiChar(c1, c2, data);
    return this;
  }

  // --- Network / WiFi (vendor) ---

  setWifi(ssid, password, keyType = constants.WIFI_WPA_WPA2_MIXED) {
    this._printer.setWifi(ssid, password, keyType);
    return this;
  }

  setWifiStatic(ssid, password, keyType, ip, mask, gateway) {
    this._printer.setWifiStatic(ssid, password, keyType, ip, mask, gateway);
    return this;
  }

  setDhcp(enabled) {
    this._printer.setDhcp(enabled);
    return this;
  }
}

module.exports = { MunbynPrinter, constants, codepageProfiles, printerProfiles, encodePdf417 };
