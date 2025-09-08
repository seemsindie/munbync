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