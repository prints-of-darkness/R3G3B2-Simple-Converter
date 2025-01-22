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

// Precomputed lookup table for r3g3b2 to r8g8b8 conversion
static const uint32_t r3g3b2_to_r8g8b8_lut[256] = {
    0x000000, 0x000055, 0x0000AA, 0x0000FF, 0x002100, 0x002155, 0x0021AA, 0x0021FF,
    0x004200, 0x004255, 0x0042AA, 0x0042FF, 0x006300, 0x006355, 0x0063AA, 0x0063FF,
    0x008400, 0x008455, 0x0084AA, 0x0084FF, 0x00A500, 0x00A555, 0x00A5AA, 0x00A5FF,
    0x00C600, 0x00C655, 0x00C6AA, 0x00C6FF, 0x00E700, 0x00E755, 0x00E7AA, 0x00E7FF,
    0x240000, 0x240055, 0x2400AA, 0x2400FF, 0x242100, 0x242155, 0x2421AA, 0x2421FF,
    0x244200, 0x244255, 0x2442AA, 0x2442FF, 0x246300, 0x246355, 0x2463AA, 0x2463FF,
    0x248400, 0x248455, 0x2484AA, 0x2484FF, 0x24A500, 0x24A555, 0x24A5AA, 0x24A5FF,
    0x24C600, 0x24C655, 0x24C6AA, 0x24C6FF, 0x24E700, 0x24E755, 0x24E7AA, 0x24E7FF,
    0x480000, 0x480055, 0x4800AA, 0x4800FF, 0x482100, 0x482155, 0x4821AA, 0x4821FF,
    0x484200, 0x484255, 0x4842AA, 0x4842FF, 0x486300, 0x486355, 0x4863AA, 0x4863FF,
    0x488400, 0x488455, 0x4884AA, 0x4884FF, 0x48A500, 0x48A555, 0x48A5AA, 0x48A5FF,
    0x48C600, 0x48C655, 0x48C6AA, 0x48C6FF, 0x48E700, 0x48E755, 0x48E7AA, 0x48E7FF,
    0x6C0000, 0x6C0055, 0x6C00AA, 0x6C00FF, 0x6C2100, 0x6C2155, 0x6C21AA, 0x6C21FF,
    0x6C4200, 0x6C4255, 0x6C42AA, 0x6C42FF, 0x6C6300, 0x6C6355, 0x6C63AA, 0x6C63FF,
    0x6C8400, 0x6C8455, 0x6C84AA, 0x6C84FF, 0x6CA500, 0x6CA555, 0x6CA5AA, 0x6CA5FF,
    0x6CC600, 0x6CC655, 0x6CC6AA, 0x6CC6FF, 0x6CE700, 0x6CE755, 0x6CE7AA, 0x6CE7FF,
    0x900000, 0x900055, 0x9000AA, 0x9000FF, 0x902100, 0x902155, 0x9021AA, 0x9021FF,
    0x904200, 0x904255, 0x9042AA, 0x9042FF, 0x906300, 0x906355, 0x9063AA, 0x9063FF,
    0x908400, 0x908455, 0x9084AA, 0x9084FF, 0x90A500, 0x90A555, 0x90A5AA, 0x90A5FF,
    0x90C600, 0x90C655, 0x90C6AA, 0x90C6FF, 0x90E700, 0x90E755, 0x90E7AA, 0x90E7FF,
    0xB40000, 0xB40055, 0xB400AA, 0xB400FF, 0xB42100, 0xB42155, 0xB421AA, 0xB421FF,
    0xB44200, 0xB44255, 0xB442AA, 0xB442FF, 0xB46300, 0xB46355, 0xB463AA, 0xB463FF,
    0xB48400, 0xB48455, 0xB484AA, 0xB484FF, 0xB4A500, 0xB4A555, 0xB4A5AA, 0xB4A5FF,
    0xB4C600, 0xB4C655, 0xB4C6AA, 0xB4C6FF, 0xB4E700, 0xB4E755, 0xB4E7AA, 0xB4E7FF,
    0xD80000, 0xD80055, 0xD800AA, 0xD800FF, 0xD82100, 0xD82155, 0xD821AA, 0xD821FF,
    0xD84200, 0xD84255, 0xD842AA, 0xD842FF, 0xD86300, 0xD86355, 0xD863AA, 0xD863FF,
    0xD88400, 0xD88455, 0xD884AA, 0xD884FF, 0xD8A500, 0xD8A555, 0xD8A5AA, 0xD8A5FF,
    0xD8C600, 0xD8C655, 0xD8C6AA, 0xD8C6FF, 0xD8E700, 0xD8E755, 0xD8E7AA, 0xD8E7FF,
    0xFC0000, 0xFC0055, 0xFC00AA, 0xFC00FF, 0xFC2100, 0xFC2155, 0xFC21AA, 0xFC21FF,
    0xFC4200, 0xFC4255, 0xFC42AA, 0xFC42FF, 0xFC6300, 0xFC6355, 0xFC63AA, 0xFC63FF,
    0xFC8400, 0xFC8455, 0xFC84AA, 0xFC84FF, 0xFCA500, 0xFCA555, 0xFCA5AA, 0xFCA5FF,
    0xFCC600, 0xFCC655, 0xFCC6AA, 0xFCC6FF, 0xFCE700, 0xFCE755, 0xFCE7AA, 0xFCE7FF
};

unsigned char r8g8b8_to_r3g3b2(uint8_t red, uint8_t green, uint8_t blue);
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
                    pixel332 = r8g8b8_to_r3g3b2(pixel[0], pixel[1], pixel[2]);
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

unsigned char r8g8b8_to_r3g3b2(uint8_t red, uint8_t green, uint8_t blue) {
    return ((red & 0xE0) | ((green & 0xE0) >> 3) | (blue >> 6));
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

