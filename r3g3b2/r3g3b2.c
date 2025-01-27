/*
    r3g3b2.c

    Converts an RGB image to 8bpp RGB332. (needed for LT7683 TFT graphics controller)

    MJM 2025

    also, remember to get a roast beef sandwich this week, its imperative.

*/

#include <stdio.h>
#include <stdint.h>
#include <stdlib.h>
#include <string.h>
#include <math.h>

#include "bayer16x16.h"
#include "color_lut.h"
//#include "blue_noise.h" //actually white not blue

#define STB_IMAGE_IMPLEMENTATION
#include "stb_image.h"

#define STB_IMAGE_WRITE_IMPLEMENTATION
#include "stb_image_write.h"

#define LUT_SIZE 256
#define MAX_FILENAME_LENGTH 1024

// --- Data Structures ---
typedef struct {
    char infilename[MAX_FILENAME_LENGTH];
    char outfilename[MAX_FILENAME_LENGTH];
    char debug_filename[MAX_FILENAME_LENGTH];
    int dither_method;
    float gamma;
    float contrast;
    float brightness;
    int debug_mode;
} ProgramOptions;

typedef struct {
    uint8_t* data;
    int width;
    int height;
} ImageData;

// --- Function Prototypes ---
// LUT and image processing
void initialize_luts(float gamma, float contrast, float brightness, uint8_t* gamma_lut, uint8_t* contrast_brightness_lut);
void process_image_lut(ImageData* image, const uint8_t* gamma_lut, const uint8_t* contrast_brightness_lut);
uint8_t rgbToRgb332(uint8_t r, uint8_t g, uint8_t b);
void rgb332ToRgb(uint8_t rgb332, uint8_t* r, uint8_t* g, uint8_t* b);

// Dithering algorithms
typedef void (*DitherFunc)(ImageData* image);
void floydSteinbergDither(ImageData* image);
void jarvisDither(ImageData* image);
void atkinsonDither(ImageData* image);
void bayer16x16Dither(ImageData* image);
//void blueNoiseDither(ImageData* image);

// File IO
int write_image_data_to_file(const char* filename, const char* array_name, const ImageData* image);
int load_image(const char* filename, ImageData* image);
void free_image_data(ImageData* image);

// Command line processing
void init_program_options(ProgramOptions* opts);
int parse_command_line_args(int argc, char* argv[], ProgramOptions* opts);
int process_image(ProgramOptions* opts);

// --- Helper Functions ---

//  Error reporting for failed memory allocation
void mem_alloc_failed(const char* location)
{
    fprintf(stderr, "Memory allocation failed in %s\n", location);
    exit(EXIT_FAILURE);
}

// --- LUT and image processing functions ---

// Function to initialize the look-up tables (LUTs) for color conversion
void initialize_luts(float gamma, float contrast, float brightness, uint8_t* gamma_lut, uint8_t* contrast_brightness_lut) {
    float value, factor;

    if (gamma_lut == NULL || contrast_brightness_lut == NULL) {
        fprintf(stderr, "Error: Null pointer passed to LUT initialisation.");
        exit(EXIT_FAILURE);
    }

    factor = (259.0f * (contrast + 255.0f)) / (255.0f * (259.0f - contrast));

    for (int i = 0; i < LUT_SIZE; i++) {
        value = i / 255.0f;
        value = powf(value, 1.0f / gamma);
        value = factor * (value * brightness - 0.5f) + 0.5f;
        value = fmaxf(0.0f, fminf(value, 1.0f));
        contrast_brightness_lut[i] = (uint8_t)(value * 255.0f);
        gamma_lut[i] = (uint8_t)(powf(i / 255.0f, 1.0f / gamma) * 255.0f);
    }
    return;
}

void process_image_lut(ImageData* image, const uint8_t* gamma_lut, const uint8_t* contrast_brightness_lut) {

    if (image == NULL || image->data == NULL || gamma_lut == NULL || contrast_brightness_lut == NULL)
    {
        fprintf(stderr, "Error: Null pointer passed to process_image_lut.");
        return;
    }

    for (int y = 0; y < image->height; y++) {
        for (int x = 0; x < image->width; x++) {
            int idx = (y * image->width + x) * 3;

            image->data[idx]     = contrast_brightness_lut[gamma_lut[image->data[idx]]];
            image->data[idx + 1] = contrast_brightness_lut[gamma_lut[image->data[idx + 1]]];
            image->data[idx + 2] = contrast_brightness_lut[gamma_lut[image->data[idx + 2]]];
        }
    }
}

