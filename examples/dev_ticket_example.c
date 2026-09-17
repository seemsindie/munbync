/*
 * 🚀 EPIC DEV TICKET EXAMPLE 🚀
 *
 * This example creates an awesome development ticket with:
 * - Project info and ticket details
 * - Inverted headers and cool formatting
 * - Big scannable barcode for time tracking
 * - Multiple fonts and text effects
 * - Current timestamp
 * - Some nerdy stats for the geeks (with ASCII art instead of emojis)
 */

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <time.h>
#include "munbyn_printer.h"

// Simple smiley face bitmap (16x16 pixels)
static const uint8_t smiley_bitmap[] = {
    0x00, 0x00,  // ................
    0x03, 0xC0,  // ......####......
    0x0C, 0x30,  // ....##....##....
    0x10, 0x08,  // ...#........#...
    0x20, 0x04,  // ..#..........#..
    0x24, 0x24,  // ..#..#....#..#..
    0x48, 0x12,  // .#..#......#..#.
    0x48, 0x12,  // .#..#......#..#.
    0x48, 0x12,  // .#..#......#..#.
    0x48, 0x12,  // .#..#......#..#.
    0x24, 0x24,  // ..#..#....#..#..
    0x27, 0xE4,  // ..#..######..#..
    0x10, 0x08,  // ...#........#...
    0x0C, 0x30,  // ....##....##....
    0x03, 0xC0,  // ......####......
    0x00, 0x00   // ................
};

// Rocket ship bitmap (16x16 pixels)
static const uint8_t rocket_bitmap[] = {
    0x01, 0x80,  // .......##.......
    0x01, 0x80,  // .......##.......
    0x03, 0xC0,  // ......####......
    0x03, 0xC0,  // ......####......
    0x07, 0xE0,  // .....######.....
    0x0F, 0xF0,  // ....########....
    0x1F, 0xF8,  // ...##########...
    0x1F, 0xF8,  // ...##########...
    0x1F, 0xF8,  // ...##########...
    0x0F, 0xF0,  // ....########....
    0x0F, 0xF0,  // ....########....
    0x37, 0xEC,  // ..##.######.##..
    0x60, 0x06,  // .##..........##.
    0x40, 0x02,  // .#............#.
    0xC0, 0x03,  // ##............##
    0x80, 0x01   // #..............#
};

