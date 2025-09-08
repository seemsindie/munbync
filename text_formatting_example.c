#include "munbyn_printer.h"
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>

static void print_section_header(munbyn_handle_t printer, const char* title)
{
    // Reset to defaults
    munbyn_set_justification(printer, MUNBYN_JUSTIFY_CENTER);
    munbyn_set_emphasis(printer, true);
    munbyn_set_underline(printer, 1);
    
    munbyn_write_data(printer, (const uint8_t*)title, strlen(title));
    munbyn_line_feed(printer);
    
    // Reset formatting
    munbyn_set_emphasis(printer, false);
    munbyn_set_underline(printer, 0);
    munbyn_set_justification(printer, MUNBYN_JUSTIFY_LEFT);
    munbyn_line_feed(printer);
}

static void print_demo_text(munbyn_handle_t printer, const char* description, const char* text)
{
    printf("Printing: %s\n", description);
    
    // Print description in small font
    munbyn_set_font(printer, MUNBYN_FONT_B);
    munbyn_write_data(printer, (const uint8_t*)description, strlen(description));
    munbyn_line_feed(printer);
    
    // Reset to normal font for demo text
    munbyn_set_font(printer, MUNBYN_FONT_A);
    munbyn_write_data(printer, (const uint8_t*)text, strlen(text));
    munbyn_line_feed(printer);
    munbyn_line_feed(printer);
}

