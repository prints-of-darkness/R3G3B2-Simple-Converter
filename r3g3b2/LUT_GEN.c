#include <stdio.h>
#include <stdint.h>
#include <math.h>

#define GAMMA 1.1
#define CONTRAST_FACTOR 1.0

uint32_t generate_corrected_color(uint8_t value, uint8_t max_value) {
    double normalized = (double)value / max_value;
    double corrected = pow(normalized, GAMMA) * 255 * CONTRAST_FACTOR;
    return (uint32_t)(fmin(fmax(corrected, 0), 255));
}

void generate_brightness_corrected_lut(uint32_t lut[256]) {
    for (int i = 0; i < 256; i++) {
        uint8_t r = (i >> 5) & 0x07;
        uint8_t g = (i >> 2) & 0x07;
        uint8_t b = i & 0x03;

        uint32_t r8 = generate_corrected_color(r, 7);
        uint32_t g8 = generate_corrected_color(g, 7);
        uint32_t b8 = generate_corrected_color(b, 3);

        lut[i] = (r8 << 16) | (g8 << 8) | b8;
    }
}

int main() {
    uint32_t corrected_lut[256];
    generate_brightness_corrected_lut(corrected_lut);

    printf("static const uint32_t r3g3b2_to_r8g8b8_lut[256] = {\n");
    for (int i = 0; i < 256; i++) {
        printf("0x%08X", corrected_lut[i]);
        if (i < 255) printf(", ");
        if ((i + 1) % 8 == 0) printf("\n");
    }
    printf("};\n");

    return 0;
}