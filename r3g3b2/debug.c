
#include "constrains.h"
#include "debug.h"

#define STB_IMAGE_WRITE_IMPLEMENTATION
#include "stb_image_write.h"

int write_debug_image(const char* file_ext, ImageData* image, ProgramOptions* opts)
{
    char processed_filename[MAX_FILENAME_LENGTH];
    int snprintf_result;

    if (opts->debug_mode) {
        snprintf_result = snprintf(processed_filename, MAX_FILENAME_LENGTH, "%s_%s", opts->debug_filename, file_ext);
        if (snprintf_result < 0 || snprintf_result >= MAX_FILENAME_LENGTH) {
            fprintf(stderr, "Error: Could not create debug filename.\n");
            return EXIT_FAILURE;
        }
        stbi_write_bmp(processed_filename, image->width, image->height, RGB_COMPONENTS, image->data);
    }
    return EXIT_SUCCESS;
}
