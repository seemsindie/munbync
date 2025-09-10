#include "munbyn_printer.h"
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <stdarg.h>

// Platform-specific includes
#ifdef _WIN32
#include <windows.h>
#else
#include <unistd.h>
#include <fcntl.h>
#include <termios.h>
#include <sys/select.h>
#include <sys/time.h>
#include <errno.h>
#endif

#define ESC 0x1B
#define FS 0x1C
#define GS 0x1D

struct munbyn_printer
{
    bool initialized;
    munbyn_connection_t transport_type;
    
    // Transport-specific data
    union {
        struct {
#ifdef _WIN32
            HANDLE fd;
#else
            int fd;
#endif
            char device_path[256];
        } usb;
        
        struct {
#ifdef _WIN32
            HANDLE fd;
#else
            int fd;
#endif
            char port_name[256];
            int baud_rate;
        } serial;
        
        struct {
            int socket_fd;
            char ip_address[256];
            int port;
            int timeout_ms;
        } network;
        
        struct {
            int socket_fd;
            char address[32];
        } bluetooth;
    } transport;

    // Current printer state
    munbyn_font_t current_font;
    uint8_t current_modes;
    munbyn_justify_t current_justify;
    int barcode_height;
    int barcode_width;
    munbyn_hri_position_t hri_position;
    munbyn_hri_font_t hri_font;
    munbyn_codepage_t current_codepage;
    munbyn_international_charset_t current_charset;
    
    // Motion units (GS P command)
    uint8_t horizontal_motion_unit;  // Default: 180 (1/180 inch)
    uint8_t vertical_motion_unit;    // Default: 180 (1/180 inch)
};

#ifdef _WIN32
#else

static munbyn_error_t open_serial_port(munbyn_handle_t handle)
{
    handle->transport.serial.fd = open(handle->transport.serial.port_name, O_RDWR | O_NOCTTY | O_NDELAY);

    if (handle->transport.serial.fd == -1)
    {
        return MUNBYN_ERROR_INVALID_HANDLE;
    }

    struct termios options;
    if (tcgetattr(handle->transport.serial.fd, &options) != 0)
    {
        return MUNBYN_ERROR_INVALID_HANDLE;
    }

    // Set baud rate
    speed_t baud;
    switch (handle->transport.serial.baud_rate)
    {
    case 9600:
        baud = B9600;
        break;
    case 19200:
        baud = B19200;
        break;
    case 38400:
        baud = B38400;
        break;
    case 57600:
        baud = B57600;
        break;
    case 115200:
        baud = B115200;
        break;
    default:
        baud = B9600;
        break;
    }

    cfsetispeed(&options, baud);
    cfsetospeed(&options, baud);

    // Configure 8N1
    options.c_cflag &= ~PARENB; // No parity
    options.c_cflag &= ~CSTOPB; // 1 stop bit
    options.c_cflag &= ~CSIZE; // Clear size bits
    options.c_cflag |= CS8; // 8 data bits

    // Raw input/output
    options.c_lflag &= ~(ICANON | ECHO | ECHOE | ISIG);
    options.c_iflag &= ~(IXON | IXOFF | IXANY);
    options.c_oflag &= ~OPOST;

    // Set timeout
    options.c_cc[VMIN] = 0;
    options.c_cc[VTIME] = 10; // 1 second timeout

    if (tcsetattr(handle->transport.serial.fd, TCSANOW, &options) != 0) {
        close(handle->transport.serial.fd);
        handle->transport.serial.fd = -1;
        return MUNBYN_ERROR_COMMUNICATION;
    }

    return MUNBYN_OK;
}

static munbyn_error_t open_usb_port(munbyn_handle_t handle)
{
    handle->transport.usb.fd = open(handle->transport.usb.device_path, O_RDWR | O_NOCTTY);

    if (handle->transport.usb.fd == -1)
    {
        return MUNBYN_ERROR_INVALID_HANDLE;
    }

    return MUNBYN_OK;
}

#endif

// Core API Implementation

munbyn_error_t munbyn_open(const munbyn_connection_params_t* params, munbyn_handle_t* handle)
{
    if (!params || !handle) {
        return MUNBYN_ERROR_INVALID_PARAMETER;
    }

    munbyn_handle_t printer = malloc(sizeof(struct munbyn_printer));
    if (!printer) {
        return MUNBYN_ERROR_INVALID_HANDLE;
    }

    memset(printer, 0, sizeof(struct munbyn_printer));
    printer->transport_type = params->type;

    munbyn_error_t result = MUNBYN_OK;

    switch (params->type) {
        case MUNBYN_CONNECTION_USB:
            strncpy(printer->transport.usb.device_path, params->config.usb.device_path, 
                    sizeof(printer->transport.usb.device_path) - 1);
#ifndef _WIN32
            result = open_usb_port(printer);
#endif
            break;

        case MUNBYN_CONNECTION_SERIAL:
            strncpy(printer->transport.serial.port_name, params->config.serial.port_name, 
                    sizeof(printer->transport.serial.port_name) - 1);
            printer->transport.serial.baud_rate = params->config.serial.baud_rate;
#ifndef _WIN32
            result = open_serial_port(printer);
#endif
            break;

        case MUNBYN_CONNECTION_NETWORK:
        case MUNBYN_CONNECTION_BLUETOOTH:
            // TODO: Implement network and bluetooth transports
            result = MUNBYN_ERROR_INVALID_PARAMETER;
            break;

        default:
            result = MUNBYN_ERROR_INVALID_PARAMETER;
            break;
    }

    if (result != MUNBYN_OK) {
        free(printer);
        return result;
    }

    printer->initialized = true;
    *handle = printer;
    return MUNBYN_OK;
}

munbyn_error_t munbyn_close(munbyn_handle_t handle)
{
    if (!handle || !handle->initialized) {
        return MUNBYN_ERROR_INVALID_HANDLE;
    }

    switch (handle->transport_type) {
        case MUNBYN_CONNECTION_USB:
#ifndef _WIN32
            if (handle->transport.usb.fd != -1) {
                close(handle->transport.usb.fd);
            }
#endif
            break;

        case MUNBYN_CONNECTION_SERIAL:
#ifndef _WIN32
            if (handle->transport.serial.fd != -1) {
                close(handle->transport.serial.fd);
            }
#endif
            break;

        case MUNBYN_CONNECTION_NETWORK:
        case MUNBYN_CONNECTION_BLUETOOTH:
            // TODO: Implement close for network and bluetooth
            break;
    }

    handle->initialized = false;
    free(handle);
    return MUNBYN_OK;
}

munbyn_error_t munbyn_open_usb(const char* device_path, munbyn_handle_t* handle)
{
    if (!device_path || !handle) {
        return MUNBYN_ERROR_INVALID_PARAMETER;
    }

    munbyn_connection_params_t params = {0};
    params.type = MUNBYN_CONNECTION_USB;
    strncpy(params.config.usb.device_path, device_path, sizeof(params.config.usb.device_path) - 1);

    return munbyn_open(&params, handle);
}

