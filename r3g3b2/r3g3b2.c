#include <stdio.h>
#include <stdint.h>
#include <stdlib.h>
#include <string.h>
#include <math.h>
#include <stdbool.h>

#include "bayer16x16.h"
#include "color_map.h"

#define DEBUG_BUILD // uncomment for debug capabilities

#define STB_IMAGE_IMPLEMENTATION
#include "stb_image.h"

#ifdef DEBUG_BUILD
#define STB_IMAGE_WRITE_IMPLEMENTATION
#include "stb_image_write.h"
#endif

// --- Constants ---
#define MAX_FILENAME_LENGTH 1024
#define RGB_COMPONENTS 3
#define MAX_COLOUR_VALUE 255

#define LUT_SIZE 256

// --- Data Structures ---
typedef struct {
    char infilename[MAX_FILENAME_LENGTH];
    char outfilename[MAX_FILENAME_LENGTH];
#ifdef DEBUG_BUILD
    char debug_filename[MAX_FILENAME_LENGTH];
#endif
    int dither_method;
    float gamma;
    float contrast;
    float brightness;
    bool debug_mode;
} ProgramOptions;

typedef struct {
    uint8_t* data;
    int width;
    int height;
} ImageData;

// Error Diffusion Matrix Structure
typedef struct {
    int x_offset;
    int y_offset;
    float weight;
} ErrorDiffusionEntry;

// --- Function Prototypes ---
// Memory management
void free_image_memory(ImageData* image);

// LUT and image processing
static void initialize_luts(float gamma, float contrast, float brightness, uint8_t* gamma_lut, uint8_t* contrast_brightness_lut);
static void process_image_with_luts(ImageData* image, const uint8_t* gamma_lut, const uint8_t* contrast_brightness_lut);
static uint8_t rgbToRgb332(uint8_t r, uint8_t g, uint8_t b);

// Dithering algorithms
typedef void (*DitherFunc)(ImageData* image);
void floydSteinbergDither(ImageData* image);
void jarvisDither(ImageData* image);
void atkinsonDither(ImageData* image);
void bayer16x16Dither(ImageData* image);
static void genericDither(ImageData* image, const ErrorDiffusionEntry* matrix, int matrix_size);

// File IO
static int write_c_header(FILE* fp, const char* array_name);
static int write_image_data(FILE* fp, const char* array_name, const ImageData* image);
static int write_image_struct(FILE* fp, const char* array_name, const ImageData* image);
static int write_c_footer(FILE* fp, const char* array_name);
int write_image_data_to_file(const char* filename, const char* array_name, const ImageData* image);
int load_image(const char* filename, ImageData* image);
char* trim_filename_copy(const char* filename, char* dest, size_t dest_size);

// Command line processing
void init_program_options(ProgramOptions* opts);
int parse_command_line_args(int argc, char* argv[], ProgramOptions* opts);
int process_image(ProgramOptions* opts);


// Helper function to reduce bit depth of a colour
static uint8_t reduceBits(uint8_t value, int max_value);

// Function to calculate Euclidean distance
float colorDistance(const RGBColor* color1, const RGBColor* color2);

// Function to map a single RGB color to its closest R3G3B2 color, using the LUT
uint8_t mapColorToR3G3B2_Reduced(uint8_t r, uint8_t g, uint8_t b);

// Modified quantize pixel function to use color mapping with a reduced table
void quantize_pixel_with_map_reduced(uint8_t* r, uint8_t* g, uint8_t* b);


// --- Memory Management Functions ---
void free_image_memory(ImageData* image) {
    if (!image) return;

    if (image->data)
    {
        stbi_image_free(image->data);
    }
    image->data = NULL;
    image->width = 0;
    image->height = 0;
}

// --- LUT and image processing functions ---

