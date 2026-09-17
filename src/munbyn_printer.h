#ifndef MUNBYN_PRINTER_H
#define MUNBYN_PRINTER_H

#include <stdint.h>
#include <stdbool.h>
#include <stddef.h>

#ifdef __cplusplus
extern "C" {
#endif

// Error codes
typedef enum {
    MUNBYN_OK = 0,
    MUNBYN_ERROR_INVALID_HANDLE = -1,
    MUNBYN_ERROR_COMMUNICATION = -2,
    MUNBYN_ERROR_INVALID_PARAMETER = -3,
    MUNBYN_ERROR_BUFFER_OVERFLOW = -4,
    MUNBYN_ERROR_TIMEOUT = -5,
    MUNBYN_ERROR_NOT_INITIALIZED = -6
} munbyn_error_t;

// Print justification (ESC a n command)
typedef enum {
    MUNBYN_JUSTIFY_LEFT = 0,         // ESC a 0 - Left justification
    MUNBYN_JUSTIFY_CENTER = 1,       // ESC a 1 - Centering
    MUNBYN_JUSTIFY_RIGHT = 2,        // ESC a 2 - Right justification
    MUNBYN_JUSTIFY_LEFT_ALT = 48,    // ESC a 48 - Left justification (alternative)
    MUNBYN_JUSTIFY_CENTER_ALT = 49,  // ESC a 49 - Centering (alternative)
    MUNBYN_JUSTIFY_RIGHT_ALT = 50,   // ESC a 50 - Right justification (alternative)
} munbyn_justify_t;

// Font types (ESC M n command)
typedef enum {
    MUNBYN_FONT_A = 0,        // ESC M 0 - Character font A (12×24)
    MUNBYN_FONT_B = 1,        // ESC M 1 - Character font B (9×17)
    MUNBYN_FONT_A_ALT = 48,   // ESC M 48 - Character font A (12×24) alternative
    MUNBYN_FONT_B_ALT = 49,   // ESC M 49 - Character font B (9×17) alternative
} munbyn_font_t;

// Print modes
typedef enum {
    MUNBYN_MODE_NORMAL = 0x00,
    MUNBYN_MODE_EMPHASIZED = 0x08,
    MUNBYN_MODE_DOUBLE_HEIGHT = 0x10,
    MUNBYN_MODE_DOUBLE_WIDTH = 0x20,
    MUNBYN_MODE_UNDERLINE = 0x80
} munbyn_print_mode_t;

// Cut modes (based on GS V m command)
typedef enum {
    MUNBYN_CUT_ONE_POINT_UNCUT = 0,     // GS V 0 - one point left uncut
    MUNBYN_CUT_PARTIAL = 1,             // GS V 1 - partial cut
    MUNBYN_CUT_ONE_POINT_UNCUT_ALT = 48, // GS V 48 - one point left uncut (alternative)
    MUNBYN_CUT_PARTIAL_ALT = 49,        // GS V 49 - partial cut (alternative)
} munbyn_cut_mode_t;

// International character sets (ESC R n)
typedef enum {
    MUNBYN_INTL_USA = 0,
    MUNBYN_INTL_FRANCE = 1,
    MUNBYN_INTL_GERMANY = 2,
    MUNBYN_INTL_UK = 3,
    MUNBYN_INTL_DENMARK_I = 4,
    MUNBYN_INTL_SWEDEN = 5,
    MUNBYN_INTL_ITALY = 6,
    MUNBYN_INTL_SPAIN_I = 7,
    MUNBYN_INTL_JAPAN = 8,
    MUNBYN_INTL_NORWAY = 9,
    MUNBYN_INTL_DENMARK_II = 10,
    MUNBYN_INTL_SPAIN_II = 11,
    MUNBYN_INTL_LATIN_AMERICA = 12,
    MUNBYN_INTL_KOREA = 13,
    MUNBYN_INTL_SLOVENIA_CROATIA = 14,
    MUNBYN_INTL_CHINA = 15
} munbyn_international_charset_t;

// Code pages for character set selection (ESC t n)
typedef enum {
    MUNBYN_CODEPAGE_PC437 = 0,         // PC437(Std.Europe)
    MUNBYN_CODEPAGE_KATAKANA = 1,      // Katakana
    MUNBYN_CODEPAGE_PC850 = 2,         // PC850(Multilingual)
    MUNBYN_CODEPAGE_PC860 = 3,         // PC860(Portugal)
    MUNBYN_CODEPAGE_PC863 = 4,         // PC863(Canadian)
    MUNBYN_CODEPAGE_PC865 = 5,         // PC865(Nordic)
    MUNBYN_CODEPAGE_WEST_EUROPE = 6,   // West Europe
    MUNBYN_CODEPAGE_GREEK = 7,         // Greek
    MUNBYN_CODEPAGE_HEBREW = 8,        // Hebrew
    MUNBYN_CODEPAGE_EAST_EUROPE = 9,   // East Europe
    MUNBYN_CODEPAGE_IRAN = 10,         // Iran
    MUNBYN_CODEPAGE_WCP1252 = 11,      // WCP1252
    MUNBYN_CODEPAGE_PC866 = 12,        // PC866(Cyrillic#2)
    MUNBYN_CODEPAGE_PC852 = 13,        // PC852(Latin2)
    MUNBYN_CODEPAGE_PC858 = 14,        // PC858
    MUNBYN_CODEPAGE_IRAN_II = 15,      // IranII
    MUNBYN_CODEPAGE_LATVIAN = 16,      // Latvian
    MUNBYN_CODEPAGE_ARABIC = 17,       // Arabic
    MUNBYN_CODEPAGE_PT151125 = 18,     // PT151125
    MUNBYN_CODEPAGE_PC747 = 19,        // PC747
    MUNBYN_CODEPAGE_WPC1257 = 20,      // WPC1257
    MUNBYN_CODEPAGE_THAI = 21,         // Thai
    MUNBYN_CODEPAGE_VIETNAM = 22,      // Vietnam
    MUNBYN_CODEPAGE_PC864 = 23,        // PC864
    MUNBYN_CODEPAGE_PC1001 = 24,       // PC1001
    MUNBYN_CODEPAGE_UIGUR = 25,        // Uigur
    MUNBYN_CODEPAGE_HEBREW_ALT = 26,   // Hebrew
    MUNBYN_CODEPAGE_WPC1255 = 27,      // WPC1255(Israel)
    MUNBYN_CODEPAGE_PC437_ALT = 28,    // PC437(Std.Europe)
    MUNBYN_CODEPAGE_KATAKANA_ALT = 29, // Katakana
    MUNBYN_CODEPAGE_PC437_ALT2 = 30,   // PC437(Std.Europe)
    MUNBYN_CODEPAGE_PC866_MULT = 31,   // PC866(Mult)
    MUNBYN_CODEPAGE_PC852_LATIN2 = 32, // PC852(Latin-2)
    MUNBYN_CODEPAGE_PC866_PORT = 33,   // PC866(Portuguese)
    MUNBYN_CODEPAGE_PC865_TEST = 34,   // PC865(TestEscPos)
    MUNBYN_CODEPAGE_PC863_CAN = 35,    // PC863(Canadian)
    MUNBYN_CODEPAGE_PC865_NORDIC = 36, // PC865(Nordic)
    MUNBYN_CODEPAGE_PC866_RUSSIAN = 37, // PC866(Russian)
    MUNBYN_CODEPAGE_PC855_BULG = 38,   // PC855(Bulgarian)
    MUNBYN_CODEPAGE_PC857_TURKEY = 39, // PC857(Turkey)
    MUNBYN_CODEPAGE_PC862_HEBREW = 40, // PC862(Hebrew)
    MUNBYN_CODEPAGE_PC864_ARABIC = 41, // PC864(Arabic)
    MUNBYN_CODEPAGE_PC737_GREEK = 42,  // PC737(Greek)
    MUNBYN_CODEPAGE_PC851_GREEK = 43,  // PC851(Greek)
    MUNBYN_CODEPAGE_PC869_GREEK = 44,  // PC869(Greek)
    MUNBYN_CODEPAGE_PC928_GREEK = 45,  // PC928(Greek)
    MUNBYN_CODEPAGE_PC772_LITH = 46,   // PC772(Lithuanian)
    MUNBYN_CODEPAGE_PC774_LITH = 47,   // PC774(Lithuanian)
    MUNBYN_CODEPAGE_PC874_THAI = 48,   // PC874(Thai)
    MUNBYN_CODEPAGE_WPC1252_LATIN1 = 49, // WPC1252(Latin1)
    MUNBYN_CODEPAGE_WPC1250_LATIN2 = 50, // WPC1250(Latin-2)
    MUNBYN_CODEPAGE_WPC1251_CYR = 51,     // WPC1251(Cyrillic)
    MUNBYN_CODEPAGE_PC3840_IBM_RUS = 52,  // PC3840(IBM-Russian)
    MUNBYN_CODEPAGE_PC3841_GOST = 53,     // PC3841(Gost)
    MUNBYN_CODEPAGE_PC3843_POLISH = 54,   // PC3843(Polish)
    MUNBYN_CODEPAGE_PC3844_CS2 = 55,      // PC3844(CS2)
    MUNBYN_CODEPAGE_PC3845_HUNG = 56,     // PC3845(Hungarian)
    MUNBYN_CODEPAGE_PC3846_TURK = 57,     // PC3846(Turkish)
    MUNBYN_CODEPAGE_PC3847_BR_ABNT = 58,  // PC3847(Brazil-ABNT)
    MUNBYN_CODEPAGE_PC3848_BRAZIL = 59,   // PC3848(Brazil)
    MUNBYN_CODEPAGE_PC1001_ARABIC = 60,   // PC1001(Arabic)
    MUNBYN_CODEPAGE_PC2001_LITH = 61,     // PC2001(Lithuanian)
    MUNBYN_CODEPAGE_PC3001_EST1 = 62,     // PC3001(Estonian-1)
    MUNBYN_CODEPAGE_PC3002_EST2 = 63,     // PC3002(Eston-2)
    MUNBYN_CODEPAGE_PC3011_LAT1 = 64,     // PC3011(Latvian-1)
    MUNBYN_CODEPAGE_PC3012_LAT2 = 65,     // PC3012(Latv-2)
    MUNBYN_CODEPAGE_PC3021_BULG = 66,     // PC3021(Bulgarian)
    MUNBYN_CODEPAGE_PC3041_MALTESE = 67   // PC3041(Maltese)
} munbyn_codepage_t;

// Barcode types
typedef enum {
    MUNBYN_BARCODE_UPC_A = 0,
    MUNBYN_BARCODE_UPC_E = 1,
    MUNBYN_BARCODE_JAN13 = 2,
    MUNBYN_BARCODE_JAN8 = 3,
    MUNBYN_BARCODE_CODE39 = 4,
    MUNBYN_BARCODE_ITF = 5,
    MUNBYN_BARCODE_CODEBAR = 6,
    MUNBYN_BARCODE_CODE93 = 72,
    MUNBYN_BARCODE_CODE128 = 73,
    MUNBYN_BARCODE_GS1_128 = 74,
    MUNBYN_BARCODE_GS1_DATABAR_OMNI = 75,
    MUNBYN_BARCODE_GS1_DATABAR_TRUNCATED = 76,
    MUNBYN_BARCODE_GS1_DATABAR_LIMITED = 77,
    MUNBYN_BARCODE_GS1_DATABAR_EXPANDED = 78
} munbyn_barcode_t;

// HRI position for barcodes
typedef enum {
    MUNBYN_HRI_NONE = 0,               // GS H 0 - Do not print
    MUNBYN_HRI_ABOVE = 1,              // GS H 1 - Above the bar code
    MUNBYN_HRI_BELOW = 2,              // GS H 2 - Below the bar code
    MUNBYN_HRI_BOTH = 3,               // GS H 3 - Both above and below the bar code
    MUNBYN_HRI_NONE_ALT = 48,          // GS H 48 - Do not print (alternative)
    MUNBYN_HRI_ABOVE_ALT = 49,         // GS H 49 - Above the bar code (alternative)
    MUNBYN_HRI_BELOW_ALT = 50,         // GS H 50 - Below the bar code (alternative)
    MUNBYN_HRI_BOTH_ALT = 51           // GS H 51 - Both above and below (alternative)
} munbyn_hri_position_t;

// HRI font for barcodes (GS f n)
typedef enum {
    MUNBYN_HRI_FONT_STANDARD = 0,        // GS f 0/48 - Standard ASCII (12×24)
    MUNBYN_HRI_FONT_COMPRESSED = 1,      // GS f 1/49 - Compressed ASCII (9×17)
    MUNBYN_HRI_FONT_STANDARD_ALT = 48,   // GS f 48 - Standard ASCII (12×24)
    MUNBYN_HRI_FONT_COMPRESSED_ALT = 49  // GS f 49 - Compressed ASCII (9×17)
} munbyn_hri_font_t;

// Optimal cutting configuration constants (based on testing)
#define MUNBYN_OPTIMAL_FEED_LINES     7    // Provides best symmetrical cutting
#define MUNBYN_ACCEPTABLE_FEED_LINES  6    // Alternative if 7 lines too much spacing

// Image mode
typedef enum {
    MUNBYN_IMAGE_NORMAL = 0,
    MUNBYN_IMAGE_DOUBLE_WIDTH = 1,
    MUNBYN_IMAGE_DOUBLE_HEIGHT = 2,
    MUNBYN_IMAGE_QUADRUPLE = 3,
    // Alternative values supported by some firmware
    MUNBYN_IMAGE_NORMAL_ALT = 48,
    MUNBYN_IMAGE_DOUBLE_WIDTH_ALT = 49,
    MUNBYN_IMAGE_DOUBLE_HEIGHT_ALT = 50,
    MUNBYN_IMAGE_QUADRUPLE_ALT = 51
} munbyn_image_mode_t;

// Printer status bits
typedef struct {
    bool paper_present;
    bool cover_closed;
    bool online;
    bool error_occurred;
    bool cut_error;
    bool recoverable_error;
    bool unrecoverable_error;
} munbyn_status_t;

// Connection types
typedef enum {
    MUNBYN_CONNECTION_USB = 0,
    MUNBYN_CONNECTION_SERIAL = 1,
    MUNBYN_CONNECTION_NETWORK = 2,
    MUNBYN_CONNECTION_BLUETOOTH = 3
} munbyn_connection_t;

typedef struct {
    munbyn_connection_t type;
    union {
        struct {
            char device_path[256]; // e.g., "/dev/usb/lp0"
        } usb;
        struct {
            char port_name[256];
            int baud_rate;
        } serial;
        struct {
            char ip_address[256];
            int port;
            int timeout_ms;
        } network;
        struct {
            char address[32]; // MAC address
        } bluetooth;
    } config;
} munbyn_connection_params_t;

// Opaque printer handle
typedef struct munbyn_printer* munbyn_handle_t;

// Core API - handles multiple transport types
munbyn_error_t munbyn_open(const munbyn_connection_params_t* params, munbyn_handle_t* handle);
munbyn_error_t munbyn_close(munbyn_handle_t handle);

// Convenience functions for specific transports
munbyn_error_t munbyn_open_usb(const char* device_path, munbyn_handle_t* handle);
munbyn_error_t munbyn_open_serial(const char* port_name, int baud_rate, munbyn_handle_t* handle);
munbyn_error_t munbyn_open_network(const char* ip_address, int port, int timeout_ms, munbyn_handle_t* handle);

// Common operations
munbyn_error_t munbyn_write_data(munbyn_handle_t handle, const uint8_t* data, size_t length);
munbyn_error_t munbyn_read_data(munbyn_handle_t handle, uint8_t* buffer, size_t buffer_size, size_t* bytes_read);
// Requires all four valid one-byte replies; leave ASB disabled.
// On failure, status is zeroed. Reconnect after a timeout before retrying.
munbyn_error_t munbyn_get_status(munbyn_handle_t handle, munbyn_status_t* status);

// Drawer connector pins (ESC p m command)
typedef enum {
    MUNBYN_DRAWER_PIN_2 = 0,      // ESC p 0 - Drawer kick-out connector pin 2
    MUNBYN_DRAWER_PIN_5 = 1,      // ESC p 1 - Drawer kick-out connector pin 5
    MUNBYN_DRAWER_PIN_2_ALT = 48, // ESC p 48 - Drawer kick-out connector pin 2 (alternative)
    MUNBYN_DRAWER_PIN_5_ALT = 49, // ESC p 49 - Drawer kick-out connector pin 5 (alternative)
} munbyn_drawer_pin_t;

// Basic printer operations
munbyn_error_t munbyn_initialize(munbyn_handle_t handle);
munbyn_error_t munbyn_cut_paper(munbyn_handle_t handle, munbyn_cut_mode_t mode);
munbyn_error_t munbyn_feed_and_cut(munbyn_handle_t handle, uint8_t feed_amount);
munbyn_error_t munbyn_feed_lines(munbyn_handle_t handle, uint8_t lines);
munbyn_error_t munbyn_open_drawer(munbyn_handle_t handle, munbyn_drawer_pin_t pin, uint8_t on_time, uint8_t off_time);
munbyn_error_t munbyn_open_drawer_default(munbyn_handle_t handle, munbyn_drawer_pin_t pin);

// System/Diagnostic operations
munbyn_error_t munbyn_self_test(munbyn_handle_t handle);

// Convenience function for optimal receipt printing and cutting
munbyn_error_t munbyn_print_and_cut(munbyn_handle_t handle, const char* text, munbyn_cut_mode_t cut_mode);

// Text control commands
munbyn_error_t munbyn_line_feed(munbyn_handle_t handle);
munbyn_error_t munbyn_carriage_return(munbyn_handle_t handle);
munbyn_error_t munbyn_horizontal_tab(munbyn_handle_t handle);
munbyn_error_t munbyn_set_horizontal_tab_positions(munbyn_handle_t handle, const uint8_t* positions, size_t count);
munbyn_error_t munbyn_clear_horizontal_tab_positions(munbyn_handle_t handle);

// Character set and codepage commands
munbyn_error_t munbyn_set_international_charset(munbyn_handle_t handle, munbyn_international_charset_t charset);
munbyn_error_t munbyn_set_codepage(munbyn_handle_t handle, munbyn_codepage_t codepage);

// Helper function to get codepage name
const char* munbyn_get_codepage_name(munbyn_codepage_t codepage);

// Text formatting and alignment commands
munbyn_error_t munbyn_set_justification(munbyn_handle_t handle, munbyn_justify_t justify);
munbyn_error_t munbyn_set_font(munbyn_handle_t handle, munbyn_font_t font);
munbyn_error_t munbyn_set_text_mode(munbyn_handle_t handle, uint8_t modes);
munbyn_error_t munbyn_set_emphasis(munbyn_handle_t handle, bool enabled);
munbyn_error_t munbyn_set_double_strike(munbyn_handle_t handle, bool enabled);
munbyn_error_t munbyn_set_underline(munbyn_handle_t handle, uint8_t mode);
munbyn_error_t munbyn_set_underline_kanji(munbyn_handle_t handle, uint8_t mode);
munbyn_error_t munbyn_set_line_spacing_default(munbyn_handle_t handle);
munbyn_error_t munbyn_set_line_spacing(munbyn_handle_t handle, uint8_t spacing);
munbyn_error_t munbyn_set_motion_units(munbyn_handle_t handle, uint8_t horizontal, uint8_t vertical);
munbyn_error_t munbyn_set_character_spacing(munbyn_handle_t handle, uint8_t spacing);
munbyn_error_t munbyn_set_left_margin(munbyn_handle_t handle, uint16_t margin);
munbyn_error_t munbyn_set_print_area_width(munbyn_handle_t handle, uint16_t width);

// Text rotation and inversion
munbyn_error_t munbyn_set_rotate_90(munbyn_handle_t handle, bool enabled);
munbyn_error_t munbyn_set_upside_down(munbyn_handle_t handle, bool enabled);

// Advanced text effects
munbyn_error_t munbyn_set_inverted_text(munbyn_handle_t handle, bool enabled);
munbyn_error_t munbyn_set_text_scale(munbyn_handle_t handle, uint8_t width_scale, uint8_t height_scale);
munbyn_error_t munbyn_cancel_all_formatting(munbyn_handle_t handle);

// Print direction and orientation
munbyn_error_t munbyn_set_print_direction(munbyn_handle_t handle, uint8_t direction);
munbyn_error_t munbyn_set_relative_horizontal_position(munbyn_handle_t handle, int16_t position);
munbyn_error_t munbyn_set_absolute_horizontal_position(munbyn_handle_t handle, uint16_t position);

// Barcode operations
munbyn_error_t munbyn_set_barcode_height(munbyn_handle_t handle, uint8_t height);
munbyn_error_t munbyn_set_barcode_width(munbyn_handle_t handle, uint8_t width);
munbyn_error_t munbyn_set_hri_position(munbyn_handle_t handle, munbyn_hri_position_t position);
munbyn_error_t munbyn_set_hri_font(munbyn_handle_t handle, munbyn_hri_font_t font);
munbyn_error_t munbyn_print_barcode(munbyn_handle_t handle, munbyn_barcode_t type, const char* data);

// QR code error-correction levels (GS ( k function 069, fn='E')
typedef enum {
    MUNBYN_QR_EC_L = 48,   // '0' - Low      (~7% recovery)
    MUNBYN_QR_EC_M = 49,   // '1' - Medium   (~15% recovery)
    MUNBYN_QR_EC_Q = 50,   // '2' - Quartile (~25% recovery)
    MUNBYN_QR_EC_H = 51,   // '3' - High     (~30% recovery)
} munbyn_qr_ec_t;

// Firmware-dependent 2D extensions (GS ( k); absent from the bundled manual).
// String payloads cannot contain NUL. Physical symbol capacity also depends on
// encoding, error correction, module size and available print width.
// QR code: module_size 1-16 (dot size), ec_level selects recovery level.
munbyn_error_t munbyn_print_qr(munbyn_handle_t handle, const char* data,
                               uint8_t module_size, munbyn_qr_ec_t ec_level);
// Native PDF417 is unsupported on the tested ITPP047 firmware; use raster output.
// PDF417: data 1..65532 bytes; columns 0 = auto (else 1-30); ec_level 0-8.
munbyn_error_t munbyn_print_pdf417(munbyn_handle_t handle, const char* data,
                                   uint8_t columns, uint8_t ec_level);

// Raster bit image (GS v 0 m xL xH yL yH d1..dk)
// The bitmap must be 1-bit-per-pixel, packed MSB-first in each byte,
// laid out row by row from top to bottom. Each row is (width+7)/8 bytes.
// width and height are specified in dots (pixels).
munbyn_error_t munbyn_print_raster_image(
    munbyn_handle_t handle,
    munbyn_image_mode_t mode,
    const uint8_t* bitmap,
    uint16_t width_pixels,
    uint16_t height_pixels
);

// --- Other bit-image commands ---

// ESC * m nL nH d1..dk - Select bit-image mode (column format).
// mode: 0/1 = 8-dot single/double density (1 byte per column),
//       32/33 = 24-dot single/double density (3 bytes per column).
// width_dots is the number of image columns; data length must equal
// width_dots * (mode < 32 ? 1 : 3).
munbyn_error_t munbyn_print_bit_image(munbyn_handle_t handle, uint8_t mode,
                                      uint16_t width_dots, const uint8_t* data, size_t length);

// GS * x y d1..d(x*y*8) - Define the downloaded bit image.
// x = width in bytes (1-255), y = height in bytes (1-48); x*y <= 912.
// length must be x*y*8 (manual pp. 35-36).
munbyn_error_t munbyn_define_downloaded_bit_image(munbyn_handle_t handle, uint8_t x, uint8_t y,
                                                  const uint8_t* data, size_t length);

// GS / m - Print the downloaded bit image (m selects scaling 0-3 / 48-51).
munbyn_error_t munbyn_print_downloaded_bit_image(munbyn_handle_t handle, uint8_t mode);

// FS p n m - Print NV (non-volatile) bit image number n at scaling m.
munbyn_error_t munbyn_print_nv_bit_image(munbyn_handle_t handle, uint8_t n, uint8_t mode);

// FS q n [xL xH yL yH d1..dk]1..n - Define NV bit image(s).
// image_data must contain num_images consecutive [xL xH yL yH d...] blocks.
// Total blocks <=65536 bytes; dimensions x=1..1023, y=1..288 (8-dot units).
// NOTE: replaces all NV images and resets the printer; use sparingly.
munbyn_error_t munbyn_define_nv_bit_image(munbyn_handle_t handle, uint8_t num_images,
                                          const uint8_t* image_data, size_t length);

// --- Page mode ---
munbyn_error_t munbyn_select_page_mode(munbyn_handle_t handle);       // ESC L
munbyn_error_t munbyn_select_standard_mode(munbyn_handle_t handle);   // ESC S
munbyn_error_t munbyn_print_page_mode(munbyn_handle_t handle);        // ESC FF (print page buffer)
munbyn_error_t munbyn_form_feed(munbyn_handle_t handle);             // FF
munbyn_error_t munbyn_cancel_page_data(munbyn_handle_t handle);      // CAN
// ESC W xL xH yL yH dxL dxH dyL dyH - set print area in page mode.
munbyn_error_t munbyn_set_page_area(munbyn_handle_t handle, uint16_t x, uint16_t y,
                                    uint16_t dx, uint16_t dy);
// GS $ nL nH - absolute vertical print position in page mode.
munbyn_error_t munbyn_set_absolute_vertical_position(munbyn_handle_t handle, uint16_t position);
// GS \ nL nH - relative vertical print position in page mode.
munbyn_error_t munbyn_set_relative_vertical_position(munbyn_handle_t handle, int16_t position);

// --- Misc text / position / user-defined characters ---

// ESC J n - print buffer and feed n vertical motion units.
munbyn_error_t munbyn_print_and_feed_units(munbyn_handle_t handle, uint8_t units);
// ESC = n - select the peripheral device that receives data (bit0 = printer).
munbyn_error_t munbyn_set_peripheral_device(munbyn_handle_t handle, uint8_t n);
// ESC % n - select (n!=0) or cancel (n=0) the user-defined character set.
munbyn_error_t munbyn_select_user_defined_charset(munbyn_handle_t handle, bool enabled);
// ESC & y c1 c2 d... - define user-defined characters for codes c1..c2.
// y=3, codes 32..126; data contains exactly one width+bitmap block per code.
// Width <=12 for font A, <=9 for font B (manual p. 14).
munbyn_error_t munbyn_define_user_defined_chars(munbyn_handle_t handle, uint8_t y,
                                                uint8_t c1, uint8_t c2,
                                                const uint8_t* data, size_t length);
// ESC ? n - cancel the user-defined character for code n.
munbyn_error_t munbyn_cancel_user_defined_char(munbyn_handle_t handle, uint8_t code);

// --- Status & real-time ---

// DLE ENQ n - real-time request to the printer (n=1/2 recover, etc.).
munbyn_error_t munbyn_realtime_request(munbyn_handle_t handle, uint8_t n);
// DLE DC4 1 m t - drawer pulse: pin 0/1, on_time 1..8 in 100 ms units.
munbyn_error_t munbyn_realtime_drawer_pulse(munbyn_handle_t handle, uint8_t pin, uint8_t on_time);
// GS r n - transmit status (n=1/49 paper sensor, 2/50 drawer). Reads one byte.
munbyn_error_t munbyn_transmit_status(munbyn_handle_t handle, uint8_t n, uint8_t* out);
// GS a n - enable/disable Automatic Status Back (ASB) features bitmask.
munbyn_error_t munbyn_set_asb(munbyn_handle_t handle, uint8_t n);
// ESC c 3 n - select paper sensor(s) that output paper-end signals.
munbyn_error_t munbyn_set_paper_end_sensors(munbyn_handle_t handle, uint8_t n);
// ESC c 4 n - select paper sensor(s) that stop printing.
munbyn_error_t munbyn_set_stop_print_sensors(munbyn_handle_t handle, uint8_t n);
// GS ( A - enter hex-dump mode: n=0/48, m=1/49 (manual p. 37).
// Historical API name retained; this is distinct from munbyn_self_test().
munbyn_error_t munbyn_execute_test_print(munbyn_handle_t handle, uint8_t n, uint8_t m);

// --- Mechanism, sound, macros ---

// ESC c 5 n - enable (true) or disable (false) the panel buttons (FEED).
munbyn_error_t munbyn_set_panel_buttons(munbyn_handle_t handle, bool enabled);
// ESC B n t - sound the buzzer n times, t x 50ms each (MUNBYN-specific).
munbyn_error_t munbyn_buzzer(munbyn_handle_t handle, uint8_t count, uint8_t duration);
// ESC C m t n - beeper + alarm light: m beeps, t interval, n mode
// (0=none, 1=buzzer, 2=light, 3=both) (MUNBYN-specific).
munbyn_error_t munbyn_buzzer_alarm(munbyn_handle_t handle, uint8_t count, uint8_t interval, uint8_t mode);
// GS : - start/end macro definition (toggles).
munbyn_error_t munbyn_macro_define_toggle(munbyn_handle_t handle);
// GS ^ r t m - execute the macro r times, t wait, m mode.
munbyn_error_t munbyn_execute_macro(munbyn_handle_t handle, uint8_t times, uint8_t wait, uint8_t mode);

// --- Kanji ---
// FS ! n - set Kanji print mode(s) (bitmask).
munbyn_error_t munbyn_set_kanji_mode(munbyn_handle_t handle, uint8_t modes);
// FS & - select (enter) Kanji character mode.
munbyn_error_t munbyn_select_kanji(munbyn_handle_t handle);
// FS . - cancel (exit) Kanji character mode.
munbyn_error_t munbyn_cancel_kanji(munbyn_handle_t handle);
// FS S n1 n2 - set left/right Kanji character spacing.
munbyn_error_t munbyn_set_kanji_spacing(munbyn_handle_t handle, uint8_t left, uint8_t right);
// FS W n - turn quadruple-size Kanji mode on/off.
munbyn_error_t munbyn_set_kanji_quad_size(munbyn_handle_t handle, bool enabled);

// --- Network / WiFi (vendor 1F 1B 1F commands) ---
// NOTE: reverse-engineered from the official PrinterTest tool; not verified on
// all firmware. Send these over USB — running them over the network link will
// drop the connection. After setWifi, power-cycle the printer.

// WiFi encryption / key types (index into the tool's "Key Type" dropdown).
typedef enum {
    MUNBYN_WIFI_WEP64 = 0,
    MUNBYN_WIFI_WEP128 = 1,
    MUNBYN_WIFI_WPA_AES_PSK = 2,
    MUNBYN_WIFI_WPA_TKIP_PSK = 3,
    MUNBYN_WIFI_WPA_TKIP_AES_PSK = 4,
    MUNBYN_WIFI_WPA2_AES_PSK = 5,
    MUNBYN_WIFI_WPA2_TKIP = 6,
    MUNBYN_WIFI_WPA2_TKIP_AES_PSK = 7,
    MUNBYN_WIFI_WPA_WPA2_MIXED = 8,
} munbyn_wifi_keytype_t;

// 1F 1B 1F B3 <keyt> <ssid> 00 <password> 00 - set WiFi SSID + password
// (DHCP case: the printer keeps its existing IP mode). Follow with setDhcp(true)
// + a power-cycle to associate.
munbyn_error_t munbyn_set_wifi(munbyn_handle_t handle, const char* ssid,
                               const char* password, munbyn_wifi_keytype_t key_type);

// 1F 1B 1F B4 <ip[4]> <mask[4]> <gateway[4]> <keyt> <ssid> 00 <password> 00 -
// set WiFi credentials together with a static IP. ip/mask/gateway are 4 octets
// each in normal order (e.g. {192,168,1,50}). Power-cycle afterwards.
munbyn_error_t munbyn_set_wifi_static(munbyn_handle_t handle, const char* ssid,
                                      const char* password, munbyn_wifi_keytype_t key_type,
                                      const uint8_t ip[4], const uint8_t mask[4],
                                      const uint8_t gateway[4]);
// 1F 1B 1F 28 13 14 04 n - enable (true) / disable (false) DHCP (official).
munbyn_error_t munbyn_set_dhcp(munbyn_handle_t handle, bool enabled);

#ifdef __cplusplus
}
#endif

#endif // MUNBYN_PRINTER_H