/*
 * Comprehensive Munbyn Printer Barcode Example
 * 
 * This example demonstrates ALL supported barcode types, HRI positioning
 * options, and various configurations.
 */

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <time.h>
#include "munbyn_printer.h"

void print_error(const char* operation, munbyn_error_t error) {
    fprintf(stderr, "Error in %s: %d\n", operation, error);
}


void print_separator(munbyn_handle_t handle, const char* title) {
    char separator[64];
    snprintf(separator, sizeof(separator), "\n=== %s ===\n", title);
    munbyn_write_data(handle, (const uint8_t*)separator, strlen(separator));
    munbyn_line_feed(handle);
}

void demo_upc_a_barcode(munbyn_handle_t handle) {
    printf("Testing UPC-A barcode...\n");
    print_separator(handle, "UPC-A Demo");
    
    munbyn_set_barcode_height(handle, 100);
    munbyn_set_barcode_width(handle, 3);
    munbyn_set_hri_position(handle, MUNBYN_HRI_BELOW);
    
    const char* text = "UPC-A (12 digits)";
    munbyn_write_data(handle, (const uint8_t*)text, strlen(text));
    munbyn_line_feed(handle);
    
    // UPC-A requires 11 or 12 digits
    munbyn_error_t result = munbyn_print_barcode(handle, MUNBYN_BARCODE_UPC_A, "012345678905");
    if (result != MUNBYN_OK) {
        print_error("UPC-A barcode", result);
    }
    munbyn_line_feed(handle);
    munbyn_line_feed(handle);
    
    // Add delay to prevent paper misalignment
    sleep(1);  // 1 second delay
}

void demo_upc_e_barcode(munbyn_handle_t handle) {
    printf("Testing UPC-E barcode...\n");
    print_separator(handle, "UPC-E Demo");
    
    munbyn_set_hri_position(handle, MUNBYN_HRI_ABOVE);
    
    const char* text = "UPC-E (12 digits)";
    munbyn_write_data(handle, (const uint8_t*)text, strlen(text));
    munbyn_line_feed(handle);
    
    // UPC-E requires 11 or 12 digits (using valid UPC-E compressible pattern)
    munbyn_error_t result = munbyn_print_barcode(handle, MUNBYN_BARCODE_UPC_E, "042100005264");
    if (result != MUNBYN_OK) {
        print_error("UPC-E barcode", result);
    }
    munbyn_line_feed(handle);
    munbyn_line_feed(handle);
    sleep(1);  // 1 second delay
}

void demo_ean13_barcode(munbyn_handle_t handle) {
    printf("Testing EAN13 (JAN13) barcode...\n");
    print_separator(handle, "EAN13/JAN13 Demo");
    
    munbyn_set_hri_position(handle, MUNBYN_HRI_BELOW);
    
    const char* text = "EAN13/JAN13 (13 digits)";
    munbyn_write_data(handle, (const uint8_t*)text, strlen(text));
    munbyn_line_feed(handle);
    
    // EAN13 requires 12 or 13 digits (using valid check digit: 8)
    munbyn_error_t result = munbyn_print_barcode(handle, MUNBYN_BARCODE_JAN13, "1234567890128");
    if (result != MUNBYN_OK) {
        print_error("EAN13 barcode", result);
    }
    munbyn_line_feed(handle);
    munbyn_line_feed(handle);
    sleep(1);  // 1 second delay
}

void demo_ean8_barcode(munbyn_handle_t handle) {
    printf("Testing EAN8 (JAN8) barcode...\n");
    print_separator(handle, "EAN8/JAN8 Demo");
    
    munbyn_set_hri_position(handle, MUNBYN_HRI_BOTH);
    
    const char* text = "EAN8/JAN8 (8 digits)";
    munbyn_write_data(handle, (const uint8_t*)text, strlen(text));
    munbyn_line_feed(handle);
    
    // EAN8 requires 7 or 8 digits (using valid check digit: 0)
    munbyn_error_t result = munbyn_print_barcode(handle, MUNBYN_BARCODE_JAN8, "95200002");
    if (result != MUNBYN_OK) {
        print_error("EAN8 barcode", result);
    }
    munbyn_line_feed(handle);
    munbyn_line_feed(handle);
    sleep(1);  // 1 second delay
}

void demo_code39_barcode(munbyn_handle_t handle) {
    printf("Testing CODE39 barcode...\n");
    print_separator(handle, "CODE39 Demo");
    
    munbyn_set_barcode_height(handle, 80);
    munbyn_set_barcode_width(handle, 2);
    munbyn_set_hri_position(handle, MUNBYN_HRI_BELOW);
    
    const char* text = "CODE39 (Alphanumeric)";
    munbyn_write_data(handle, (const uint8_t*)text, strlen(text));
    munbyn_line_feed(handle);
    
    // CODE39 supports alphanumeric
    munbyn_error_t result = munbyn_print_barcode(handle, MUNBYN_BARCODE_CODE39, "HELLO123");
    if (result != MUNBYN_OK) {
        print_error("CODE39 barcode", result);
    }
    munbyn_line_feed(handle);
    munbyn_line_feed(handle);
    sleep(1);  // 1 second delay
}