// Function to initialize the look-up tables (LUTs) for color conversion
static void initialize_luts(float gamma, float contrast, float brightness, uint8_t* gamma_lut, uint8_t* contrast_brightness_lut)
{
    if (!gamma_lut || !contrast_brightness_lut)
    {
        fprintf(stderr, "Error: Null pointer passed to LUT initialization.\n");
        exit(EXIT_FAILURE);
    }

    float factor = (259.0f * (contrast + 255.0f)) / (255.0f * (259.0f - contrast));

    for (int i = 0; i < LUT_SIZE; i++)
    {
        float value = (float)i / (float)MAX_COLOUR_VALUE;                // Normalize the input to the 0-1 range
        value = powf(value, 1.0f / gamma);                               // Apply gamma correction
        value = factor * (value * brightness - 0.5f) + 0.5f;              // Apply contrast and brightness adjustments
        value = fmaxf(0.0f, fminf(value, 1.0f));                        // Clamp the value to ensure it stays within 0-1
        contrast_brightness_lut[i] = (uint8_t)(value * (float)MAX_COLOUR_VALUE);   // Scale the result back to 0-255 and store in the contrast/brightness LUT
        gamma_lut[i] = (uint8_t)(powf((float)i / (float)MAX_COLOUR_VALUE, 1.0f / gamma) * (float)MAX_COLOUR_VALUE); // Apply gamma correction and store in the gamma LUT
    }
}

static void process_image_with_luts(ImageData* image, const uint8_t* gamma_lut, const uint8_t* contrast_brightness_lut)
{
    if (!image || !image->data || !gamma_lut || !contrast_brightness_lut)
    {
        fprintf(stderr, "Error: Null pointer passed to process_image_with_luts.\n");
        return;
    }

    for (int y = 0; y < image->height; y++) {
        for (int x = 0; x < image->width; x++) {
            int idx = (y * image->width + x) * RGB_COMPONENTS;
            image->data[idx] = contrast_brightness_lut[gamma_lut[image->data[idx]]];     // Red channel
            image->data[idx + 1] = contrast_brightness_lut[gamma_lut[image->data[idx + 1]]]; // Green channel
            image->data[idx + 2] = contrast_brightness_lut[gamma_lut[image->data[idx + 2]]]; // Blue channel
        }
    }
}

static uint8_t rgbToRgb332(uint8_t r, uint8_t g, uint8_t b)
{
    return ((r & 0xE0) | ((g & 0xE0) >> 3) | (b >> 6));
}

// --- Dithering Algorithm Functions ---

void floydSteinbergDither(ImageData* image)
{
    const ErrorDiffusionEntry matrix[] = {
        { 1, 0, 7.0f / 16.0f },
        {-1, 1, 3.0f / 16.0f },
        { 0, 1, 5.0f / 16.0f },
        { 1, 1, 1.0f / 16.0f }
    };
    genericDither(image, matrix, sizeof(matrix) / sizeof(matrix[0]));
}

void jarvisDither(ImageData* image)
{
    const ErrorDiffusionEntry matrix[] = {
    { 1, 0, 7.0f / 48.0f }, { 2, 0, 5.0f / 48.0f },
    {-2, 1, 3.0f / 48.0f }, {-1, 1, 5.0f / 48.0f }, { 0, 1, 7.0f / 48.0f }, { 1, 1, 5.0f / 48.0f }, { 2, 1, 3.0f / 48.0f },
    {-2, 2, 1.0f / 48.0f }, {-1, 2, 3.0f / 48.0f }, { 0, 2, 5.0f / 48.0f }, { 1, 2, 3.0f / 48.0f }, { 2, 2, 1.0f / 48.0f }
    };
    genericDither(image, matrix, sizeof(matrix) / sizeof(matrix[0]));
}

