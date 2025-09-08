#include "munbyn_printer.h"
#include <stdio.h>
#include <string.h>
#include <unistd.h>

int main() {
    munbyn_handle_t printer = NULL;
    munbyn_error_t result;
    munbyn_status_t status;
    
    printf("=== Munbyn Printer Feed Line Test for Optimal Cutting ===\n");
    printf("This test will help determine the optimal feed lines for symmetrical cutting.\n\n");
    
    // Open the printer
    result = munbyn_open_usb("/dev/usb/lp0", &printer);
    if (result != MUNBYN_OK) {
        printf("Failed to open USB printer: error %d\n", result);
        printf("Make sure the printer is connected and the device path is correct.\n");
        return 1;
    }
    
    printf("USB printer opened successfully!\n");
    
    // Initialize the printer
    result = munbyn_initialize(printer);
    if (result != MUNBYN_OK) {
        printf("Failed to initialize printer: error %d\n", result);
        munbyn_close(printer);
        return 1;
    }
    
    printf("Printer initialized successfully!\n");
    
    // Check printer status
    result = munbyn_get_status(printer, &status);
    if (result == MUNBYN_OK) {
        printf("Printer Status: Online=%s, Paper=%s, Cover=%s\n", 
               status.online ? "Yes" : "No",
               status.paper_present ? "Yes" : "No", 
               status.cover_closed ? "Yes" : "No");
               
        if (!status.paper_present) {
            printf("WARNING: No paper detected! Please load paper.\n");
        }
        if (!status.cover_closed) {
            printf("WARNING: Cover is open! Please close the cover.\n");
        }
    }
    
    printf("\nStarting feed line tests...\n");
    printf("Each test will print text and then feed different amounts before cutting.\n");
    printf("Observe the cutting position to determine which feed count gives\n");
    printf("the most symmetrical result (equal spacing above and below text).\n\n");
    
    // Test different feed line counts
    int feed_counts[] = {3, 4, 5, 6, 7, 8, 9, 10};
    int num_tests = sizeof(feed_counts) / sizeof(feed_counts[0]);
    
    for (int i = 0; i < num_tests; i++) {
        int feed_lines = feed_counts[i];
        
        printf("Test %d: %d feed lines\n", i + 1, feed_lines);
        
        // Test 1: Single line text
        char single_line_text[100];
        snprintf(single_line_text, sizeof(single_line_text), 
                "SINGLE LINE TEST - %d FEED LINES\n", feed_lines);
        
        result = munbyn_write_data(printer, (const uint8_t*)single_line_text, strlen(single_line_text));
        if (result != MUNBYN_OK) {
            printf("Failed to write single line text: error %d\n", result);
            continue;
        }
        
        // Feed the specified number of lines
        result = munbyn_feed_lines(printer, feed_lines);
        if (result != MUNBYN_OK) {
            printf("Failed to feed %d lines: error %d\n", feed_lines, result);
            continue;
        }
        
        // Perform partial cut
        result = munbyn_cut_paper(printer, MUNBYN_CUT_PARTIAL);
        if (result != MUNBYN_OK) {
            printf("Failed to cut paper: error %d\n", result);
            continue;
        }
        
        printf("  Single line test with %d feed lines completed\n", feed_lines);
        
        // Wait a moment before next test
        sleep(1);
        
        // Test 2: Double line text  
        char double_line_text[150];
        snprintf(double_line_text, sizeof(double_line_text), 
                "DOUBLE LINE TEST - %d FEED LINES\nSecond line of text for testing\n", feed_lines);
        
        result = munbyn_write_data(printer, (const uint8_t*)double_line_text, strlen(double_line_text));
        if (result != MUNBYN_OK) {
            printf("Failed to write double line text: error %d\n", result);
            continue;
        }
        
        // Feed the specified number of lines
        result = munbyn_feed_lines(printer, feed_lines);
        if (result != MUNBYN_OK) {
            printf("Failed to feed %d lines: error %d\n", feed_lines, result);
            continue;
        }
        
        // Perform partial cut
        result = munbyn_cut_paper(printer, MUNBYN_CUT_PARTIAL);
        if (result != MUNBYN_OK) {
            printf("Failed to cut paper: error %d\n", result);
            continue;
        }
        
        printf("  Double line test with %d feed lines completed\n", feed_lines);
        
        // Wait before next iteration
        sleep(1);
    }
    
    printf("\n=== Additional Tests with Different Text Formats ===\n");
    
    // Test with emphasized text
    const char emphasized_test[] = "EMPHASIZED TEXT TEST - 6 FEED LINES\nBold and prominent text\n";
    munbyn_write_data(printer, (const uint8_t*)emphasized_test, strlen(emphasized_test));
    munbyn_feed_lines(printer, 6);
    munbyn_cut_paper(printer, MUNBYN_CUT_PARTIAL);
    printf("Emphasized text test completed\n");
    
    sleep(1);
    
    // Test with longer text
    const char long_test[] = "LONG TEXT TEST - 7 FEED LINES\nThis is a longer line of text to see how it affects cutting positioning and symmetry\n";
    munbyn_write_data(printer, (const uint8_t*)long_test, strlen(long_test));
    munbyn_feed_lines(printer, 7);
    munbyn_cut_paper(printer, MUNBYN_CUT_PARTIAL);
    printf("Long text test completed\n");
    
    sleep(1);
    
    // Test with three lines of text
    const char triple_test[] = "THREE LINE TEST - 8 FEED LINES\nSecond line of text\nThird line for comparison\n";
    munbyn_write_data(printer, (const uint8_t*)triple_test, strlen(triple_test));
    munbyn_feed_lines(printer, 8);
    munbyn_cut_paper(printer, MUNBYN_CUT_PARTIAL);
    printf("Three line text test completed\n");
    
    // Final summary print
    printf("\n=== Test Summary ===\n");
    const char summary[] = "\n=== FEED LINE TEST COMPLETE ===\nExamine each cut section to determine\nwhich feed count provides the most\nsymmetrical spacing for clean cuts.\n\nRecommended: Compare 5-7 feed lines\nfor optimal results.\n";
    munbyn_write_data(printer, (const uint8_t*)summary, strlen(summary));
    munbyn_feed_lines(printer, 6);  // Use a middle value for summary
    munbyn_cut_paper(printer, MUNBYN_CUT_PARTIAL);
    
    // Close the printer
    result = munbyn_close(printer);
    if (result != MUNBYN_OK) {
        printf("Failed to close printer: error %d\n", result);
    } else {
        printf("Printer closed successfully.\n");
    }
    
    printf("\n=== Test Instructions ===\n");
    printf("1. Examine each printed section\n");
    printf("2. Look for symmetrical spacing above and below the text\n");
    printf("3. The optimal feed count should provide equal white space\n");
    printf("4. Note which feed line count (3-10) gives the best results\n");
    printf("5. Use that value in your production code for clean cuts\n");
    
    return 0;
}