munbyn_error_t munbyn_open_serial(const char* port_name, int baud_rate, munbyn_handle_t* handle)
{
    if (!port_name || !handle) {
        return MUNBYN_ERROR_INVALID_PARAMETER;
    }

    munbyn_connection_params_t params = {0};
    params.type = MUNBYN_CONNECTION_SERIAL;
    strncpy(params.config.serial.port_name, port_name, sizeof(params.config.serial.port_name) - 1);
    params.config.serial.baud_rate = baud_rate;

    return munbyn_open(&params, handle);
}

munbyn_error_t munbyn_write_data(munbyn_handle_t handle, const uint8_t* data, size_t length)
{
    if (!handle || !handle->initialized || !data || length == 0) {
        return MUNBYN_ERROR_INVALID_PARAMETER;
    }

    int fd = -1;
    switch (handle->transport_type) {
        case MUNBYN_CONNECTION_USB:
#ifndef _WIN32
            fd = handle->transport.usb.fd;
#endif
            break;

        case MUNBYN_CONNECTION_SERIAL:
#ifndef _WIN32
            fd = handle->transport.serial.fd;
#endif
            break;

        default:
            return MUNBYN_ERROR_INVALID_PARAMETER;
    }

#ifndef _WIN32
    if (fd == -1) {
        return MUNBYN_ERROR_INVALID_HANDLE;
    }

    ssize_t bytes_written = write(fd, data, length);
    if (bytes_written != (ssize_t)length) {
        return MUNBYN_ERROR_COMMUNICATION;
    }
#endif

    return MUNBYN_OK;
}

munbyn_error_t munbyn_read_data(munbyn_handle_t handle, uint8_t* buffer, size_t buffer_size, size_t* bytes_read)
{
    if (!handle || !handle->initialized || !buffer || !bytes_read || buffer_size == 0) {
        return MUNBYN_ERROR_INVALID_PARAMETER;
    }

    int fd = -1;
    switch (handle->transport_type) {
        case MUNBYN_CONNECTION_USB:
#ifndef _WIN32
            fd = handle->transport.usb.fd;
#endif
            break;

        case MUNBYN_CONNECTION_SERIAL:
#ifndef _WIN32
            fd = handle->transport.serial.fd;
#endif
            break;

        default:
            return MUNBYN_ERROR_INVALID_PARAMETER;
    }

#ifndef _WIN32
    if (fd == -1) {
        return MUNBYN_ERROR_INVALID_HANDLE;
    }

    ssize_t result = read(fd, buffer, buffer_size);
    if (result < 0) {
        *bytes_read = 0;
        return MUNBYN_ERROR_COMMUNICATION;
    }

    *bytes_read = (size_t)result;
#endif

    return MUNBYN_OK;
}

// Basic printer operations implementation

munbyn_error_t munbyn_get_status(munbyn_handle_t handle, munbyn_status_t* status)
{
    if (!handle || !handle->initialized || !status) {
        return MUNBYN_ERROR_INVALID_PARAMETER;
    }

    // Clear the status structure
    memset(status, 0, sizeof(munbyn_status_t));

    // DLE EOT 1 - Request printer status
    uint8_t status_cmd[] = {0x10, 0x04, 0x01};
    uint8_t response[4] = {0};
    size_t bytes_read = 0;

    munbyn_error_t result = munbyn_write_data(handle, status_cmd, sizeof(status_cmd));
    if (result != MUNBYN_OK) {
        return result;
    }

    // Small delay to allow printer to respond
#ifndef _WIN32
    struct timeval timeout = {0, 100000}; // 100ms
    select(0, NULL, NULL, NULL, &timeout);
#else
    Sleep(100); // 100ms
#endif

    result = munbyn_read_data(handle, response, sizeof(response), &bytes_read);
    if (result != MUNBYN_OK) {
        return result;
    }

    if (bytes_read > 0) {
        uint8_t status_byte = response[0];
        
        // Parse status bits according to ESC/POS specification
        // For ITPP047, let's use more standard interpretations
        status->online = (status_byte & 0x08) == 0;       // Bit 3: 0=online, 1=offline
        status->paper_present = (status_byte & 0x20) == 0; // Bit 5: 0=paper present, 1=paper out  
        status->cover_closed = (status_byte & 0x04) == 0;  // Bit 2: 0=cover closed, 1=cover open
        status->error_occurred = (status_byte & 0x40) != 0; // Bit 6: 1=error occurred
        status->cut_error = false;                         // Not available in basic status
        status->recoverable_error = (status_byte & 0x08) != 0; // Based on offline status
        status->unrecoverable_error = (status_byte & 0x40) != 0; // Same as error_occurred
    } else {
        // If no response received, assume default safe values
        status->online = true;
        status->paper_present = true;
        status->cover_closed = true;
        status->error_occurred = false;
        status->cut_error = false;
        status->recoverable_error = false;
        status->unrecoverable_error = false;
    }

    return MUNBYN_OK;
}

munbyn_error_t munbyn_initialize(munbyn_handle_t handle)
{
    if (!handle || !handle->initialized) {
        return MUNBYN_ERROR_INVALID_PARAMETER;
    }

    // ESC @ - Initialize printer
    uint8_t init_cmd[] = {ESC, 0x40};

    munbyn_error_t result = munbyn_write_data(handle, init_cmd, sizeof(init_cmd));
    if (result == MUNBYN_OK) {
        // Reset internal state
        handle->current_font = MUNBYN_FONT_A;
        handle->current_modes = MUNBYN_MODE_NORMAL;
        
        // Set default motion units (GS P defaults)
        handle->horizontal_motion_unit = 180;  // 1/180 inch
        handle->vertical_motion_unit = 180;    // 1/180 inch (corrected from manual)
        handle->current_justify = MUNBYN_JUSTIFY_LEFT;
        handle->barcode_height = 162; // Default height
        handle->barcode_width = 3;    // Default width
        handle->hri_position = MUNBYN_HRI_NONE;
        handle->hri_font = MUNBYN_HRI_FONT_STANDARD; // Default HRI font
        handle->current_codepage = MUNBYN_CODEPAGE_PC437; // Default codepage
        handle->current_charset = MUNBYN_INTL_USA;        // Default international charset
    }

    return result;
}

munbyn_error_t munbyn_cut_paper(munbyn_handle_t handle, munbyn_cut_mode_t mode)
{
    if (!handle || !handle->initialized) {
        return MUNBYN_ERROR_INVALID_PARAMETER;
    }

    uint8_t cut_cmd[3];
    
    // Validate mode parameter according to manual specification
    if (mode == MUNBYN_CUT_ONE_POINT_UNCUT || mode == MUNBYN_CUT_PARTIAL || 
        mode == MUNBYN_CUT_ONE_POINT_UNCUT_ALT || mode == MUNBYN_CUT_PARTIAL_ALT) {
        
        // GS V m - Select cut mode and cut paper
        cut_cmd[0] = GS;
        cut_cmd[1] = 0x56;
        cut_cmd[2] = (uint8_t)mode;
        return munbyn_write_data(handle, cut_cmd, 3);
    }

    return MUNBYN_ERROR_INVALID_PARAMETER;
}

