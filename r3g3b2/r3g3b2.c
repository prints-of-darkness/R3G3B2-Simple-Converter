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

#include "bayer16x16.h"     //bayer matrix
#include "color_lut.h"      //color LUT

#define STB_IMAGE_IMPLEMENTATION
#include "stb_image.h"

#define STB_IMAGE_WRITE_IMPLEMENTATION
#include "stb_image_write.h"

#define LUT_SIZE 256
#define MAX_FILENAME_LENGTH 1024

// Lookup tables for color conversion
uint8_t gamma_lut[LUT_SIZE];                    // Gamma correction lookup table
uint8_t contrast_brightness_lut[LUT_SIZE];      // Contrast and brightness correction lookup table

// Dithering functions
void jarvisDither(uint8_t* image, int width, int height);           // Jarvis dithering algorithm
void atkinsonDither(uint8_t* data, int width, int height);          // Atkinson dithering algorithm
void bayer16x16Dither(uint8_t* data, int width, int height);        // Bayer 32x32 dithering algorithm
void floydSteinbergDither(uint8_t* data, int width, int height);    // Floyd-Steinberg dithering algorithm

void write_data_to_file(FILE* fp, const char* array_name, uint8_t* data, int width, int height);

void process_image_lut(uint8_t* data, int width, int height);        // Process image using lookup tables
void initialize_luts(float gamma, float contrast, float brightness); // Initialize lookup tables for color conversion

int main(int argc, char* argv[]) {

    FILE* fp;
    uint8_t* data = NULL;

    float gamma = 1.0f;        // Default gamma value          [ 0.8f,    1.0f, 2.0f   ]
    float contrast = 0.0f;     // Default contrast value       [ -100.0f, 0,0f, 100.0f ]
    float brightness = 1.0f;   // Local brightness variable    [ 0.5f,    1.0f, 1.5f   ]

    int dither_method = -1;    // 0: Floyd-Steinberg, 1: Jarvis, 2: Atkinson 3: Bayer 16x16
    int debug = 0;             // Debug mode set to false
    int x = 0, y = 0, n = 0;

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
            printf("  -dm <method>              : Set dithering method (0: Floyd-Steinberg, 1: Jarvis, 2: Atkinson, 3: Bayer 16x16)\n");
            printf("  -debug <debug_filename>   : Enable debug mode and specify debug file prefix\n");
            printf("  -g <gamma>                : Set gamma value (default: 1.0)\n");
            printf("  -c <contrast>             : Set contrast value (default: 0.0)\n");
            printf("  -b <brightness>           : Set brightness value (default: 1.0)\n");
            printf("  -h                        : Display this help message\n");
            // Provide an example usage
            printf("Example: r3g3b2 -i tst.png -o tst.h -dm 0 -debug debug_output -g 1.0 -c 0.0 -b 1.0\n");
            // Exit the program after displaying the help message
            return 0;
        }
        // Handle unknown options
        else {
            printf("Unknown option: %s\n", argv[i]); // Print an error message for unknown options
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

    // Load image data from file using stb_image library
    if ((data = stbi_load(infilename, &x, &y, &n, 3)) != NULL) {
        
        initialize_luts(gamma, contrast, brightness);   // Initialize lookup tables for gamma, contrast, and brightness adjustments
        process_image_lut(data, x, y);                  // Process image data using lookup tables

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
            case 1: // Jarvis dithering
                jarvisDither(data, x, y);
                break;
            case 2: // Atkinson dithering
                atkinsonDither(data, x, y);
                break;
            case 3: //Bayer 32x32 dithering
                bayer16x16Dither(data, x, y);
                break;
            default: // Default to Floyd-Steinberg dithering
                floydSteinbergDither(data, x, y);
                break;
            }
        }

        // Open the output file and extract the filename without extension
        if ((fp = fopen(outfilename, "w")) != NULL) {
            strncpy(array_name, outfilename, MAX_FILENAME_LENGTH - 1);
            char* dot = strrchr(array_name, '.');
            if (dot) *dot = '\0';

            write_data_to_file(fp, array_name, data, x, y);
            fclose(fp);
        }
        else {
            printf("Failed to open output file.\n");
            stbi_image_free(data);
            return 1;
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
    else {
        printf("Failed to load image.\n");
    }

	return 1;
}

// Function to initialize the look-up tables (LUTs) for color conversion
void initialize_luts(float gamma, float contrast, float brightness) {

    // Calculate the contrast factor
    float value = 0;
    float factor = (259.0f * (contrast + 255.0f)) / (255.0f * (259.0f - contrast));

    // Iterate over all possible 8-bit color values
    for (int i = 0; i < LUT_SIZE; i++) {
        // Normalize the color value to the range [0, 1]
        value = i / 255.0f;

        // Apply gamma correction to the normalized color value
        value = powf(value, 1.0f / gamma);

        // Apply brightness and contrast adjustments to the color value
        value = factor * (value * brightness - 0.5f) + 0.5f;

        // Clamp the color value to the range [0, 1] and convert it back to an 8-bit value
        value = fmaxf(0.0f, fminf(value, 1.0f));
        contrast_brightness_lut[i] = (uint8_t)(value * 255.0f);

        // Create a separate gamma LUT for flexibility
        gamma_lut[i] = (uint8_t)(powf(i / 255.0f, 1.0f / gamma) * 255.0f);
    }
    return;
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
    return;
}

void write_data_to_file(FILE* fp, const char* array_name, uint8_t* data, int width, int height)
{
    uint8_t pixel332 = 0;
    uint8_t* pixel = NULL;
    int _y = 0, _x = 0;

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

    fprintf(fp, "static const uint8_t %s_data[%d] = {\n", array_name, width * height);

    // Iterate over each pixel in the image
    for (_y = 0; _y < height; _y++) {
        for (_x = 0; _x < width; _x++) {
            // Calculate the memory address of the current pixel
            pixel = data + (_y * width + _x) * 3;

            // Convert 24-bit RGB (888) to 8-bit RGB (332)
            // by masking and shifting the color components
            pixel332 = ((pixel[0] & 0xE0) | ((pixel[1] & 0xE0) >> 3) | (pixel[2] >> 6));

            // Print the converted pixel value in hexadecimal format
            fprintf(fp, "0x%.2X, ", pixel332);

            // back-convert the pixel to 24-bit RGB (888) for debug dumping
            // Use the color lookup table to convert the 8-bit RGB (332) value back to 24-bit RGB (888)
            pixel[0] = (color_lut[pixel332] >> 16) & 0xFF;
            pixel[1] = (color_lut[pixel332] >> 8) & 0xFF;
            pixel[2] = color_lut[pixel332] & 0xFF;
        }
        // Print a newline character after each row of pixels
        fprintf(fp, "\n");
    }

    fprintf(fp, "};\n\n");
    fprintf(fp, "static const Image_t %s_image = {\n", array_name);
    fprintf(fp, "    .data = %s_data,\n", array_name);
    fprintf(fp, "    .width = %d,\n", width);
    fprintf(fp, "    .height = %d,\n", height);
    fprintf(fp, "    .format_id = RGB332_FORMAT_ID\n");
    fprintf(fp, "};\n\n");
    fprintf(fp, "#endif // %s_H\n", array_name);

    return;
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
    return;
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
    return;
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
    return;
}

void bayer16x16Dither(uint8_t* data, int width, int height) {
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
            threshold = BAYER_MATRIX_16X16[y % 16][x % 16];

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
    return;
}