void demo_itf_barcode(munbyn_handle_t handle) {
    printf("Testing ITF barcode...\n");
    print_separator(handle, "ITF Demo");
    
    munbyn_set_barcode_width(handle, 3);
    munbyn_set_hri_position(handle, MUNBYN_HRI_ABOVE);
    
    const char* text = "ITF (Even number of digits)";
    munbyn_write_data(handle, (const uint8_t*)text, strlen(text));
    munbyn_line_feed(handle);
    
    // ITF requires even number of digits
    munbyn_error_t result = munbyn_print_barcode(handle, MUNBYN_BARCODE_ITF, "123456");
    if (result != MUNBYN_OK) {
        print_error("ITF barcode", result);
    }
    munbyn_line_feed(handle);
    munbyn_line_feed(handle);
    sleep(1);  // 1 second delay
}

void demo_codabar_barcode(munbyn_handle_t handle) {
    printf("Testing CODABAR barcode...\n");
    print_separator(handle, "CODABAR Demo");
    
    munbyn_set_hri_position(handle, MUNBYN_HRI_BELOW);
    
    const char* text = "CODABAR (Library/Medical)";
    munbyn_write_data(handle, (const uint8_t*)text, strlen(text));
    munbyn_line_feed(handle);
    
    // CODABAR with industry-standard library pattern
    munbyn_error_t result = munbyn_print_barcode(handle, MUNBYN_BARCODE_CODEBAR, "A0123456789B");
    if (result != MUNBYN_OK) {
        print_error("CODABAR barcode", result);
    }
    munbyn_line_feed(handle);
    munbyn_line_feed(handle);
    sleep(1);  // 1 second delay
}

void demo_code93_barcode(munbyn_handle_t handle) {
    printf("Testing CODE93 barcode...\n");
    print_separator(handle, "CODE93 Demo");
    
    munbyn_set_barcode_height(handle, 120);
    munbyn_set_barcode_width(handle, 3);
    munbyn_set_hri_position(handle, MUNBYN_HRI_BELOW);
    
    const char* text = "CODE93 (Full ASCII)";
    munbyn_write_data(handle, (const uint8_t*)text, strlen(text));
    munbyn_line_feed(handle);
    
    // CODE93 supports full ASCII
    munbyn_error_t result = munbyn_print_barcode(handle, MUNBYN_BARCODE_CODE93, "CODE93-TEST");
    if (result != MUNBYN_OK) {
        print_error("CODE93 barcode", result);
    }
    munbyn_line_feed(handle);
    munbyn_line_feed(handle);
    sleep(1);  // 1 second delay
}

void demo_code128_barcode(munbyn_handle_t handle) {
    printf("Testing CODE128 barcode...\n");
    print_separator(handle, "CODE128 Demo");
    
    munbyn_set_barcode_height(handle, 100);
    munbyn_set_barcode_width(handle, 3);
    munbyn_set_hri_position(handle, MUNBYN_HRI_ABOVE);
    
    const char* text = "CODE128 (High Density)";
    munbyn_write_data(handle, (const uint8_t*)text, strlen(text));
    munbyn_line_feed(handle);
    
    // CODE128 - high density barcode
    printf("Printing CODE128 with data: MUNBYN128\n");
    munbyn_error_t result = munbyn_print_barcode(handle, MUNBYN_BARCODE_CODE128, "MUNBYN128");
    if (result != MUNBYN_OK) {
        print_error("CODE128 barcode", result);
        printf("CODE128 error code: %d\n", result);
    } else {
        printf("CODE128 printed successfully!\n");
    }
    munbyn_line_feed(handle);
    munbyn_line_feed(handle);
    sleep(1);  // 1 second delay
}

