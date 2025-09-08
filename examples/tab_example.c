#include "munbyn_printer.h"
#include <stdio.h>
#include <string.h>
#include <stdlib.h>

static void print_line_with_tabs(munbyn_handle_t printer, const char* a, const char* b, const char* c)
{
    munbyn_write_data(printer, (const uint8_t*)a, strlen(a));
    munbyn_horizontal_tab(printer);
    munbyn_write_data(printer, (const uint8_t*)b, strlen(b));
    munbyn_horizontal_tab(printer);
    munbyn_write_data(printer, (const uint8_t*)c, strlen(c));
    munbyn_line_feed(printer);
}

int main(int argc, char* argv[])
{
    const char* device_path = "/dev/usb/lp0";
    if (argc > 1) {
        device_path = argv[1];
    }

    printf("Horizontal Tab Demo\n");
    printf("Using device: %s\n\n", device_path);

    munbyn_handle_t printer = NULL;
    munbyn_error_t result;

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

    // Section 1: Default tab stops (every 8 columns at 12x24)
    munbyn_write_data(printer, (const uint8_t*)"Default tab stops (every 8 cols)", 33);
    munbyn_line_feed(printer);
    print_line_with_tabs(printer, "Col1", "Col2", "Col3");
    print_line_with_tabs(printer, "Apple", "Banana", "Cherry");
    munbyn_line_feed(printer);

    // Section 2: Custom tab stops using ESC D
    // Positions measured from start of line in character columns
    uint8_t tabs[] = {15, 20, 28};
    munbyn_set_horizontal_tab_positions(printer, tabs, sizeof(tabs));
    munbyn_write_data(printer, (const uint8_t*)"Custom tab stops at 15,20,28", 28);
    munbyn_line_feed(printer);
    print_line_with_tabs(printer, "Item", "Qty", "Price");
    print_line_with_tabs(printer, "USB Cable", "2", "$7.98");
    print_line_with_tabs(printer, "Receipt Paper", "1", "$3.49");
    munbyn_line_feed(printer);

    // Section 3: Clearing tab stops falls back to defaults
    munbyn_clear_horizontal_tab_positions(printer);
    munbyn_write_data(printer, (const uint8_t*)"Tabs cleared (back to defaults)", 32);
    munbyn_line_feed(printer);
    print_line_with_tabs(printer, "A", "B", "C");

    // Finish
    munbyn_feed_lines(printer, MUNBYN_ACCEPTABLE_FEED_LINES);
    munbyn_cut_paper(printer, MUNBYN_CUT_PARTIAL);
    munbyn_close(printer);

    printf("Tab example completed.\n");
    return 0;
}


