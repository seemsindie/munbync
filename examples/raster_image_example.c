#include "munbyn_printer.h"
#include <stdio.h>
#include <string.h>
#include <stdint.h>

// Simple 24x24 smiley bitmap, 1bpp, MSB-first, rows top-to-bottom
// Each row is (24+7)/8 = 3 bytes. Height = 24 rows. Total = 72 bytes.
static const uint8_t SMILEY_24x24[] = {
    0x00,0x00,0x00,
    0x03,0xFF,0x80,
    0x0F,0xFF,0xE0,
    0x1F,0xFF,0xF0,
    0x3F,0xFF,0xF8,
    0x7F,0xFF,0xFC,
    0x7E,0x00,0x7C,
    0xFC,0x00,0x3E,
    0xF8,0x00,0x1E,
    0xF8,0x00,0x1E,
    0xF8,0x00,0x1E,
    0xF8,0x00,0x1E,
    0xF8,0xC3,0x1E,
    0xF8,0xC3,0x1E,
    0xF8,0x00,0x1E,
    0xF8,0x00,0x1E,
    0xF8,0x3C,0x1E,
    0xF8,0x3C,0x1E,
    0xFC,0x00,0x3E,
    0x7E,0x00,0x7C,
    0x7F,0xFF,0xFC,
    0x3F,0xFF,0xF8,
    0x1F,0xFF,0xF0,
    0x0F,0xFF,0xE0,
};

int main(int argc, char* argv[]) {
    const char* device_path = (argc > 1) ? argv[1] : "/dev/usb/lp0";

    munbyn_handle_t printer = NULL;
    if (munbyn_open_usb(device_path, &printer) != MUNBYN_OK) {
        fprintf(stderr, "Failed to open printer at %s\n", device_path);
        return 1;
    }

    munbyn_initialize(printer);

    const char* title = "Raster image demo (24x24)\n";
    munbyn_write_data(printer, (const uint8_t*)title, strlen(title));

    // Center the image for visibility
    munbyn_set_justification(printer, MUNBYN_JUSTIFY_CENTER);
    munbyn_print_raster_image(printer, MUNBYN_IMAGE_QUADRUPLE, SMILEY_24x24, 24, 24);
    munbyn_line_feed(printer);
    munbyn_line_feed(printer);
    munbyn_set_justification(printer, MUNBYN_JUSTIFY_LEFT);

    munbyn_feed_and_cut(printer, MUNBYN_OPTIMAL_FEED_LINES);
    munbyn_close(printer);
    return 0;
}


