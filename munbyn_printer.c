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
    munbyn_codepage_t current_codepage;
    munbyn_international_charset_t current_charset;
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
        handle->current_justify = MUNBYN_JUSTIFY_LEFT;
        handle->barcode_height = 162; // Default height
        handle->barcode_width = 3;    // Default width
        handle->hri_position = MUNBYN_HRI_NONE;
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

    uint8_t cut_cmd[4];
    
    if (mode == MUNBYN_CUT_PARTIAL) {
        // GS V 1 - Partial cut (leaves 1 point uncut)
        cut_cmd[0] = GS;
        cut_cmd[1] = 0x56;
        cut_cmd[2] = 0x01;
        return munbyn_write_data(handle, cut_cmd, 3);
    } else if (mode == MUNBYN_CUT_FULL) {
        // GS V 0 - Full cut
        cut_cmd[0] = GS;
        cut_cmd[1] = 0x56;
        cut_cmd[2] = 0x00;
        return munbyn_write_data(handle, cut_cmd, 3);
    }

    return MUNBYN_ERROR_INVALID_PARAMETER;
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

munbyn_error_t munbyn_open_drawer(munbyn_handle_t handle, uint8_t pin)
{
    if (!handle || !handle->initialized) {
        return MUNBYN_ERROR_INVALID_PARAMETER;
    }

    if (pin != 0 && pin != 1) {
        return MUNBYN_ERROR_INVALID_PARAMETER;
    }

    // ESC p m t1 t2 - Generate pulse on pin
    // m: pin number (0 or 1)
    // t1: ON time (t1 * 2ms)
    // t2: OFF time (t2 * 2ms)
    uint8_t drawer_cmd[] = {ESC, 0x70, pin, 25, 250}; // 50ms ON, 500ms OFF
    
    return munbyn_write_data(handle, drawer_cmd, sizeof(drawer_cmd));
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

    // LF - Line Feed (move cursor down one line)
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

    // Validate justification parameter
    if (justify < MUNBYN_JUSTIFY_LEFT || justify > MUNBYN_JUSTIFY_RIGHT) {
        return MUNBYN_ERROR_INVALID_PARAMETER;
    }

    // ESC a n - Set justification
    // n = 0: Left justification
    // n = 1: Center justification  
    // n = 2: Right justification
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

    // Validate font parameter
    if (font < MUNBYN_FONT_A || font > MUNBYN_FONT_B) {
        return MUNBYN_ERROR_INVALID_PARAMETER;
    }

    // ESC M n - Select character font
    // n = 0: Font A (12×24)
    // n = 1: Font B (9×17)
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

    // Validate underline mode (0-2)
    if (mode > 2) {
        return MUNBYN_ERROR_INVALID_PARAMETER;
    }

    // ESC - n - Turn underline mode on/off
    // n = 0: Cancel underline mode
    // n = 1: Select underline mode (1-dot thick)
    // n = 2: Select underline mode (2-dot thick)
    uint8_t underline_cmd[] = {ESC, 0x2D, mode};
    
    munbyn_error_t result = munbyn_write_data(handle, underline_cmd, sizeof(underline_cmd));
    if (result == MUNBYN_OK) {
        if (mode > 0) {
            handle->current_modes |= MUNBYN_MODE_UNDERLINE;
        } else {
            handle->current_modes &= ~MUNBYN_MODE_UNDERLINE;
        }
    }
    
    return result;
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

    // ESC 3 n - Set line spacing to n/180 inch
    // n can be 0-255 (0-255/180 inch)
    uint8_t line_spacing_cmd[] = {ESC, 0x33, spacing};
    
    return munbyn_write_data(handle, line_spacing_cmd, sizeof(line_spacing_cmd));
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
    // Margin = (nL + nH × 256) × horizontal motion unit
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
    // Width = (nL + nH × 256) × horizontal motion unit
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
    // n = 0: Cancel 90° clockwise rotation mode
    // n = 1: Select 90° clockwise rotation mode
    uint8_t rotate_cmd[] = {ESC, 0x56, enabled ? 1 : 0};
    
    return munbyn_write_data(handle, rotate_cmd, sizeof(rotate_cmd));
}

munbyn_error_t munbyn_set_upside_down(munbyn_handle_t handle, bool enabled)
{
    if (!handle || !handle->initialized) {
        return MUNBYN_ERROR_INVALID_PARAMETER;
    }

    // ESC { n - Turn upside-down print mode on/off
    // n = 0: Cancel upside-down print mode
    // n = 1: Select upside-down print mode
    uint8_t upside_down_cmd[] = {ESC, 0x7B, enabled ? 1 : 0};
    
    return munbyn_write_data(handle, upside_down_cmd, sizeof(upside_down_cmd));
}

munbyn_error_t munbyn_set_character_smoothing(munbyn_handle_t handle, bool enabled)
{
    if (!handle || !handle->initialized) {
        return MUNBYN_ERROR_INVALID_PARAMETER;
    }

    // GS b n - Turn smoothing mode on/off
    // n = 0: Cancel smoothing mode  
    // n = 1: Select smoothing mode
    uint8_t smoothing_cmd[] = {GS, 0x62, enabled ? 1 : 0};
    
    return munbyn_write_data(handle, smoothing_cmd, sizeof(smoothing_cmd));
}