void atkinsonDither(ImageData* image)
{
    const ErrorDiffusionEntry matrix[] = {
    { 1, 0, 1.0f / 8.0f }, { 2, 0, 1.0f / 8.0f },
    {-1, 1, 1.0f / 8.0f }, { 0, 1, 1.0f / 8.0f }, { 1, 1, 1.0f / 8.0f },
    { 0, 2, 1.0f / 8.0f }
    };
    genericDither(image, matrix, sizeof(matrix) / sizeof(matrix[0]));
}

static void genericDither(ImageData* image, const ErrorDiffusionEntry* matrix, int matrix_size)
{
    if (!image || !image->data)
    {
        fprintf(stderr, "Error: Null pointer passed to genericDither.\n");
        return;
    }

    const int width = image->width;
    const int height = image->height;

    for (int y = 0; y < height; y++)
    {
        for (int x = 0; x < width; x++)
        {
            int idx = (y * width + x) * RGB_COMPONENTS;
            uint8_t oldR = image->data[idx];
            uint8_t oldG = image->data[idx + 1];
            uint8_t oldB = image->data[idx + 2];

            uint8_t newR = oldR;
            uint8_t newG = oldG;
            uint8_t newB = oldB;

            //quantize_pixel(&newR, &newG, &newB);
            quantize_pixel_with_map_reduced(&newR, &newG, &newB);

            image->data[idx] = newR;
            image->data[idx + 1] = newG;
            image->data[idx + 2] = newB;

            float errorR = (float)(oldR - newR);
            float errorG = (float)(oldG - newG);
            float errorB = (float)(oldB - newB);

            for (int i = 0; i < matrix_size; i++)
            {
                int nx = x + matrix[i].x_offset;
                int ny = y + matrix[i].y_offset;

                if (nx >= 0 && nx < width && ny >= 0 && ny < height)
                {
                    int adj_idx = (ny * width + nx) * RGB_COMPONENTS;
                    image->data[adj_idx] = (uint8_t)fmin(MAX_COLOUR_VALUE, fmax(0, image->data[adj_idx] + errorR * matrix[i].weight));
                    image->data[adj_idx + 1] = (uint8_t)fmin(MAX_COLOUR_VALUE, fmax(0, image->data[adj_idx + 1] + errorG * matrix[i].weight));
                    image->data[adj_idx + 2] = (uint8_t)fmin(MAX_COLOUR_VALUE, fmax(0, image->data[adj_idx + 2] + errorB * matrix[i].weight));
                }
            }
        }
    }
}


void bayer16x16Dither(ImageData* image) {
    if (!image || !image->data) {
        fprintf(stderr, "Error: Null pointer passed to bayer16x16Dither.\n");
        return;
    }

    const int width = image->width;
    const int height = image->height;

    for (int y = 0; y < height; y++) {
        for (int x = 0; x < width; x++) {
            int idx = (y * width + x) * RGB_COMPONENTS;

            int bayer_threshold = BAYER_MATRIX_16X16[y % BAYER_SIZE][x % BAYER_SIZE];
            float normalized_bayer = (float)(bayer_threshold - 128);

            int r = image->data[idx];
            int g = image->data[idx + 1];
            int b = image->data[idx + 2];

            r = (int)round((float)r + (normalized_bayer / 8.0f));
            g = (int)round((float)g + (normalized_bayer / 8.0f));
            b = (int)round((float)b + (normalized_bayer / 8.0f));

            r = (r > MAX_COLOUR_VALUE) ? MAX_COLOUR_VALUE : (r < 0) ? 0 : r;
            g = (g > MAX_COLOUR_VALUE) ? MAX_COLOUR_VALUE : (g < 0) ? 0 : g;
            b = (b > MAX_COLOUR_VALUE) ? MAX_COLOUR_VALUE : (b < 0) ? 0 : b;

            //quantize_pixel((uint8_t*)&r, (uint8_t*)&g, (uint8_t*)&b);
            quantize_pixel_with_map_reduced((uint8_t*)&r, (uint8_t*)&g, (uint8_t*)&b);

            image->data[idx] = r;
            image->data[idx + 1] = g;
            image->data[idx + 2] = b;
        }
    }
}

