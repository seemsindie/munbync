/*
 * Munbyn Printer Barcode Example
 * 
 * This example demonstrates barcode printing functionality
 * including different barcode types, HRI positioning, and
 * height/width configuration.
 */

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include "munbyn_printer.h"

void print_error(const char* operation, munbyn_error_t error) {
    fprintf(stderr, "Error in %s: %d\n", operation, error);
}

void demo_barcode_configuration(munbyn_handle_t handle) {
    printf("=== Barcode Configuration Demo ===\n");
    
    munbyn_error_t result;
    
    // Set barcode height (default is 162 dots)
    printf("Setting barcode height to 100 dots...\n");
    result = munbyn_set_barcode_height(handle, 100);
    if (result != MUNBYN_OK) {
        print_error("set_barcode_height", result);
        return;
    }
    
    // Set barcode width (default is 3)
    printf("Setting barcode width to 2 dots...\n");
    result = munbyn_set_barcode_width(handle, 2);
    if (result != MUNBYN_OK) {
        print_error("set_barcode_width", result);
        return;
    }
    
    // Set HRI position to below barcode
    printf("Setting HRI position to below barcode...\n");
    result = munbyn_set_hri_position(handle, MUNBYN_HRI_BELOW);
    if (result != MUNBYN_OK) {
        print_error("set_hri_position", result);
        return;
    }
    
    // Set HRI font to compressed for denser labels
    printf("Setting HRI font to compressed...\n");
    result = munbyn_set_hri_font(handle, MUNBYN_HRI_FONT_COMPRESSED);
    if (result != MUNBYN_OK) {
        print_error("set_hri_font", result);
        return;
    }
    
    printf("Configuration complete.\n\n");
}

void demo_code39_barcode(munbyn_handle_t handle) {
    printf("=== CODE39 Barcode Demo ===\n");
    
    const char* text = "CODE39 Demo";
    
    // Print regular text
    munbyn_error_t result = munbyn_write_data(handle, (const uint8_t*)text, strlen(text));
    if (result != MUNBYN_OK) {
        print_error("write text", result);
        return;
    }
    
    result = munbyn_line_feed(handle);
    if (result != MUNBYN_OK) {
        print_error("line_feed", result);
        return;
    }
    
    // Print CODE39 barcode
    printf("Printing CODE39 barcode: HELLO123\n");
    result = munbyn_print_barcode(handle, MUNBYN_BARCODE_CODE39, "HELLO123");
    if (result != MUNBYN_OK) {
        print_error("print_barcode CODE39", result);
        return;
    }
    
    // Add some spacing
    result = munbyn_line_feed(handle);
    if (result != MUNBYN_OK) {
        print_error("line_feed", result);
        return;
    }
    result = munbyn_line_feed(handle);
    if (result != MUNBYN_OK) {
        print_error("line_feed", result);
        return;
    }
}

void demo_ean13_barcode(munbyn_handle_t handle) {
    printf("=== EAN13 (JAN13) Barcode Demo ===\n");
    
    const char* text = "EAN13 Demo";
    
    // Print regular text
    munbyn_error_t result = munbyn_write_data(handle, (const uint8_t*)text, strlen(text));
    if (result != MUNBYN_OK) {
        print_error("write text", result);
        return;
    }
    
    result = munbyn_line_feed(handle);
    if (result != MUNBYN_OK) {
        print_error("line_feed", result);
        return;
    }
    
    // Print EAN13 barcode (13 digits with valid check digit)
    printf("Printing EAN13 barcode: 1234567890128\n");
    result = munbyn_print_barcode(handle, MUNBYN_BARCODE_JAN13, "1234567890128");
    if (result != MUNBYN_OK) {
        print_error("print_barcode EAN13", result);
        return;
    }
    
    // Add some spacing
    result = munbyn_line_feed(handle);
    if (result != MUNBYN_OK) {
        print_error("line_feed", result);
        return;
    }
    result = munbyn_line_feed(handle);
    if (result != MUNBYN_OK) {
        print_error("line_feed", result);
        return;
    }
}

void demo_code128_barcode(munbyn_handle_t handle) {
    printf("=== CODE128 Barcode Demo ===\n");
    
    const char* text = "CODE128 Demo";
    
    // Set HRI position to above for variety
    munbyn_error_t result = munbyn_set_hri_position(handle, MUNBYN_HRI_ABOVE);
    if (result != MUNBYN_OK) {
        print_error("set_hri_position", result);
        return;
    }

     // Set HRI font to standard for variety
     printf("Setting HRI font to compressed...\n");
     result = munbyn_set_hri_font(handle, MUNBYN_HRI_FONT_STANDARD);
     if (result != MUNBYN_OK) {
         print_error("set_hri_font", result);
         return;
     }
    
    // Print regular text
    result = munbyn_write_data(handle, (const uint8_t*)text, strlen(text));
    if (result != MUNBYN_OK) {
        print_error("write text", result);
        return;
    }
    
    result = munbyn_line_feed(handle);
    if (result != MUNBYN_OK) {
        print_error("line_feed", result);
        return;
    }
    
    // Print CODE128 barcode
    printf("Printing CODE128 barcode: MunbynPrinter2024\n");
    result = munbyn_print_barcode(handle, MUNBYN_BARCODE_CODE128, "{BMunbynPrinter2024");
    if (result != MUNBYN_OK) {
        print_error("print_barcode CODE128", result);
        return;
    }
    
    // Add some spacing
    result = munbyn_line_feed(handle);
    if (result != MUNBYN_OK) {
        print_error("line_feed", result);
        return;
    }
    result = munbyn_line_feed(handle);
    if (result != MUNBYN_OK) {
        print_error("line_feed", result);
        return;
    }
}