void demo_hri_variations(munbyn_handle_t handle) {
    printf("Testing HRI position variations...\n");
    print_separator(handle, "HRI Position Variations");
    
    munbyn_set_barcode_height(handle, 80);
    munbyn_set_barcode_width(handle, 3);
    
    // Test standard HRI positions
    const char* positions[] = {"NONE", "ABOVE", "BELOW", "BOTH"};
    munbyn_hri_position_t hri_values[] = {
        MUNBYN_HRI_NONE, MUNBYN_HRI_ABOVE, MUNBYN_HRI_BELOW, MUNBYN_HRI_BOTH
    };
    
    for (int i = 0; i < 4; i++) {
        char label[64];
        snprintf(label, sizeof(label), "HRI %s:", positions[i]);
        munbyn_write_data(handle, (const uint8_t*)label, strlen(label));
        munbyn_line_feed(handle);
        
        munbyn_set_hri_position(handle, hri_values[i]);
        munbyn_print_barcode(handle, MUNBYN_BARCODE_CODE39, "TEST123");
        munbyn_line_feed(handle);
        munbyn_line_feed(handle);
        sleep(1);  // 1 second delay
    }
    
    // Test alternative HRI positions
    printf("Testing alternative HRI values...\n");
    munbyn_write_data(handle, (const uint8_t*)"HRI ALT BELOW (50):", 19);
    munbyn_line_feed(handle);
    munbyn_set_hri_position(handle, MUNBYN_HRI_BELOW_ALT);
    munbyn_print_barcode(handle, MUNBYN_BARCODE_CODE39, "ALT50");
    munbyn_line_feed(handle);
    munbyn_line_feed(handle);
    sleep(1);  // 1 second delay
}

void demo_size_variations(munbyn_handle_t handle) {
    printf("Testing size variations...\n");
    print_separator(handle, "Size Variations");
    
    munbyn_set_hri_position(handle, MUNBYN_HRI_BELOW);
    
    // Adjusted for 80mm paper - removed XLarge
    const char* sizes[] = {"Small", "Medium", "Large"};
    uint8_t heights[] = {50, 100, 150};
    uint8_t widths[] = {2, 3, 4};  // Max width 4 for 80mm paper
    
    for (int i = 0; i < 3; i++) {  // Only 3 sizes now
        char label[64];
        snprintf(label, sizeof(label), "%s (H:%d W:%d):", sizes[i], heights[i], widths[i]);
        munbyn_write_data(handle, (const uint8_t*)label, strlen(label));
        munbyn_line_feed(handle);
        
        munbyn_set_barcode_height(handle, heights[i]);
        munbyn_set_barcode_width(handle, widths[i]);
        munbyn_print_barcode(handle, MUNBYN_BARCODE_CODE39, sizes[i]);
        munbyn_line_feed(handle);
        munbyn_line_feed(handle);
        sleep(1);  // 1 second delay
    }
}

int main(int argc, char* argv[]) {
    printf("Comprehensive Munbyn Printer Barcode Test\n");
    printf("==========================================\n\n");
    
    // Default device path - can be overridden with command line argument
    const char* device_path = (argc > 1) ? argv[1] : "/dev/usb/lp0";
    
    printf("Connecting to printer at: %s\n", device_path);
    printf("(Use: %s /path/to/device to specify different device)\n\n", argv[0]);
    
    munbyn_handle_t handle;
    munbyn_error_t result;
    
    // Open printer connection
    result = munbyn_open_usb(device_path, &handle);
    if (result != MUNBYN_OK) {
        print_error("open_usb", result);
        printf("\nTip: Make sure the printer is connected and you have the correct device path.\n");
        printf("Common paths: /dev/usb/lp0, /dev/usb/lp1, /dev/ttyUSB0, /dev/ttyACM0\n");
        return 1;
    }
    
    printf("✓ Connected to printer successfully!\n\n");
    
    // Initialize printer
    result = munbyn_initialize(handle);
    if (result != MUNBYN_OK) {
        print_error("initialize", result);
        munbyn_close(handle);
        return 1;
    }
    
    printf("✓ Printer initialized!\n\n");
    
    // Print header
    const char* header = "COMPREHENSIVE BARCODE TEST\n";
    munbyn_write_data(handle, (const uint8_t*)header, strlen(header));
    munbyn_line_feed(handle);
    
    // Test all barcode types
    demo_upc_a_barcode(handle);
    demo_upc_e_barcode(handle);
    demo_ean13_barcode(handle);
    demo_ean8_barcode(handle);
    demo_code39_barcode(handle);
    demo_itf_barcode(handle);
    demo_codabar_barcode(handle);
    demo_code93_barcode(handle);
    demo_code128_barcode(handle);
    
    // Test HRI and size variations
    demo_hri_variations(handle);
    demo_size_variations(handle);
    
    // Print completion message
    print_separator(handle, "Test Complete");
    const char* completion_msg = "All barcode types tested!\nCheck each barcode carefully.\n";
    munbyn_write_data(handle, (const uint8_t*)completion_msg, strlen(completion_msg));
    
    // Feed and cut paper
    result = munbyn_feed_and_cut(handle, 7);
    if (result != MUNBYN_OK) {
        print_error("feed_and_cut", result);
    }
    
    // Close printer connection
    result = munbyn_close(handle);
    if (result != MUNBYN_OK) {
        print_error("close", result);
        return 1;
    }
    
    printf("✓ Comprehensive barcode test completed successfully!\n");
    printf("Check your printed receipt for all barcode types and variations.\n");
    
    return 0;
}