int main(int argc, char *argv[])
{
    const char* device_path = "/dev/usb/lp0";
    
    if (argc > 1) {
        device_path = argv[1];
    }
    
    printf("Text Formatting Demo\n");
    printf("Using device: %s\n\n", device_path);
    
    munbyn_handle_t printer;
    munbyn_error_t result;
    
    // Open and initialize printer
    result = munbyn_open_usb(device_path, &printer);
    if (result != MUNBYN_OK) {
        printf("Failed to open printer: %d\n", result);
        return 1;
    }
    
    result = munbyn_initialize(printer);
    if (result != MUNBYN_OK) {
        printf("Failed to initialize printer: %d\n", result);
        munbyn_close(printer);
        return 1;
    }
    
    // Demo header
    print_section_header(printer, "TEXT FORMATTING DEMO");
    
    // 1. Text Justification Demo
    print_section_header(printer, "1. TEXT JUSTIFICATION");
    
    munbyn_set_justification(printer, MUNBYN_JUSTIFY_LEFT);
    print_demo_text(printer, "Left Aligned:", "This text is left aligned");
    
    munbyn_set_justification(printer, MUNBYN_JUSTIFY_CENTER);
    print_demo_text(printer, "Center Aligned:", "This text is centered");
    
    munbyn_set_justification(printer, MUNBYN_JUSTIFY_RIGHT);
    print_demo_text(printer, "Right Aligned:", "This text is right aligned");
    
    // Reset to left alignment
    munbyn_set_justification(printer, MUNBYN_JUSTIFY_LEFT);
    
    // 2. Font Selection Demo
    print_section_header(printer, "2. FONT SELECTION");
    
    munbyn_set_font(printer, MUNBYN_FONT_A);
    print_demo_text(printer, "Font A (12x24):", "Font A text sample - larger");
    
    munbyn_set_font(printer, MUNBYN_FONT_B);
    print_demo_text(printer, "Font B (9x17):", "Font B text sample - smaller");
    
    // Reset to Font A
    munbyn_set_font(printer, MUNBYN_FONT_A);
    
    // 3. Text Emphasis Demo
    print_section_header(printer, "3. TEXT EMPHASIS");
    
    munbyn_write_data(printer, (const uint8_t*)"Normal text", 11);
    munbyn_line_feed(printer);
    
    munbyn_set_emphasis(printer, true);
    munbyn_write_data(printer, (const uint8_t*)"Emphasized (bold) text", 22);
    munbyn_line_feed(printer);
    munbyn_set_emphasis(printer, false);
    
    munbyn_set_double_strike(printer, true);
    munbyn_write_data(printer, (const uint8_t*)"Double-strike text", 18);
    munbyn_line_feed(printer);
    munbyn_set_double_strike(printer, false);
    
    munbyn_line_feed(printer);
    
    // 4. Underline Demo
    print_section_header(printer, "4. UNDERLINE STYLES");
    
    munbyn_write_data(printer, (const uint8_t*)"No underline", 12);
    munbyn_line_feed(printer);
    
    munbyn_set_underline(printer, 1);
    munbyn_write_data(printer, (const uint8_t*)"1-dot underline", 15);
    munbyn_line_feed(printer);
    
    munbyn_set_underline(printer, 2);
    munbyn_write_data(printer, (const uint8_t*)"2-dot underline", 15);
    munbyn_line_feed(printer);
    munbyn_set_underline(printer, 0);
    
    munbyn_line_feed(printer);
    
    // 5. Combined Text Modes Demo
    print_section_header(printer, "5. COMBINED MODES");
    
    // Using munbyn_set_text_mode for combined effects
    uint8_t double_width_height = MUNBYN_MODE_DOUBLE_WIDTH | MUNBYN_MODE_DOUBLE_HEIGHT;
    munbyn_set_text_mode(printer, double_width_height);
    munbyn_write_data(printer, (const uint8_t*)"BIG TEXT", 8);
    munbyn_line_feed(printer);
    
    uint8_t emphasized_underline = MUNBYN_MODE_EMPHASIZED | MUNBYN_MODE_UNDERLINE;
    munbyn_set_text_mode(printer, emphasized_underline);
    munbyn_write_data(printer, (const uint8_t*)"Bold + Underlined", 17);
    munbyn_line_feed(printer);
    
    // Reset to normal
    munbyn_set_text_mode(printer, MUNBYN_MODE_NORMAL);
    munbyn_line_feed(printer);
    
    // 6. Line Spacing Demo
    print_section_header(printer, "6. LINE SPACING");
    
    munbyn_write_data(printer, (const uint8_t*)"Default line spacing:", 21);
    munbyn_line_feed(printer);
    munbyn_write_data(printer, (const uint8_t*)"Line 1", 6);
    munbyn_line_feed(printer);
    munbyn_write_data(printer, (const uint8_t*)"Line 2", 6);
    munbyn_line_feed(printer);
    munbyn_line_feed(printer);
    
    munbyn_write_data(printer, (const uint8_t*)"Custom line spacing (60/180 inch):", 35);
    munbyn_line_feed(printer);
    munbyn_set_line_spacing(printer, 60);
    munbyn_write_data(printer, (const uint8_t*)"Spaced Line 1", 13);
    munbyn_line_feed(printer);
    munbyn_write_data(printer, (const uint8_t*)"Spaced Line 2", 13);
    munbyn_line_feed(printer);
    
    // Reset to default line spacing
    munbyn_set_line_spacing_default(printer);
    munbyn_line_feed(printer);
    
    // 7. Character Spacing Demo
    print_section_header(printer, "7. CHARACTER SPACING");
    
    munbyn_write_data(printer, (const uint8_t*)"Normal spacing: HELLO", 21);
    munbyn_line_feed(printer);
    
    munbyn_set_character_spacing(printer, 5);
    munbyn_write_data(printer, (const uint8_t*)"Wide spacing: HELLO", 19);
    munbyn_line_feed(printer);
    
    // Reset character spacing
    munbyn_set_character_spacing(printer, 0);
    munbyn_line_feed(printer);
    
    // 8. Text Rotation Demo
    print_section_header(printer, "8. TEXT ROTATION");
    
    munbyn_write_data(printer, (const uint8_t*)"Normal orientation", 18);
    munbyn_line_feed(printer);
    
    munbyn_set_rotate_90(printer, true);
    munbyn_write_data(printer, (const uint8_t*)"90° rotated", 11);
    munbyn_line_feed(printer);
    munbyn_set_rotate_90(printer, false);
    
    munbyn_set_upside_down(printer, true);
    munbyn_write_data(printer, (const uint8_t*)"Upside down", 11);
    munbyn_line_feed(printer);
    munbyn_set_upside_down(printer, false);
    
    munbyn_line_feed(printer);
    
    // 9. Margin and Width Demo
    print_section_header(printer, "9. MARGINS & WIDTH");
    
    munbyn_write_data(printer, (const uint8_t*)"Normal margin", 13);
    munbyn_line_feed(printer);
    
    munbyn_set_left_margin(printer, 50);
    munbyn_write_data(printer, (const uint8_t*)"Indented text", 13);
    munbyn_line_feed(printer);
    
    // Reset margin
    munbyn_set_left_margin(printer, 0);
    munbyn_line_feed(printer);
    
    // Final section
    print_section_header(printer, "DEMO COMPLETE");
    
    munbyn_set_justification(printer, MUNBYN_JUSTIFY_CENTER);
    munbyn_write_data(printer, (const uint8_t*)"Thank you for using", 19);
    munbyn_line_feed(printer);
    munbyn_write_data(printer, (const uint8_t*)"Munbyn Printer Library", 23);
    munbyn_line_feed(printer);
    munbyn_line_feed(printer);
    
    // Feed paper and cut
    munbyn_feed_lines(printer, 3);
    munbyn_cut_paper(printer, MUNBYN_CUT_PARTIAL);
    
    // Close printer
    munbyn_close(printer);
    
    printf("Text formatting demo completed successfully!\n");
    return 0;
}
