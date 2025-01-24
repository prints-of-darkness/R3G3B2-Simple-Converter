/*
    rgb332.c

    Converts an RGB image to 8bpp RGB332. (needed for LT7683 TFT graphics controller)

    MJM 2025

*/

#include <stdio.h>
#include <stdint.h>
#include <stdlib.h>
#include <string.h>

#define STB_IMAGE_IMPLEMENTATION
#include "stb_image.h"

#define STB_IMAGE_WRITE_IMPLEMENTATION
#include "stb_image_write.h"

#define LUT_SIZE 256
#define MAX_FILENAME_LENGTH 1024

uint8_t gamma_lut[LUT_SIZE];
uint32_t color_lut[LUT_SIZE];
uint8_t contrast_brightness_lut[LUT_SIZE];

void process_image_lut(uint8_t* data, int width, int height);
void floydSteinbergDither(uint8_t* data, int width, int height);
void initialize_luts(float gamma, float contrast, float brightness);

int main(int argc, char* argv[])
{
    FILE* fp;

    uint8_t* pixel = NULL;
    uint8_t* data = NULL;

    uint8_t pixel332 = 0;

    float gamma      = 1.0f;        // Default gamma value          [ 0.8f,    1.0f, 2.0f   ]
    float contrast   = 0.0f;        // Default contrast value       [ -100.0f, 0,0f, 100.0f ]
    float brightness = 1.0f;        // Local brightness variable    [ 0.5f,    1.0f, 1.5f   ]

    int dither = 0, debug = 0;      // Default modes set to false
    int x = 0, y = 0, n = 0;
    int _y = 0, _x = 0;

    char infilename[MAX_FILENAME_LENGTH]  = { 0 }; //.bmp, .jpg, .png
    char outfilename[MAX_FILENAME_LENGTH] = { 0 };    
    char array_name[MAX_FILENAME_LENGTH]  = { 0 };

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
        else if (strcmp(argv[i], "-b") == 0 && i + 1 < argc) {
            brightness = (float)atof(argv[i + 1]);
            printf("Brightness set to: %.2f\n", brightness);
            i++;
        }
        else if (strcmp(argv[i], "-h") == 0) {
            printf("Usage: rgb332 -i <input file> -o <output file> [-d] [-debug] [-g <gamma>] [-c <contrast>] [-b <brightness>]\n");
            printf("  -i <input file>   : Specify input file\n");
            printf("  -o <output file>  : Specify output file\n");
            printf("  -d                : Enable dithering\n");
            printf("  -debug            : Enable debug mode\n");
            printf("  -g <gamma>        : Set gamma value (default: 1.0)\n");
            printf("  -c <contrast>     : Set contrast value (default: 0.0)\n");
            printf("  -b <brightness>   : Set brightness value (default: 1.0)\n");
            printf("  -h                : Display this help message\n");
            printf("Example: rgb332 -i tst.png -o tst.h -d -debug -g 1.0 -c 0.0 -b 1.0\n");
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

    initialize_luts(gamma, contrast, brightness);
    if ((data = stbi_load(infilename, &x, &y, &n, 3)) != NULL) {
        process_image_lut(data, x, y);

        if (debug) {
            stbi_write_bmp("processed_image.bmp", x, y, 3, data);
        }

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

                    // convert 888 to 332
                    pixel332 = ((pixel[0] & 0xE0) | ((pixel[1] & 0xE0) >> 3) | (pixel[2] >> 6));
                    fprintf(fp, "0x%.2X, ", pixel332);
                    if (debug) {
                        // back convert to 888 for debug_image.bmp
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

void initialize_luts(float gamma, float contrast, float brightness) {
   
    uint8_t r3 = 0, g3 = 0, b2 = 0;
    uint8_t r8 = 0, g8 = 0, b8 = 0;

    float value = 0;
    float factor = (259.0f * (contrast + 255.0f)) / (255.0f * (259.0f - contrast));

    for (int i = 0; i < LUT_SIZE; i++) {
        value = i / 255.0f;

        // Expand to 888
        r3 = (i >> 5) & 0x7;
        g3 = (i >> 2) & 0x7;
        b2 = i & 0x3;

        // Prep for LUT
        r8 = (r3 << 5) | (r3 << 2) | (r3 >> 1);
        g8 = (g3 << 5) | (g3 << 2) | (g3 >> 1);
        b8 = (b2 << 6) | (b2 << 4) | (b2 << 2) | b2;

        // Gamma correction
        value = powf(value, 1.0f / gamma);

        // Brightness and contrast
        value = factor * (value * brightness - 0.5f) + 0.5f;

        // Clamp and convert back to 8-bit
        value = fmaxf(0.0f, fminf(value, 1.0f));
        contrast_brightness_lut[i] = (uint8_t)(value * 255.0f);

        // Separate gamma LUT for flexibility
        gamma_lut[i] = (uint8_t)(powf(i / 255.0f, 1.0f / gamma) * 255.0f);

        // Assign 32bits to color LUT
        color_lut[i] = (r8 << 16) | (g8 << 8) | b8;
    }
}

void process_image_lut(uint8_t* data, int width, int height) {
    int x = 0, y = 0, idx = 0;
    for (y = 0; y < height; y++) {
        for (x = 0; x < width; x++) {
            idx = (y * width + x) * 3;
            data[idx]     = contrast_brightness_lut[gamma_lut[data[idx]]];
            data[idx + 1] = contrast_brightness_lut[gamma_lut[data[idx + 1]]];
            data[idx + 2] = contrast_brightness_lut[gamma_lut[data[idx + 2]]];
        }
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