munbyn_error_t munbyn_feed_and_cut(munbyn_handle_t handle, uint8_t feed_amount)
{
    if (!handle || !handle->initialized) {
        return MUNBYN_ERROR_INVALID_PARAMETER;
    }

    // GS V 66 n - Feeds paper (cutting position + [n × (vertical motion unit)]), 
    // and cuts the paper partially (one point left uncut)
    uint8_t feed_cut_cmd[4];
    feed_cut_cmd[0] = GS;
    feed_cut_cmd[1] = 0x56;
    feed_cut_cmd[2] = 66;
    feed_cut_cmd[3] = feed_amount;
    
    return munbyn_write_data(handle, feed_cut_cmd, 4);
}

munbyn_error_t munbyn_feed_lines(munbyn_handle_t handle, uint8_t lines)
{
    if (!handle || !handle->initialized) {
        return MUNBYN_ERROR_INVALID_PARAMETER;
    }

    // ESC d n - Print and feed n lines
    uint8_t feed_cmd[] = {ESC, 0x64, lines};
    
    return munbyn_write_data(handle, feed_cmd, sizeof(feed_cmd));
}

munbyn_error_t munbyn_open_drawer(munbyn_handle_t handle, munbyn_drawer_pin_t pin, uint8_t on_time, uint8_t off_time)
{
    if (!handle || !handle->initialized) {
        return MUNBYN_ERROR_INVALID_PARAMETER;
    }

    // Validate pin parameter according to manual specification
    if (pin != MUNBYN_DRAWER_PIN_2 && pin != MUNBYN_DRAWER_PIN_5 && 
        pin != MUNBYN_DRAWER_PIN_2_ALT && pin != MUNBYN_DRAWER_PIN_5_ALT) {
        return MUNBYN_ERROR_INVALID_PARAMETER;
    }

    // ESC p m t1 t2 - Generate pulse on connector pin
    // m: connector pin selection (0, 1, 48, 49)
    //    0, 48: Drawer kick-out connector pin 2
    //    1, 49: Drawer kick-out connector pin 5
    // t1: ON time (t1 × 2ms), range 0-255
    // t2: OFF time (t2 × 2ms), range 0-255
    //     If t2 < t1, the OFF time is [t1 × 2ms]
    uint8_t drawer_cmd[] = {ESC, 0x70, (uint8_t)pin, on_time, off_time};
    
    return munbyn_write_data(handle, drawer_cmd, sizeof(drawer_cmd));
}

munbyn_error_t munbyn_open_drawer_default(munbyn_handle_t handle, munbyn_drawer_pin_t pin)
{
    // Use default timing: 50ms ON, 500ms OFF (25 × 2ms, 250 × 2ms)
    return munbyn_open_drawer(handle, pin, 25, 250);
}

munbyn_error_t munbyn_print_and_cut(munbyn_handle_t handle, const char* text, munbyn_cut_mode_t cut_mode)
{
    if (!handle || !text) {
        return MUNBYN_ERROR_INVALID_PARAMETER;
    }
    
    munbyn_error_t result;
    
    // Print the text
    result = munbyn_write_data(handle, (const uint8_t*)text, strlen(text));
    if (result != MUNBYN_OK) {
        return result;
    }
    
    // Feed optimal number of lines for symmetrical cutting
    result = munbyn_feed_lines(handle, MUNBYN_OPTIMAL_FEED_LINES);
    if (result != MUNBYN_OK) {
        return result;
    }
    
    // Perform the cut
    return munbyn_cut_paper(handle, cut_mode);
}

// Text control commands implementation

munbyn_error_t munbyn_line_feed(munbyn_handle_t handle)
{
    if (!handle || !handle->initialized) {
        return MUNBYN_ERROR_INVALID_PARAMETER;
    }

    // LF - Line Feed (print and move cursor down one line)
    uint8_t lf_cmd[] = {0x0A};
    
    return munbyn_write_data(handle, lf_cmd, sizeof(lf_cmd));
}

munbyn_error_t munbyn_carriage_return(munbyn_handle_t handle)
{
    if (!handle || !handle->initialized) {
        return MUNBYN_ERROR_INVALID_PARAMETER;
    }

    // CR - Carriage Return (move cursor to beginning of current line)
    uint8_t cr_cmd[] = {0x0D};
    
    return munbyn_write_data(handle, cr_cmd, sizeof(cr_cmd));
}

munbyn_error_t munbyn_horizontal_tab(munbyn_handle_t handle)
{
    if (!handle || !handle->initialized) {
        return MUNBYN_ERROR_INVALID_PARAMETER;
    }

    // HT - Horizontal Tab (move to next horizontal tab position)
    uint8_t ht_cmd[] = {0x09};

    return munbyn_write_data(handle, ht_cmd, sizeof(ht_cmd));
}

munbyn_error_t munbyn_set_horizontal_tab_positions(munbyn_handle_t handle, const uint8_t* positions, size_t count)
{
    if (!handle || !handle->initialized) {
        return MUNBYN_ERROR_INVALID_PARAMETER;
    }

    // k must be 1..32 according to the manual. If 0, use the clear function.
    if (count == 0 || count > 32 || positions == NULL) {
        return MUNBYN_ERROR_INVALID_PARAMETER;
    }

    // Validate ascending order and range 1..255
    for (size_t i = 0; i < count; i++) {
        if (positions[i] < 1 || positions[i] > 255) {
            return MUNBYN_ERROR_INVALID_PARAMETER;
        }
        if (i > 0 && positions[i] <= positions[i - 1]) {
            return MUNBYN_ERROR_INVALID_PARAMETER;
        }
    }

    // ESC D n1 ... nk NUL
    uint8_t cmd[2 + 32 + 1];
    size_t idx = 0;
    cmd[idx++] = ESC;
    cmd[idx++] = 0x44; // 'D'
    for (size_t i = 0; i < count; i++) {
        cmd[idx++] = positions[i];
    }
    cmd[idx++] = 0x00; // Terminator

    return munbyn_write_data(handle, cmd, idx);
}

munbyn_error_t munbyn_clear_horizontal_tab_positions(munbyn_handle_t handle)
{
    if (!handle || !handle->initialized) {
        return MUNBYN_ERROR_INVALID_PARAMETER;
    }

    // ESC D NUL — cancels all horizontal tab positions
    uint8_t cmd[] = {ESC, 0x44, 0x00};
    return munbyn_write_data(handle, cmd, sizeof(cmd));
}

// Character set and codepage commands implementation

