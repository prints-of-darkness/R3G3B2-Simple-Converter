/*
    rgb332.c

    Converts a RGB image to 8-bit RGB332. (needed for LT7683 TFT graphics controller)

    MJM 2025

    For checking image conversion see: https://notisrac.github.io/FileToCArray/
           (outputs are almost the same, but not exactly the same)
*/



#include <stdio.h>
#include <stdint.h>
#include <stdlib.h>
#include <string.h>

#define STB_IMAGE_IMPLEMENTATION
#include "stb_image.h"

#define STB_IMAGE_WRITE_IMPLEMENTATION
#include "stb_image_write.h"

#define MAX_FILENAME_LENGTH 1024

#define GAMMA 1.0
#define CONTRAST_FACTOR 1.0

static const uint32_t r3g3b2_to_r8g8b8_lut[256] = {
    0x00000000, 0x00000055, 0x000000AA, 0x000000FF, 0x00002400, 0x00002455, 0x000024AA, 0x000024FF,
    0x00004800, 0x00004855, 0x000048AA, 0x000048FF, 0x00006D00, 0x00006D55, 0x00006DAA, 0x00006DFF,
    0x00009100, 0x00009155, 0x000091AA, 0x000091FF, 0x0000B600, 0x0000B655, 0x0000B6AA, 0x0000B6FF,
    0x0000DA00, 0x0000DA55, 0x0000DAAA, 0x0000DAFF, 0x0000FF00, 0x0000FF55, 0x0000FFAA, 0x0000FFFF,
    0x00240000, 0x00240055, 0x002400AA, 0x002400FF, 0x00242400, 0x00242455, 0x002424AA, 0x002424FF,
    0x00244800, 0x00244855, 0x002448AA, 0x002448FF, 0x00246D00, 0x00246D55, 0x00246DAA, 0x00246DFF,
    0x00249100, 0x00249155, 0x002491AA, 0x002491FF, 0x0024B600, 0x0024B655, 0x0024B6AA, 0x0024B6FF,
    0x0024DA00, 0x0024DA55, 0x0024DAAA, 0x0024DAFF, 0x0024FF00, 0x0024FF55, 0x0024FFAA, 0x0024FFFF,
    0x00480000, 0x00480055, 0x004800AA, 0x004800FF, 0x00482400, 0x00482455, 0x004824AA, 0x004824FF,
    0x00484800, 0x00484855, 0x004848AA, 0x004848FF, 0x00486D00, 0x00486D55, 0x00486DAA, 0x00486DFF,
    0x00489100, 0x00489155, 0x004891AA, 0x004891FF, 0x0048B600, 0x0048B655, 0x0048B6AA, 0x0048B6FF,
    0x0048DA00, 0x0048DA55, 0x0048DAAA, 0x0048DAFF, 0x0048FF00, 0x0048FF55, 0x0048FFAA, 0x0048FFFF,
    0x006D0000, 0x006D0055, 0x006D00AA, 0x006D00FF, 0x006D2400, 0x006D2455, 0x006D24AA, 0x006D24FF,
    0x006D4800, 0x006D4855, 0x006D48AA, 0x006D48FF, 0x006D6D00, 0x006D6D55, 0x006D6DAA, 0x006D6DFF,
    0x006D9100, 0x006D9155, 0x006D91AA, 0x006D91FF, 0x006DB600, 0x006DB655, 0x006DB6AA, 0x006DB6FF,
    0x006DDA00, 0x006DDA55, 0x006DDAAA, 0x006DDAFF, 0x006DFF00, 0x006DFF55, 0x006DFFAA, 0x006DFFFF,
    0x00910000, 0x00910055, 0x009100AA, 0x009100FF, 0x00912400, 0x00912455, 0x009124AA, 0x009124FF,
    0x00914800, 0x00914855, 0x009148AA, 0x009148FF, 0x00916D00, 0x00916D55, 0x00916DAA, 0x00916DFF,
    0x00919100, 0x00919155, 0x009191AA, 0x009191FF, 0x0091B600, 0x0091B655, 0x0091B6AA, 0x0091B6FF,
    0x0091DA00, 0x0091DA55, 0x0091DAAA, 0x0091DAFF, 0x0091FF00, 0x0091FF55, 0x0091FFAA, 0x0091FFFF,
    0x00B60000, 0x00B60055, 0x00B600AA, 0x00B600FF, 0x00B62400, 0x00B62455, 0x00B624AA, 0x00B624FF,
    0x00B64800, 0x00B64855, 0x00B648AA, 0x00B648FF, 0x00B66D00, 0x00B66D55, 0x00B66DAA, 0x00B66DFF,
    0x00B69100, 0x00B69155, 0x00B691AA, 0x00B691FF, 0x00B6B600, 0x00B6B655, 0x00B6B6AA, 0x00B6B6FF,
    0x00B6DA00, 0x00B6DA55, 0x00B6DAAA, 0x00B6DAFF, 0x00B6FF00, 0x00B6FF55, 0x00B6FFAA, 0x00B6FFFF,
    0x00DA0000, 0x00DA0055, 0x00DA00AA, 0x00DA00FF, 0x00DA2400, 0x00DA2455, 0x00DA24AA, 0x00DA24FF,
    0x00DA4800, 0x00DA4855, 0x00DA48AA, 0x00DA48FF, 0x00DA6D00, 0x00DA6D55, 0x00DA6DAA, 0x00DA6DFF,
    0x00DA9100, 0x00DA9155, 0x00DA91AA, 0x00DA91FF, 0x00DAB600, 0x00DAB655, 0x00DAB6AA, 0x00DAB6FF,
    0x00DADA00, 0x00DADA55, 0x00DADAAA, 0x00DADAFF, 0x00DAFF00, 0x00DAFF55, 0x00DAFFAA, 0x00DAFFFF,
    0x00FF0000, 0x00FF0055, 0x00FF00AA, 0x00FF00FF, 0x00FF2400, 0x00FF2455, 0x00FF24AA, 0x00FF24FF,
    0x00FF4800, 0x00FF4855, 0x00FF48AA, 0x00FF48FF, 0x00FF6D00, 0x00FF6D55, 0x00FF6DAA, 0x00FF6DFF,
    0x00FF9100, 0x00FF9155, 0x00FF91AA, 0x00FF91FF, 0x00FFB600, 0x00FFB655, 0x00FFB6AA, 0x00FFB6FF,
    0x00FFDA00, 0x00FFDA55, 0x00FFDAAA, 0x00FFDAFF, 0x00FFFF00, 0x00FFFF55, 0x00FFFFAA, 0x00FFFFFF
};

