#include "munbyn_printer.h"
#include <stdio.h>
#include <unistd.h>

int main() {
    munbyn_handle_t printer = NULL;
    munbyn_error_t result;
    munbyn_status_t status;

    printf("MUNBYN ITPP047 Self-Test Example\n");
    printf("================================\n\n");

    // Try different transport methods in order of preference

    // Method 1: Try USB connection
    printf("Attempting USB connection to /dev/usb/lp0...\n");
    result = munbyn_open_usb("/dev/usb/lp0", &printer);

    if (result != MUNBYN_OK) {
        printf("USB connection failed. Trying serial connection...\n");

        // Method 2: Try serial connection (common USB-to-serial devices)
        const char* serial_ports[] = {
            "/dev/ttyUSB0",
            "/dev/ttyACM0",
            "/dev/ttyS0",
            NULL
        };

        for (int i = 0; serial_ports[i] != NULL && result != MUNBYN_OK; i++) {
            printf("Attempting serial connection to %s...\n", serial_ports[i]);
            result = munbyn_open_serial(serial_ports[i], 9600, &printer);
        }
    }

    if (result != MUNBYN_OK) {
        printf("Failed to open printer connection. Please check:\n");
        printf("  - Printer is connected and powered on\n");
        printf("  - USB/Serial device path is correct\n");
        printf("  - User has permission to access the device\n");
        printf("  - Driver is properly installed\n");
        return 1;
    }

    printf("Printer connection established successfully!\n\n");

    // Initialize the printer to ensure clean state
    printf("Initializing printer...\n");
    result = munbyn_initialize(printer);
    if (result != MUNBYN_OK) {
        printf("Failed to initialize printer: error %d\n", result);
        munbyn_close(printer);
        return 1;
    }
    printf("Printer initialized successfully!\n\n");

    // Get printer status before self-test
    printf("Checking printer status before self-test...\n");
    result = munbyn_get_status(printer, &status);
    if (result == MUNBYN_OK) {
        printf("Printer Status:\n");
        printf("  Online: %s\n", status.online ? "Yes" : "No");
        printf("  Paper Present: %s\n", status.paper_present ? "Yes" : "No");
        printf("  Cover Closed: %s\n", status.cover_closed ? "Yes" : "No");
        printf("  Error Occurred: %s\n", status.error_occurred ? "Yes" : "No");

        if (!status.online || !status.paper_present || !status.cover_closed || status.error_occurred) {
            printf("\nWarning: Printer is not in optimal state for self-test.\n");
            printf("Please ensure paper is loaded and cover is closed.\n\n");
        }
    } else {
        printf("Failed to get printer status: error %d\n\n", result);
    }

    // Perform self-test
    printf("Executing printer self-test...\n");
    printf("Command: Proprietary 0x1F 0x1B 0x1F 0x67 (Unit Separator + ESC + Unit Separator + 'g')\n");
    printf("\nNote: The self-test will print diagnostic information including:\n");
    printf("  - Character set samples\n");
    printf("  - Print quality test patterns\n");
    printf("  - Font samples\n");
    printf("  - Internal configuration info\n\n");

    result = munbyn_self_test(printer);
    if (result != MUNBYN_OK) {
        printf("Self-test command failed: error %d\n", result);
        printf("This could indicate:\n");
        printf("  - Communication error with printer\n");
        printf("  - Firmware doesn't support this command\n");
        printf("  - Printer is in error state\n");
    } else {
        printf("Self-test command sent successfully!\n");
        printf("Check the printed output for test results.\n");

        // Wait a moment for the self-test to complete
        printf("Waiting for self-test to complete (10 seconds)...\n");
        sleep(10);

        // Check status after self-test
        printf("\nChecking printer status after self-test...\n");
        result = munbyn_get_status(printer, &status);
        if (result == MUNBYN_OK) {
            printf("Post-test Status:\n");
            printf("  Online: %s\n", status.online ? "Yes" : "No");
            printf("  Paper Present: %s\n", status.paper_present ? "Yes" : "No");
            printf("  Cover Closed: %s\n", status.cover_closed ? "Yes" : "No");
            printf("  Error Occurred: %s\n", status.error_occurred ? "Yes" : "No");

            if (status.error_occurred) {
                printf("\nWarning: Error detected after self-test.\n");
            } else {
                printf("\nSelf-test completed without errors.\n");
            }
        }
    }

    // Feed some paper to separate self-test output from next print job
    printf("\nFeeding paper to separate output...\n");
    result = munbyn_feed_lines(printer, 3);
    if (result == MUNBYN_OK) {
        printf("Paper fed successfully.\n");
    }

    // Clean shutdown
    printf("\nClosing printer connection...\n");
    munbyn_close(printer);
    printf("Connection closed.\n");

    printf("\nSelf-test example completed!\n");
    printf("==========================\n");

    return 0;
}