uint8_t rgbToRgb332(uint8_t r, uint8_t g, uint8_t b) {
    return ((r & 0xE0) | ((g & 0xE0) >> 3) | (b >> 6));
}

void rgb332ToRgb(uint8_t rgb332, uint8_t* r, uint8_t* g, uint8_t* b) {
    if (r == NULL || g == NULL || b == NULL) {
        fprintf(stderr, "Error: Null pointer passed to rgb332ToRgb.");
        return;
    }
    *r = (color_lut[rgb332] >> 16) & 0xFF;
    *g = (color_lut[rgb332] >> 8) & 0xFF;
    *b = color_lut[rgb332] & 0xFF;
}

// --- Dithering Algorithm Functions ---

//void blueNoiseDither(ImageData* image) {
//    int width = image->width;
//    int height = image->height;
//
//    if (image == NULL || image->data == NULL || blue_noise_texture == NULL) {
//        fprintf(stderr, "Error: Null pointer passed to blueNoiseDither.\n");
//        return;
//    }
//
//    for (int y = 0; y < height; y++) {
//        for (int x = 0; x < width; x++) {
//            int idx = (y * width + x) * 3;
//            uint8_t oldR = image->data[idx];
//            uint8_t oldG = image->data[idx + 1];
//            uint8_t oldB = image->data[idx + 2];
//
//            // Get blue noise from the texture and scale it to the 0-255 range
//            float noiseR = (float)blue_noise_texture[y % BLUE_NOISE_SIZE][x % BLUE_NOISE_SIZE] / 255.0f;
//            float noiseG = (float)blue_noise_texture[y % BLUE_NOISE_SIZE][x % BLUE_NOISE_SIZE] / 255.0f;
//            float noiseB = (float)blue_noise_texture[y % BLUE_NOISE_SIZE][x % BLUE_NOISE_SIZE] / 255.0f;
//
//            // Apply dither, clamp to 0-255 range, and quantize
//            int ditheredR = (int)fmin(255, fmax(0, (float)oldR + (noiseR * 32) - 16));
//            int ditheredG = (int)fmin(255, fmax(0, (float)oldG + (noiseG * 32) - 16));
//            int ditheredB = (int)fmin(255, fmax(0, (float)oldB + (noiseB * 32) - 16));
//
//            // Apply the quantization directly using bit shifting and clamping to keep it in the valid range
//            image->data[idx]     = (ditheredR & 0xE0);
//            image->data[idx + 1] = (ditheredG & 0xE0);
//            image->data[idx + 2] = (ditheredB & 0xC0);
//        }
//    }
//}

