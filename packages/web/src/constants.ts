// ESC/POS control characters
export const ESC = 0x1b;
export const FS = 0x1c;
export const GS = 0x1d;

// Justification
export const JUSTIFY_LEFT = 0;
export const JUSTIFY_CENTER = 1;
export const JUSTIFY_RIGHT = 2;

// Fonts
export const FONT_A = 0;
export const FONT_B = 1;

// Print modes (bitmask for ESC ! n)
export const MODE_NORMAL = 0x00;
export const MODE_EMPHASIZED = 0x08;
export const MODE_DOUBLE_HEIGHT = 0x10;
export const MODE_DOUBLE_WIDTH = 0x20;
export const MODE_UNDERLINE = 0x80;

// Cut modes
export const CUT_ONE_POINT_UNCUT = 0;
export const CUT_PARTIAL = 1;

// Drawer pins
export const DRAWER_PIN_2 = 0;
export const DRAWER_PIN_5 = 1;

// Barcode types
export const BARCODE_UPC_A = 0;
export const BARCODE_UPC_E = 1;
export const BARCODE_JAN13 = 2;
export const BARCODE_JAN8 = 3;
export const BARCODE_CODE39 = 4;
export const BARCODE_ITF = 5;
export const BARCODE_CODEBAR = 6;
export const BARCODE_CODE93 = 72;
export const BARCODE_CODE128 = 73;
export const BARCODE_GS1_128 = 74;
export const BARCODE_GS1_DATABAR_OMNI = 75;
export const BARCODE_GS1_DATABAR_TRUNCATED = 76;
export const BARCODE_GS1_DATABAR_LIMITED = 77;
export const BARCODE_GS1_DATABAR_EXPANDED = 78;

// HRI position
export const HRI_NONE = 0;
export const HRI_ABOVE = 1;
export const HRI_BELOW = 2;
export const HRI_BOTH = 3;

// HRI font
export const HRI_FONT_STANDARD = 0;
export const HRI_FONT_COMPRESSED = 1;

// WiFi key types (vendor 1F 1B 1F B3 dropdown index)
export const WIFI_WEP64 = 0;
export const WIFI_WEP128 = 1;
export const WIFI_WPA_AES_PSK = 2;
export const WIFI_WPA_TKIP_PSK = 3;
export const WIFI_WPA_TKIP_AES_PSK = 4;
export const WIFI_WPA2_AES_PSK = 5;
export const WIFI_WPA2_TKIP = 6;
export const WIFI_WPA2_TKIP_AES_PSK = 7;
export const WIFI_WPA_WPA2_MIXED = 8;

// QR error-correction levels (GS ( k function 069)
export const QR_EC_L = 48;
export const QR_EC_M = 49;
export const QR_EC_Q = 50;
export const QR_EC_H = 51;

// Image modes
export const IMAGE_NORMAL = 0;
export const IMAGE_DOUBLE_WIDTH = 1;
export const IMAGE_DOUBLE_HEIGHT = 2;
export const IMAGE_QUADRUPLE = 3;

// Optimal feed lines for cutting
export const OPTIMAL_FEED_LINES = 7;

// --- Alternative ('ALT') command argument values (ASCII '0','1',...) ---
// Some firmware accepts these in place of the numeric 0/1/2/3 forms.
export const JUSTIFY_LEFT_ALT = 48;
export const JUSTIFY_CENTER_ALT = 49;
export const JUSTIFY_RIGHT_ALT = 50;

export const FONT_A_ALT = 48;
export const FONT_B_ALT = 49;

export const CUT_ONE_POINT_UNCUT_ALT = 48;
export const CUT_PARTIAL_ALT = 49;

export const DRAWER_PIN_2_ALT = 48;
export const DRAWER_PIN_5_ALT = 49;

export const HRI_NONE_ALT = 48;
export const HRI_ABOVE_ALT = 49;
export const HRI_BELOW_ALT = 50;
export const HRI_BOTH_ALT = 51;

export const HRI_FONT_STANDARD_ALT = 48;
export const HRI_FONT_COMPRESSED_ALT = 49;

export const IMAGE_NORMAL_ALT = 48;
export const IMAGE_DOUBLE_WIDTH_ALT = 49;
export const IMAGE_DOUBLE_HEIGHT_ALT = 50;
export const IMAGE_QUADRUPLE_ALT = 51;