munbyn_error_t munbyn_set_international_charset(munbyn_handle_t handle, munbyn_international_charset_t charset)
{
    if (!handle || !handle->initialized) {
        return MUNBYN_ERROR_INVALID_PARAMETER;
    }

    // Validate charset parameter
    if (charset < MUNBYN_INTL_USA || charset > MUNBYN_INTL_CHINA) {
        return MUNBYN_ERROR_INVALID_PARAMETER;
    }

    // ESC R n - Select international character set
    uint8_t charset_cmd[] = {ESC, 0x52, (uint8_t)charset};
    
    munbyn_error_t result = munbyn_write_data(handle, charset_cmd, sizeof(charset_cmd));
    if (result == MUNBYN_OK) {
        handle->current_charset = charset;
    }
    
    return result;
}

munbyn_error_t munbyn_set_codepage(munbyn_handle_t handle, munbyn_codepage_t codepage)
{
    if (!handle || !handle->initialized) {
        return MUNBYN_ERROR_INVALID_PARAMETER;
    }

    // Validate codepage parameter
    if (codepage < MUNBYN_CODEPAGE_PC437 || codepage > MUNBYN_CODEPAGE_PC3041_MALTESE) {
        return MUNBYN_ERROR_INVALID_PARAMETER;
    }

    // ESC t n - Select character code table
    uint8_t codepage_cmd[] = {ESC, 0x74, (uint8_t)codepage};
    
    munbyn_error_t result = munbyn_write_data(handle, codepage_cmd, sizeof(codepage_cmd));
    if (result == MUNBYN_OK) {
        handle->current_codepage = codepage;
    }
    
    return result;
}

// Helper function to get codepage name
const char* munbyn_get_codepage_name(munbyn_codepage_t codepage)
{
    switch (codepage) {
        case MUNBYN_CODEPAGE_PC437: return "PC437(Std.Europe)";
        case MUNBYN_CODEPAGE_KATAKANA: return "Katakana";
        case MUNBYN_CODEPAGE_PC850: return "PC850(Multilingual)";
        case MUNBYN_CODEPAGE_PC860: return "PC860(Portugal)";
        case MUNBYN_CODEPAGE_PC863: return "PC863(Canadian)";
        case MUNBYN_CODEPAGE_PC865: return "PC865(Nordic)";
        case MUNBYN_CODEPAGE_WEST_EUROPE: return "West Europe";
        case MUNBYN_CODEPAGE_GREEK: return "Greek";
        case MUNBYN_CODEPAGE_HEBREW: return "Hebrew";
        case MUNBYN_CODEPAGE_EAST_EUROPE: return "East Europe";
        case MUNBYN_CODEPAGE_IRAN: return "Iran";
        case MUNBYN_CODEPAGE_WCP1252: return "WCP1252";
        case MUNBYN_CODEPAGE_PC866: return "PC866(Cyrillic#2)";
        case MUNBYN_CODEPAGE_PC852: return "PC852(Latin2)";
        case MUNBYN_CODEPAGE_PC858: return "PC858";
        case MUNBYN_CODEPAGE_IRAN_II: return "IranII";
        case MUNBYN_CODEPAGE_LATVIAN: return "Latvian";
        case MUNBYN_CODEPAGE_ARABIC: return "Arabic";
        case MUNBYN_CODEPAGE_PT151125: return "PT151125";
        case MUNBYN_CODEPAGE_PC747: return "PC747";
        case MUNBYN_CODEPAGE_WPC1257: return "WPC1257";
        case MUNBYN_CODEPAGE_THAI: return "Thai";
        case MUNBYN_CODEPAGE_VIETNAM: return "Vietnam";
        case MUNBYN_CODEPAGE_PC864: return "PC864";
        case MUNBYN_CODEPAGE_PC1001: return "PC1001";
        case MUNBYN_CODEPAGE_UIGUR: return "Uigur";
        case MUNBYN_CODEPAGE_HEBREW_ALT: return "Hebrew";
        case MUNBYN_CODEPAGE_WPC1255: return "WPC1255(Israel)";
        case MUNBYN_CODEPAGE_PC437_ALT: return "PC437(Std.Europe)";
        case MUNBYN_CODEPAGE_KATAKANA_ALT: return "Katakana";
        case MUNBYN_CODEPAGE_PC437_ALT2: return "PC437(Std.Europe)";
        case MUNBYN_CODEPAGE_PC866_MULT: return "PC866(Mult)";
        case MUNBYN_CODEPAGE_PC852_LATIN2: return "PC852(Latin-2)";
        case MUNBYN_CODEPAGE_PC866_PORT: return "PC866(Portuguese)";
        case MUNBYN_CODEPAGE_PC865_TEST: return "PC865(TestEscPos)";
        case MUNBYN_CODEPAGE_PC863_CAN: return "PC863(Canadian)";
        case MUNBYN_CODEPAGE_PC865_NORDIC: return "PC865(Nordic)";
        case MUNBYN_CODEPAGE_PC866_RUSSIAN: return "PC866(Russian)";
        case MUNBYN_CODEPAGE_PC855_BULG: return "PC855(Bulgarian)";
        case MUNBYN_CODEPAGE_PC857_TURKEY: return "PC857(Turkey)";
        case MUNBYN_CODEPAGE_PC862_HEBREW: return "PC862(Hebrew)";
        case MUNBYN_CODEPAGE_PC864_ARABIC: return "PC864(Arabic)";
        case MUNBYN_CODEPAGE_PC737_GREEK: return "PC737(Greek)";
        case MUNBYN_CODEPAGE_PC851_GREEK: return "PC851(Greek)";
        case MUNBYN_CODEPAGE_PC869_GREEK: return "PC869(Greek)";
        case MUNBYN_CODEPAGE_PC928_GREEK: return "PC928(Greek)";
        case MUNBYN_CODEPAGE_PC772_LITH: return "PC772(Lithuanian)";
        case MUNBYN_CODEPAGE_PC774_LITH: return "PC774(Lithuanian)";
        case MUNBYN_CODEPAGE_PC874_THAI: return "PC874(Thai)";
        case MUNBYN_CODEPAGE_WPC1252_LATIN1: return "WPC1252(Latin1)";
        case MUNBYN_CODEPAGE_WPC1250_LATIN2: return "WPC1250(Latin-2)";
        case MUNBYN_CODEPAGE_WPC1251_CYR: return "WPC1251(Cyrillic)";
        case MUNBYN_CODEPAGE_PC3840_IBM_RUS: return "PC3840(IBM-Russian)";
        case MUNBYN_CODEPAGE_PC3841_GOST: return "PC3841(Gost)";
        case MUNBYN_CODEPAGE_PC3843_POLISH: return "PC3843(Polish)";
        case MUNBYN_CODEPAGE_PC3844_CS2: return "PC3844(CS2)";
        case MUNBYN_CODEPAGE_PC3845_HUNG: return "PC3845(Hungarian)";
        case MUNBYN_CODEPAGE_PC3846_TURK: return "PC3846(Turkish)";
        case MUNBYN_CODEPAGE_PC3847_BR_ABNT: return "PC3847(Brazil-ABNT)";
        case MUNBYN_CODEPAGE_PC3848_BRAZIL: return "PC3848(Brazil)";
        case MUNBYN_CODEPAGE_PC1001_ARABIC: return "PC1001(Arabic)";
        case MUNBYN_CODEPAGE_PC2001_LITH: return "PC2001(Lithuanian)";
        case MUNBYN_CODEPAGE_PC3001_EST1: return "PC3001(Estonian-1)";
        case MUNBYN_CODEPAGE_PC3002_EST2: return "PC3002(Eston-2)";
        case MUNBYN_CODEPAGE_PC3011_LAT1: return "PC3011(Latvian-1)";
        case MUNBYN_CODEPAGE_PC3012_LAT2: return "PC3012(Latv-2)";
        case MUNBYN_CODEPAGE_PC3021_BULG: return "PC3021(Bulgarian)";
        case MUNBYN_CODEPAGE_PC3041_MALTESE: return "PC3041(Maltese)";
        default: return "Unknown";
    }
}