// --- Dithering Algorithm Functions ---
void floydSteinbergDither(ImageData* image) {
    int width = image->width;
    int height = image->height;

    if (image == NULL || image->data == NULL)
    {
        fprintf(stderr, "Error: Null pointer passed to floydSteinbergDither.");
        return;
    }
    for (int y = 0; y < height; y++) {
        for (int x = 0; x < width; x++) {
            int idx = (y * width + x) * 3;
            uint8_t oldR = image->data[idx];
            uint8_t oldG = image->data[idx + 1];
            uint8_t oldB = image->data[idx + 2];

            uint8_t newR = (oldR & 0xE0);
            uint8_t newG = (oldG & 0xE0);
            uint8_t newB = (oldB & 0xC0);

            image->data[idx]     = newR;
            image->data[idx + 1] = newG;
            image->data[idx + 2] = newB;

            int errorR = oldR - newR;
            int errorG = oldG - newG;
            int errorB = oldB - newB;

            if (x < width - 1) {
                image->data[idx + 3] = (uint8_t)fmin(255, fmax(0, image->data[idx + 3] + errorR * 7 / 16));
                image->data[idx + 4] = (uint8_t)fmin(255, fmax(0, image->data[idx + 4] + errorG * 7 / 16));
                image->data[idx + 5] = (uint8_t)fmin(255, fmax(0, image->data[idx + 5] + errorB * 7 / 16));
            }

            if (y < height - 1) {
                if (x > 0) {
                    int next_row_left_idx = ((y + 1) * width + x - 1) * 3;
                    image->data[next_row_left_idx]     = (uint8_t)fmin(255, fmax(0, image->data[next_row_left_idx]     + errorR * 3 / 16));
                    image->data[next_row_left_idx + 1] = (uint8_t)fmin(255, fmax(0, image->data[next_row_left_idx + 1] + errorG * 3 / 16));
                    image->data[next_row_left_idx + 2] = (uint8_t)fmin(255, fmax(0, image->data[next_row_left_idx + 2] + errorB * 3 / 16));
                }
                int next_row_idx = ((y + 1) * width + x) * 3;
                image->data[next_row_idx]     = (uint8_t)fmin(255, fmax(0, image->data[next_row_idx]     + errorR * 5 / 16));
                image->data[next_row_idx + 1] = (uint8_t)fmin(255, fmax(0, image->data[next_row_idx + 1] + errorG * 5 / 16));
                image->data[next_row_idx + 2] = (uint8_t)fmin(255, fmax(0, image->data[next_row_idx + 2] + errorB * 5 / 16));

                if (x < width - 1) {
                    int next_row_right_idx = ((y + 1) * width + x + 1) * 3;
                    image->data[next_row_right_idx]     = (uint8_t)fmin(255, fmax(0, image->data[next_row_right_idx]     + errorR * 1 / 16));
                    image->data[next_row_right_idx + 1] = (uint8_t)fmin(255, fmax(0, image->data[next_row_right_idx + 1] + errorG * 1 / 16));
                    image->data[next_row_right_idx + 2] = (uint8_t)fmin(255, fmax(0, image->data[next_row_right_idx + 2] + errorB * 1 / 16));
                }
            }
        }
    }
}

void jarvisDither(ImageData* image) {
    int width = image->width;
    int height = image->height;

    if (image == NULL || image->data == NULL)
    {
        fprintf(stderr, "Error: Null pointer passed to jarvisDither.");
        return;
    }

    int matrix[3][5] = {
    {0, 0, 0, 7, 5},
    {3, 5, 7, 5, 3},
    {1, 3, 5, 3, 1}
    };

    for (int y = 0; y < height; y++) {
        for (int x = 0; x < width; x++) {
            int idx = (y * width + x) * 3;
            uint8_t oldR = image->data[idx];
            uint8_t oldG = image->data[idx + 1];
            uint8_t oldB = image->data[idx + 2];

            uint8_t newR = (oldR & 0xE0);
            uint8_t newG = (oldG & 0xE0);
            uint8_t newB = (oldB & 0xC0);

            image->data[idx]     = newR;
            image->data[idx + 1] = newG;
            image->data[idx + 2] = newB;

            int errorR = oldR - newR;
            int errorG = oldG - newG;
            int errorB = oldB - newB;

            for (int i = 0; i < 3; i++) {
                for (int j = 0; j < 5; j++) {
                    int nx = x + j - 2;
                    int ny = y + i;

                    if (nx >= 0 && nx < width && ny < height) {
                        int adj_idx = (ny * width + nx) * 3;

                        image->data[adj_idx]     = (uint8_t)fmin(255, fmax(0, image->data[adj_idx]     + errorR * matrix[i][j] / 48));
                        image->data[adj_idx + 1] = (uint8_t)fmin(255, fmax(0, image->data[adj_idx + 1] + errorG * matrix[i][j] / 48));
                        image->data[adj_idx + 2] = (uint8_t)fmin(255, fmax(0, image->data[adj_idx + 2] + errorB * matrix[i][j] / 48));
                    }
                }
            }
        }
    }
}