// --- International character sets (ESC R n) ---
export const INTL_USA = 0;
export const INTL_FRANCE = 1;
export const INTL_GERMANY = 2;
export const INTL_UK = 3;
export const INTL_DENMARK_I = 4;
export const INTL_SWEDEN = 5;
export const INTL_ITALY = 6;
export const INTL_SPAIN_I = 7;
export const INTL_JAPAN = 8;
export const INTL_NORWAY = 9;
export const INTL_DENMARK_II = 10;
export const INTL_SPAIN_II = 11;
export const INTL_LATIN_AMERICA = 12;
export const INTL_KOREA = 13;
export const INTL_SLOVENIA_CROATIA = 14;
export const INTL_CHINA = 15;

// --- Code pages (ESC t n) — mirrors munbyn_codepage_t ---
export const CODEPAGE_PC437 = 0;
export const CODEPAGE_KATAKANA = 1;
export const CODEPAGE_PC850 = 2;
export const CODEPAGE_PC860 = 3;
export const CODEPAGE_PC863 = 4;
export const CODEPAGE_PC865 = 5;
export const CODEPAGE_WEST_EUROPE = 6;
export const CODEPAGE_GREEK = 7;
export const CODEPAGE_HEBREW = 8;
export const CODEPAGE_EAST_EUROPE = 9;
export const CODEPAGE_IRAN = 10;
export const CODEPAGE_WCP1252 = 11;
export const CODEPAGE_PC866 = 12;
export const CODEPAGE_PC852 = 13;
export const CODEPAGE_PC858 = 14;
export const CODEPAGE_IRAN_II = 15;
export const CODEPAGE_LATVIAN = 16;
export const CODEPAGE_ARABIC = 17;
export const CODEPAGE_PT151125 = 18;
export const CODEPAGE_PC747 = 19;
export const CODEPAGE_WPC1257 = 20;
export const CODEPAGE_THAI = 21;
export const CODEPAGE_VIETNAM = 22;
export const CODEPAGE_PC864 = 23;
export const CODEPAGE_PC1001 = 24;
export const CODEPAGE_UIGUR = 25;
export const CODEPAGE_HEBREW_ALT = 26;
export const CODEPAGE_WPC1255 = 27;
export const CODEPAGE_PC437_ALT = 28;
export const CODEPAGE_KATAKANA_ALT = 29;
export const CODEPAGE_PC437_ALT2 = 30;
export const CODEPAGE_PC866_MULT = 31;
export const CODEPAGE_PC852_LATIN2 = 32;
export const CODEPAGE_PC866_PORT = 33;
export const CODEPAGE_PC865_TEST = 34;
export const CODEPAGE_PC863_CAN = 35;
export const CODEPAGE_PC865_NORDIC = 36;
export const CODEPAGE_PC866_RUSSIAN = 37;
export const CODEPAGE_PC855_BULG = 38;
export const CODEPAGE_PC857_TURKEY = 39;
export const CODEPAGE_PC862_HEBREW = 40;
export const CODEPAGE_PC864_ARABIC = 41;
export const CODEPAGE_PC737_GREEK = 42;
export const CODEPAGE_PC851_GREEK = 43;
export const CODEPAGE_PC869_GREEK = 44;
export const CODEPAGE_PC928_GREEK = 45;
export const CODEPAGE_PC772_LITH = 46;
export const CODEPAGE_PC774_LITH = 47;
export const CODEPAGE_PC874_THAI = 48;
export const CODEPAGE_WPC1252_LATIN1 = 49;
export const CODEPAGE_WPC1250_LATIN2 = 50;
export const CODEPAGE_WPC1251_CYR = 51;
export const CODEPAGE_PC3840_IBM_RUS = 52;
export const CODEPAGE_PC3841_GOST = 53;
export const CODEPAGE_PC3843_POLISH = 54;
export const CODEPAGE_PC3844_CS2 = 55;
export const CODEPAGE_PC3845_HUNG = 56;
export const CODEPAGE_PC3846_TURK = 57;
export const CODEPAGE_PC3847_BR_ABNT = 58;
export const CODEPAGE_PC3848_BRAZIL = 59;
export const CODEPAGE_PC1001_ARABIC = 60;
export const CODEPAGE_PC2001_LITH = 61;
export const CODEPAGE_PC3001_EST1 = 62;
export const CODEPAGE_PC3002_EST2 = 63;
export const CODEPAGE_PC3011_LAT1 = 64;
export const CODEPAGE_PC3012_LAT2 = 65;
export const CODEPAGE_PC3021_BULG = 66;
export const CODEPAGE_PC3041_MALTESE = 67;

// Length-prefixed barcode selectors from manual 1.00.
export const BARCODE_UPC_A_B = 65;
export const BARCODE_UPC_E_B = 66;
export const BARCODE_JAN13_B = 67;
export const BARCODE_JAN8_B = 68;
export const BARCODE_CODE39_B = 69;
export const BARCODE_ITF_B = 70;
export const BARCODE_CODABAR_B = 71;