void demo_codabar_barcode(munbyn_handle_t handle) {
    printf("=== CODABAR Barcode Demo ===\n");
    
    const char* text = "CODABAR Demo (Library/Medical)";
    
    // Print regular text
    munbyn_error_t result = munbyn_write_data(handle, (const uint8_t*)text, strlen(text));
    if (result != MUNBYN_OK) {
        print_error("write text", result);
        return;
    }
    
    result = munbyn_line_feed(handle);
    if (result != MUNBYN_OK) {
        print_error("line_feed", result);
        return;
    }
    
    // Print CODABAR barcode with industry-standard pattern
    printf("Printing CODABAR barcode: A0123456789B\n");
    result = munbyn_print_barcode(handle, MUNBYN_BARCODE_CODEBAR, "A0123456789B");
    if (result != MUNBYN_OK) {
        print_error("print_barcode CODABAR", result);
        return;
    }
    
    // Add some spacing
    result = munbyn_line_feed(handle);
    if (result != MUNBYN_OK) {
        print_error("line_feed", result);
        return;
    }
    result = munbyn_line_feed(handle);
    if (result != MUNBYN_OK) {
        print_error("line_feed", result);
        return;
    }
}

void demo_different_sizes(munbyn_handle_t handle) {
    printf("=== Different Barcode Sizes Demo ===\n");
    
    const char* text = "Size Variations";
    
    // Print regular text
    munbyn_error_t result = munbyn_write_data(handle, (const uint8_t*)text, strlen(text));
    if (result != MUNBYN_OK) {
        print_error("write text", result);
        return;
    }
    
    result = munbyn_line_feed(handle);
    if (result != MUNBYN_OK) {
        print_error("line_feed", result);
        return;
    }
    
    // Small barcode
    printf("Printing small barcode (height: 50, width: 2)...\n");
    result = munbyn_set_barcode_height(handle, 50);
    if (result != MUNBYN_OK) {
        print_error("set_barcode_height", result);
        return;
    }
    
    result = munbyn_set_barcode_width(handle, 2);
    if (result != MUNBYN_OK) {
        print_error("set_barcode_width", result);
        return;
    }
    
    result = munbyn_set_hri_position(handle, MUNBYN_HRI_BELOW);
    if (result != MUNBYN_OK) {
        print_error("set_hri_position", result);
        return;
    }
    
    result = munbyn_print_barcode(handle, MUNBYN_BARCODE_CODE39, "SMALL");
    if (result != MUNBYN_OK) {
        print_error("print_barcode small", result);
        return;
    }
    
    result = munbyn_line_feed(handle);
    if (result != MUNBYN_OK) {
        print_error("line_feed", result);
        return;
    }
    
    // Large barcode
    printf("Printing large barcode (height: 200, width: 5)...\n");
    result = munbyn_set_barcode_height(handle, 200);
    if (result != MUNBYN_OK) {
        print_error("set_barcode_height", result);
        return;
    }
    
    result = munbyn_set_barcode_width(handle, 5);
    if (result != MUNBYN_OK) {
        print_error("set_barcode_width", result);
        return;
    }
    
    result = munbyn_print_barcode(handle, MUNBYN_BARCODE_CODE39, "LARGE");
    if (result != MUNBYN_OK) {
        print_error("print_barcode large", result);
        return;
    }
    
    result = munbyn_line_feed(handle);
    if (result != MUNBYN_OK) {
        print_error("line_feed", result);
        return;
    }
    result = munbyn_line_feed(handle);
    if (result != MUNBYN_OK) {
        print_error("line_feed", result);
        return;
    }
}

int main(int argc, char* argv[]) {
    printf("Munbyn Printer Barcode Example\n");
    printf("==============================\n\n");
    
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
    
    // Run barcode demonstrations
    demo_barcode_configuration(handle);
    demo_code39_barcode(handle);
    demo_ean13_barcode(handle);
    demo_code128_barcode(handle);
    demo_codabar_barcode(handle);
    // demo_different_sizes(handle);
    
    // Print completion message
    const char* completion_msg = "\n=== Barcode Demo Complete ===\n";
    result = munbyn_write_data(handle, (const uint8_t*)completion_msg, strlen(completion_msg));
    if (result != MUNBYN_OK) {
        print_error("write completion message", result);
    }
    
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
    
    printf("✓ Barcode demonstration completed successfully!\n");
    printf("Check your printed receipt for the various barcode examples.\n");
    
    return 0;
}