void atkinsonDither(ImageData* image) {
    int width = image->width;
    int height = image->height;

    if (image == NULL || image->data == NULL)
    {
        fprintf(stderr, "Error: Null pointer passed to atkinsonDither.");
        return;
    }

    for (int y = 0; y < height; y++) {
        for (int x = 0; x < width; x++) {
            int idx = (y * width + x) * 3;
            uint8_t oldR = image->data[idx];
            uint8_t oldG = image->data[idx + 1];
            uint8_t oldB = image->data[idx + 2];

            uint8_t newR = (oldR & 0xE0);
            uint8_t newG = (oldG & 0xE0);
            uint8_t newB = (oldB & 0xC0);

            image->data[idx]     = newR;
            image->data[idx + 1] = newG;
            image->data[idx + 2] = newB;

            int errorR = (oldR - newR) / 8;
            int errorG = (oldG - newG) / 8;
            int errorB = (oldB - newB) / 8;

            if (x < width - 1) {
                image->data[idx + 3] = (uint8_t)fmin(255, fmax(0, image->data[idx + 3] + errorR * 7 / 16));
                image->data[idx + 4] = (uint8_t)fmin(255, fmax(0, image->data[idx + 4] + errorG * 7 / 16));
                image->data[idx + 5] = (uint8_t)fmin(255, fmax(0, image->data[idx + 5] + errorB * 7 / 16));
            }
            if (x < width - 2) {
                image->data[idx + 6] = (uint8_t)fmin(255, fmax(0, image->data[idx + 6] + errorR * 7 / 16));
                image->data[idx + 7] = (uint8_t)fmin(255, fmax(0, image->data[idx + 7] + errorG * 7 / 16));
                image->data[idx + 8] = (uint8_t)fmin(255, fmax(0, image->data[idx + 8] + errorB * 7 / 16));
            }

            if (y < height - 1) {
                int next_row_idx = ((y + 1) * width + x) * 3;
                image->data[next_row_idx]     = (uint8_t)fmin(255, fmax(0, image->data[next_row_idx]     + errorR));
                image->data[next_row_idx + 1] = (uint8_t)fmin(255, fmax(0, image->data[next_row_idx + 1] + errorG));
                image->data[next_row_idx + 2] = (uint8_t)fmin(255, fmax(0, image->data[next_row_idx + 2] + errorB));

                if (x > 0) {
                    int next_row_left_idx = ((y + 1) * width + x - 1) * 3;
                    image->data[next_row_left_idx]     = (uint8_t)fmin(255, fmax(0, image->data[next_row_left_idx]     + errorR));
                    image->data[next_row_left_idx + 1] = (uint8_t)fmin(255, fmax(0, image->data[next_row_left_idx + 1] + errorG));
                    image->data[next_row_left_idx + 2] = (uint8_t)fmin(255, fmax(0, image->data[next_row_left_idx + 2] + errorB));
                }

                if (x < width - 1) {
                    int next_row_right_idx = ((y + 1) * width + x + 1) * 3;
                    image->data[next_row_right_idx]     = (uint8_t)fmin(255, fmax(0, image->data[next_row_right_idx]     + errorR));
                    image->data[next_row_right_idx + 1] = (uint8_t)fmin(255, fmax(0, image->data[next_row_right_idx + 1] + errorG));
                    image->data[next_row_right_idx + 2] = (uint8_t)fmin(255, fmax(0, image->data[next_row_right_idx + 2] + errorB));
                }
            }

            if (y < height - 2) {
                int next_row_2_idx = ((y + 2) * width + x) * 3;
                image->data[next_row_2_idx]     = (uint8_t)fmin(255, fmax(0, image->data[next_row_2_idx]     + errorR));
                image->data[next_row_2_idx + 1] = (uint8_t)fmin(255, fmax(0, image->data[next_row_2_idx + 1] + errorG));
                image->data[next_row_2_idx + 2] = (uint8_t)fmin(255, fmax(0, image->data[next_row_2_idx + 2] + errorB));
            }
        }
    }
}

void bayer16x16Dither(ImageData* image) {
    if (image == NULL || image->data == NULL) {
        fprintf(stderr, "Error: Null pointer passed to bayer16x16Dither.\n");
        return;
    }

    int width = image->width;
    int height = image->height;

    for (int y = 0; y < height; y++) {
        for (int x = 0; x < width; x++) {
            int idx = (y * width + x) * 3;
            uint8_t oldR = image->data[idx];
            uint8_t oldG = image->data[idx + 1];
            uint8_t oldB = image->data[idx + 2];

            // Get the threshold from the Bayer Matrix, and scale it to the range 0-1
            float thresholdR = (float)BAYER_MATRIX_16X16[y % BAYER_SIZE][x % BAYER_SIZE] / 255.0f;
            float thresholdG = (float)BAYER_MATRIX_16X16[y % BAYER_SIZE][x % BAYER_SIZE] / 255.0f;
            float thresholdB = (float)BAYER_MATRIX_16X16[y % BAYER_SIZE][x % BAYER_SIZE] / 255.0f;

            int ditheredR = (int)fmin(255, fmax(0, (float)oldR + (thresholdR * 32) - 16));
            int ditheredG = (int)fmin(255, fmax(0, (float)oldG + (thresholdG * 32) - 16));
            int ditheredB = (int)fmin(255, fmax(0, (float)oldB + (thresholdB * 32) - 16));

            image->data[idx]     = (ditheredR & 0xE0);
            image->data[idx + 1] = (ditheredG & 0xE0);
            image->data[idx + 2] = (ditheredB & 0xC0);
        }
    }
}