int main(void)
{
    munbyn_handle_t printer;
    time_t now;
    struct tm *timeinfo;
    char timestamp[64];
    char barcode_data[32];

    // Get current time for ticket
    time(&now);
    timeinfo = localtime(&now);
    strftime(timestamp, sizeof(timestamp), "%Y-%m-%d %H:%M:%S", timeinfo);

    // Create unique barcode data (timestamp-based ID for time tracking)
    snprintf(barcode_data, sizeof(barcode_data), "DEV%02d%02d%02d%02d%02d",
             timeinfo->tm_mon + 1, timeinfo->tm_mday,
             timeinfo->tm_hour, timeinfo->tm_min, timeinfo->tm_sec);

    // Open printer
    munbyn_error_t result = munbyn_open_usb("/dev/usb/lp0", &printer);
    if (result != MUNBYN_OK) {
        printf("🔥 Printer connection failed: %d\n", result);
        printf("💡 Make sure your Munbyn printer is connected!\n");
        return 1;
    }

    printf("🖨️  Connected to printer successfully!\n");
    munbyn_initialize(printer);

    // === HEADER SECTION - INVERTED STYLE ===
    // Add rocket icon
    munbyn_set_justification(printer, MUNBYN_JUSTIFY_CENTER);
    munbyn_print_raster_image(printer, MUNBYN_IMAGE_NORMAL, rocket_bitmap, 16, 16);
    munbyn_line_feed(printer);

    munbyn_set_inverted_text(printer, true);
    munbyn_set_text_scale(printer, 2, 2);  // Big and bold

    const char* header = " DEV TICKET SYSTEM ";
    munbyn_write_data(printer, (const uint8_t*)header, strlen(header));
    munbyn_line_feed(printer);

    munbyn_set_inverted_text(printer, false);
    munbyn_set_text_scale(printer, 1, 1);
    munbyn_line_feed(printer);

    // === PROJECT INFO ===
    munbyn_set_justification(printer, MUNBYN_JUSTIFY_LEFT);
    munbyn_set_font(printer, MUNBYN_FONT_B);  // Smaller font
    munbyn_set_emphasis(printer, true);

    const char* project_line = "PROJECT: MunbynC Library";
    munbyn_write_data(printer, (const uint8_t*)project_line, strlen(project_line));
    munbyn_line_feed(printer);

    const char* client_line = "CLIENT:  TechCorp Industries";
    munbyn_write_data(printer, (const uint8_t*)client_line, strlen(client_line));
    munbyn_line_feed(printer);

    munbyn_set_emphasis(printer, false);
    munbyn_line_feed(printer);

    // === TICKET DETAILS ===
    munbyn_set_font(printer, MUNBYN_FONT_A);  // Back to standard font
    munbyn_set_inverted_text(printer, true);

    const char* ticket_header = " TICKET #TC-2025-0913 ";
    munbyn_write_data(printer, (const uint8_t*)ticket_header, strlen(ticket_header));
    munbyn_line_feed(printer);

    munbyn_set_inverted_text(printer, false);
    munbyn_line_feed(printer);

    // Ticket description with mixed formatting
    munbyn_set_underline(printer, 1);
    const char* desc_header = "DESCRIPTION:";
    munbyn_write_data(printer, (const uint8_t*)desc_header, strlen(desc_header));
    munbyn_set_underline(printer, 0);
    munbyn_line_feed(printer);

    const char* description = "Implement thermal printer driver with\nadvanced formatting capabilities and\nbarcode generation for receipt systems.";
    munbyn_write_data(printer, (const uint8_t*)description, strlen(description));
    munbyn_line_feed(printer);
    munbyn_line_feed(printer);

    // === PRIORITY AND STATUS ===
    munbyn_set_double_strike(printer, true);
    munbyn_set_emphasis(printer, true);
    const char* priority = "PRIORITY: HIGH";
    munbyn_write_data(printer, (const uint8_t*)priority, strlen(priority));
    munbyn_line_feed(printer);

    const char* status = "STATUS:   IN PROGRESS";
    munbyn_write_data(printer, (const uint8_t*)status, strlen(status));
    munbyn_line_feed(printer);

    munbyn_set_double_strike(printer, false);
    munbyn_set_emphasis(printer, false);
    munbyn_line_feed(printer);

    // === TIMESTAMP ===
    munbyn_set_font(printer, MUNBYN_FONT_B);
    char time_line[80];
    snprintf(time_line, sizeof(time_line), "PRINTED: %s", timestamp);
    munbyn_write_data(printer, (const uint8_t*)time_line, strlen(time_line));
    munbyn_line_feed(printer);
    munbyn_line_feed(printer);

    // === BIG BARCODE FOR TIME TRACKING ===
    munbyn_set_justification(printer, MUNBYN_JUSTIFY_CENTER);
    munbyn_set_font(printer, MUNBYN_FONT_A);
    munbyn_set_inverted_text(printer, true);

    const char* barcode_header = " TIME TRACKING BARCODE ";
    munbyn_write_data(printer, (const uint8_t*)barcode_header, strlen(barcode_header));
    munbyn_line_feed(printer);

    munbyn_set_inverted_text(printer, false);
    munbyn_line_feed(printer);

    // Configure barcode for maximum readability
    munbyn_set_barcode_height(printer, 120);  // Extra tall for easy scanning
    munbyn_set_barcode_width(printer, 4);     // Extra wide bars
    munbyn_set_hri_position(printer, MUNBYN_HRI_BELOW);
    munbyn_set_hri_font(printer, MUNBYN_HRI_FONT_STANDARD);

    // Print the tracking barcode using CODE39 (more compatible)
    printf("Printing barcode: %s\n", barcode_data);

    // First try with a simple test barcode
    const char* test_barcode = "DEV123";
    printf("Testing simple barcode: %s\n", test_barcode);
    munbyn_error_t barcode_result = munbyn_print_barcode(printer, MUNBYN_BARCODE_CODE39, test_barcode);
    if (barcode_result != MUNBYN_OK) {
        printf("Simple barcode failed: %d\n", barcode_result);
    }
    munbyn_line_feed(printer);

    // Try with the full barcode
    barcode_result = munbyn_print_barcode(printer, MUNBYN_BARCODE_CODE39, barcode_data);
    if (barcode_result != MUNBYN_OK) {
        printf("Full barcode failed: %d\n", barcode_result);
        // Try ITF (Interleaved 2 of 5) which only needs numbers
        printf("Trying ITF barcode with numbers only\n");
        const char* number_only = "0913115610";
        barcode_result = munbyn_print_barcode(printer, MUNBYN_BARCODE_ITF, number_only);
        if (barcode_result != MUNBYN_OK) {
            printf("ITF barcode also failed: %d\n", barcode_result);
            // Final fallback: print as text
            munbyn_set_emphasis(printer, true);
            munbyn_write_data(printer, (const uint8_t*)barcode_data, strlen(barcode_data));
            munbyn_set_emphasis(printer, false);
        }
    }
    munbyn_line_feed(printer);

    const char* scan_instruction = "Scan for check-in/check-out";
    munbyn_write_data(printer, (const uint8_t*)scan_instruction, strlen(scan_instruction));
    munbyn_line_feed(printer);
    munbyn_line_feed(printer);

    // === NERDY STATS SECTION ===
    munbyn_set_justification(printer, MUNBYN_JUSTIFY_LEFT);
    munbyn_set_inverted_text(printer, true);

    const char* stats_header = " NERDY STATS [o_o] ";
    munbyn_write_data(printer, (const uint8_t*)stats_header, strlen(stats_header));
    munbyn_line_feed(printer);

    munbyn_set_inverted_text(printer, false);
    munbyn_set_font(printer, MUNBYN_FONT_B);  // Smaller font for stats
    munbyn_line_feed(printer);

    // Calculate some fun stats
    char stats[256];
    snprintf(stats, sizeof(stats),
        "Barcode ID:    %s\n"
        "Unix Time:     %ld\n"
        "Day of Year:   %d\n"
        "Week Number:   %d\n"
        "Hour of Day:   %02d\n"
        "Ticket Hash:   0x%08X",
        barcode_data,
        (long)now,
        timeinfo->tm_yday + 1,
        (timeinfo->tm_yday / 7) + 1,
        timeinfo->tm_hour,  // Hour instead of GMT offset
        (unsigned int)(now % 0xFFFFFFFF)  // Simple hash
    );

    munbyn_write_data(printer, (const uint8_t*)stats, strlen(stats));
    munbyn_line_feed(printer);
    munbyn_line_feed(printer);

    // === DEVELOPER INFO ===
    munbyn_set_font(printer, MUNBYN_FONT_A);
    munbyn_set_emphasis(printer, true);

    const char* dev_info = "ASSIGNED TO: CodeMaster3000";
    munbyn_write_data(printer, (const uint8_t*)dev_info, strlen(dev_info));
    munbyn_line_feed(printer);

    munbyn_set_emphasis(printer, false);
    munbyn_set_font(printer, MUNBYN_FONT_B);

    const char* contact = "Contact: dev@techcorp.com";
    munbyn_write_data(printer, (const uint8_t*)contact, strlen(contact));
    munbyn_line_feed(printer);

    const char* repo = "Repo: github.com/techcorp/munbync";
    munbyn_write_data(printer, (const uint8_t*)repo, strlen(repo));
    munbyn_line_feed(printer);
    munbyn_line_feed(printer);

    // === FOOTER WITH STYLE ===
    munbyn_set_justification(printer, MUNBYN_JUSTIFY_CENTER);
    munbyn_set_font(printer, MUNBYN_FONT_A);

    // Cool separator
    const char* separator = "================================";
    munbyn_write_data(printer, (const uint8_t*)separator, strlen(separator));
    munbyn_line_feed(printer);

    munbyn_set_text_scale(printer, 2, 1);  // Wide text
    const char* footer = "HAPPY CODING! ^_^";
    munbyn_write_data(printer, (const uint8_t*)footer, strlen(footer));
    munbyn_line_feed(printer);

    munbyn_set_text_scale(printer, 1, 1);
    munbyn_write_data(printer, (const uint8_t*)separator, strlen(separator));
    munbyn_line_feed(printer);

    // Add smiley face
    munbyn_print_raster_image(printer, MUNBYN_IMAGE_NORMAL, smiley_bitmap, 16, 16);
    munbyn_line_feed(printer);

    // Extra space before cutting
    munbyn_line_feed(printer);
    munbyn_line_feed(printer);

    // Cut the paper with style
    munbyn_feed_and_cut(printer, MUNBYN_OPTIMAL_FEED_LINES);

    // Clean up
    munbyn_close(printer);

    printf("Epic dev ticket printed successfully!\n");
    printf("Barcode ID: %s\n", barcode_data);
    printf("Timestamp: %s\n", timestamp);
    printf("Ready for time tracking system integration!\n");

    return 0;
}