// Text formatting and alignment commands implementation

munbyn_error_t munbyn_set_justification(munbyn_handle_t handle, munbyn_justify_t justify)
{
    if (!handle || !handle->initialized) {
        return MUNBYN_ERROR_INVALID_PARAMETER;
    }

    // Validate justification parameter according to manual specification
    if (justify != MUNBYN_JUSTIFY_LEFT && justify != MUNBYN_JUSTIFY_CENTER && justify != MUNBYN_JUSTIFY_RIGHT &&
        justify != MUNBYN_JUSTIFY_LEFT_ALT && justify != MUNBYN_JUSTIFY_CENTER_ALT && justify != MUNBYN_JUSTIFY_RIGHT_ALT) {
        return MUNBYN_ERROR_INVALID_PARAMETER;
    }

    // ESC a n - Set justification
    // n = 0, 48: Left justification
    // n = 1, 49: Centering
    // n = 2, 50: Right justification
    uint8_t justify_cmd[] = {ESC, 0x61, (uint8_t)justify};
    
    munbyn_error_t result = munbyn_write_data(handle, justify_cmd, sizeof(justify_cmd));
    if (result == MUNBYN_OK) {
        handle->current_justify = justify;
    }
    
    return result;
}

munbyn_error_t munbyn_set_font(munbyn_handle_t handle, munbyn_font_t font)
{
    if (!handle || !handle->initialized) {
        return MUNBYN_ERROR_INVALID_PARAMETER;
    }

    // Validate font parameter according to manual specification
    if (font != MUNBYN_FONT_A && font != MUNBYN_FONT_B && 
        font != MUNBYN_FONT_A_ALT && font != MUNBYN_FONT_B_ALT) {
        return MUNBYN_ERROR_INVALID_PARAMETER;
    }

    // ESC M n - Select character font
    // n = 0, 48: Font A (12×24)
    // n = 1, 49: Font B (9×17)
    uint8_t font_cmd[] = {ESC, 0x4D, (uint8_t)font};
    
    munbyn_error_t result = munbyn_write_data(handle, font_cmd, sizeof(font_cmd));
    if (result == MUNBYN_OK) {
        handle->current_font = font;
    }
    
    return result;
}

munbyn_error_t munbyn_set_text_mode(munbyn_handle_t handle, uint8_t modes)
{
    if (!handle || !handle->initialized) {
        return MUNBYN_ERROR_INVALID_PARAMETER;
    }

    // ESC ! n - Select print mode(s)
    // Bit 0: Font selection (0: Font A, 1: Font B)
    // Bit 3: Emphasized mode
    // Bit 4: Double-height mode
    // Bit 5: Double-width mode
    // Bit 7: Underline mode
    uint8_t mode_cmd[] = {ESC, 0x21, modes};
    
    munbyn_error_t result = munbyn_write_data(handle, mode_cmd, sizeof(mode_cmd));
    if (result == MUNBYN_OK) {
        handle->current_modes = modes;
        // Update font based on bit 0
        handle->current_font = (modes & 0x01) ? MUNBYN_FONT_B : MUNBYN_FONT_A;
    }
    
    return result;
}

munbyn_error_t munbyn_set_emphasis(munbyn_handle_t handle, bool enabled)
{
    if (!handle || !handle->initialized) {
        return MUNBYN_ERROR_INVALID_PARAMETER;
    }

    // ESC E n - Turn emphasized mode on/off
    // n = 0: Cancel emphasized mode
    // n = 1: Select emphasized mode
    uint8_t emphasis_cmd[] = {ESC, 0x45, enabled ? 1 : 0};
    
    munbyn_error_t result = munbyn_write_data(handle, emphasis_cmd, sizeof(emphasis_cmd));
    if (result == MUNBYN_OK) {
        if (enabled) {
            handle->current_modes |= MUNBYN_MODE_EMPHASIZED;
        } else {
            handle->current_modes &= ~MUNBYN_MODE_EMPHASIZED;
        }
    }
    
    return result;
}

munbyn_error_t munbyn_set_double_strike(munbyn_handle_t handle, bool enabled)
{
    if (!handle || !handle->initialized) {
        return MUNBYN_ERROR_INVALID_PARAMETER;
    }

    // ESC G n - Turn double-strike mode on/off
    // n = 0: Cancel double-strike mode
    // n = 1: Select double-strike mode
    uint8_t double_strike_cmd[] = {ESC, 0x47, enabled ? 1 : 0};
    
    return munbyn_write_data(handle, double_strike_cmd, sizeof(double_strike_cmd));
}

munbyn_error_t munbyn_set_underline(munbyn_handle_t handle, uint8_t mode)
{
    if (!handle || !handle->initialized) {
        return MUNBYN_ERROR_INVALID_PARAMETER;
    }

    // Validate underline mode (0-2 or 48-50)
    if (mode > 2 && (mode < 48 || mode > 50)) {
        return MUNBYN_ERROR_INVALID_PARAMETER;
    }

    // ESC - n - Turn underline mode on/off
    // n = 0,48: Cancel underline mode
    // n = 1,49: Select underline mode (1-dot thick)
    // n = 2,50: Select underline mode (2-dot thick)
    uint8_t underline_cmd[] = {ESC, 0x2D, mode};
    
    munbyn_error_t result = munbyn_write_data(handle, underline_cmd, sizeof(underline_cmd));
    if (result == MUNBYN_OK) {
        // Update mode based on enabled state (both 0,48 = off, others = on)
        if (mode == 0 || mode == 48) {
            handle->current_modes &= ~MUNBYN_MODE_UNDERLINE;
        } else {
            handle->current_modes |= MUNBYN_MODE_UNDERLINE;
        }
    }
    
    return result;
}

