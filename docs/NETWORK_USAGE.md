# Network Printer Support for Munbyn Library

## Overview

The Munbyn printer library now supports network connectivity, allowing you to connect to printers over TCP/IP using standard network printer ports (typically port 9100).

## Quick Start

### Basic Usage

```c
#include "munbyn_printer.h"

int main() {
    munbyn_handle_t printer;
    munbyn_error_t result;
    
    // Connect to network printer at 192.168.1.69:9100 with 5 second timeout
    result = munbyn_open_network("192.168.1.69", 9100, 5000, &printer);
    if (result != MUNBYN_OK) {
        printf("Failed to connect to printer: error %d\n", result);
        return 1;
    }
    
    // Initialize printer
    munbyn_initialize(printer);
    
    // Print some text
    const char* text = "Hello Network Printer!\n";
    munbyn_write_data(printer, (const uint8_t*)text, strlen(text));
    
    // Cut paper
    munbyn_feed_and_cut(printer, 7);
    
    // Clean up
    munbyn_close(printer);
    return 0;
}
```

### Building with Network Support

The library automatically includes network support. No special compilation flags are needed on Linux.

```bash
make clean
make library
make build/examples/network_example
```

## API Reference

### munbyn_open_network()

```c
munbyn_error_t munbyn_open_network(const char* ip_address, int port, int timeout_ms, munbyn_handle_t* handle);
```

**Parameters:**
- `ip_address`: IP address of the printer (e.g., "192.168.1.69")
- `port`: TCP port number (typically 9100 for network printers)
- `timeout_ms`: Connection timeout in milliseconds (0 for no timeout)
- `handle`: Pointer to receive the printer handle

**Returns:**
- `MUNBYN_OK` on success
- `MUNBYN_ERROR_INVALID_PARAMETER` for invalid parameters
- `MUNBYN_ERROR_COMMUNICATION` for network connection failures

**Example:**
```c
munbyn_handle_t printer;
munbyn_error_t result = munbyn_open_network("192.168.1.69", 9100, 5000, &printer);
```

### Generic Network Connection

You can also use the generic `munbyn_open()` function:

```c
munbyn_connection_params_t params = {0};
params.type = MUNBYN_CONNECTION_NETWORK;
strncpy(params.config.network.ip_address, "192.168.1.69", sizeof(params.config.network.ip_address) - 1);
params.config.network.port = 9100;
params.config.network.timeout_ms = 5000;

munbyn_handle_t printer;
munbyn_error_t result = munbyn_open(&params, &printer);
```

## Network Configuration

### Finding Your Printer's IP Address

1. **Printer Settings Menu**: Check the network settings on your printer's display
2. **Router Admin Page**: Look for connected devices in your router's web interface
3. **Network Discovery**: Use tools like `nmap` to scan your network:
   ```bash
   nmap -p 9100 192.168.1.0/24
   ```

### Testing Connectivity

Before using the library, test basic connectivity:

```bash
# Test if printer responds on port 9100
telnet 192.168.1.69 9100

# Test network reachability
ping 192.168.1.69

# Check if port is open
nc -zv 192.168.1.69 9100
```

### Common Port Numbers

- **9100**: Standard network printer port (most common)
- **631**: IPP (Internet Printing Protocol)
- **515**: LPR/LPD (Line Printer Remote/Line Printer Daemon)

## Error Handling

```c
munbyn_error_t result = munbyn_open_network("192.168.1.69", 9100, 5000, &printer);

switch (result) {
    case MUNBYN_OK:
        printf("Connected successfully!\n");
        break;
    case MUNBYN_ERROR_INVALID_PARAMETER:
        printf("Invalid IP address or port\n");
        break;
    case MUNBYN_ERROR_COMMUNICATION:
        printf("Network connection failed - check IP/port/connectivity\n");
        break;
    default:
        printf("Unknown error: %d\n", result);
        break;
}
```

## Troubleshooting

### Connection Failures

1. **Verify printer IP address**: Check printer display or network settings
2. **Check network connectivity**: `ping 192.168.1.69`
3. **Verify port is open**: `nc -zv 192.168.1.69 9100`
4. **Firewall settings**: Ensure outbound connections on port 9100 are allowed
5. **Printer network settings**: Ensure printer is configured for network printing

### Timeout Issues

- Increase timeout value in `munbyn_open_network()`
- Check network latency and reliability
- Verify printer is not overloaded with other jobs

### Print Quality Issues

Network printing uses the same ESC/POS commands as USB/serial connections. If you experience issues:

1. Check printer status: `munbyn_get_status()`
2. Verify paper is loaded and cover is closed
3. Test with simple text before complex formatting

## Example Programs

### Basic Network Test
```bash
./build/examples/network_example
```

### Custom IP/Port
Modify `examples/network_example.c` to use different IP addresses or ports.

## Integration Notes

- All existing `munbyn_*` functions work identically with network connections
- Network connections support the same formatting, barcode, and image printing features
- The library handles network-specific socket operations transparently
- Use `munbyn_close()` to properly close network connections

## Performance Considerations

- Network printing may have slightly higher latency than USB/serial
- Large raster images may take longer to transmit over network
- Consider timeout values appropriate for your network environment
- Multiple concurrent connections to the same printer may cause conflicts