// --- File IO Functions ---
int load_image(const char* filename, ImageData* image) {
    int n;
    if (!filename || !image) {
        fprintf(stderr, "Error: Null pointer passed to load_image.\n");
        return EXIT_FAILURE;
    }

    image->data = stbi_load(filename, &image->width, &image->height, &n, RGB_COMPONENTS);

    if (!image->data) {
        fprintf(stderr, "Failed to load image: %s\n", filename);
        return EXIT_FAILURE;
    }
    return EXIT_SUCCESS;
}

char* trim_filename_copy(const char* filename, char* dest, size_t dest_size)
{
    if (!filename || !dest || dest_size == 0) {
        return NULL;
    }
    dest[0] = '\0';

    if (filename[0] == '\0') {
        return dest;
    }

    const char* dot = strrchr(filename, '.');
    size_t length;

    if (!dot || dot == filename) {
        length = strlen(filename);
        if (length >= dest_size)
        {
            strncpy(dest, filename, dest_size - 1);
            dest[dest_size - 1] = '\0';
        }
        else {
            strcpy(dest, filename);
        }
        return dest;
    }
    else {
        length = dot - filename;
        if (length >= dest_size) {
            strncpy(dest, filename, dest_size - 1);
            dest[dest_size - 1] = '\0';
        }
        else {
            strncpy(dest, filename, length);
            dest[length] = '\0';
        }
        return dest;
    }
}

int write_image_data_to_file(const char* filename, const char* array_name, const ImageData* image)
{
    FILE* fp = NULL;

    if (!filename || !array_name || !image || !image->data)
    {
        fprintf(stderr, "Error: Null pointer passed to write_image_data_to_file.\n");
        return EXIT_FAILURE;
    }

    fp = fopen(filename, "w");
    if (!fp) {
        perror("Failed to open output file");
        return EXIT_FAILURE;
    }

    if (write_c_header(fp, array_name) != EXIT_SUCCESS)   goto error_close_file;
    if (write_image_data(fp, array_name, image) != EXIT_SUCCESS)  goto error_close_file;
    if (write_image_struct(fp, array_name, image) != EXIT_SUCCESS) goto error_close_file;
    if (write_c_footer(fp, array_name) != EXIT_SUCCESS)  goto error_close_file;

    fclose(fp);
    return EXIT_SUCCESS;

error_close_file:
    if (fp != NULL)
    {
        fclose(fp);
    }
    return EXIT_FAILURE;
}

static int write_c_header(FILE* fp, const char* array_name)
{
    if (!fp || !array_name) return EXIT_FAILURE;

    if (fprintf(fp, "#ifndef %s_H\n", array_name) < 0)      return EXIT_FAILURE;
    if (fprintf(fp, "#define %s_H\n\n", array_name) < 0)    return EXIT_FAILURE;
    if (fprintf(fp, "#include \"image_types.h\"\n\n") < 0)  return EXIT_FAILURE;

    if (fprintf(fp, "/*\n") < 0) return EXIT_FAILURE;
    if (fprintf(fp, "Contents of image_types.h:\n\n") < 0)      return EXIT_FAILURE;
    if (fprintf(fp, "#ifndef IMAGE_TYPES_H\n") < 0)             return EXIT_FAILURE;
    if (fprintf(fp, "#define IMAGE_TYPES_H\n\n") < 0)           return EXIT_FAILURE;
    if (fprintf(fp, "#include <stdint.h>\n\n") < 0)             return EXIT_FAILURE;
    if (fprintf(fp, "#define RGB332_FORMAT_ID 0x332\n\n") < 0)  return EXIT_FAILURE;
    if (fprintf(fp, "typedef struct {\n") < 0)                  return EXIT_FAILURE;
    if (fprintf(fp, "    const uint8_t* data;\n") < 0)          return EXIT_FAILURE;
    if (fprintf(fp, "    uint16_t width;\n") < 0)               return EXIT_FAILURE;
    if (fprintf(fp, "    uint16_t height;\n") < 0)              return EXIT_FAILURE;
    if (fprintf(fp, "    uint16_t format_id;\n") < 0)           return EXIT_FAILURE;
    if (fprintf(fp, "} Image_t;\n\n") < 0)                      return EXIT_FAILURE;
    if (fprintf(fp, "#endif // IMAGE_TYPES_H\n") < 0)           return EXIT_FAILURE;
    if (fprintf(fp, "*/\n\n") < 0)                              return EXIT_FAILURE;

    return EXIT_SUCCESS;
}