munbyn_error_t munbyn_set_underline_kanji(munbyn_handle_t handle, uint8_t mode)
{
    if (!handle || !handle->initialized) {
        return MUNBYN_ERROR_INVALID_PARAMETER;
    }

    // Validate underline mode (0-2 or 48-50)
    if (mode > 2 && (mode < 48 || mode > 50)) {
        return MUNBYN_ERROR_INVALID_PARAMETER;
    }

    // FS - n - Turn underline mode on/off for Kanji characters
    // n = 0,48: Turns off underline mode for Kanji characters
    // n = 1,49: Turns on underline mode for Kanji characters (1-dot thick)
    // n = 2,50: Turns on underline mode for Kanji characters (2-dot thick)
    uint8_t underline_cmd[] = {FS, 0x2D, mode};
    
    return munbyn_write_data(handle, underline_cmd, sizeof(underline_cmd));
}

munbyn_error_t munbyn_set_line_spacing_default(munbyn_handle_t handle)
{
    if (!handle || !handle->initialized) {
        return MUNBYN_ERROR_INVALID_PARAMETER;
    }

    // ESC 2 - Select default line spacing (approximately 4.23mm or 1/6 inch)
    uint8_t line_spacing_cmd[] = {ESC, 0x32};
    
    return munbyn_write_data(handle, line_spacing_cmd, sizeof(line_spacing_cmd));
}

munbyn_error_t munbyn_set_line_spacing(munbyn_handle_t handle, uint8_t spacing)
{
    if (!handle || !handle->initialized) {
        return MUNBYN_ERROR_INVALID_PARAMETER;
    }

    // ESC 3 n - Set line spacing to n × vertical motion unit
    // n can be 0-255, with default vertical motion unit (1/180 inch): n/180 inch
    // ESC 3 30 = 30/180 = 1/6 inch = same as ESC 2 default
    uint8_t line_spacing_cmd[] = {ESC, 0x33, spacing};
    
    return munbyn_write_data(handle, line_spacing_cmd, sizeof(line_spacing_cmd));
}

munbyn_error_t munbyn_set_motion_units(munbyn_handle_t handle, uint8_t horizontal, uint8_t vertical)
{
    if (!handle || !handle->initialized) {
        return MUNBYN_ERROR_INVALID_PARAMETER;
    }

    // GS P x y - Set horizontal and vertical motion units
    // x: horizontal motion unit (0-255), when 0 uses default
    // y: vertical motion unit (0-255), when 0 uses default
    // Motion unit = approximately 25.4/n mm (1/n inches)
    uint8_t motion_cmd[] = {GS, 0x50, horizontal, vertical};
    
    munbyn_error_t result = munbyn_write_data(handle, motion_cmd, sizeof(motion_cmd));
    if (result == MUNBYN_OK) {
        // Update internal state, using defaults if 0 specified
        handle->horizontal_motion_unit = (horizontal == 0) ? 180 : horizontal;
        handle->vertical_motion_unit = (vertical == 0) ? 180 : vertical;
    }
    
    return result;
}

munbyn_error_t munbyn_set_character_spacing(munbyn_handle_t handle, uint8_t spacing)
{
    if (!handle || !handle->initialized) {
        return MUNBYN_ERROR_INVALID_PARAMETER;
    }

    // ESC SP n - Set right-side character spacing
    // n = 0-255 (0-255/120 inch spacing)
    uint8_t char_spacing_cmd[] = {ESC, 0x20, spacing};
    
    return munbyn_write_data(handle, char_spacing_cmd, sizeof(char_spacing_cmd));
}

munbyn_error_t munbyn_set_left_margin(munbyn_handle_t handle, uint16_t margin)
{
    if (!handle || !handle->initialized) {
        return MUNBYN_ERROR_INVALID_PARAMETER;
    }

    // GS L nL nH - Set left margin
    // Margin = (nL + nH × 256) × horizontal motion unit (default: 1/180 inch)
    uint8_t nL = margin & 0xFF;
    uint8_t nH = (margin >> 8) & 0xFF;
    
    uint8_t margin_cmd[] = {GS, 0x4C, nL, nH};
    
    return munbyn_write_data(handle, margin_cmd, sizeof(margin_cmd));
}

munbyn_error_t munbyn_set_print_area_width(munbyn_handle_t handle, uint16_t width)
{
    if (!handle || !handle->initialized) {
        return MUNBYN_ERROR_INVALID_PARAMETER;
    }

    // GS W nL nH - Set print area width
    // Width = (nL + nH × 256) × horizontal motion unit (default: 1/180 inch)
    // Default: nL=0, nH=2 (512 units) or nL=104, nH=1 (360 units for 58mm paper)
    uint8_t nL = width & 0xFF;
    uint8_t nH = (width >> 8) & 0xFF;
    
    uint8_t width_cmd[] = {GS, 0x57, nL, nH};
    
    return munbyn_write_data(handle, width_cmd, sizeof(width_cmd));
}

munbyn_error_t munbyn_set_rotate_90(munbyn_handle_t handle, bool enabled)
{
    if (!handle || !handle->initialized) {
        return MUNBYN_ERROR_INVALID_PARAMETER;
    }

    // ESC V n - Turn 90° clockwise rotation mode on/off
    // n = 0,48: Cancel 90° clockwise rotation mode
    // n = 1,49: Select 90° clockwise rotation mode
    // Note: This implementation uses 0,1 values (alternatives 48,49 are functionally identical)
    uint8_t rotate_cmd[] = {ESC, 0x56, enabled ? 1 : 0};
    
    return munbyn_write_data(handle, rotate_cmd, sizeof(rotate_cmd));
}

munbyn_error_t munbyn_set_upside_down(munbyn_handle_t handle, bool enabled)
{
    if (!handle || !handle->initialized) {
        return MUNBYN_ERROR_INVALID_PARAMETER;
    }

    // ESC { n - Turn upside-down print mode on/off
    // LSB of n = 0: Cancel upside-down print mode
    // LSB of n = 1: Select upside-down print mode
    // Only the lowest bit of n is valid (range 0-255)
    uint8_t upside_down_cmd[] = {ESC, 0x7B, enabled ? 1 : 0};
    
    return munbyn_write_data(handle, upside_down_cmd, sizeof(upside_down_cmd));
}

// Advanced text effects implementation

munbyn_error_t munbyn_set_inverted_text(munbyn_handle_t handle, bool enabled)
{
    if (!handle || !handle->initialized) {
        return MUNBYN_ERROR_INVALID_PARAMETER;
    }

    // GS B n - Turn white/black reverse printing mode on/off
    // n = 0 (LSB = 0): White/black reverse mode off
    // n = 1 (LSB = 1): White/black reverse mode on
    // Only the lowest bit of n is valid
    uint8_t reverse_cmd[] = {GS, 0x42, enabled ? 1 : 0};
    
    return munbyn_write_data(handle, reverse_cmd, sizeof(reverse_cmd));
}

