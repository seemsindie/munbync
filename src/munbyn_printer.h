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
    MUNBYN_BARCODE_CODE128 = 73
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

// Common operations
munbyn_error_t munbyn_write_data(munbyn_handle_t handle, const uint8_t* data, size_t length);
munbyn_error_t munbyn_read_data(munbyn_handle_t handle, uint8_t* buffer, size_t buffer_size, size_t* bytes_read);
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

#ifdef __cplusplus
}
#endif

#endif // MUNBYN_PRINTER_H