// --- File IO Functions ---
int load_image(const char* filename, ImageData* image) {
    int n;

    if (filename == NULL || image == NULL) {
        fprintf(stderr, "Error: Null pointer passed to load_image.");
        return 1;
    }

    image->data = stbi_load(filename, &image->width, &image->height, &n, 3);
    if (image->data == NULL) {
        fprintf(stderr, "Failed to load image: %s\n", filename);
        return 1;
    }
    return 0;
}

void free_image_data(ImageData* image) {
    if (image && image->data) {
        stbi_image_free(image->data);
        image->data = NULL;
        image->width = 0;
        image->height = 0;
    }
}

int write_image_data_to_file(const char* filename, const char* array_name, const ImageData* image) {
    FILE* fp = NULL;

    if (filename == NULL || array_name == NULL || image == NULL || image->data == NULL) {
        fprintf(stderr, "Error: Null pointer passed to write_image_data_to_file.");
        return 1;
    }

    fp = fopen(filename, "w");
    if (fp == NULL) {
        fprintf(stderr, "Failed to open output file: %s\n", filename);
        return 1;
    }

    fprintf(fp, "#ifndef %s_H\n", array_name);
    fprintf(fp, "#define %s_H\n\n", array_name);
    fprintf(fp, "#include \"image_types.h\"\n\n");

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

    fprintf(fp, "static const uint8_t %s_data[%d] = {\n", array_name, image->width * image->height);

    for (int y = 0; y < image->height; y++) {
        for (int x = 0; x < image->width; x++) {
            int idx = (y * image->width + x) * 3;
            uint8_t rgb332 = rgbToRgb332(image->data[idx], image->data[idx + 1], image->data[idx + 2]);
            fprintf(fp, "0x%.2X, ", rgb332);

            rgb332ToRgb(rgb332, &image->data[idx], &image->data[idx + 1], &image->data[idx + 2]);
        }
        fprintf(fp, "\n");
    }

    fprintf(fp, "};\n\n");
    fprintf(fp, "static const Image_t %s_image = {\n", array_name);
    fprintf(fp, "    .data = %s_data,\n", array_name);
    fprintf(fp, "    .width = %d,\n", image->width);
    fprintf(fp, "    .height = %d,\n", image->height);
    fprintf(fp, "    .format_id = RGB332_FORMAT_ID\n");
    fprintf(fp, "};\n\n");
    fprintf(fp, "#endif // %s_H\n", array_name);

    fclose(fp);
    return 0;
}

// --- Command Line Processing ---
void init_program_options(ProgramOptions* opts) {
    memset(opts, 0, sizeof(ProgramOptions));
    opts->dither_method  = -1;
    opts->gamma          = 1.0f;
    opts->contrast       = 0.0f;
    opts->brightness     = 1.0f;
    opts->debug_mode     = 0;
}

