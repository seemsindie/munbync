#include "munbyn_printer.h"
#include <stdio.h>
#include <string.h>

int main() {
    munbyn_handle_t printer = NULL;
    munbyn_error_t result;
    munbyn_status_t status;
    
    printf("Testing Munbyn printer with USB transport...\n");
    
    // Method 1: Using the convenience function
    result = munbyn_open_usb("/dev/usb/lp0", &printer);
    if (result != MUNBYN_OK) {
        printf("Failed to open USB printer: error %d\n", result);
        return 1;
    }
    
    printf("USB printer opened successfully!\n");
    
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
    
    // Test writing some data
    const char test_data[] = "Hello from Munbyn printer!\nTesting basic functionality\n";
    printf("Printing test message...\n");
    result = munbyn_write_data(printer, (const uint8_t*)test_data, strlen(test_data));
    if (result != MUNBYN_OK) {
        printf("Failed to write data: error %d\n", result);
    } else {
        printf("Data written successfully!\n");
    }
    
    // Feed lines to position paper above cutter (optimal setting from testing)
    printf("Feeding %d lines to position paper for cutting...\n", MUNBYN_OPTIMAL_FEED_LINES);
    result = munbyn_feed_lines(printer, MUNBYN_OPTIMAL_FEED_LINES);
    if (result != MUNBYN_OK) {
        printf("Failed to feed lines: error %d\n", result);
    } else {
        printf("Paper positioned successfully!\n");
    }
    
    // Test partial cut
    printf("Performing partial cut...\n");
    result = munbyn_cut_paper(printer, MUNBYN_CUT_PARTIAL);
    if (result != MUNBYN_OK) {
        printf("Failed to cut paper: error %d\n", result);
    } else {
        printf("Partial cut performed successfully!\n");
    }
    
    // Test cash drawer (pin 0)
    printf("Opening cash drawer (pin 0)...\n");
    result = munbyn_open_drawer(printer, 0);
    if (result != MUNBYN_OK) {
        printf("Failed to open drawer: error %d\n", result);
    } else {
        printf("Drawer opened successfully!\n");
    }
    
    // Close the printer
    result = munbyn_close(printer);
    if (result != MUNBYN_OK) {
        printf("Failed to close printer: error %d\n", result);
    } else {
        printf("Printer closed successfully.\n");
    }
    
    printf("\n--- Testing with connection params structure ---\n");
    
    // Method 2: Using the general open function with params
    munbyn_connection_params_t params = {0};
    params.type = MUNBYN_CONNECTION_USB;
    strncpy(params.config.usb.device_path, "/dev/usb/lp0", 
            sizeof(params.config.usb.device_path) - 1);
    
    result = munbyn_open(&params, &printer);
    if (result != MUNBYN_OK) {
        printf("Failed to open USB printer with params: error %d\n", result);
        return 1;
    }
    
    printf("USB printer opened with params successfully!\n");
    
    // Test another partial cut with different text
    const char test_data2[] = "Second test message\nConnection params method\n";
    munbyn_write_data(printer, (const uint8_t*)test_data2, strlen(test_data2));
    munbyn_feed_lines(printer, MUNBYN_OPTIMAL_FEED_LINES);  // Using optimal feed lines
    
    printf("Performing another partial cut...\n");
    result = munbyn_cut_paper(printer, MUNBYN_CUT_PARTIAL);
    if (result != MUNBYN_OK) {
        printf("Failed to perform partial cut: error %d\n", result);
    } else {
        printf("Partial cut performed successfully!\n");
    }
    
    // Close again
    munbyn_close(printer);
    printf("Printer closed.\n");
    
    return 0;
}