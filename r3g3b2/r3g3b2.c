/*
    r3g3b2.c

    Converts an RGB image to 8bpp RGB332. (needed for LT7683 TFT graphics controller)

    (MJM + AI) 2025

*/

#include <stdio.h>
#include <stdint.h>
#include <stdlib.h>
#include <string.h>
#include <math.h>


#include "bayer16x16.h"
#include "color_lut.h"

#define DEBUG_BUILD //uncomment for debug capabilities

#define STB_IMAGE_IMPLEMENTATION
#include "stb_image.h"

#ifdef DEBUG_BUILD
#define STB_IMAGE_WRITE_IMPLEMENTATION
#include "stb_image_write.h"
#endif

#define LUT_SIZE 256
#define MAX_FILENAME_LENGTH 1024

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
    int debug_mode;
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
void* mem_alloc_checked(size_t size, const char* location);

// LUT and image processing
static void initialize_luts(float gamma, float contrast, float brightness, uint8_t* gamma_lut, uint8_t* contrast_brightness_lut);
static void process_image_lut(ImageData* image, const uint8_t* gamma_lut, const uint8_t* contrast_brightness_lut);
static uint8_t rgbToRgb332(uint8_t r, uint8_t g, uint8_t b);
static void rgb332ToRgb(uint8_t rgb332, uint8_t* r, uint8_t* g, uint8_t* b);

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
void free_image_data(ImageData* image);
char* trim_filename_copy(const char* filename, char* dest, size_t dest_size);

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

// Allocation check function
void* mem_alloc_checked(size_t size, const char* location) {
    void* ptr = malloc(size);
    if (!ptr) {
        mem_alloc_failed(location);
    }
    return ptr;
}

// --- LUT and image processing functions ---

// Function to initialize the look-up tables (LUTs) for color conversion
static void initialize_luts(float gamma, float contrast, float brightness, uint8_t* gamma_lut, uint8_t* contrast_brightness_lut) {
    float value, factor;

    if (!gamma_lut || !contrast_brightness_lut) {
        fprintf(stderr, "Error: Null pointer passed to LUT initialization.\n");
        exit(EXIT_FAILURE);
    }

    // Calculate the contrast factor based on the given contrast
    factor = (259.0f * (contrast + 255.0f)) / (255.0f * (259.0f - contrast));

    for (int i = 0; i < LUT_SIZE; i++) {
        value = i / 255.0f;                 // Normalize the input to the 0-1 range
        value = powf(value, 1.0f / gamma);   // Apply gamma correction
        value = factor * (value * brightness - 0.5f) + 0.5f;  // Apply contrast and brightness adjustments
        value = fmaxf(0.0f, fminf(value, 1.0f)); // Clamp the value to ensure it stays within 0-1

        contrast_brightness_lut[i] = (uint8_t)(value * 255.0f);  // Scale the result back to 0-255 and store in the contrast/brightness LUT
        gamma_lut[i] = (uint8_t)(powf(i / 255.0f, 1.0f / gamma) * 255.0f); // Apply gamma correction and store in the gamma LUT
    }
    return;
}

static void process_image_lut(ImageData* image, const uint8_t* gamma_lut, const uint8_t* contrast_brightness_lut) {
    if (!image || !image->data || !gamma_lut || !contrast_brightness_lut)
    {
        fprintf(stderr, "Error: Null pointer passed to process_image_lut.\n");
        return;
    }

    for (int y = 0; y < image->height; y++) {
        for (int x = 0; x < image->width; x++) {
            // Calculate the index into the image data array
            int idx = (y * image->width + x) * 3;

            // Apply the LUT transformations
            image->data[idx]     = contrast_brightness_lut[gamma_lut[image->data[idx]]];   // Red channel
            image->data[idx + 1] = contrast_brightness_lut[gamma_lut[image->data[idx + 1]]]; // Green channel
            image->data[idx + 2] = contrast_brightness_lut[gamma_lut[image->data[idx + 2]]]; // Blue channel
        }
    }
}

static uint8_t rgbToRgb332(uint8_t r, uint8_t g, uint8_t b) {
    // Extract the top 3 bits of red, top 3 bits of green (shifted), and top 2 bits of blue (shifted)
    // and combine them into an RGB332 byte
    return ((r & 0xE0) | ((g & 0xE0) >> 3) | (b >> 6));
}

