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

uint8_t gamma_contrast_lut[256];
uint32_t color_lut[256];            //r3g3b2 to r8g8b8 LUT

void init_color_lut(void);
void init_gamma_contrast_lut(float gamma, float contrast);
void floydSteinbergDither(uint8_t* data, int width, int height);
uint8_t r8g8b8_to_r3g3b2(uint8_t red, uint8_t green, uint8_t blue);

int main(int argc, char* argv[])
{
    FILE* fp;

    uint8_t* pixel = NULL;
    uint8_t* data = NULL;

    uint8_t pixel332 = 0;

    float gamma = 1.0f;             // Default gamma value
    float contrast = 0.0f;          // Default contrast value

    int dither = 0, debug = 0;      // Default modes set to false
    int x = 0, y = 0, n = 0, _y = 0, _x = 0;

    char infilename[MAX_FILENAME_LENGTH] = { 0 };
    char outfilename[MAX_FILENAME_LENGTH] = { 0 };    
    char array_name[MAX_FILENAME_LENGTH] = { 0 };

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
        else if (strcmp(argv[i], "-g") == 0 && i + 1 < argc) {
            gamma = (float)atof(argv[i + 1]);
            printf("Gamma set to: %.2f\n", gamma);
            i++;
        }
        else if (strcmp(argv[i], "-c") == 0 && i + 1 < argc) {
            contrast = (float)atof(argv[i + 1]);
            printf("Contrast set to: %.2f\n", contrast);
            i++;
        }
        else if (strcmp(argv[i], "-h") == 0) {
            printf("Usage: rgb332 -i <input file> -o <output file> [-d] [-debug] [-g <gamma>] [-c <contrast>]\n");
            printf("  -i <input file>   : Specify input file\n");
            printf("  -o <output file>  : Specify output file\n");
            printf("  -d                : Enable dithering\n");
            printf("  -debug            : Enable debug mode\n");
            printf("  -g <gamma>        : Set gamma value (default: 1.0)\n");
            printf("  -c <contrast>     : Set contrast value (default: 0.0)\n");
            printf("  -h                : Display this help message\n");
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

    init_color_lut();
    init_gamma_contrast_lut(gamma, contrast);
    stbi_load(infilename, &x, &y, &n, 3);

    if ((data = stbi_load(infilename, &x, &y, &n, 3)) != NULL) {
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

            for (_y = 0; _y < y; _y++) {
                for (_x = 0; _x < x; _x++) {
                    pixel = data + (_y * x + _x) * 3;
                    pixel332 = r8g8b8_to_r3g3b2(pixel[0], pixel[1], pixel[2]);
                    fprintf(fp, "0x%.2X, ", pixel332);
                    if (debug) {
                        pixel[0] = (color_lut[pixel332] >> 16) & 0xFF;
                        pixel[1] = (color_lut[pixel332] >> 8)  & 0xFF;
                        pixel[2] =  color_lut[pixel332]        & 0xFF;
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

uint8_t r8g8b8_to_r3g3b2(uint8_t red, uint8_t green, uint8_t blue) {
    uint8_t r_adj = gamma_contrast_lut[red];
    uint8_t g_adj = gamma_contrast_lut[green];
    uint8_t b_adj = gamma_contrast_lut[blue];

    return ((r_adj & 0xE0) | ((g_adj & 0xE0) >> 3) | (b_adj >> 6));
}

void init_gamma_contrast_lut(float gamma, float contrast) {
    int i = 0;
    float value = 0.0f;
    float factor = (259.0f * (contrast + 255.0f)) / (255.0f * (259.0f - contrast));
    for (i = 0; i < 256; i++) {
        value = (float)pow(i / 255.0f, gamma) * 255.0f;
        value = factor * (value - 128.0f) + 128.0f;

        gamma_contrast_lut[i] = (uint8_t)fmaxf(0.0f, fminf(value, 255.0f));
    }
}

void init_color_lut(void) {
    int i = 0;
    uint8_t r3 = 0, g3 = 0, b2 = 0;
    uint8_t r8 = 0, g8 = 0, b8 = 0;

    for (i = 0; i < 256; i++) {
        r3 = (i >> 5) & 0x7;
        g3 = (i >> 2) & 0x7;
        b2 = i & 0x3;

        r8 = (r3 << 5) | (r3 << 2) | (r3 >> 1);
        g8 = (g3 << 5) | (g3 << 2) | (g3 >> 1);
        b8 = (b2 << 6) | (b2 << 4) | (b2 << 2) | b2;

        color_lut[i] = (r8 << 16) | (g8 << 8) | b8;
    }
}

void floydSteinbergDither(uint8_t* data, int width, int height) {
    int x = 0, y = 0;
    uint8_t oldR = 0, oldG = 0, oldB = 0;
    uint8_t newR = 0, newG = 0, newB = 0;
    int errorR = 0, errorG = 0, errorB = 0;

    for (y = 0; y < height; y++) {
        for (x = 0; x < width; x++) {
            oldR = data[(y * width + x) * 3];
            oldG = data[(y * width + x) * 3 + 1];
            oldB = data[(y * width + x) * 3 + 2];

            newR = (oldR & 0xE0);
            newG = (oldG & 0xE0);
            newB = (oldB & 0xC0);

            data[(y * width + x) * 3]     = newR;
            data[(y * width + x) * 3 + 1] = newG;
            data[(y * width + x) * 3 + 2] = newB;

            errorR = oldR - newR;
            errorG = oldG - newG;
            errorB = oldB - newB;

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
