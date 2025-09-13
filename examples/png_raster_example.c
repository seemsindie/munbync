#define STB_IMAGE_IMPLEMENTATION
#include "stb_image.h"

#include "munbyn_printer.h"
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <stdint.h>
#include <math.h>

static void usage(const char* prog) {
    fprintf(stderr, "Usage: %s /path/to/device munbyn.png [mode]\n", prog);
    fprintf(stderr, "  mode: 0=normal, 1=double-width, 2=double-height, 3=quad (default 0)\n");
}

static void free_and_exit(uint8_t* packed, uint8_t* mono, unsigned char* img) {
    free(packed);
    free(mono);
    stbi_image_free(img);
}

// Atkinson dithering to 1bpp mask (values 0 or 1; 1 = black dot)
static uint8_t* dither_to_mask_atkinson(const unsigned char* rgba, int w, int h, int channels) {
    int stride = w * channels;
    float* buf = (float*)malloc(sizeof(float) * w * h);
    if (!buf) return NULL;

    // Convert to perceived luminance and composite against white for transparency
    for (int y = 0; y < h; ++y) {
        for (int x = 0; x < w; ++x) {
            const unsigned char* p = &rgba[y * stride + x * channels];
            float r = p[0], g = p[1], b = p[2];
            float a = (channels >= 4) ? (p[3] / 255.0f) : 1.0f;
            float L = (0.2126f * r + 0.7152f * g + 0.0722f * b) * a + 255.0f * (1.0f - a);
            buf[y * w + x] = L; // 0..255
        }
    }

    for (int y = 0; y < h; ++y) {
        for (int x = 0; x < w; ++x) {
            int i = y * w + x;
            float oldp = buf[i];
            float newp = oldp < 128.0f ? 0.0f : 255.0f;
            float err = (oldp - newp) / 8.0f; // Atkinson distributes error/8 to 6 neighbors
            buf[i] = newp;

            if (x + 1 < w) buf[y * w + (x + 1)] += err;
            if (x + 2 < w) buf[y * w + (x + 2)] += err;
            if (y + 1 < h) {
                if (x > 0)      buf[(y + 1) * w + (x - 1)] += err;
                buf[(y + 1) * w + x] += err;
                if (x + 1 < w) buf[(y + 1) * w + (x + 1)] += err;
            }
            if (y + 2 < h) buf[(y + 2) * w + x] += err;
        }
    }

    uint8_t* mask = (uint8_t*)malloc((size_t)w * (size_t)h);
    if (!mask) { free(buf); return NULL; }
    for (int i = 0; i < w * h; ++i) mask[i] = (buf[i] < 128.0f) ? 1 : 0; // 1 = black
    free(buf);
    return mask;
}

// Pack MSB-first per ESC/POS
// 1-bit semantics: 1=black dot printed, 0=no dot (white/transparent)
static uint8_t* pack_bits_msb_first(const uint8_t* mask, int w, int h, int* out_width_bytes) {
    int wb = (w + 7) / 8;
    size_t total = (size_t)wb * (size_t)h;
    uint8_t* out = (uint8_t*)malloc(total);
    if (!out) return NULL;
    for (int y = 0; y < h; ++y) {
        for (int bx = 0; bx < wb; ++bx) {
            uint8_t b = 0;
            for (int bit = 0; bit < 8; ++bit) {
                int x = bx * 8 + bit;
                int idx = y * w + x;
                uint8_t on = (x < w) ? (mask[idx] ? 1 : 0) : 0; // 1 = black
                b |= (on ? 1 : 0) << (7 - bit);
            }
            out[y * wb + bx] = b;
        }
    }
    *out_width_bytes = wb;
    return out;
}

int main(int argc, char* argv[]) {
    if (argc < 3) { usage(argv[0]); return 1; }
    const char* device_path = argv[1];
    const char* png_path = argv[2];
    int mode = (argc >= 4) ? atoi(argv[3]) : 0;
    // Accept 0-3 and 48-51
    if (!((mode >= 0 && mode <= 3) || (mode >= 48 && mode <= 51))) mode = 0;

    munbyn_handle_t printer = NULL;
    if (munbyn_open_usb(device_path, &printer) != MUNBYN_OK) {
        fprintf(stderr, "Failed to open printer at %s\n", device_path);
        return 1;
    }
    munbyn_initialize(printer);

    int w, h, ch;
    unsigned char* img = stbi_load(png_path, &w, &h, &ch, 0);
    if (!img) {
        fprintf(stderr, "Failed to load PNG: %s\n", png_path);
        munbyn_close(printer);
        return 1;
    }

    // Clamp width to printer width if necessary (assume 576 dots)
    const int max_width = 576;
    if (w > max_width) {
        fprintf(stderr, "Image width %d exceeds %d; please resize beforehand.\n", w, max_width);
        stbi_image_free(img);
        munbyn_close(printer);
        return 1;
    }

    uint8_t* mask = dither_to_mask_atkinson(img, w, h, ch);
    if (!mask) { stbi_image_free(img); munbyn_close(printer); return 1; }

    int width_bytes = 0;
    uint8_t* packed = pack_bits_msb_first(mask, w, h, &width_bytes);
    if (!packed) { free(mask); stbi_image_free(img); munbyn_close(printer); return 1; }

    const char* header = "PNG raster example\n";
    munbyn_write_data(printer, (const uint8_t*)header, strlen(header));
    munbyn_set_justification(printer, MUNBYN_JUSTIFY_CENTER);
    munbyn_print_raster_image(printer, (munbyn_image_mode_t)mode, packed, (uint16_t)w, (uint16_t)h);
    munbyn_set_justification(printer, MUNBYN_JUSTIFY_LEFT);
    munbyn_line_feed(printer);
    munbyn_feed_and_cut(printer, MUNBYN_OPTIMAL_FEED_LINES);

    free_and_exit(packed, mask, img);
    munbyn_close(printer);
    return 0;
}