static void rgb332ToRgb(uint8_t rgb332, uint8_t* r, uint8_t* g, uint8_t* b) {
    if (!r || !g || !b) {
        fprintf(stderr, "Error: Null pointer passed to rgb332ToRgb.\n");
        return;
    }
    // Look up the RGB values from the color_lut using rgb332 as an index
    // Then, extract the red, green, and blue channels using bit shifts and masks
    *r = (color_lut[rgb332] >> 16) & 0xFF; // Extract red channel
    *g = (color_lut[rgb332] >> 8) & 0xFF;  // Extract green channel
    *b = color_lut[rgb332] & 0xFF;         // Extract blue channel
}

// --- Dithering Algorithm Functions ---

void floydSteinbergDither(ImageData* image)
{
    ErrorDiffusionEntry matrix[] = {
        { 1, 0, 7.0f / 16.0f },
        {-1, 1, 3.0f / 16.0f },
        { 0, 1, 5.0f / 16.0f },
        { 1, 1, 1.0f / 16.0f }
    };
    genericDither(image, matrix, sizeof(matrix) / sizeof(matrix[0]));
}

void jarvisDither(ImageData* image)
{
    ErrorDiffusionEntry matrix[] = {
    { 1, 0, 7.0f / 48.0f }, { 2, 0, 5.0f / 48.0f },
    {-2, 1, 3.0f / 48.0f }, {-1, 1, 5.0f / 48.0f }, { 0, 1, 7.0f / 48.0f }, { 1, 1, 5.0f / 48.0f }, { 2, 1, 3.0f / 48.0f },
    {-2, 2, 1.0f / 48.0f }, {-1, 2, 3.0f / 48.0f }, { 0, 2, 5.0f / 48.0f }, { 1, 2, 3.0f / 48.0f }, { 2, 2, 1.0f / 48.0f }
    };
    genericDither(image, matrix, sizeof(matrix) / sizeof(matrix[0]));
}

void atkinsonDither(ImageData* image)
{
    ErrorDiffusionEntry matrix[] = {
    { 1, 0, 1.0f / 8.0f }, { 2, 0, 1.0f / 8.0f },
    {-1, 1, 1.0f / 8.0f }, { 0, 1, 1.0f / 8.0f }, { 1, 1, 1.0f / 8.0f },
    { 0, 2, 1.0f / 8.0f }
    };
    genericDither(image, matrix, sizeof(matrix) / sizeof(matrix[0]));
}