//unsigned char r8g8b8_to_r3g3b2(uint8_t red, uint8_t green, uint8_t blue);
unsigned char r8g8b8_to_r3g3b2(uint8_t red, uint8_t green, uint8_t blue, float gamma, float contrast);
void floydSteinbergDither(uint8_t* data, int width, int height);

int main(int argc, char* argv[])
{
    FILE* fp;
    char infilename[MAX_FILENAME_LENGTH] = { 0 };
    char outfilename[MAX_FILENAME_LENGTH] = { 0 };    
    char array_name[MAX_FILENAME_LENGTH] = { 0 };

    int dither = 0;
    int debug = 0;

    for (int i = 1; i < argc; i++) {
        if (strcmp(argv[i], "-i") == 0 && i + 1 < argc) {
            strncpy(infilename, argv[i + 1], MAX_FILENAME_LENGTH - 1);
            printf("Input file: %s\n", infilename);
            i++;
        }
        else if (strcmp(argv[i], "-o") == 0 && i + 1 < argc) {
            strncpy(outfilename, argv[i + 1], MAX_FILENAME_LENGTH - 1);
            printf("Output file: %s\n", outfilename);
            i++;
        }
        else if (strcmp(argv[i], "-debug") == 0) {
            debug = 1;
            printf("Debug mode on\n");
        }
        else if (strcmp(argv[i], "-d") == 0) {
            dither = 1;
            printf("Dithering on\n");
        }
        else if (strcmp(argv[i], "-h") == 0) {
            printf("--| Help |--\n");
			printf("Usage: rgb332 -i <input file> -o <output file> [-d][-debug]\n");
            return 0;
        }
        else {
            printf("Unknown option: %s\n", argv[i]);
        }
    }

    if (infilename[0] == '\0') {
        printf("No input file specified.\n");
        return 1;
    }

    if (outfilename[0] == '\0') {
        printf("No output file specified.\n");
        return 1;
    }

    int x, y, n;
    uint8_t* data = stbi_load(infilename, &x, &y, &n, 3);

    if (data != NULL) {
        if (dither) {
            floydSteinbergDither(data, x, y);
        }

        fp = fopen(outfilename, "w");
        strncpy(array_name, outfilename, MAX_FILENAME_LENGTH - 1);
        char* dot = strrchr(array_name, '.');
        if (dot) *dot = '\0';

        if (fp != NULL) {
            fprintf(fp, "#ifndef %s_H\n", array_name);
            fprintf(fp, "#define %s_H\n\n", array_name);
            fprintf(fp, "#include \"image_types.h\"\n\n");

            // Output image_types.h content as a comment
            fprintf(fp, "/*\n");
            fprintf(fp, "Contents of image_types.h:\n\n");
            fprintf(fp, "#ifndef IMAGE_TYPES_H\n");
            fprintf(fp, "#define IMAGE_TYPES_H\n\n");
            fprintf(fp, "#include <stdint.h>\n\n");
            fprintf(fp, "#define RGB332_FORMAT_ID 0x332\n\n");
            fprintf(fp, "typedef struct {\n");
            fprintf(fp, "    const uint8_t* data;\n");
            fprintf(fp, "    uint16_t width;\n");
            fprintf(fp, "    uint16_t height;\n");
            fprintf(fp, "    uint16_t format_id;\n");
            fprintf(fp, "} Image_t;\n\n");
            fprintf(fp, "#endif // IMAGE_TYPES_H\n");
            fprintf(fp, "*/\n\n");

            fprintf(fp, "static const uint8_t %s_data[%d] = {\n", array_name, x * y);

            int _y = 0, _x = 0;
            uint8_t pixel332 = 0;
            uint8_t* pixel = NULL;

            for (_y = 0; _y < y; _y++) {
                for (_x = 0; _x < x; _x++) {
                    pixel = data + (_y * x + _x) * 3;
                    pixel332 = r8g8b8_to_r3g3b2(pixel[0], pixel[1], pixel[2], GAMMA, CONTRAST_FACTOR);
                    fprintf(fp, "0x%.2X, ", pixel332);
                    if (debug) {
                        pixel[0] = (r3g3b2_to_r8g8b8_lut[pixel332] >> 16) & 0xFF;
                        pixel[1] = (r3g3b2_to_r8g8b8_lut[pixel332] >> 8)  & 0xFF;
                        pixel[2] =  r3g3b2_to_r8g8b8_lut[pixel332]        & 0xFF;
                    }
                }
                fprintf(fp, "\n");
            }

            fprintf(fp, "};\n\n");
            fprintf(fp, "static const Image_t %s_image = {\n", array_name);
            fprintf(fp, "    .data = %s_data,\n", array_name);
            fprintf(fp, "    .width = %d,\n", x);
            fprintf(fp, "    .height = %d,\n", y);
            fprintf(fp, "    .format_id = RGB332_FORMAT_ID\n");
            fprintf(fp, "};\n\n");
            fprintf(fp, "#endif // %s_H\n", array_name);
            fclose(fp);
        }
        else {
            printf("Failed to open output file.\n");
        }

        if (debug) {
            stbi_write_bmp("debug_image.bmp", x, y, 3, data);
        }

        stbi_image_free(data);
    }
    else {
        printf("Failed to load image.\n");
        return 1;
    }

    return 0;
}