static int write_image_data(FILE* fp, const char* array_name, const ImageData* image)
{
    if (!fp || !array_name || !image || !image->data) return EXIT_FAILURE;

    if (fprintf(fp, "static const uint8_t %s_data[%d] = {\n", array_name, image->width * image->height) < 0) return EXIT_FAILURE;

    for (int y = 0; y < image->height; y++)
    {
        for (int x = 0; x < image->width; x++)
        {
            int idx = (y * image->width + x) * RGB_COMPONENTS;
            uint8_t rgb332 = rgbToRgb332(image->data[idx], image->data[idx + 1], image->data[idx + 2]);
            if (fprintf(fp, "0x%.2X, ", rgb332) < 0) return EXIT_FAILURE;
        }
        if (fprintf(fp, "\n") < 0) return EXIT_FAILURE;
    }

    if (fprintf(fp, "};\n\n") < 0)  return EXIT_FAILURE;
    return EXIT_SUCCESS;
}

static int write_image_struct(FILE* fp, const char* array_name, const ImageData* image)
{
    if (!fp || !array_name || !image) return EXIT_FAILURE;

    if (fprintf(fp, "static const Image_t %s_image = {\n", array_name) < 0) return EXIT_FAILURE;
    if (fprintf(fp, "    .data = %s_data,\n", array_name) < 0)              return EXIT_FAILURE;
    if (fprintf(fp, "    .width = %d,\n", image->width) < 0)                return EXIT_FAILURE;
    if (fprintf(fp, "    .height = %d,\n", image->height) < 0)              return EXIT_FAILURE;
    if (fprintf(fp, "    .format_id = RGB332_FORMAT_ID\n") < 0)             return EXIT_FAILURE;
    if (fprintf(fp, "};\n\n") < 0)                                          return EXIT_FAILURE;
    return EXIT_SUCCESS;
}

static int write_c_footer(FILE* fp, const char* array_name)
{
    if (!fp || !array_name) return EXIT_FAILURE;
    if (fprintf(fp, "#endif // %s_H\n", array_name) < 0) return EXIT_FAILURE;
    return EXIT_SUCCESS;
}