int parse_command_line_args(int argc, char* argv[], ProgramOptions* opts) {
    if (opts == NULL) {
        fprintf(stderr, "Error: Null pointer passed to parse_command_line_args.");
        return 1;
    }
    for (int i = 1; i < argc; ++i) {
        if (strcmp(argv[i], "-i") == 0 && i + 1 < argc) {
            strncpy(opts->infilename, argv[i + 1], MAX_FILENAME_LENGTH - 1);
            i++;
        }
        else if (strcmp(argv[i], "-o") == 0 && i + 1 < argc) {
            strncpy(opts->outfilename, argv[i + 1], MAX_FILENAME_LENGTH - 1);
            i++;
        }
        else if (strcmp(argv[i], "-dm") == 0 && i + 1 < argc) {
            opts->dither_method = atoi(argv[i + 1]);
            i++;
        }
        else if (strcmp(argv[i], "-debug") == 0 && i + 1 < argc) {
            opts->debug_mode = 1;
            strncpy(opts->debug_filename, argv[i + 1], MAX_FILENAME_LENGTH - 1);
            i++;
        }
        else if (strcmp(argv[i], "-g") == 0 && i + 1 < argc) {
            opts->gamma = (float)atof(argv[i + 1]);
            i++;
        }
        else if (strcmp(argv[i], "-c") == 0 && i + 1 < argc) {
            opts->contrast = (float)atof(argv[i + 1]);
            i++;
        }
        else if (strcmp(argv[i], "-b") == 0 && i + 1 < argc) {
            opts->brightness = (float)atof(argv[i + 1]);
            i++;
        }
        else if (strcmp(argv[i], "-h") == 0) {
            printf("Usage: r3g3b2 -i <input file> -o <output file> [-dm <method>] [-debug <debug_filename>] [-g <gamma>] [-c <contrast>] [-b <brightness>]\n");
            printf("  -i <input file>           : Specify input file\n");
            printf("  -o <output file>          : Specify output file\n");
            printf("  -dm <method>              : Set dithering method (0: Floyd-Steinberg, 1: Jarvis, 2: Atkinson, 3: Bayer 16x16)\n");
            printf("  -debug <debug_filename>   : Enable debug mode and specify debug file prefix\n");
            printf("  -g <gamma>                : Set gamma value (default: 1.0)\n");
            printf("  -c <contrast>             : Set contrast value (default: 0.0)\n");
            printf("  -b <brightness>           : Set brightness value (default: 1.0)\n");
            printf("  -h                        : Display this help message\n");
            printf("Example: r3g3b2 -i tst.png -o tst.h -dm 0 -debug debug_output -g 1.0 -c 0.0 -b 1.0\n");
            return 1;
        }
        else {
            fprintf(stderr, "Invalid option: %s\n", argv[i]);
            return 1;
        }
    }
    return 0;
}

int process_image(ProgramOptions* opts) {
    ImageData image = { 0 };
    uint8_t gamma_lut[LUT_SIZE] = { 0 };
    uint8_t contrast_brightness_lut[LUT_SIZE] = { 0 };
    DitherFunc dither_function = NULL;

    if (opts == NULL) {
        fprintf(stderr, "Error: Null pointer passed to process_image.");
        return 1;
    }

    if (opts->infilename[0] == '\0') {
        fprintf(stderr, "No input file specified.\n");
        return 1;
    }
    if (opts->outfilename[0] == '\0') {
        fprintf(stderr, "No output file specified.\n");
        return 1;
    }

    if (load_image(opts->infilename, &image)) {
        return 1;
    }

    initialize_luts(opts->gamma, opts->contrast, opts->brightness, gamma_lut, contrast_brightness_lut);
    process_image_lut(&image, gamma_lut, contrast_brightness_lut);

    if (opts->debug_mode) {
        char processed_filename[MAX_FILENAME_LENGTH];
        snprintf(processed_filename, MAX_FILENAME_LENGTH, "%s_processed.bmp", opts->debug_filename);
        stbi_write_bmp(processed_filename, image.width, image.height, 3, image.data);
    }

    switch (opts->dither_method) {
    case 0: dither_function = floydSteinbergDither; break;
    case 1: dither_function = jarvisDither; break;
    case 2: dither_function = atkinsonDither; break;
    case 3: dither_function = bayer16x16Dither; break;
    //case 4: dither_function = blueNoiseDither; break;
    default: dither_function = NULL;
    }

    if (dither_function != NULL)
        dither_function(&image);

    char array_name[MAX_FILENAME_LENGTH] = { 0 };
    strncpy(array_name, opts->outfilename, MAX_FILENAME_LENGTH - 1);
    char* dot = strrchr(array_name, '.');
    if (dot) *dot = '\0';

    if (write_image_data_to_file(opts->outfilename, array_name, &image)) {
        free_image_data(&image);
        return 1;
    }

    if (opts->debug_mode) {
        char final_filename[MAX_FILENAME_LENGTH];
        snprintf(final_filename, MAX_FILENAME_LENGTH, "%s_final.bmp", opts->debug_filename);
        stbi_write_bmp(final_filename, image.width, image.height, 3, image.data);
    }

    free_image_data(&image);
    return 0;
}

int main(int argc, char* argv[]) {
    ProgramOptions opts;
    init_program_options(&opts);

    if (parse_command_line_args(argc, argv, &opts)) {
        return 1;
    }

    return process_image(&opts);
}