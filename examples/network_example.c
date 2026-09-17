#include "munbyn_printer.h"
#include <stdio.h>
#include <string.h>

int main() {
    munbyn_handle_t printer = NULL;
    munbyn_error_t result;
    munbyn_status_t status;
    
    printf("Testing Munbyn printer with network transport...\n");
    
    // Connect to network printer at 192.168.1.69:9100 with 5 second timeout
    printf("Connecting to network printer at 192.168.1.69:9100...\n");
    result = munbyn_open_network("192.168.1.69", 9100, 5000, &printer);
    if (result != MUNBYN_OK) {
        printf("Failed to open network printer: error %d\n", result);
        printf("Make sure:\n");
        printf("  1. The printer is powered on and connected to network\n");
        printf("  2. The IP address 192.168.1.69 is correct\n");
        printf("  3. Port 9100 is open on the printer\n");
        printf("  4. Your computer can reach the printer (try: ping 192.168.1.69)\n");
        return 1;
    }
    
    printf("Network printer connected successfully!\n");
    
    // Initialize the printer to ensure clean state
    printf("Initializing printer...\n");
    result = munbyn_initialize(printer);
    if (result != MUNBYN_OK) {
        printf("Failed to initialize printer: error %d\n", result);
    } else {
        printf("Printer initialized successfully!\n");
    }
    
    // Get printer status
    printf("Getting printer status...\n");
    result = munbyn_get_status(printer, &status);
    if (result == MUNBYN_OK) {
        printf("Printer Status:\n");
        printf("  Online: %s\n", status.online ? "Yes" : "No");
        printf("  Paper Present: %s\n", status.paper_present ? "Yes" : "No");
        printf("  Cover Closed: %s\n", status.cover_closed ? "Yes" : "No");
        printf("  Error Occurred: %s\n", status.error_occurred ? "Yes" : "No");
    } else {
        printf("Failed to get printer status: error %d\n", result);
    }
    
    // Test basic text printing
    printf("Printing test message...\n");
    const char test_data[] = "Hello from Network Munbyn printer!\nIP: 192.168.1.69 Port: 9100\n";
    result = munbyn_write_data(printer, (const uint8_t*)test_data, strlen(test_data));
    if (result != MUNBYN_OK) {
        printf("Failed to write data: error %d\n", result);
    } else {
        printf("Test message printed successfully!\n");
    }
    
    // Test formatting commands
    printf("Testing text formatting...\n");
    
    // Set center justification
    result = munbyn_set_justification(printer, MUNBYN_JUSTIFY_CENTER);
    if (result == MUNBYN_OK) {
        const char center_text[] = "CENTERED TEXT\n";
        munbyn_write_data(printer, (const uint8_t*)center_text, strlen(center_text));
    }
    
    // Set emphasis (bold)
    result = munbyn_set_emphasis(printer, true);
    if (result == MUNBYN_OK) {
        const char bold_text[] = "BOLD TEXT\n";
        munbyn_write_data(printer, (const uint8_t*)bold_text, strlen(bold_text));
    }
    
    // Reset to normal
    result = munbyn_set_emphasis(printer, false);
    result = munbyn_set_justification(printer, MUNBYN_JUSTIFY_LEFT);
    
    // Print a barcode if desired
    printf("Printing barcode...\n");
    result = munbyn_set_barcode_height(printer, 50);
    if (result == MUNBYN_OK) {
        result = munbyn_set_hri_position(printer, MUNBYN_HRI_BELOW);
        if (result == MUNBYN_OK) {
            result = munbyn_print_barcode(printer, MUNBYN_BARCODE_CODE128, "{BCode128");
            if (result == MUNBYN_OK) {
                printf("Barcode printed successfully!\n");
            }
        }
    }
    
    // Add some spacing and cut the paper
    printf("Feeding paper and cutting...\n");
    result = munbyn_feed_and_cut(printer, 7);  // Feed 7 lines and cut
    if (result != MUNBYN_OK) {
        printf("Failed to feed and cut: error %d\n", result);
    } else {
        printf("Paper cut successfully!\n");
    }
    
    // Clean up
    printf("Closing connection...\n");
    result = munbyn_close(printer);
    if (result != MUNBYN_OK) {
        printf("Failed to close printer: error %d\n", result);
        return 1;
    }
    
    printf("Network printer test completed successfully!\n");
    printf("\nTo use this in your own code:\n");
    printf("  munbyn_handle_t printer;\n");
    printf("  munbyn_open_network(\"192.168.1.69\", 9100, 5000, &printer);\n");
    printf("  munbyn_write_data(printer, data, length);\n");
    printf("  munbyn_close(printer);\n");
    
    return 0;
}