int process_image(ProgramOptions* opts)
{
    ImageData image = { 0 };
    uint8_t gamma_lut[LUT_SIZE] = { 0 };
    uint8_t contrast_brightness_lut[LUT_SIZE] = { 0 };
    DitherFunc dither_function = NULL;
    char array_name[MAX_FILENAME_LENGTH] = { 0 };

#ifdef DEBUG_BUILD
    char final_filename[MAX_FILENAME_LENGTH];
    char processed_filename[MAX_FILENAME_LENGTH];
    int snprintf_result;
#endif

    if (!opts) {
        fprintf(stderr, "Error: Null pointer passed to process_image.\n");
        return EXIT_FAILURE;
    }

    if (opts->infilename[0] == '\0') {
        fprintf(stderr, "No input file specified.\n");
        return EXIT_FAILURE;
    }

    if (opts->outfilename[0] == '\0') {
        fprintf(stderr, "No output file specified.\n");
        return EXIT_FAILURE;
    }

    if (load_image(opts->infilename, &image) != EXIT_SUCCESS)
    {
        return EXIT_FAILURE;
    }

    initialize_luts(opts->gamma, opts->contrast, opts->brightness, gamma_lut, contrast_brightness_lut);
    process_image_with_luts(&image, gamma_lut, contrast_brightness_lut);

#ifdef DEBUG_BUILD
    if (opts->debug_mode)
    {
        snprintf_result = snprintf(processed_filename, MAX_FILENAME_LENGTH, "%s_processed.bmp", opts->debug_filename);
        if (snprintf_result < 0 || snprintf_result >= MAX_FILENAME_LENGTH)
        {
            fprintf(stderr, "Error: Could not create debug filename.\n");
            free_image_memory(&image);
            return EXIT_FAILURE;
        }
        stbi_write_bmp(processed_filename, image.width, image.height, RGB_COMPONENTS, image.data);
    }
#endif

    switch (opts->dither_method) {
    case 0:  dither_function = floydSteinbergDither; break;
    case 1:  dither_function = jarvisDither;         break;
    case 2:  dither_function = atkinsonDither;       break;
    case 3:  dither_function = bayer16x16Dither;     break;
    default: dither_function = NULL;
    }

    if (dither_function != NULL)
    {
        dither_function(&image);
    }

    trim_filename_copy(opts->outfilename, array_name, MAX_FILENAME_LENGTH);

    if (write_image_data_to_file(opts->outfilename, array_name, &image) != EXIT_SUCCESS) {
        free_image_memory(&image);
        return EXIT_FAILURE;
    }

#ifdef DEBUG_BUILD
    if (opts->debug_mode)
    {
        snprintf_result = snprintf(final_filename, MAX_FILENAME_LENGTH, "%s_final.bmp", opts->debug_filename);
        if (snprintf_result < 0 || snprintf_result >= MAX_FILENAME_LENGTH)
        {
            fprintf(stderr, "Error: Could not create debug filename.\n");
            free_image_memory(&image);
            return EXIT_FAILURE;
        }

        stbi_write_bmp(final_filename, image.width, image.height, RGB_COMPONENTS, image.data);
    }
#endif

    free_image_memory(&image);
    return EXIT_SUCCESS;
}

// Helper function to reduce bit depth of a colour
static uint8_t reduceBits(uint8_t value, int max_value)
{
    return (uint8_t)(value * max_value / 256.0);
}

// Function to calculate Euclidean distance
float colorDistance(const RGBColor* color1, const RGBColor* color2) {
    float dr = (float)(color2->r - color1->r);
    float dg = (float)(color2->g - color1->g);
    float db = (float)(color2->b - color1->b);
    return sqrtf(dr * dr + dg * dg + db * db);
}

// Function to map a single RGB color to its closest R3G3B2 color, using the LUT
uint8_t mapColorToR3G3B2_Reduced(uint8_t r, uint8_t g, uint8_t b)
{
    uint8_t r_reduced = reduceBits(r, RED_LEVELS - 1);
    uint8_t g_reduced = reduceBits(g, GREEN_LEVELS - 1);
    uint8_t b_reduced = reduceBits(b, BLUE_LEVELS - 1);

    int index = (r_reduced << (8)) | (g_reduced << 4) | b_reduced;
    return colorMapLUT_reduced[index];
}

// Modified quantize pixel function to use color mapping with a reduced table
void quantize_pixel_with_map_reduced(uint8_t* r, uint8_t* g, uint8_t* b)
{
    uint8_t r3g3b2_index = mapColorToR3G3B2_Reduced(*r, *g, *b);

    RGBColor closestColor;

    closestColor = r3g3b2Palette[r3g3b2_index];

    *r = closestColor.r;
    *g = closestColor.g;
    *b = closestColor.b;
}

// --- Command Line Processing ---
void init_program_options(ProgramOptions* opts) {
    if (!opts) return;
    memset(opts, 0, sizeof(ProgramOptions));
    opts->dither_method = -1;
    opts->gamma = 1.0f;
    opts->contrast = 0.0f;
    opts->brightness = 1.0f;
    opts->debug_mode = false;
}

