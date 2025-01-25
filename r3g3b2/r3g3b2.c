/*
    r3g3b2.c

    Converts an RGB image to 8bpp RGB332. (needed for LT7683 TFT graphics controller)

    MJM 2025

*/

#include <stdio.h>
#include <stdint.h>
#include <stdlib.h>
#include <string.h>
#include <math.h>

#define STB_IMAGE_IMPLEMENTATION
#include "stb_image.h"

#define STB_IMAGE_WRITE_IMPLEMENTATION
#include "stb_image_write.h"

#define LUT_SIZE 256
#define MAX_FILENAME_LENGTH 1024

// Lookup tables for color conversion
uint8_t gamma_lut[LUT_SIZE];                    // Gamma correction lookup table
uint32_t color_lut[LUT_SIZE];                   // Color conversion lookup table
uint8_t contrast_brightness_lut[LUT_SIZE];      // Contrast and brightness correction lookup table

// Bayer 32 x 32 matrix
const uint8_t bayer32x32[32][32] = {
    {0, 192, 48, 240, 12, 204, 60, 252, 3, 195, 51, 243, 15, 207, 63, 255, 1, 193, 49, 241, 13, 205, 61, 253, 2, 194, 50, 242, 14, 206, 62, 254},
    {128, 64, 176, 112, 140, 76, 188, 124, 131, 67, 179, 115, 143, 79, 191, 127, 129, 65, 177, 113, 141, 77, 189, 125, 130, 66, 178, 114, 142, 78, 190, 126},
    {32, 224, 16, 208, 44, 236, 28, 220, 35, 227, 19, 211, 47, 239, 31, 223, 33, 225, 17, 209, 45, 237, 29, 221, 34, 226, 18, 210, 46, 238, 30, 222},
    {160, 96, 144, 80, 172, 108, 156, 92, 163, 99, 147, 83, 175, 111, 159, 95, 161, 97, 145, 81, 173, 109, 157, 93, 162, 98, 146, 82, 174, 110, 158, 94},
    {8, 200, 56, 248, 4, 196, 52, 244, 11, 203, 59, 251, 7, 199, 55, 247, 9, 201, 57, 249, 5, 197, 53, 245, 10, 202, 58, 250, 6, 198, 54, 246},
    {136, 72, 184, 120, 132, 68, 180, 116, 139, 75, 187, 123, 135, 71, 183, 119, 137, 73, 185, 121, 133, 69, 181, 117, 138, 74, 186, 122, 134, 70, 182, 118},
    {40, 232, 24, 216, 36, 228, 20, 212, 43, 235, 27, 219, 39, 231, 23, 215, 41, 233, 25, 217, 37, 229, 21, 213, 42, 234, 26, 218, 38, 230, 22, 214},
    {168, 104, 152, 88, 164, 100, 148, 84, 171, 107, 155, 91, 167, 103, 151, 87, 169, 105, 153, 89, 165, 101, 149, 85, 170, 106, 154, 90, 166, 102, 150, 86},
    {2, 194, 50, 242, 14, 206, 62, 254, 1, 193, 49, 241, 13, 205, 61, 253, 3, 195, 51, 243, 15, 207, 63, 255, 0, 192, 48, 240, 12, 204, 60, 252},
    {130, 66, 178, 114, 142, 78, 190, 126, 129, 65, 177, 113, 141, 77, 189, 125, 131, 67, 179, 115, 143, 79, 191, 127, 128, 64, 176, 112, 140, 76, 188, 124},
    {34, 226, 18, 210, 46, 238, 30, 222, 33, 225, 17, 209, 45, 237, 29, 221, 35, 227, 19, 211, 47, 239, 31, 223, 32, 224, 16, 208, 44, 236, 28, 220},
    {162, 98, 146, 82, 174, 110, 158, 94, 161, 97, 145, 81, 173, 109, 157, 93, 163, 99, 147, 83, 175, 111, 159, 95, 160, 96, 144, 80, 172, 108, 156, 92},
    {10, 202, 58, 250, 6, 198, 54, 246, 9, 201, 57, 249, 5, 197, 53, 245, 11, 203, 59, 251, 7, 199, 55, 247, 8, 200, 56, 248, 4, 196, 52, 244},
    {138, 74, 186, 122, 134, 70, 182, 118, 137, 73, 185, 121, 133, 69, 181, 117, 139, 75, 187, 123, 135, 71, 183, 119, 136, 72, 184, 120, 132, 68, 180, 116},
    {42, 234, 26, 218, 38, 230, 22, 214, 41, 233, 25, 217, 37, 229, 21, 213, 43, 235, 27, 219, 39, 231, 23, 215, 40, 232, 24, 216, 36, 228, 20, 212},
    {170, 106, 154, 90, 166, 102, 150, 86, 169, 105, 153, 89, 165, 101, 149, 85, 171, 107, 155, 91, 167, 103, 151, 87, 168, 104, 152, 88, 164, 100, 148, 84},
    {1, 193, 49, 241, 13, 205, 61, 253, 2, 194, 50, 242, 14, 206, 62, 254, 0, 192, 48, 240, 12, 204, 60, 252, 3, 195, 51, 243, 15, 207, 63, 255},
    {129, 65, 177, 113, 141, 77, 189, 125, 130, 66, 178, 114, 142, 78, 190, 126, 128, 64, 176, 112, 140, 76, 188, 124, 131, 67, 179, 115, 143, 79, 191, 127},
    {33, 225, 17, 209, 45, 237, 29, 221, 34, 226, 18, 210, 46, 238, 30, 222, 32, 224, 16, 208, 44, 236, 28, 220, 35, 227, 19, 211, 47, 239, 31, 223},
    {161, 97, 145, 81, 173, 109, 157, 93, 162, 98, 146, 82, 174, 110, 158, 94, 160, 96, 144, 80, 172, 108, 156, 92, 163, 99, 147, 83, 175, 111, 159, 95},
    {9, 201, 57, 249, 5, 197, 53, 245, 10, 202, 58, 250, 6, 198, 54, 246, 8, 200, 56, 248, 4, 196, 52, 244, 11, 203, 59, 251, 7, 199, 55, 247},
    {137, 73, 185, 121, 133, 69, 181, 117, 138, 74, 186, 122, 134, 70, 182, 118, 136, 72, 184, 120, 132, 68, 180, 116, 139, 75, 187, 123, 135, 71, 183, 119},
    {41, 233, 25, 217, 37, 229, 21, 213, 42, 234, 26, 218, 38, 230, 22, 214, 40, 232, 24, 216, 36, 228, 20, 212, 43, 235, 27, 219, 39, 231, 23, 215},
    {169, 105, 153, 89, 165, 101, 149, 85, 170, 106, 154, 90, 166, 102, 150, 86, 168, 104, 152, 88, 164, 100, 148, 84, 171, 107, 155, 91, 167, 103, 151, 87},
    {3, 195, 51, 243, 15, 207, 63, 255, 0, 192, 48, 240, 12, 204, 60, 252, 2, 194, 50, 242, 14, 206, 62, 254, 1, 193, 49, 241, 13, 205, 61, 253},
    {131, 67, 179, 115, 143, 79, 191, 127, 128, 64, 176, 112, 140, 76, 188, 124, 130, 66, 178, 114, 142, 78, 190, 126, 129, 65, 177, 113, 141, 77, 189, 125},
    {35, 227, 19, 211, 47, 239, 31, 223, 32, 224, 16, 208, 44, 236, 28, 220, 34, 226, 18, 210, 46, 238, 30, 222, 33, 225, 17, 209, 45, 237, 29, 221},
    {163, 99, 147, 83, 175, 111, 159, 95, 160, 96, 144, 80, 172, 108, 156, 92, 162, 98, 146, 82, 174, 110, 158, 94, 161, 97, 145, 81, 173, 109, 157, 93},
    {11, 203, 59, 251, 7, 199, 55, 247, 8, 200, 56, 248, 4, 196, 52, 244, 10, 202, 58, 250, 6, 198, 54, 246, 9, 201, 57, 249, 5, 197, 53, 245},
    {139, 75, 187, 123, 135, 71, 183, 119, 136, 72, 184, 120, 132, 68, 180, 116, 138, 74, 186, 122, 134, 70, 182, 118, 137, 73, 185, 121, 133, 69, 181, 117},
    {43, 235, 27, 219, 39, 231, 23, 215, 40, 232, 24, 216, 36, 228, 20, 212, 42, 234, 26, 218, 38, 230, 22, 214, 41, 233, 25, 217, 37, 229, 21, 213},
    {171, 107, 155, 91, 167, 103, 151, 87, 168, 104, 152, 88, 164, 100, 148, 84, 170, 106, 154, 90, 166, 102, 150, 86, 169, 105, 153, 89, 165, 101, 149, 85}
};

