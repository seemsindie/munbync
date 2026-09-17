export const printerProfiles = Object.freeze({
  'itpp047-tested': Object.freeze({ nativeQr: true, nativePdf417: false, raster: true }),
  generic: Object.freeze({ nativeQr: null, nativePdf417: null, raster: null }),
});
export type PrinterProfile = keyof typeof printerProfiles;
export interface PrinterOptions { profile?: PrinterProfile }