munbyn_error_t munbyn_set_text_scale(munbyn_handle_t handle, uint8_t width_scale, uint8_t height_scale)
{
    if (!handle || !handle->initialized) {
        return MUNBYN_ERROR_INVALID_PARAMETER;
    }

    // Validate scale parameters (1-8 for width, 1-8 for height)
    if (width_scale < 1 || width_scale > 8 || height_scale < 1 || height_scale > 8) {
        return MUNBYN_ERROR_INVALID_PARAMETER;
    }

    // GS ! n - Select character size
    // Bits 0-3: Character height (1-8 times)
    // Bits 4-7: Character width (1-8 times)
    uint8_t scale_value = ((width_scale - 1) << 4) | (height_scale - 1);
    uint8_t scale_cmd[] = {GS, 0x21, scale_value};
    
    return munbyn_write_data(handle, scale_cmd, sizeof(scale_cmd));
}

munbyn_error_t munbyn_cancel_all_formatting(munbyn_handle_t handle)
{
    if (!handle || !handle->initialized) {
        return MUNBYN_ERROR_INVALID_PARAMETER;
    }

    munbyn_error_t result;
    
    // Cancel all text formatting modes individually for maximum compatibility
    
    // Cancel emphasis
    result = munbyn_set_emphasis(handle, false);
    if (result != MUNBYN_OK) return result;
    
    // Cancel double-strike
    result = munbyn_set_double_strike(handle, false);
    if (result != MUNBYN_OK) return result;
    
    // Cancel underline
    result = munbyn_set_underline(handle, 0);
    if (result != MUNBYN_OK) return result;
    
    // Cancel inverted text (white/black reverse mode)
    result = munbyn_set_inverted_text(handle, false);
    if (result != MUNBYN_OK) return result;
    
    // Reset text modes to normal
    result = munbyn_set_text_mode(handle, MUNBYN_MODE_NORMAL);
    if (result != MUNBYN_OK) return result;
    
    // Reset text scale to 1x1
    result = munbyn_set_text_scale(handle, 1, 1);
    if (result != MUNBYN_OK) return result;
    
    // Reset justification to left
    result = munbyn_set_justification(handle, MUNBYN_JUSTIFY_LEFT);
    if (result != MUNBYN_OK) return result;
    
    // Reset to Font A
    result = munbyn_set_font(handle, MUNBYN_FONT_A);
    if (result != MUNBYN_OK) return result;
    
    // Reset character spacing
    result = munbyn_set_character_spacing(handle, 0);
    if (result != MUNBYN_OK) return result;
    
    // Reset line spacing to default
    result = munbyn_set_line_spacing_default(handle);
    if (result != MUNBYN_OK) return result;
    
    // Cancel rotation
    result = munbyn_set_rotate_90(handle, false);
    if (result != MUNBYN_OK) return result;
    
    // Cancel upside-down printing
    result = munbyn_set_upside_down(handle, false);
    if (result != MUNBYN_OK) return result;

    return MUNBYN_OK;
}

munbyn_error_t munbyn_set_print_direction(munbyn_handle_t handle, uint8_t direction)
{
    if (!handle || !handle->initialized) {
        return MUNBYN_ERROR_INVALID_PARAMETER;
    }

    // Validate direction parameter (0-3 or 48-51)
    if (direction > 3 && (direction < 48 || direction > 51)) {
        return MUNBYN_ERROR_INVALID_PARAMETER;
    }

    // ESC T n - Select print direction in page mode
    // Sets both print direction and starting position:
    // n = 0, 48: Left to right, Upper left starting position
    // n = 1, 49: Bottom to top, Lower left starting position
    // n = 2, 50: Right to left, Lower right starting position
    // n = 3, 51: Top to bottom, Upper right starting position
    uint8_t direction_cmd[] = {ESC, 0x54, direction};
    
    return munbyn_write_data(handle, direction_cmd, sizeof(direction_cmd));
}

munbyn_error_t munbyn_set_relative_horizontal_position(munbyn_handle_t handle, int16_t position)
{
    if (!handle || !handle->initialized) {
        return MUNBYN_ERROR_INVALID_PARAMETER;
    }

    // ESC \ nL nH - Set relative print position
    // Rightward movement: N = nL + nH × 256
    // Leftward movement: nL + nH × 256 = 65536 - N (complement of 65536)
    // Motion units depend on print direction (set by ESC T)
    
    uint16_t value;
    if (position >= 0) {
        // Positive: rightward movement
        value = (uint16_t)position;
    } else {
        // Negative: leftward movement using 65536 complement
        value = 65536 + position;  // position is negative, so this is 65536 - abs(position)
    }
    
    uint8_t nL = value & 0xFF;
    uint8_t nH = (value >> 8) & 0xFF;
    
    uint8_t rel_pos_cmd[] = {ESC, 0x5C, nL, nH};
    
    return munbyn_write_data(handle, rel_pos_cmd, sizeof(rel_pos_cmd));
}

munbyn_error_t munbyn_set_absolute_horizontal_position(munbyn_handle_t handle, uint16_t position)
{
    if (!handle || !handle->initialized) {
        return MUNBYN_ERROR_INVALID_PARAMETER;
    }

    // ESC $ nL nH - Set absolute print position
    // Sets distance from beginning of line to print position
    // Distance = (nL + nH × 256) × (horizontal or vertical motion unit) inches
    // Motion units specified by GS P command
    // Standard mode: uses horizontal motion unit (x)  
    // Page mode: unit type depends on starting position (set by ESC T)
    // Settings outside printable area are ignored
    uint8_t nL = position & 0xFF;
    uint8_t nH = (position >> 8) & 0xFF;
    
    uint8_t abs_pos_cmd[] = {ESC, 0x24, nL, nH};
    
    return munbyn_write_data(handle, abs_pos_cmd, sizeof(abs_pos_cmd));
}

// Barcode operations

munbyn_error_t munbyn_set_barcode_height(munbyn_handle_t handle, uint8_t height)
{
    if (!handle || !handle->initialized) {
        return MUNBYN_ERROR_INVALID_PARAMETER;
    }
    
    // GS h n - Select bar code height
    // n specifies the number of dots in the vertical direction
    // Range: 1 ≤ n ≤ 255
    // Default: n = 162
    if (height < 1) {
        return MUNBYN_ERROR_INVALID_PARAMETER;
    }
    
    uint8_t height_cmd[] = {GS, 0x68, height};
    
    munbyn_error_t result = munbyn_write_data(handle, height_cmd, sizeof(height_cmd));
    if (result == MUNBYN_OK) {
        handle->barcode_height = height;
    }
    
    return result;
}

