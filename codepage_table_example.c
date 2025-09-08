#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include "munbyn_printer.h"

// Function to print a character table for a specific codepage
munbyn_error_t print_codepage_table(munbyn_handle_t printer, munbyn_codepage_t codepage_number, 
                                   bool show_reset_message, int feed_lines_before_cut)
{
    munbyn_error_t result;
    
    // Initialize printer to ensure clean state
    result = munbyn_initialize(printer);
    if (result != MUNBYN_OK) return result;
    
    // Header information
    const char* header[] = {
        "Empty space = space or non-printable character.\n",
        "Each row represents the first hex digit.\n", 
        "Each column represents the second hex digit.\n",
        "For example, row 7, column A, represents\n",
        "the character with hex 0x7A.\n"
    };
    
    for (int i = 0; i < 5; i++) {
        result = munbyn_write_data(printer, (const uint8_t*)header[i], strlen(header[i]));
        if (result != MUNBYN_OK) return result;
    }
    
    result = munbyn_line_feed(printer);
    if (result != MUNBYN_OK) return result;
    
    // Title with codepage info
    const char* title = "Character Table for Code Page:\n";
    result = munbyn_write_data(printer, (const uint8_t*)title, strlen(title));
    if (result != MUNBYN_OK) return result;
    
    // Get the codepage name and print it
    const char* codepage_name = munbyn_get_codepage_name(codepage_number);
    char codepage_info[128];
    snprintf(codepage_info, sizeof(codepage_info), "%s  (Page %d)\n", codepage_name, (int)codepage_number);
    result = munbyn_write_data(printer, (const uint8_t*)codepage_info, strlen(codepage_info));
    if (result != MUNBYN_OK) return result;
    
    const char* separator = "==========================\n";
    result = munbyn_write_data(printer, (const uint8_t*)separator, strlen(separator));
    if (result != MUNBYN_OK) return result;
    
    result = munbyn_line_feed(printer);
    if (result != MUNBYN_OK) return result;
    
    // Column headers
    const char* col_headers = "   0 1 2 3 4 5 6 7 8 9 A B C D E F \n";
    result = munbyn_write_data(printer, (const uint8_t*)col_headers, strlen(col_headers));
    if (result != MUNBYN_OK) return result;
    
    // CRITICAL: Set the codepage
    result = munbyn_set_codepage(printer, codepage_number);
    if (result != MUNBYN_OK) return result;
    
    // Generate character table (16x16 grid)
    for (int d1 = 0; d1 < 0x10; d1++) {
        // Print row header (first hex digit)
        char row_header[4];
        snprintf(row_header, sizeof(row_header), "%X  ", d1);
        result = munbyn_write_data(printer, (const uint8_t*)row_header, strlen(row_header));
        if (result != MUNBYN_OK) return result;
        
        // Print 16 characters for this row
        for (int d2 = 0; d2 < 0x10; d2++) {
            uint8_t character = (uint8_t)(d1 * 0x10 + d2);
            
            // Replace control characters (0x00-0x20) with space to avoid issues
            if (character <= 0x20) {
                character = 0x20;  // Space character
            }
            
            // Send the character directly as byte
            uint8_t char_with_space[2] = {character, ' '};
            result = munbyn_write_data(printer, char_with_space, 2);
            if (result != MUNBYN_OK) return result;
        }
        
        // End of row - line feed
        result = munbyn_line_feed(printer);
        if (result != MUNBYN_OK) return result;
    }
    
    // CRITICAL: Reset to default codepage (PC437) to prevent sticky codepage
    result = munbyn_set_codepage(printer, MUNBYN_CODEPAGE_PC437);
    if (result != MUNBYN_OK) return result;
    
    // Optional reset message
    if (show_reset_message) {
        const char* reset_msg = "Codepage reset to default (PC437)\n";
        result = munbyn_write_data(printer, (const uint8_t*)reset_msg, strlen(reset_msg));
        if (result != MUNBYN_OK) return result;
    }
    
    // Optional paper feed and cut
    if (feed_lines_before_cut > 0) {
        result = munbyn_feed_lines(printer, feed_lines_before_cut);
        if (result != MUNBYN_OK) return result;
    }
    
    return MUNBYN_OK;
}

int main(int argc, char* argv[])
{
    munbyn_handle_t printer;
    munbyn_error_t result;
    
    // Default values
    munbyn_codepage_t codepage = MUNBYN_CODEPAGE_PC437;
    bool show_reset_message = false;
    int feed_lines = MUNBYN_OPTIMAL_FEED_LINES;
    
    // Parse command line arguments
    if (argc >= 2) {
        int cp = atoi(argv[1]);
        if (cp >= 0 && cp <= 67) {
            codepage = (munbyn_codepage_t)cp;
        } else {
            printf("Invalid codepage %d. Valid range: 0-67\n", cp);
            return 1;
        }
    }
    
    if (argc >= 3) {
        show_reset_message = (strcmp(argv[2], "true") == 0 || strcmp(argv[2], "1") == 0);
    }
    
    if (argc >= 4) {
        feed_lines = atoi(argv[3]);
        if (feed_lines < 0) feed_lines = 0;
    }
    
    printf("Printing character table for codepage %d (%s)\n", 
           (int)codepage, munbyn_get_codepage_name(codepage));
    
    // Try to open USB printer (adjust path as needed)
    result = munbyn_open_usb("/dev/usb/lp0", &printer);
    if (result != MUNBYN_OK) {
        printf("Failed to open printer: %d\n", result);
        printf("Note: Make sure printer is connected and path is correct\n");
        printf("Usage: %s [codepage_number] [show_reset_message] [feed_lines]\n", argv[0]);
        printf("Example: %s 2 true 7  # PC850 with reset message and 7 feed lines\n", argv[0]);
        return 1;
    }

    printf("Printer opened successfully!\n");
    
    // Print the character table
    result = print_codepage_table(printer, codepage, show_reset_message, feed_lines);
    if (result != MUNBYN_OK) {
        printf("Failed to print character table: %d\n", result);
        munbyn_close(printer);
        return 1;
    }
    
    // Cut the paper
    result = munbyn_cut_paper(printer, MUNBYN_CUT_PARTIAL);
    if (result != MUNBYN_OK) {
        printf("Failed to cut paper: %d\n", result);
    }
    
    // Close printer
    munbyn_close(printer);
    printf("Character table printed successfully!\n");
    
    // Show available codepages if no arguments provided
    if (argc == 1) {
        printf("\nAvailable codepages (use as first argument):\n");
        for (int i = 0; i <= 67; i++) {
            printf("  %2d: %s\n", i, munbyn_get_codepage_name((munbyn_codepage_t)i));
        }
        printf("\nUsage examples:\n");
        printf("  %s 0        # Print PC437 table\n", argv[0]);
        printf("  %s 2        # Print PC850 table\n", argv[0]);
        printf("  %s 7 true 5 # Print Greek table with reset message, 5 feed lines\n", argv[0]);
    }
    
    return 0;
}