//unsigned char r8g8b8_to_r3g3b2(uint8_t red, uint8_t green, uint8_t blue) {
//    return ((red & 0xE0) | ((green & 0xE0) >> 3) | (blue >> 6));
//}

unsigned char r8g8b8_to_r3g3b2(uint8_t red, uint8_t green, uint8_t blue, float gamma, float contrast) {
    // Apply gamma correction
    float r = pow(red / 255.0f, gamma) * 255.0f;
    float g = pow(green / 255.0f, gamma) * 255.0f;
    float b = pow(blue / 255.0f, gamma) * 255.0f;

    // Apply contrast adjustment
    float factor = (259.0f * (contrast + 255.0f)) / (255.0f * (259.0f - contrast));
    r = factor * (r - 128.0f) + 128.0f;
    g = factor * (g - 128.0f) + 128.0f;
    b = factor * (b - 128.0f) + 128.0f;

    // Clamp values to 0-255 range
    r = fmaxf(0.0f, fminf(r, 255.0f));
    g = fmaxf(0.0f, fminf(g, 255.0f));
    b = fmaxf(0.0f, fminf(b, 255.0f));

    // Convert back to uint8_t
    uint8_t r_adj = (uint8_t)r;
    uint8_t g_adj = (uint8_t)g;
    uint8_t b_adj = (uint8_t)b;

    // Perform the original conversion
    return ((r_adj & 0xE0) | ((g_adj & 0xE0) >> 3) | (b_adj >> 6));
}


