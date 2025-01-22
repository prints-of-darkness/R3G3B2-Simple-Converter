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

/*
    GAMMA 0.8
    CONTRAST_FACTOR 1.0
*/
static const uint32_t r3g3b2_to_r8g8b8_lut[256] = {
    0x00000000, 0x0000004C, 0x000000A3, 0x000000FF, 0x00001D00, 0x00001D4C, 0x00001DA3, 0x00001DFF,
    0x00004000, 0x0000404C, 0x000040A3, 0x000040FF, 0x00006400, 0x0000644C, 0x000064A3, 0x000064FF,
    0x00008900, 0x0000894C, 0x000089A3, 0x000089FF, 0x0000B000, 0x0000B04C, 0x0000B0A3, 0x0000B0FF,
    0x0000D700, 0x0000D74C, 0x0000D7A3, 0x0000D7FF, 0x0000FF00, 0x0000FF4C, 0x0000FFA3, 0x0000FFFF,
    0x001D0000, 0x001D004C, 0x001D00A3, 0x001D00FF, 0x001D1D00, 0x001D1D4C, 0x001D1DA3, 0x001D1DFF,
    0x001D4000, 0x001D404C, 0x001D40A3, 0x001D40FF, 0x001D6400, 0x001D644C, 0x001D64A3, 0x001D64FF,
    0x001D8900, 0x001D894C, 0x001D89A3, 0x001D89FF, 0x001DB000, 0x001DB04C, 0x001DB0A3, 0x001DB0FF,
    0x001DD700, 0x001DD74C, 0x001DD7A3, 0x001DD7FF, 0x001DFF00, 0x001DFF4C, 0x001DFFA3, 0x001DFFFF,
    0x00400000, 0x0040004C, 0x004000A3, 0x004000FF, 0x00401D00, 0x00401D4C, 0x00401DA3, 0x00401DFF,
    0x00404000, 0x0040404C, 0x004040A3, 0x004040FF, 0x00406400, 0x0040644C, 0x004064A3, 0x004064FF,
    0x00408900, 0x0040894C, 0x004089A3, 0x004089FF, 0x0040B000, 0x0040B04C, 0x0040B0A3, 0x0040B0FF,
    0x0040D700, 0x0040D74C, 0x0040D7A3, 0x0040D7FF, 0x0040FF00, 0x0040FF4C, 0x0040FFA3, 0x0040FFFF,
    0x00640000, 0x0064004C, 0x006400A3, 0x006400FF, 0x00641D00, 0x00641D4C, 0x00641DA3, 0x00641DFF,
    0x00644000, 0x0064404C, 0x006440A3, 0x006440FF, 0x00646400, 0x0064644C, 0x006464A3, 0x006464FF,
    0x00648900, 0x0064894C, 0x006489A3, 0x006489FF, 0x0064B000, 0x0064B04C, 0x0064B0A3, 0x0064B0FF,
    0x0064D700, 0x0064D74C, 0x0064D7A3, 0x0064D7FF, 0x0064FF00, 0x0064FF4C, 0x0064FFA3, 0x0064FFFF,
    0x00890000, 0x0089004C, 0x008900A3, 0x008900FF, 0x00891D00, 0x00891D4C, 0x00891DA3, 0x00891DFF,
    0x00894000, 0x0089404C, 0x008940A3, 0x008940FF, 0x00896400, 0x0089644C, 0x008964A3, 0x008964FF,
    0x00898900, 0x0089894C, 0x008989A3, 0x008989FF, 0x0089B000, 0x0089B04C, 0x0089B0A3, 0x0089B0FF,
    0x0089D700, 0x0089D74C, 0x0089D7A3, 0x0089D7FF, 0x0089FF00, 0x0089FF4C, 0x0089FFA3, 0x0089FFFF,
    0x00B00000, 0x00B0004C, 0x00B000A3, 0x00B000FF, 0x00B01D00, 0x00B01D4C, 0x00B01DA3, 0x00B01DFF,
    0x00B04000, 0x00B0404C, 0x00B040A3, 0x00B040FF, 0x00B06400, 0x00B0644C, 0x00B064A3, 0x00B064FF,
    0x00B08900, 0x00B0894C, 0x00B089A3, 0x00B089FF, 0x00B0B000, 0x00B0B04C, 0x00B0B0A3, 0x00B0B0FF,
    0x00B0D700, 0x00B0D74C, 0x00B0D7A3, 0x00B0D7FF, 0x00B0FF00, 0x00B0FF4C, 0x00B0FFA3, 0x00B0FFFF,
    0x00D70000, 0x00D7004C, 0x00D700A3, 0x00D700FF, 0x00D71D00, 0x00D71D4C, 0x00D71DA3, 0x00D71DFF,
    0x00D74000, 0x00D7404C, 0x00D740A3, 0x00D740FF, 0x00D76400, 0x00D7644C, 0x00D764A3, 0x00D764FF,
    0x00D78900, 0x00D7894C, 0x00D789A3, 0x00D789FF, 0x00D7B000, 0x00D7B04C, 0x00D7B0A3, 0x00D7B0FF,
    0x00D7D700, 0x00D7D74C, 0x00D7D7A3, 0x00D7D7FF, 0x00D7FF00, 0x00D7FF4C, 0x00D7FFA3, 0x00D7FFFF,
    0x00FF0000, 0x00FF004C, 0x00FF00A3, 0x00FF00FF, 0x00FF1D00, 0x00FF1D4C, 0x00FF1DA3, 0x00FF1DFF,
    0x00FF4000, 0x00FF404C, 0x00FF40A3, 0x00FF40FF, 0x00FF6400, 0x00FF644C, 0x00FF64A3, 0x00FF64FF,
    0x00FF8900, 0x00FF894C, 0x00FF89A3, 0x00FF89FF, 0x00FFB000, 0x00FFB04C, 0x00FFB0A3, 0x00FFB0FF,
    0x00FFD700, 0x00FFD74C, 0x00FFD7A3, 0x00FFD7FF, 0x00FFFF00, 0x00FFFF4C, 0x00FFFFA3, 0x00FFFFFF
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

