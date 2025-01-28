
#define STB_IMAGE_IMPLEMENTATION
#include "stb_image.h"

#include "dither.h"
#include "debug.h"
#include "constrains.h"
#include "fileio.h"

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
        if (length >= dest_size) {
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

    if (!filename || !array_name || !image || !image->data) {
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
    if (fp != NULL) {
        fclose(fp);
    }
    return EXIT_FAILURE;
}

int write_c_header(FILE* fp, const char* array_name)
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

int write_image_data(FILE* fp, const char* array_name, const ImageData* image)
{
    if (!fp || !array_name || !image || !image->data) return EXIT_FAILURE;

    if (fprintf(fp, "static const uint8_t %s_data[%d] = {\n", array_name, image->width * image->height) < 0) return EXIT_FAILURE;

    for (int y = 0; y < image->height; y++) {
        for (int x = 0; x < image->width; x++) {
            int idx = (y * image->width + x) * RGB_COMPONENTS;
            uint8_t rgb332 = rgbToRgb332(image->data[idx], image->data[idx + 1], image->data[idx + 2]);
            if (fprintf(fp, "0x%.2X, ", rgb332) < 0) return EXIT_FAILURE;
        }
        if (fprintf(fp, "\n") < 0) return EXIT_FAILURE;
    }

    if (fprintf(fp, "};\n\n") < 0)  return EXIT_FAILURE;
    return EXIT_SUCCESS;
}

int write_image_struct(FILE* fp, const char* array_name, const ImageData* image)
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

int write_c_footer(FILE* fp, const char* array_name)
{
    if (!fp || !array_name) return EXIT_FAILURE;
    if (fprintf(fp, "#endif // %s_H\n", array_name) < 0) return EXIT_FAILURE;
    return EXIT_SUCCESS;
}
