#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include "munbyn_printer.h"

int main(void)
{
    munbyn_handle_t printer;
    munbyn_error_t result;
    
    // Try to open USB printer (adjust path as needed)
    result = munbyn_open_usb("/dev/usb/lp0", &printer);
    if (result != MUNBYN_OK) {
        printf("Failed to open printer: %d\n", result);
        printf("Note: Make sure printer is connected and path is correct\n");
        return 1;
    }

    printf("Printer opened successfully!\n");
    
    // Initialize printer
    result = munbyn_initialize(printer);
    if (result != MUNBYN_OK) {
        printf("Failed to initialize printer: %d\n", result);
        munbyn_close(printer);
        return 1;
    }
    
    printf("Testing new character set and formatting commands...\n");
    
    // Test basic text formatting commands
    const char* test_text = "Testing character formatting:";
    munbyn_write_data(printer, (const uint8_t*)test_text, strlen(test_text));
    munbyn_line_feed(printer);
    munbyn_line_feed(printer);
    
    // Explicit conversion using manual selectors. Confirm these on the unit's sheet.
    printf("Testing PC437 codepage...\n");
    result = munbyn_print_encoded(printer, "PC437: àáâäåæçèéêë",
                                  MUNBYN_ENCODING_CP437, MUNBYN_MANUAL_CODEPAGE_CP437);
    if (result != MUNBYN_OK) { munbyn_close(printer); return 1; }
    munbyn_carriage_return(printer);
    munbyn_line_feed(printer);
    
    printf("Testing PC850 codepage...\n");
    result = munbyn_print_encoded(printer, "PC850: àáâãäåæçèéêë",
                                  MUNBYN_ENCODING_CP850, MUNBYN_MANUAL_CODEPAGE_CP850);
    if (result != MUNBYN_OK) { munbyn_close(printer); return 1; }
    munbyn_line_feed(printer);
    
    // Test international character sets
    printf("Testing USA international charset...\n");
    munbyn_set_international_charset(printer, MUNBYN_INTL_USA);
    const char* usa_text = "USA charset: $#@";
    munbyn_write_data(printer, (const uint8_t*)usa_text, strlen(usa_text));
    munbyn_line_feed(printer);
    
    printf("Testing German international charset...\n");
    munbyn_set_international_charset(printer, MUNBYN_INTL_GERMANY);
    const char* german_text = "German charset test";
    munbyn_write_data(printer, (const uint8_t*)german_text, strlen(german_text));
    munbyn_line_feed(printer);
    
    // Display current codepage info
    printf("Current selector: %d (manual CP850; confirm on the printer sheet)\n",
           MUNBYN_MANUAL_CODEPAGE_CP850);
    
    // Test formatting with new commands
    munbyn_line_feed(printer);
    const char* format_demo = "Format Demo:";
    munbyn_write_data(printer, (const uint8_t*)format_demo, strlen(format_demo));
    munbyn_line_feed(printer);
    
    const char* line1 = "Line 1";
    munbyn_write_data(printer, (const uint8_t*)line1, strlen(line1));
    munbyn_carriage_return(printer); // Return to start of line
    const char* overwrite = "OVERWRITTEN";
    munbyn_write_data(printer, (const uint8_t*)overwrite, strlen(overwrite));
    munbyn_line_feed(printer);
    
    const char* line2 = "Line 2 with LF";
    munbyn_write_data(printer, (const uint8_t*)line2, strlen(line2));
    munbyn_line_feed(printer);
    
    // Feed some lines and cut
    munbyn_feed_lines(printer, MUNBYN_OPTIMAL_FEED_LINES);
    munbyn_cut_paper(printer, MUNBYN_CUT_PARTIAL);
    
    // Close printer
    munbyn_close(printer);
    printf("Test completed successfully!\n");
    
    return 0;
}
