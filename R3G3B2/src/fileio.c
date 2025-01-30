/*********************************************************
 *                                                       *
 * MJM + AI 2025                                         *
 * This code is in the public domain.                    *
 * http://creativecommons.org/publicdomain/zero/1.0/     *
 *                                                       *
 *********************************************************/
#include <stdint.h>
#include <stdlib.h>
#include <stdio.h>

#include "dither.h"
#include "debug.h"
#include "constrains.h"
#include "fileio.h"
#include "image_typedef.h"
#include "error.h"

#define STB_IMAGE_IMPLEMENTATION
#include "stb_image.h"

void free_image_memory(ImageData* image)
{
    if (!image) return;

    if (image->data) {
        stbi_image_free(image->data);
    }
    image->data = NULL;
    image->width = 0;
    image->height = 0;
}

int load_image(const char* filename, ImageData* image)
{
    int n;
    if (!filename || !image) {
        return fileio_error("Null pointer passed to load_image.");
    }

    image->data = stbi_load(filename, &image->width, &image->height, &n, RGB_COMPONENTS);

    if (!image->data) {
        fprintf(stderr, "Failed to load image: %s\n", filename);
        return EXIT_FAILURE;
    }
    return EXIT_SUCCESS;
}

static const char* image_types_header =
"#ifndef IMAGE_TYPES_H\n"
"#define IMAGE_TYPES_H\n\n"
"#include <stdint.h>\n\n"
"#define RGB332_FORMAT_ID 0x332\n\n"
"typedef struct {\n"
"    const uint8_t* data;\n"
"    uint16_t width;\n"
"    uint16_t height;\n"
"    uint16_t format_id;\n"
"} Image_t;\n\n"
"#endif // IMAGE_TYPES_H\n";

static int write_c_header(FILE* fp, const char* array_name)
{
    if (!fp || !array_name) {
        return fileio_error("Null pointer passed to write_c_header.");
    }

    if (fprintf(fp, "#ifndef %s_H\n", array_name) < 0)        return fileio_perror("Failed to write to file");
    if (fprintf(fp, "#define %s_H\n\n", array_name) < 0)      return fileio_perror("Failed to write to file");
    if (fprintf(fp, "#include \"image_types.h\"\n\n") < 0)    return fileio_perror("Failed to write to file");
    if (fprintf(fp, "/*\n%s*/\n\n", image_types_header) < 0)  return fileio_perror("Failed to write to file");

    return EXIT_SUCCESS;
}

static int write_image_data(FILE* fp, const char* array_name, const ImageData* image)
{
    if (!fp || !array_name || !image || !image->data) {
        return fileio_error("Null pointer passed to write_image_data.");
    }

    if (fprintf(fp, "static const uint8_t %s_data[%zu] = {\n", array_name, (size_t)image->width * image->height) < 0) return fileio_perror("Failed to write to file");

    for (size_t y = 0; y < image->height; y++) {
        for (size_t x = 0; x < image->width; x++) {
            size_t idx = (y * image->width + x) * RGB_COMPONENTS;
            uint8_t rgb332 = rgbToRgb332(image->data[idx], image->data[idx + 1], image->data[idx + 2]);
            if (fprintf(fp, "0x%.2X, ", rgb332) < 0) return fileio_perror("Failed to write to file");
        }
        if (fprintf(fp, "\n") < 0) return fileio_perror("Failed to write to file");
    }

    if (fprintf(fp, "};\n\n") < 0) return fileio_perror("Failed to write to file");
    return EXIT_SUCCESS;
}

static int write_image_struct(FILE* fp, const char* array_name, const ImageData* image)
{
    if (!fp || !array_name || !image) {
        return fileio_error("Null pointer passed to write_image_struct.");
    }

    if (fprintf(fp, "static const Image_t %s_image = {\n", array_name) < 0) return fileio_perror("Failed to write to file");
    if (fprintf(fp, "    .data = %s_data,\n", array_name) < 0)              return fileio_perror("Failed to write to file");
    if (fprintf(fp, "    .width = %d,\n", image->width) < 0)                return fileio_perror("Failed to write to file");
    if (fprintf(fp, "    .height = %d,\n", image->height) < 0)              return fileio_perror("Failed to write to file");
    if (fprintf(fp, "    .format_id = RGB332_FORMAT_ID\n") < 0)             return fileio_perror("Failed to write to file");
    if (fprintf(fp, "};\n\n") < 0)                                          return fileio_perror("Failed to write to file");
    return EXIT_SUCCESS;
}

static int write_c_footer(FILE* fp, const char* array_name)
{
    if (!fp || !array_name) {
        return fileio_error("Null pointer passed to write_c_footer.");
    }
    if (fprintf(fp, "#endif // %s_H\n", array_name) < 0) return fileio_perror("Failed to write to file");
    return EXIT_SUCCESS;
}


static int write_binary_data(FILE* fp, const ImageData* image)
{
    if (!fp || !image || !image->data) {
        return fileio_error("Null pointer passed to write_binary_data.");
    }
    // Write binary metadata header
    ImageMetadata metadata;
    metadata.width = image->width;
    metadata.height = image->height;
    metadata.format_id = RGB332_FORMAT_ID; // Use the defined format ID

    if (fwrite(&metadata, sizeof(ImageMetadata), 1, fp) != 1) {
        return fileio_perror("Failed to write binary metadata");
    };

    // Write raw binary data
    for (size_t y = 0; y < image->height; y++) {
        for (size_t x = 0; x < image->width; x++) {
            size_t idx = (y * image->width + x) * RGB_COMPONENTS;
            uint8_t rgb332 = rgbToRgb332(image->data[idx], image->data[idx + 1], image->data[idx + 2]);
            if (fwrite(&rgb332, 1, 1, fp) != 1) { // Write 1 byte at a time
                return fileio_perror("Failed to write to file");
            }
        }
    }
    return EXIT_SUCCESS;
}

int write_image_data_to_file(const char* filename, const char* array_name, const ImageData* image, bool header_output, bool bin_output)
{
    FILE* fp = NULL;
    int result = EXIT_FAILURE;

    if (!filename || !array_name || !image || !image->data) {
        return fileio_error("Null pointer passed to write_image_data_to_file.");
    }

    if (bin_output) {
        fp = fopen(filename, "wb"); // Use "wb" for binary mode
        if (!fp) {
            return fileio_perror("Failed to open output file");
        }

        if (write_binary_data(fp, image) != EXIT_SUCCESS) goto cleanup;

    }
    else if (header_output) {
        fp = fopen(filename, "w");
        if (!fp) {
            return fileio_perror("Failed to open output file");
        }
        // Write C header file
        if (write_c_header(fp, array_name) != EXIT_SUCCESS)            goto cleanup;
        if (write_image_data(fp, array_name, image) != EXIT_SUCCESS)   goto cleanup;
        if (write_image_struct(fp, array_name, image) != EXIT_SUCCESS) goto cleanup;
        if (write_c_footer(fp, array_name) != EXIT_SUCCESS)            goto cleanup;
    }
    else {
        return fileio_error("Must select -b or -h output option");
    }

    result = EXIT_SUCCESS;

cleanup:
    if (fp)
        fclose(fp);

    return result;
}