static void genericDither(ImageData* image, const ErrorDiffusionEntry* matrix, int matrix_size)
{
    int width = image->width;
    int height = image->height;

    if (!image || !image->data)
    {
        fprintf(stderr, "Error: Null pointer passed to genericDither.\n");
        return;
    }

    for (int y = 0; y < height; y++)
    {
        for (int x = 0; x < width; x++)
        {
            int idx = (y * width + x) * 3;
            uint8_t oldR = image->data[idx];
            uint8_t oldG = image->data[idx + 1];
            uint8_t oldB = image->data[idx + 2];

            uint8_t newR = (oldR & 0xE0);
            uint8_t newG = (oldG & 0xE0);
            uint8_t newB = (oldB & 0xC0);

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
                    int adj_idx = (ny * width + nx) * 3;
                    image->data[adj_idx] = (uint8_t)fmin(255, fmax(0, image->data[adj_idx] + errorR * matrix[i].weight));
                    image->data[adj_idx + 1] = (uint8_t)fmin(255, fmax(0, image->data[adj_idx + 1] + errorG * matrix[i].weight));
                    image->data[adj_idx + 2] = (uint8_t)fmin(255, fmax(0, image->data[adj_idx + 2] + errorB * matrix[i].weight));
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

    int width = image->width;
    int height = image->height;

    for (int y = 0; y < height; y++) {
        for (int x = 0; x < width; x++) {
            // Calculate the index for the current pixel
            int idx = (y * width + x) * 3;

            // Get dither threshold from the bayer matrix
            int bayer_threshold = BAYER_MATRIX_16X16[y % BAYER_SIZE][x % BAYER_SIZE];
            // Normalize bayer threshold to -128 to 127
            float normalized_bayer = (float)(bayer_threshold - 128);

            int r = image->data[idx];
            int g = image->data[idx + 1];
            int b = image->data[idx + 2];

            // Apply scaled bayer dither
            r = (int)round((float)r + (normalized_bayer / 8.0f));
            g = (int)round((float)g + (normalized_bayer / 8.0f));
            b = (int)round((float)b + (normalized_bayer / 8.0f));

            // Quantize
            r = (r > 255) ? 255 : (r < 0) ? 0 : r;
            g = (g > 255) ? 255 : (g < 0) ? 0 : g;
            b = (b > 255) ? 255 : (b < 0) ? 0 : b;

            image->data[idx] = (r & 0xE0);
            image->data[idx + 1] = (g & 0xE0);
            image->data[idx + 2] = (b & 0xC0);
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

    // Load the image using stb_image, requesting 3 components (RGB)
    image->data = stbi_load(filename, &image->width, &image->height, &n, 3);

    // Check if image loading failed
    if (!image->data) {
        fprintf(stderr, "Failed to load image: %s\n", filename);
        return EXIT_FAILURE;
    }

    return EXIT_SUCCESS; // Image loaded successfully
}

void free_image_data(ImageData* image) {
    // Check if the image and its data are valid before attempting to free memory
    if (image && image->data) {
        // Free the image data using stb_image_free
        stbi_image_free(image->data);
        // Reset the image data pointer, width and height to avoid use after free
        image->data = NULL;
        image->width = 0;
        image->height = 0;
    }
}

char* trim_filename_copy(const char* filename, char* dest, size_t dest_size) {
    // Check for invalid inputs
    if (!filename || !dest || dest_size == 0) {
        return NULL;
    }

    // Clear the destination string by setting first byte to null terminator
    dest[0] = '\0';

    // Check for empty filename
    if (filename[0] == '\0') {
        return dest;
    }

    // Find the last occurrence of a dot in the filename
    char* dot = strrchr(filename, '.');
    size_t length;

    // If there is no dot, or if the dot is the first character, then the entire filename should be copied
    if (!dot || dot == filename) {
        length = strlen(filename);
        if (length >= dest_size)
        {
            // If the filename is longer than the destination buffer, then copy only as much as will fit
            strncpy(dest, filename, dest_size - 1);
            dest[dest_size - 1] = '\0';
        }
        else {
            // If the filename is smaller than the buffer, then copy entire filename
            strcpy(dest, filename);
        }
        return dest;
    }
    else {
        // Calculate the length of the filename up to the dot
        length = dot - filename;
        if (length >= dest_size) {
            // If the length is longer than the destination buffer, then copy only as much as will fit
            strncpy(dest, filename, dest_size - 1);
            dest[dest_size - 1] = '\0';
        }
        else {
            // If the filename up to the dot is smaller than the buffer, then copy it
            strncpy(dest, filename, length);
            dest[length] = '\0';
        }
        return dest;
    }
}

int write_image_data_to_file(const char* filename, const char* array_name, const ImageData* image) {
    FILE* fp;

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

    if (write_c_header(fp, array_name) != EXIT_SUCCESS)            goto error_close_file;
    if (write_image_data(fp, array_name, image) != EXIT_SUCCESS)   goto error_close_file;
    if (write_image_struct(fp, array_name, image) != EXIT_SUCCESS) goto error_close_file;
    if (write_c_footer(fp, array_name) != EXIT_SUCCESS)            goto error_close_file;

    fclose(fp);
    return EXIT_SUCCESS;

error_close_file:
    fclose(fp);
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
            int idx = (y * image->width + x) * 3;
            uint8_t rgb332 = rgbToRgb332(image->data[idx], image->data[idx + 1], image->data[idx + 2]);
            if (fprintf(fp, "0x%.2X, ", rgb332) < 0) return EXIT_FAILURE;

            rgb332ToRgb(rgb332, &image->data[idx], &image->data[idx + 1], &image->data[idx + 2]);
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

int process_image(ProgramOptions* opts) {
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

    // Check for invalid inputs
    if (!opts) {
        fprintf(stderr, "Error: Null pointer passed to process_image.\n");
        return EXIT_FAILURE;
    }

    // Check if input filename has been specified
    if (opts->infilename[0] == '\0') {
        fprintf(stderr, "No input file specified.\n");
        return EXIT_FAILURE;
    }
    // Check if output filename has been specified
    if (opts->outfilename[0] == '\0') {
        fprintf(stderr, "No output file specified.\n");
        return EXIT_FAILURE;
    }

    // Load the input image
    if (load_image(opts->infilename, &image) != EXIT_SUCCESS) {
        return EXIT_FAILURE;
    }

    // Initialise the look up tables for gamma, contrast, and brightness
    initialize_luts(opts->gamma, opts->contrast, opts->brightness, gamma_lut, contrast_brightness_lut);

    // Process the image data by applying the look up tables
    process_image_lut(&image, gamma_lut, contrast_brightness_lut);

    // If debug mode is enabled, write a debug image
#ifdef DEBUG_BUILD
    if (opts->debug_mode) {
        snprintf_result = snprintf(processed_filename, MAX_FILENAME_LENGTH, "%s_processed.bmp", opts->debug_filename);
        if (snprintf_result < 0 || snprintf_result >= MAX_FILENAME_LENGTH) {
            fprintf(stderr, "Error: Could not create debug filename.\n");
            free_image_data(&image);
            return EXIT_FAILURE;
        }
        stbi_write_bmp(processed_filename, image.width, image.height, 3, image.data);
    }
#endif
    // Select the appropriate dithering function based on the specified method
    switch (opts->dither_method) {
    case 0:  dither_function = floydSteinbergDither; break;
    case 1:  dither_function = jarvisDither;         break;
    case 2:  dither_function = atkinsonDither;       break;
    case 3:  dither_function = bayer16x16Dither;     break;
    default: dither_function = NULL;
    }

    // Apply the dithering to the image if a dither function has been selected
    if (dither_function != NULL)
        dither_function(&image);

    // Trim the output filename to create an array name
    trim_filename_copy(opts->outfilename, array_name, MAX_FILENAME_LENGTH);

    // Write the image data to the file
    if (write_image_data_to_file(opts->outfilename, array_name, &image) != EXIT_SUCCESS) {
        free_image_data(&image);
        return EXIT_FAILURE;
    }

    // If debug mode is enabled, write a debug image
#ifdef DEBUG_BUILD
    if (opts->debug_mode) {
        snprintf_result = snprintf(final_filename, MAX_FILENAME_LENGTH, "%s_final.bmp", opts->debug_filename);
        if (snprintf_result < 0 || snprintf_result >= MAX_FILENAME_LENGTH) {
            fprintf(stderr, "Error: Could not create debug filename.\n");
            free_image_data(&image);
            return EXIT_FAILURE;
        }
        stbi_write_bmp(final_filename, image.width, image.height, 3, image.data);
    }
#endif
    // Free the image data
    free_image_data(&image);
    return EXIT_SUCCESS;
}

// --- Command Line Processing ---
void init_program_options(ProgramOptions* opts) {
    // Initialize the entire struct to 0
    memset(opts, 0, sizeof(ProgramOptions));

    // Set the default program options
    opts->dither_method = -1; // -1 Indicates that no dither method is selected
    opts->gamma = 1.0f;
    opts->contrast = 0.0f;
    opts->brightness = 1.0f;
    opts->debug_mode = 0; // debug mode is off by default
}

int parse_command_line_args(int argc, char* argv[], ProgramOptions* opts) {
    // Check if the options pointer is null
    if (opts == NULL) {
        fprintf(stderr, "Error: Null pointer passed to parse_command_line_args.\n");
        return 1;
    }
    // Iterate through each of the command line arguments
    for (int i = 1; i < argc; ++i) {
        // Parse the input file
        if (strcmp(argv[i], "-i") == 0 && i + 1 < argc) {
            strncpy(opts->infilename, argv[i + 1], MAX_FILENAME_LENGTH - 1);
            i++;
            // Parse the output file
        }
        else if (strcmp(argv[i], "-o") == 0 && i + 1 < argc) {
            strncpy(opts->outfilename, argv[i + 1], MAX_FILENAME_LENGTH - 1);
            i++;
            // Parse the dither method
        }
        else if (strcmp(argv[i], "-dm") == 0 && i + 1 < argc) {
            opts->dither_method = atoi(argv[i + 1]);
            i++;
            // Parse the debug mode
        }
#ifdef DEBUG_BUILD
        else if (strcmp(argv[i], "-debug") == 0 && i + 1 < argc) {
            opts->debug_mode = 1;
            strncpy(opts->debug_filename, argv[i + 1], MAX_FILENAME_LENGTH - 1);
            i++;
            // Parse the gamma
        }
#endif
        else if (strcmp(argv[i], "-g") == 0 && i + 1 < argc) {
            opts->gamma = (float)atof(argv[i + 1]);
            i++;
            // Parse the contrast
        }
        else if (strcmp(argv[i], "-c") == 0 && i + 1 < argc) {
            opts->contrast = (float)atof(argv[i + 1]);
            i++;
            // Parse the brightness
        }
        else if (strcmp(argv[i], "-b") == 0 && i + 1 < argc) {
            opts->brightness = (float)atof(argv[i + 1]);
            i++;
            // Print the help message
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
            // Report if invalid option is passed
        }
        else {
            fprintf(stderr, "Invalid option: %s\n", argv[i]);
            return 1;
        }
    }
    return 0;
}

int main(int argc, char* argv[]) {
    // Create a ProgramOptions struct
    ProgramOptions opts;

    // Initialize the program options struct with default values
    init_program_options(&opts);

    // Parse the command line arguments
    if (parse_command_line_args(argc, argv, &opts)) {
        return 1; // Return error if there is an issue with the command line args
    }

    // Process the image using the parsed options
    return process_image(&opts);
}