void floydSteinbergDither(uint8_t* data, int width, int height) {
    for (int y = 0; y < height; y++) {
        for (int x = 0; x < width; x++) {
            uint8_t oldR = data[(y * width + x) * 3];
            uint8_t oldG = data[(y * width + x) * 3 + 1];
            uint8_t oldB = data[(y * width + x) * 3 + 2];

            uint8_t newR = (oldR & 0xE0);
            uint8_t newG = (oldG & 0xE0);
            uint8_t newB = (oldB & 0xC0);

            data[(y * width + x) * 3]     = newR;
            data[(y * width + x) * 3 + 1] = newG;
            data[(y * width + x) * 3 + 2] = newB;

            int errorR = oldR - newR;
            int errorG = oldG - newG;
            int errorB = oldB - newB;

            if (x < width - 1) {
                data[(y * width + x + 1) * 3]     = min(255, max(0, data[(y * width + x + 1) * 3] + errorR * 7 / 16));
                data[(y * width + x + 1) * 3 + 1] = min(255, max(0, data[(y * width + x + 1) * 3 + 1] + errorG * 7 / 16));
                data[(y * width + x + 1) * 3 + 2] = min(255, max(0, data[(y * width + x + 1) * 3 + 2] + errorB * 7 / 16));
            }

            if (y < height - 1) {
                if (x > 0) {
                    data[((y + 1) * width + x - 1) * 3]     = min(255, max(0, data[((y + 1) * width + x - 1) * 3] + errorR * 3 / 16));
                    data[((y + 1) * width + x - 1) * 3 + 1] = min(255, max(0, data[((y + 1) * width + x - 1) * 3 + 1] + errorG * 3 / 16));
                    data[((y + 1) * width + x - 1) * 3 + 2] = min(255, max(0, data[((y + 1) * width + x - 1) * 3 + 2] + errorB * 3 / 16));
                }

                data[((y + 1) * width + x) * 3]     = min(255, max(0, data[((y + 1) * width + x) * 3] + errorR * 5 / 16));
                data[((y + 1) * width + x) * 3 + 1] = min(255, max(0, data[((y + 1) * width + x) * 3 + 1] + errorG * 5 / 16));
                data[((y + 1) * width + x) * 3 + 2] = min(255, max(0, data[((y + 1) * width + x) * 3 + 2] + errorB * 5 / 16));

                if (x < width - 1) {
                    data[((y + 1) * width + x + 1) * 3]     = min(255, max(0, data[((y + 1) * width + x + 1) * 3] + errorR * 1 / 16));
                    data[((y + 1) * width + x + 1) * 3 + 1] = min(255, max(0, data[((y + 1) * width + x + 1) * 3 + 1] + errorG * 1 / 16));
                    data[((y + 1) * width + x + 1) * 3 + 2] = min(255, max(0, data[((y + 1) * width + x + 1) * 3 + 2] + errorB * 1 / 16));
                }
            }
        }
    }
}

//#include <stdio.h>
//#include <stdint.h>
//#include <math.h>
//
//#define GAMMA 1.0
//#define CONTRAST_FACTOR 1.0
//
//uint32_t generate_corrected_color(uint8_t value, uint8_t max_value) {
//    double normalized = (double)value / max_value;
//    double corrected = pow(normalized, GAMMA) * 255 * CONTRAST_FACTOR;
//    return (uint32_t)(fmin(fmax(corrected, 0), 255));
//}
//
//void generate_brightness_corrected_lut(uint32_t lut[256]) {
//    for (int i = 0; i < 256; i++) {
//        uint8_t r = (i >> 5) & 0x07;
//        uint8_t g = (i >> 2) & 0x07;
//        uint8_t b = i & 0x03;
//
//        uint32_t r8 = generate_corrected_color(r, 7);
//        uint32_t g8 = generate_corrected_color(g, 7);
//        uint32_t b8 = generate_corrected_color(b, 3);
//
//        lut[i] = (r8 << 16) | (g8 << 8) | b8;
//    }
//}
//
//int main() {
//    uint32_t corrected_lut[256];
//    generate_brightness_corrected_lut(corrected_lut);
//
//    printf("static const uint32_t r3g3b2_to_r8g8b8_lut[256] = {\n");
//    for (int i = 0; i < 256; i++) {
//        printf("0x%08X", corrected_lut[i]);
//        if (i < 255) printf(", ");
//        if ((i + 1) % 8 == 0) printf("\n");
//    }
//    printf("};\n");
//
//    return 0;
//}