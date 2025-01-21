
/*

	code to convert r3g3b2 array to r8g8b8 bmp
        used for debugging

        MJM 2025
*/

#include <stdio.h>
#include <stdlib.h>
#include <stdint.h>
#include <string.h>
#include <ctype.h>

#define STB_IMAGE_WRITE_IMPLEMENTATION
#include "stb_image_write.h"

uint32_t r3g3b2_to_r8g8b8(uint8_t pixel) {
    uint8_t r = (pixel & 0xE0) | ((pixel & 0xE0) >> 3);
    uint8_t g = ((pixel & 0x1C) << 3) | ((pixel & 0x1C) >> 2);
    uint8_t b = ((pixel & 0x03) << 6) | ((pixel & 0x03) << 4) | ((pixel & 0x03) << 2) | (pixel & 0x03);
    return (r << 16) | (g << 8) | b;
}

char* get_base_filename(const char* path) {
    char* filename = strrchr(path, '/');
    filename = filename ? filename + 1 : (char*)path;
    char* dot = strrchr(filename, '.');
    if (dot) *dot = '\0';
    return _strdup(filename);
}

int main(int argc, char* argv[]) {
    if (argc != 2) {
        printf("Usage: %s <image_header.h>\n", argv[0]);
        return 1;
    }

    FILE* file = fopen(argv[1], "r");
    if (!file) {
        printf("Failed to open header file: %s\n", argv[1]);
        return 1;
    }

    char* base_filename = get_base_filename(argv[1]);
    char data_name[256];
    snprintf(data_name, sizeof(data_name), "%s_data", base_filename);

    int width = 0, height = 0;
    char line[8192];
    int line_count = 0;

    while (fgets(line, sizeof(line), file)) {
        line_count++;
        if (strstr(line, ".width") && sscanf(line, "%*[^0-9]%d", &width) == 1) {
            printf("Width found: %d at line %d\n", width, line_count);
        }
        if (strstr(line, ".height") && sscanf(line, "%*[^0-9]%d", &height) == 1) {
            printf("Height found: %d at line %d\n", height, line_count);
        }
        if (width != 0 && height != 0) break;
    }

    if (width == 0 || height == 0) {
        printf("Width or height not found in the file.\n");
    }

    printf("Parsed dimensions: %d x %d\n", width, height);

    rewind(file);

    printf("Looking for pixel data starting with: %s\n", data_name);

    int found_data = 0;
    while (fgets(line, sizeof(line), file)) {
        if (strstr(line, data_name)) {
            found_data = 1;
            printf("Found start of pixel data: %s\n", data_name);
            break;
        }
    }

    if (!found_data) {
        printf("Error: Could not find pixel data in the file.\n");
    }

    uint8_t* data = malloc(width * height * 3);
    if (!data) {
        printf("Error: Failed to allocate memory for image data.\n");
    }

    // Parse data from header file
    int pixel_count = 0;
    while (fgets(line, sizeof(line), file) && pixel_count < width * height) {
        char* token = strtok(line, ",");
        while (token && pixel_count < width * height) {
            // Skip whitespace and '0x' prefix
            while (*token && isspace(*token)) token++;
            if (strncmp(token, "0x", 2) == 0) token += 2;

            // Convert hex string to integer
            uint8_t pixel;
            if (sscanf(token, "%hhx", &pixel) == 1) {
                uint32_t rgb = r3g3b2_to_r8g8b8(pixel);
                data[pixel_count * 3] = (rgb >> 16) & 0xFF;     // R
                data[pixel_count * 3 + 1] = (rgb >> 8) & 0xFF;  // G
                data[pixel_count * 3 + 2] = rgb & 0xFF;         // B
                pixel_count++;
            }
            token = strtok(NULL, ",");
        }
    }

    if (pixel_count < width * height) {
        printf("Warning: Not enough pixel data found. Expected %d pixels, got %d.\n", width * height, pixel_count);
    }

    if (pixel_count != width * height) {
        printf("Warning: Pixel count mismatch. Expected %d, got %d.\n", width * height, pixel_count);
    }

    fclose(file);

    char output_filename[256];
    char* input_name = strrchr(argv[1], '/');
    if (input_name == NULL) input_name = argv[1];
    else input_name++;
    snprintf(output_filename, sizeof(output_filename), "%.*s.bmp", (int)(strrchr(input_name, '.') - input_name), input_name);

    stbi_write_bmp(output_filename, width, height, 3, data);

    free(data);
    printf("Image saved as %s\n", output_filename);

    return 0;
}