munbyn_error_t munbyn_set_barcode_width(munbyn_handle_t handle, uint8_t width)
{
    if (!handle || !handle->initialized) {
        return MUNBYN_ERROR_INVALID_PARAMETER;
    }
    
    // GS w n - Set bar code width
    // n specifies the horizontal size of the bar code
    // Range: 2 ≤ n ≤ 6
    // Default: n = 3
    //
    // Multi-level bar codes: UPC-A, UPC-E, JAN13(EAN13), JAN8(EAN8), CODE93, CODE128
    // Binary-level bar codes: CODE39, ITF, CODABAR
    //
    // For multi-level: n = module width (mm)
    // For binary-level: n = thin element width (mm), thick = n * 2.5
    if (width < 2 || width > 6) {
        return MUNBYN_ERROR_INVALID_PARAMETER;
    }
    
    uint8_t width_cmd[] = {GS, 0x77, width};
    
    munbyn_error_t result = munbyn_write_data(handle, width_cmd, sizeof(width_cmd));
    if (result == MUNBYN_OK) {
        handle->barcode_width = width;
    }
    
    return result;
}

munbyn_error_t munbyn_set_hri_position(munbyn_handle_t handle, munbyn_hri_position_t position)
{
    if (!handle || !handle->initialized) {
        return MUNBYN_ERROR_INVALID_PARAMETER;
    }
    
    // GS H n - Select print position of HRI characters
    // Valid values: 0 ≤ n ≤ 3, 48 ≤ n ≤ 51
    // 0,48: Do not print
    // 1,49: Above the bar code (top of bar code)
    // 2,50: Below the bar code
    // 3,51: Both above and below the bar code
    if (!((position >= 0 && position <= 3) || (position >= 48 && position <= 51))) {
        return MUNBYN_ERROR_INVALID_PARAMETER;
    }
    
    uint8_t hri_cmd[] = {GS, 0x48, (uint8_t)position};
    
    munbyn_error_t result = munbyn_write_data(handle, hri_cmd, sizeof(hri_cmd));
    if (result == MUNBYN_OK) {
        handle->hri_position = position;
    }
    
    return result;
}

munbyn_error_t munbyn_set_hri_font(munbyn_handle_t handle, munbyn_hri_font_t font)
{
    if (!handle || !handle->initialized) {
        return MUNBYN_ERROR_INVALID_PARAMETER;
    }

    // GS f n - Select font for HRI characters used when printing a barcode
    // n = 0,48: Standard ASCII (12×24)
    // n = 1,49: Compressed ASCII (9×17)
    if (!(font == MUNBYN_HRI_FONT_STANDARD || font == MUNBYN_HRI_FONT_COMPRESSED ||
          font == MUNBYN_HRI_FONT_STANDARD_ALT || font == MUNBYN_HRI_FONT_COMPRESSED_ALT)) {
        return MUNBYN_ERROR_INVALID_PARAMETER;
    }

    uint8_t cmd[] = {GS, 0x66, (uint8_t)font};
    munbyn_error_t result = munbyn_write_data(handle, cmd, sizeof(cmd));
    if (result == MUNBYN_OK) {
        handle->hri_font = font;
    }
    return result;
}

munbyn_error_t munbyn_print_barcode(munbyn_handle_t handle, munbyn_barcode_t type, const char* data)
{
    if (!handle || !handle->initialized || !data) {
        return MUNBYN_ERROR_INVALID_PARAMETER;
    }
    
    size_t data_len = strlen(data);
    if (data_len == 0) {
        return MUNBYN_ERROR_INVALID_PARAMETER;
    }
    
    // Validate data length based on barcode type specifications from manual
    switch (type) {
        case MUNBYN_BARCODE_UPC_A:
            if (data_len != 11 && data_len != 12) {
                return MUNBYN_ERROR_INVALID_PARAMETER;
            }
            break;
            
        case MUNBYN_BARCODE_UPC_E:
            if (data_len != 11 && data_len != 12) {
                return MUNBYN_ERROR_INVALID_PARAMETER;
            }
            break;
            
        case MUNBYN_BARCODE_JAN13:  // EAN13
            if (data_len != 12 && data_len != 13) {
                return MUNBYN_ERROR_INVALID_PARAMETER;
            }
            break;
            
        case MUNBYN_BARCODE_JAN8:   // EAN8
            if (data_len != 7 && data_len != 8) {
                return MUNBYN_ERROR_INVALID_PARAMETER;
            }
            break;
            
        case MUNBYN_BARCODE_CODE39:
            if (data_len < 1 || data_len > 255) {
                return MUNBYN_ERROR_INVALID_PARAMETER;
            }
            break;
            
        case MUNBYN_BARCODE_ITF:
            // ITF requires even number of digits (even number length)
            if (data_len < 1 || data_len > 255 || (data_len % 2) != 0) {
                return MUNBYN_ERROR_INVALID_PARAMETER;
            }
            break;
            
        case MUNBYN_BARCODE_CODEBAR:
            if (data_len < 1 || data_len > 255) {
                return MUNBYN_ERROR_INVALID_PARAMETER;
            }
            break;
            
        case MUNBYN_BARCODE_CODE93:
            if (data_len < 1 || data_len > 255) {
                return MUNBYN_ERROR_INVALID_PARAMETER;
            }
            break;
            
        case MUNBYN_BARCODE_CODE128:
            if (data_len < 2 || data_len > 255) {
                return MUNBYN_ERROR_INVALID_PARAMETER;
            }
            break;
            
        default:
            return MUNBYN_ERROR_INVALID_PARAMETER;
    }
    
    uint8_t* cmd;
    size_t cmd_size;
    
    // CODE93 and CODE128 use method 2 (GS k m n d1...dn)
    // According to reference manual: method 2 for barcode types 65-73
    // All other barcodes use method 1 (GS k m d1...dk NUL)
    if (type == MUNBYN_BARCODE_CODE93 || type == MUNBYN_BARCODE_CODE128) {
        // Method 2: GS k m n d1...dn
        cmd_size = 4 + data_len;  // 3 (command) + 1 (length) + data_len
        cmd = malloc(cmd_size);
        if (!cmd) {
            return MUNBYN_ERROR_BUFFER_OVERFLOW;
        }
        
        cmd[0] = GS;
        cmd[1] = 0x6B;  // 'k'
        cmd[2] = (uint8_t)type;
        cmd[3] = (uint8_t)data_len;  // Length of data
        
        // Copy data (no NUL terminator for method 2)
        memcpy(&cmd[4], data, data_len);
    } else {
        // Method 1: GS k m d1...dk NUL
        cmd_size = 3 + data_len + 1;  // 3 (command) + data_len + 1 (NUL)
        cmd = malloc(cmd_size);
        if (!cmd) {
            return MUNBYN_ERROR_BUFFER_OVERFLOW;
        }
        
        cmd[0] = GS;
        cmd[1] = 0x6B;  // 'k'
        cmd[2] = (uint8_t)type;
        
        // Copy data
        memcpy(&cmd[3], data, data_len);
        
        // Add NUL terminator
        cmd[3 + data_len] = 0x00;
    }
    
    munbyn_error_t result = munbyn_write_data(handle, cmd, cmd_size);
    
    free(cmd);
    return result;
}