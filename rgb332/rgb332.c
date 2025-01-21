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

#define MAX_FILENAME_LENGTH 256

unsigned char rgbTo332(unsigned char red, unsigned char green, unsigned char blue);
void floydSteinbergDither(uint8_t* data, int width, int height);

int main(int argc, char* argv[])
{
    FILE* fp;
    char infilename[MAX_FILENAME_LENGTH] = { 0 };
    char outfilename[MAX_FILENAME_LENGTH] = { 0 };
    char array_name[MAX_FILENAME_LENGTH] = { 0 };

    int dither = 0;

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
        else if (strcmp(argv[i], "-d") == 0) {
            dither = 1;
            printf("Dithering on\n");
        }
        else if (strcmp(argv[i], "-h") == 0) {
            printf("--| Help |--\n");
			printf("Usage: rgb332 -i <input file> -o <output file> [-d]\n");
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
                    pixel332 = rgbTo332(pixel[0], pixel[1], pixel[2]);
                    fprintf(fp, "0x%.2X, ", pixel332);
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
        stbi_image_free(data);
    }
    else {
        printf("Failed to load image.\n");
        return 1;
    }

    return 0;
}

unsigned char rgbTo332(unsigned char red, unsigned char green, unsigned char blue) {
    return ((red & 0xE0) | ((green & 0xE0) >> 3) | (blue >> 6));
}

//void floydSteinbergDither(uint8_t* data, int width, int height) {
//    for (int y = 0; y < height; y++) {
//        for (int x = 0; x < width; x++) {
//            for (int c = 0; c < 3; c++) {
//                int index = (y * width + x) * 3 + c;
//                int oldPixel = data[index];
//                int newPixel = oldPixel;
//
//                data[index] = newPixel;
//                int error = oldPixel - newPixel;
//
//                if (x < width - 1)
//                    data[index + 3] = (data[index + 3] + error * 7 / 16) > 255 ? 255 : ((data[index + 3] + error * 7 / 16) < 0 ? 0 : (data[index + 3] + error * 7 / 16));
//
//                if (y < height - 1) {
//                    if (x > 0)
//                        data[index + 3 * (width - 1)] = (data[index + 3 * (width - 1)] + error * 3 / 16) > 255 ? 255 : ((data[index + 3 * (width - 1)] + error * 3 / 16) < 0 ? 0 : (data[index + 3 * (width - 1)] + error * 3 / 16));
//
//                    data[index + 3 * width] = (data[index + 3 * width] + error * 5 / 16) > 255 ? 255 : ((data[index + 3 * width] + error * 5 / 16) < 0 ? 0 : (data[index + 3 * width] + error * 5 / 16));
//
//                    if (x < width - 1)
//                        data[index + 3 * (width + 1)] = (data[index + 3 * (width + 1)] + error * 1 / 16) > 255 ? 255 : ((data[index + 3 * (width + 1)] + error * 1 / 16) < 0 ? 0 : (data[index + 3 * (width + 1)] + error * 1 / 16));
//                }
//            }
//        }
//    }
//}

void floydSteinbergDither(uint8_t* data, int width, int height) {
    for (int y = 0; y < height; y++) {
        for (int x = 0; x < width; x++) {
            uint8_t oldR = data[(y * width + x) * 3];
            uint8_t oldG = data[(y * width + x) * 3 + 1];
            uint8_t oldB = data[(y * width + x) * 3 + 2];

            uint8_t newR = (oldR & 0xE0);
            uint8_t newG = (oldG & 0xE0);
            uint8_t newB = (oldB & 0xC0);

            data[(y * width + x) * 3] = newR;
            data[(y * width + x) * 3 + 1] = newG;
            data[(y * width + x) * 3 + 2] = newB;

            int errorR = oldR - newR;
            int errorG = oldG - newG;
            int errorB = oldB - newB;

            if (x < width - 1) {
                data[(y * width + x + 1) * 3] = min(255, max(0, data[(y * width + x + 1) * 3] + errorR * 7 / 16));
                data[(y * width + x + 1) * 3 + 1] = min(255, max(0, data[(y * width + x + 1) * 3 + 1] + errorG * 7 / 16));
                data[(y * width + x + 1) * 3 + 2] = min(255, max(0, data[(y * width + x + 1) * 3 + 2] + errorB * 7 / 16));
            }

            if (y < height - 1) {
                if (x > 0) {
                    data[((y + 1) * width + x - 1) * 3] = min(255, max(0, data[((y + 1) * width + x - 1) * 3] + errorR * 3 / 16));
                    data[((y + 1) * width + x - 1) * 3 + 1] = min(255, max(0, data[((y + 1) * width + x - 1) * 3 + 1] + errorG * 3 / 16));
                    data[((y + 1) * width + x - 1) * 3 + 2] = min(255, max(0, data[((y + 1) * width + x - 1) * 3 + 2] + errorB * 3 / 16));
                }

                data[((y + 1) * width + x) * 3] = min(255, max(0, data[((y + 1) * width + x) * 3] + errorR * 5 / 16));
                data[((y + 1) * width + x) * 3 + 1] = min(255, max(0, data[((y + 1) * width + x) * 3 + 1] + errorG * 5 / 16));
                data[((y + 1) * width + x) * 3 + 2] = min(255, max(0, data[((y + 1) * width + x) * 3 + 2] + errorB * 5 / 16));

                if (x < width - 1) {
                    data[((y + 1) * width + x + 1) * 3] = min(255, max(0, data[((y + 1) * width + x + 1) * 3] + errorR * 1 / 16));
                    data[((y + 1) * width + x + 1) * 3 + 1] = min(255, max(0, data[((y + 1) * width + x + 1) * 3 + 1] + errorG * 1 / 16));
                    data[((y + 1) * width + x + 1) * 3 + 2] = min(255, max(0, data[((y + 1) * width + x + 1) * 3 + 2] + errorB * 1 / 16));
                }
            }
        }
    }
}