// Dithering functions
void jarvisDither(uint8_t* image, int width, int height);           // Jarvis dithering algorithm
void atkinsonDither(uint8_t* data, int width, int height);          // Atkinson dithering algorithm
void bayer32x32Dither(uint8_t* data, int width, int height);        // Bayer 32x32 dithering algorithm
void process_image_lut(uint8_t* data, int width, int height);       // Process image using lookup tables
void floydSteinbergDither(uint8_t* data, int width, int height);    // Floyd-Steinberg dithering algorithm

// Lookup table initialization function
void initialize_luts(float gamma, float contrast, float brightness); // Initialize lookup tables for color conversion

int main(int argc, char* argv[]) {

    FILE* fp;

    uint8_t* pixel = NULL;
    uint8_t* data = NULL;

    uint8_t pixel332 = 0;

    float gamma = 1.0f;        // Default gamma value          [ 0.8f,    1.0f, 2.0f   ]
    float contrast = 0.0f;     // Default contrast value       [ -100.0f, 0,0f, 100.0f ]
    float brightness = 1.0f;   // Local brightness variable    [ 0.5f,    1.0f, 1.5f   ]

    int dither_method = -1;    // 0: Floyd-Steinberg, 1: Jarvis, 2: Atkinson 3: Bayer 32x32
    int debug = 0;             // Debug mode set to false
    int x = 0, y = 0, n = 0;
    int _y = 0, _x = 0;

    char infilename[MAX_FILENAME_LENGTH] = { 0 }; //.bmp, .jpg, .png
    char outfilename[MAX_FILENAME_LENGTH] = { 0 };
    char array_name[MAX_FILENAME_LENGTH] = { 0 };
    char debug_filename[MAX_FILENAME_LENGTH] = { 0 };

    // Iterate through command line arguments
    for (int i = 1; i < argc; i++) {
        // Check if the current argument is the input file flag
        if (strcmp(argv[i], "-i") == 0 && i + 1 < argc) {
            strncpy(infilename, argv[i + 1], MAX_FILENAME_LENGTH - 1);  // Copy the input file name to the infilename variable
            printf("Input file: %s\n", infilename);                     // Print the input file name to the console
            i++;                                                        // Increment the argument index to skip the input file name
        }
        // Check if the current argument is the output file flag
        else if (strcmp(argv[i], "-o") == 0 && i + 1 < argc) {            
            strncpy(outfilename, argv[i + 1], MAX_FILENAME_LENGTH - 1); // Copy the output file name to the outfilename variable
            printf("Output file: %s\n", outfilename);                   // Print the output file name to the console
            i++;                                                        // Increment the argument index to skip the output file name
        }
        // Check if the current argument is the debug mode flag
        else if (strcmp(argv[i], "-debug") == 0 && i + 1 < argc) {            
            debug = 1;                                                          // Enable debug mode            
            strncpy(debug_filename, argv[i + 1], MAX_FILENAME_LENGTH - 1);      // Copy the debug file prefix to the debug_filename variable            
            printf("Debug mode on, debug file prefix: %s\n", debug_filename);   // Print the debug file prefix to the console            
            i++;                                                                // Increment the argument index to skip the debug file prefix
        }
        // Check if the current argument is the dither method flag
        else if (strcmp(argv[i], "-dm") == 0 && i + 1 < argc) {            
            dither_method = atoi(argv[i + 1]);                      // Set the dither method to the specified value
            printf("Dither method set to: %d\n", dither_method);    // Print the dither method to the console
            i++;                                                    // Increment the argument index to skip the dither method value
        }
        // Check if the current argument is the gamma flag
        else if (strcmp(argv[i], "-g") == 0 && i + 1 < argc) {
            gamma = (float)atof(argv[i + 1]);                       // Set the gamma value to the specified value
            printf("Gamma set to: %.2f\n", gamma);                  // Print the gamma value to the console
            i++;                                                    // Increment the argument index to skip the gamma value
        }
        // Check if the current argument is the contrast flag
        else if (strcmp(argv[i], "-c") == 0 && i + 1 < argc) {
            contrast = (float)atof(argv[i + 1]);                    // Set the contrast value to the specified value
            printf("Contrast set to: %.2f\n", contrast);            // Print the contrast value to the console
            i++;                                                    // Increment the argument index to skip the contrast value
        }
        // Check if the current argument is the brightness flag
        else if (strcmp(argv[i], "-b") == 0 && i + 1 < argc) {
            brightness = (float)atof(argv[i + 1]);                  // Set the brightness value to the specified value
            printf("Brightness set to: %.2f\n", brightness);        // Print the brightness value to the console
            i++;                                                    // Increment the argument index to skip the brightness value
        }
        // Handle the help option
        else if (strcmp(argv[i], "-h") == 0) {
            // Print the usage message
            printf("Usage: r3g3b2 -i <input file> -o <output file> [-d] [-debug <debug_filename>] [-g <gamma>] [-c <contrast>] [-b <brightness>]\n");
            // Explain each option
            printf("  -i <input file>           : Specify input file\n");
            printf("  -o <output file>          : Specify output file\n");
            printf("  -dm <method>              : Set dithering method (0: Floyd-Steinberg, 1: Jarvis, 2: Atkinson, 3: Bayer 32x32)\n");
            printf("  -debug <debug_filename>   : Enable debug mode and specify debug file prefix\n");
            printf("  -g <gamma>                : Set gamma value (default: 1.0)\n");
            printf("  -c <contrast>             : Set contrast value (default: 0.0)\n");
            printf("  -b <brightness>           : Set brightness value (default: 1.0)\n");
            printf("  -h                        : Display this help message\n");
            // Provide an example usage
            printf("Example: r3g3b2 -i tst.png -o tst.h -d -debug debug_output -g 1.0 -c 0.0 -b 1.0\n");
            // Exit the program after displaying the help message
            return 0;
        }
        // Handle unknown options
        else {
            // Print an error message for unknown options
            printf("Unknown option: %s\n", argv[i]);
        }
    }

    // Check if the input file name is empty
    if (infilename[0] == '\0') {
        printf("No input file specified.\n");
        return 1;
    }

    // Check if the output file name is empty
    if (outfilename[0] == '\0') {
        printf("No output file specified.\n");
        return 1;
    }

    // Initialize lookup tables for gamma, contrast, and brightness adjustments
    initialize_luts(gamma, contrast, brightness);

    // Load image data from file using stb_image library
    if ((data = stbi_load(infilename, &x, &y, &n, 3)) != NULL) {
        // Process image data using lookup tables
        process_image_lut(data, x, y);

        // If debug mode is enabled, save processed image to file
        if (debug) {
            // Construct filename for processed image
            char processed_filename[MAX_FILENAME_LENGTH];
            snprintf(processed_filename, MAX_FILENAME_LENGTH, "%s_processed.bmp", debug_filename);
            // Write processed image to file using stb_image library
            stbi_write_bmp(processed_filename, x, y, 3, data);
        }

        // Apply dithering if a valid method is selected
        if (dither_method != -1) {
            switch (dither_method) {
            //case 0: // Floyd-Steinberg dithering
            //    floydSteinbergDither(data, x, y);
            //    break;
            case 1: // Jarvis dithering
                jarvisDither(data, x, y);
                break;
            case 2: // Atkinson dithering
                atkinsonDither(data, x, y);
                break;
            case 3:
                bayer32x32Dither(data, x, y);
                break;
            default: // Default to Floyd-Steinberg dithering
                floydSteinbergDither(data, x, y);
                break;
            }
        }

        // Open the output file and extract the filename without extension
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

            // Iterate over each pixel in the image
            for (_y = 0; _y < y; _y++) {
                for (_x = 0; _x < x; _x++) {
                    // Calculate the memory address of the current pixel
                    pixel = data + (_y * x + _x) * 3;

                    // Convert 24-bit RGB (888) to 8-bit RGB (332)
                    // by masking and shifting the color components
                    pixel332 = ((pixel[0] & 0xE0) | ((pixel[1] & 0xE0) >> 3) | (pixel[2] >> 6));

                    // Print the converted pixel value in hexadecimal format
                    fprintf(fp, "0x%.2X, ", pixel332);

                    // If debug mode is enabled, back-convert the pixel to 24-bit RGB (888)
                    // for debugging purposes
                    if (debug) {
                        // Use the color lookup table to convert the 8-bit RGB (332) value back to 24-bit RGB (888)
                        pixel[0] = (color_lut[pixel332] >> 16) & 0xFF;
                        pixel[1] = (color_lut[pixel332] >> 8) & 0xFF;
                        pixel[2] = color_lut[pixel332] & 0xFF;
                    }
                }
                // Print a newline character after each row of pixels
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
        // Handle image loading and output file opening errors
        if (data == NULL) {
            printf("Failed to load image.\n");
            return 1;
        }

        if (fp == NULL) {
            printf("Failed to open output file.\n");
        }

        // Write final image to file in debug mode
        if (debug) {
            char final_filename[MAX_FILENAME_LENGTH];
            snprintf(final_filename, MAX_FILENAME_LENGTH, "%s_final.bmp", debug_filename);
            stbi_write_bmp(final_filename, x, y, 3, data);
        }

        // Free allocated image data
        stbi_image_free(data);

        return 0;
    }
}

// Function to initialize the look-up tables (LUTs) for color conversion
void initialize_luts(float gamma, float contrast, float brightness) {
   
    // Initialize variables to store 3-bit and 8-bit color values
    uint8_t r3 = 0, g3 = 0, b2 = 0;
    uint8_t r8 = 0, g8 = 0, b8 = 0;

    // Calculate the contrast factor
    float value = 0;
    float factor = (259.0f * (contrast + 255.0f)) / (255.0f * (259.0f - contrast));

    // Iterate over all possible 8-bit color values
    for (int i = 0; i < LUT_SIZE; i++) {
        // Normalize the color value to the range [0, 1]
        value = i / 255.0f;

        // Expand the 8-bit color value to 3-bit red, 3-bit green, and 2-bit blue components
        r3 = (i >> 5) & 0x7;
        g3 = (i >> 2) & 0x7;
        b2 = i & 0x3;

        // Convert the 3-bit color components to 8-bit values
        r8 = (r3 << 5) | (r3 << 2) | (r3 >> 1);
        g8 = (g3 << 5) | (g3 << 2) | (g3 >> 1);
        b8 = (b2 << 6) | (b2 << 4) | (b2 << 2) | b2;

        // Apply gamma correction to the normalized color value
        value = powf(value, 1.0f / gamma);

        // Apply brightness and contrast adjustments to the color value
        value = factor * (value * brightness - 0.5f) + 0.5f;

        // Clamp the color value to the range [0, 1] and convert it back to an 8-bit value
        value = fmaxf(0.0f, fminf(value, 1.0f));
        contrast_brightness_lut[i] = (uint8_t)(value * 255.0f);

        // Create a separate gamma LUT for flexibility
        gamma_lut[i] = (uint8_t)(powf(i / 255.0f, 1.0f / gamma) * 255.0f);

        // Assign the 32-bit color value to the color LUT
        color_lut[i] = (r8 << 16) | (g8 << 8) | b8;
    }
}

void process_image_lut(uint8_t* data, int width, int height) {
    // Initialize loop counters
    int x = 0, y = 0, idx = 0;

    // Iterate over each pixel in the image
    for (y = 0; y < height; y++) {
        for (x = 0; x < width; x++) {
            // Calculate the index of the current pixel in the data array
            idx = (y * width + x) * 3;

            // Apply contrast and brightness correction using the lookup tables
            // First, apply gamma correction to the pixel values
            // Then, apply contrast and brightness correction
            data[idx]     = contrast_brightness_lut[gamma_lut[data[idx]]];
            data[idx + 1] = contrast_brightness_lut[gamma_lut[data[idx + 1]]];
            data[idx + 2] = contrast_brightness_lut[gamma_lut[data[idx + 2]]];
        }
    }
}

void floydSteinbergDither(uint8_t* data, int width, int height) {
    // Initialize variables for dithering
    int x = 0, y = 0;
    uint8_t oldR = 0, oldG = 0, oldB = 0;
    uint8_t newR = 0, newG = 0, newB = 0;
    int errorR = 0, errorG = 0, errorB = 0;

    // Extract RGB values from the data array
    for (y = 0; y < height; y++) {
        for (x = 0; x < width; x++) {
            // Get the old RGB values
            oldR = data[(y * width + x) * 3];   // Red
            oldG = data[(y * width + x) * 3 + 1]; // Green
            oldB = data[(y * width + x) * 3 + 2]; // Blue

            // Apply bit masks to reduce color depth
            newR = (oldR & 0xE0); // Keep 3 most significant bits of Red
            newG = (oldG & 0xE0); // Keep 3 most significant bits of Green
            newB = (oldB & 0xC0); // Keep 2 most significant bits of Blue

            // Update current pixel with new color values
            data[(y * width + x) * 3]     = newR;
            data[(y * width + x) * 3 + 1] = newG;
            data[(y * width + x) * 3 + 2] = newB;

            // Calculate color errors
            errorR = oldR - newR;
            errorG = oldG - newG;
            errorB = oldB - newB;

            // Diffuse error to adjacent pixel (if within bounds)
            if (x < width - 1) {
                // Apply error diffusion to red, green, and blue channels
                data[(y * width + x + 1) * 3]     = min(255, max(0, data[(y * width + x + 1) * 3] + errorR * 7 / 16));
                data[(y * width + x + 1) * 3 + 1] = min(255, max(0, data[(y * width + x + 1) * 3 + 1] + errorG * 7 / 16));
                data[(y * width + x + 1) * 3 + 2] = min(255, max(0, data[(y * width + x + 1) * 3 + 2] + errorB * 7 / 16));
            }

            // If the current pixel is not at the bottom row
            if (y < height - 1) {
                // If the current pixel is not at the leftmost column
                if (x > 0) {
                    // Diffuse the red error to the pixel to the left and below
                    data[((y + 1) * width + x - 1) * 3] = min(255, max(0, data[((y + 1) * width + x - 1) * 3] + errorR * 3 / 16));
                    // Diffuse the green error to the pixel to the left and below
                    data[((y + 1) * width + x - 1) * 3 + 1] = min(255, max(0, data[((y + 1) * width + x - 1) * 3 + 1] + errorG * 3 / 16));
                    // Diffuse the blue error to the pixel to the left and below
                    data[((y + 1) * width + x - 1) * 3 + 2] = min(255, max(0, data[((y + 1) * width + x - 1) * 3 + 2] + errorB * 3 / 16));
                }

                // Diffuse the red error to the pixel below
                data[((y + 1) * width + x) * 3] = min(255, max(0, data[((y + 1) * width + x) * 3] + errorR * 5 / 16));
                // Diffuse the green error to the pixel below
                data[((y + 1) * width + x) * 3 + 1] = min(255, max(0, data[((y + 1) * width + x) * 3 + 1] + errorG * 5 / 16));
                // Diffuse the blue error to the pixel below
                data[((y + 1) * width + x) * 3 + 2] = min(255, max(0, data[((y + 1) * width + x) * 3 + 2] + errorB * 5 / 16));

                // If the current pixel is not at the rightmost column
                if (x < width - 1) {
                    // Diffuse the red error to the pixel to the right and below
                    data[((y + 1) * width + x + 1) * 3] = min(255, max(0, data[((y + 1) * width + x + 1) * 3] + errorR * 1 / 16));
                    // Diffuse the green error to the pixel to the right and below
                    data[((y + 1) * width + x + 1) * 3 + 1] = min(255, max(0, data[((y + 1) * width + x + 1) * 3 + 1] + errorG * 1 / 16));
                    // Diffuse the blue error to the pixel to the right and below
                    data[((y + 1) * width + x + 1) * 3 + 2] = min(255, max(0, data[((y + 1) * width + x + 1) * 3 + 2] + errorB * 1 / 16));
                }
            }
        }
    }
}

void jarvisDither(uint8_t* data, int width, int height) {
    // Initialize variables for dithering
    int x = 0, y = 0;
    int errorR = 0, errorG = 0, errorB = 0;
    int i = 0, j = 0, idx = 0, nx = 0, ny = 0; 

    uint8_t oldR = 0, oldG = 0, oldB = 0; 
    uint8_t newR = 0, newG = 0, newB = 0;

    // Jarvis, Judice, and Ninke dithering matrix
    int matrix[3][5] = {
    {0, 0, 0, 7, 5},
    {3, 5, 7, 5, 3},
    {1, 3, 5, 3, 1}
    };

    // Iterate over each pixel in the image
    for (y = 0; y < height; y++) {
        for (x = 0; x < width; x++) {
            // Extract the RGB values from the data array
            oldR = data[(y * width + x) * 3];     // Red
            oldG = data[(y * width + x) * 3 + 1]; // Green
            oldB = data[(y * width + x) * 3 + 2]; // Blue

            // Apply bit masks to reduce color depth
            newR = (oldR & 0xE0); // Keep 3 most significant bits of Red
            newG = (oldG & 0xE0); // Keep 3 most significant bits of Green
            newB = (oldB & 0xC0); // Keep 2 most significant bits of Blue

            // Update current pixel with new color values
            data[(y * width + x) * 3]     = newR;
            data[(y * width + x) * 3 + 1] = newG;
            data[(y * width + x) * 3 + 2] = newB;

            // Calculate color errors
            errorR = oldR - newR;
            errorG = oldG - newG;
            errorB = oldB - newB;

            // Diffuse error to adjacent pixels using the Jarvis, Judice, and Ninke dithering matrix
            // This loop iterates over the 3x5 dithering matrix
            for (i = 0; i < 3; i++) {
                // This loop iterates over each column of the dithering matrix
                for (j = 0; j < 5; j++) {
                    // Calculate the x and y coordinates of the adjacent pixel
                    nx = x + j - 2;
                    ny = y + i;
                    
                    // Check if the adjacent pixel is within the image boundaries
                    if (nx >= 0 && nx < width && ny < height) {
                        // Calculate the index of the adjacent pixel in the data array
                        idx = (ny * width + nx) * 3;
                        
                        // Diffuse the error to the red, green, and blue channels of the adjacent pixel
                        // using the corresponding value from the dithering matrix
                        data[idx] = min(255, max(0, data[idx] + errorR * matrix[i][j] / 48));
                        data[idx + 1] = min(255, max(0, data[idx + 1] + errorG * matrix[i][j] / 48));
                        data[idx + 2] = min(255, max(0, data[idx + 2] + errorB * matrix[i][j] / 48));
                    }
                }
            }
        }
    }
}

void atkinsonDither(uint8_t* data, int width, int height) {
    // Initialize variables for color dithering
    int x = 0, y = 0;
    uint8_t oldR = 0, oldG = 0, oldB = 0;
    uint8_t newR = 0, newG = 0, newB = 0;
    int errorR = 0, errorG = 0, errorB = 0;

    // Iterate over each pixel in the image
    for (y = 0; y < height; y++) {
        for (x = 0; x < width; x++) {
            // Extract the RGB values from the data array
            oldR = data[(y * width + x) * 3];     // Red
            oldG = data[(y * width + x) * 3 + 1]; // Green
            oldB = data[(y * width + x) * 3 + 2]; // Blue

            // Apply bit masks to reduce color depth
            newR = (oldR & 0xE0); // Keep 3 most significant bits of Red
            newG = (oldG & 0xE0); // Keep 3 most significant bits of Green
            newB = (oldB & 0xC0); // Keep 2 most significant bits of Blue

            // Update current pixel with new color values
            data[(y * width + x) * 3]     = newR;
            data[(y * width + x) * 3 + 1] = newG;
            data[(y * width + x) * 3 + 2] = newB;

            // Calculate color errors
            errorR = (oldR - newR) / 8;
            errorG = (oldG - newG) / 8;
            errorB = (oldB - newB) / 8;
            // Diffuse error to adjacent pixels
            if (x < width - 1) {
                // Apply error diffusion to red, green, and blue channels
                data[(y * width + x + 1) * 3]     = min(255, max(0, data[(y * width + x + 1) * 3] + errorR * 7 / 16));
                data[(y * width + x + 1) * 3 + 1] = min(255, max(0, data[(y * width + x + 1) * 3 + 1] + errorG * 7 / 16));
                data[(y * width + x + 1) * 3 + 2] = min(255, max(0, data[(y * width + x + 1) * 3 + 2] + errorB * 7 / 16));
            }
            if (x < width - 2) {
                // Apply error diffusion to red, green, and blue channels
                data[(y * width + x + 2) * 3]     = min(255, max(0, data[(y * width + x + 2) * 3] + errorR * 7 / 16));
                data[(y * width + x + 2) * 3 + 1] = min(255, max(0, data[(y * width + x + 2) * 3 + 1] + errorG * 7 / 16));
                data[(y * width + x + 2) * 3 + 2] = min(255, max(0, data[(y * width + x + 2) * 3 + 2] + errorB * 7 / 16));
            }
            // Diffuse error to pixels below and to the sides
            if (y < height - 1) {
                // Pixel directly below
                data[((y + 1) * width + x) * 3] = min(255, max(0, data[((y + 1) * width + x) * 3] + errorR));
                data[((y + 1) * width + x) * 3 + 1] = min(255, max(0, data[((y + 1) * width + x) * 3 + 1] + errorG));
                data[((y + 1) * width + x) * 3 + 2] = min(255, max(0, data[((y + 1) * width + x) * 3 + 2] + errorB));

                // Pixel below and to the left
                if (x > 0) {
                    data[((y + 1) * width + x - 1) * 3] = min(255, max(0, data[((y + 1) * width + x - 1) * 3] + errorR));
                    data[((y + 1) * width + x - 1) * 3 + 1] = min(255, max(0, data[((y + 1) * width + x - 1) * 3 + 1] + errorG));
                    data[((y + 1) * width + x - 1) * 3 + 2] = min(255, max(0, data[((y + 1) * width + x - 1) * 3 + 2] + errorB));
                }

                // Pixel below and to the right
                if (x < width - 1) {
                    data[((y + 1) * width + x + 1) * 3] = min(255, max(0, data[((y + 1) * width + x + 1) * 3] + errorR));
                    data[((y + 1) * width + x + 1) * 3 + 1] = min(255, max(0, data[((y + 1) * width + x + 1) * 3 + 1] + errorG));
                    data[((y + 1) * width + x + 1) * 3 + 2] = min(255, max(0, data[((y + 1) * width + x + 1) * 3 + 2] + errorB));
                }
            }

            // Pixel two rows below
            if (y < height - 2) {
                data[((y + 2) * width + x) * 3] = min(255, max(0, data[((y + 2) * width + x) * 3] + errorR));
                data[((y + 2) * width + x) * 3 + 1] = min(255, max(0, data[((y + 2) * width + x) * 3 + 1] + errorG));
                data[((y + 2) * width + x) * 3 + 2] = min(255, max(0, data[((y + 2) * width + x) * 3 + 2] + errorB));
            }
        }
    }
}

void bayer32x32Dither(uint8_t* data, int width, int height) {
    // Initialize variables for iterating over the image
    int y = 0, x = 0, idx = 0;
    uint8_t r = 0, g = 0, b = 0, threshold = 0;

    // Iterate over each pixel in the image
    for (y = 0; y < height; y++) {
        for (x = 0; x < width; x++) {
            // Calculate the index of the current pixel in the data array
            idx = (y * width + x) * 3;

            // Extract the RGB values from the data array
            r = data[idx];     // Red
            g = data[idx + 1]; // Green
            b = data[idx + 2]; // Blue

            // Apply Bayer dithering using the 32x32 threshold matrix
            threshold = bayer32x32[y % 32][x % 32];

            // Convert the RGB values to RGB332 format
            // If the pixel value is greater than the threshold, set it to the maximum value (0xE0 for R and G, 0xC0 for B)
            // Otherwise, set it to 0
            r = (r > threshold) ? 0xE0 : 0;
            g = (g > threshold) ? 0xE0 : 0;
            b = (b > threshold) ? 0xC0 : 0;

            // Store the resulting RGB332 values back in the data array
            data[idx] = r;
            data[idx + 1] = g;
            data[idx + 2] = b;
        }
    }
}