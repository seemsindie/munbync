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

// Print justification
typedef enum {
    MUNBYN_JUSTIFY_LEFT = 0,
    MUNBYN_JUSTIFY_CENTER = 1,
    MUNBYN_JUSTIFY_RIGHT = 2
} munbyn_justify_t;

// Font types
typedef enum {
    MUNBYN_FONT_A = 0,
    MUNBYN_FONT_B = 1
} munbyn_font_t;

// Print modes
typedef enum {
    MUNBYN_MODE_NORMAL = 0x00,
    MUNBYN_MODE_EMPHASIZED = 0x08,
    MUNBYN_MODE_DOUBLE_HEIGHT = 0x10,
    MUNBYN_MODE_DOUBLE_WIDTH = 0x20,
    MUNBYN_MODE_UNDERLINE = 0x80
} munbyn_print_mode_t;

// Cut modes
typedef enum {
    MUNBYN_CUT_PARTIAL = 0,
    MUNBYN_CUT_FULL = 1,
} munbyn_cut_mode_t;

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
    MUNBYN_HRI_NONE = 0,
    MUNBYN_HRI_ABOVE = 1,
    MUNBYN_HRI_BELOW = 2,
    MUNBYN_HRI_BOTH = 3
} munbyn_hri_position_t;

// Optimal cutting configuration constants (based on testing)
#define MUNBYN_OPTIMAL_FEED_LINES     7    // Provides best symmetrical cutting
#define MUNBYN_ACCEPTABLE_FEED_LINES  6    // Alternative if 7 lines too much spacing

// Image mode
typedef enum {
    MUNBYN_IMAGE_NORMAL = 0,
    MUNBYN_IMAGE_DOUBLE_WIDTH = 1,
    MUNBYN_IMAGE_DOUBLE_HEIGHT = 2,
    MUNBYN_IMAGE_QUADRUPLE = 3
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

// Basic printer operations
munbyn_error_t munbyn_initialize(munbyn_handle_t handle);
munbyn_error_t munbyn_cut_paper(munbyn_handle_t handle, munbyn_cut_mode_t mode);
munbyn_error_t munbyn_feed_lines(munbyn_handle_t handle, uint8_t lines);
munbyn_error_t munbyn_open_drawer(munbyn_handle_t handle, uint8_t pin);

// Convenience function for optimal receipt printing and cutting
munbyn_error_t munbyn_print_and_cut(munbyn_handle_t handle, const char* text, munbyn_cut_mode_t cut_mode);

#ifdef __cplusplus
}
#endif

#endif // MUNBYN_PRINTER_H