int parse_command_line_args(int argc, char* argv[], ProgramOptions* opts) {
    if (opts == NULL) {
        fprintf(stderr, "Error: Null pointer passed to parse_command_line_args.\n");
        return 1;
    }

    for (int i = 1; i < argc; ++i) {
        if (strcmp(argv[i], "-i") == 0)
        {
            if (i + 1 < argc)
            {
                strncpy(opts->infilename, argv[i + 1], MAX_FILENAME_LENGTH - 1);
                i++;
            }
            else {
                fprintf(stderr, "Error: -i option requires an argument.\n");
                return 1;
            }
        }
        else if (strcmp(argv[i], "-o") == 0) {
            if (i + 1 < argc)
            {
                strncpy(opts->outfilename, argv[i + 1], MAX_FILENAME_LENGTH - 1);
                i++;
            }
            else {
                fprintf(stderr, "Error: -o option requires an argument.\n");
                return 1;
            }
        }
        else if (strcmp(argv[i], "-dm") == 0) {
            if (i + 1 < argc)
            {
                opts->dither_method = atoi(argv[i + 1]);
                i++;
            }
            else {
                fprintf(stderr, "Error: -dm option requires an argument.\n");
                return 1;
            }
        }
#ifdef DEBUG_BUILD
        else if (strcmp(argv[i], "-debug") == 0) {
            if (i + 1 < argc)
            {
                opts->debug_mode = true;
                strncpy(opts->debug_filename, argv[i + 1], MAX_FILENAME_LENGTH - 1);
                i++;
            }
            else {
                fprintf(stderr, "Error: -debug option requires an argument.\n");
                return 1;
            }
        }
#endif
        else if (strcmp(argv[i], "-g") == 0) {
            if (i + 1 < argc)
            {
                opts->gamma = (float)atof(argv[i + 1]);
                i++;
            }
            else {
                fprintf(stderr, "Error: -g option requires an argument.\n");
                return 1;
            }
        }
        else if (strcmp(argv[i], "-c") == 0) {
            if (i + 1 < argc)
            {
                opts->contrast = (float)atof(argv[i + 1]);
                i++;
            }
            else {
                fprintf(stderr, "Error: -c option requires an argument.\n");
                return 1;
            }
        }
        else if (strcmp(argv[i], "-b") == 0) {
            if (i + 1 < argc)
            {
                opts->brightness = (float)atof(argv[i + 1]);
                i++;
            }
            else {
                fprintf(stderr, "Error: -b option requires an argument.\n");
                return 1;
            }
        }
        else if (strcmp(argv[i], "-h") == 0) {
            printf("Usage: r3g3b2 -i <input file> -o <output file> [-dm <method>] [-g <gamma>] [-c <contrast>] [-b <brightness>]\n");
            printf("  -i <input file>           : Specify input file\n");
            printf("  -o <output file>          : Specify output file\n");
            printf("  -dm <method>              : Set dithering method (0: Floyd-Steinberg, 1: Jarvis, 2: Atkinson, 3: Bayer 16x16)\n");
#ifdef DEBUG_BUILD
            printf("  -debug <debug_filename>   : Enable debug mode and specify debug file prefix\n");
#endif
            printf("  -g <gamma>                : Set gamma value (default: 1.0)\n");
            printf("  -c <contrast>             : Set contrast value (default: 0.0)\n");
            printf("  -b <brightness>           : Set brightness value (default: 1.0)\n");
            printf("  -h                        : Display this help message\n");
            printf("Example: r3g3b2 -i tst.png -o tst.h -dm 0 -g 1.0 -c 0.0 -b 1.0\n");
            return 1;
        }
        else {
            fprintf(stderr, "Invalid option: %s\n", argv[i]);
            return 1;
        }
    }
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