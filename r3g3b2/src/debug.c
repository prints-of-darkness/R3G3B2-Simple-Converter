#include "fileio.h"
#include "constrains.h"
#include "debug.h"
#include "error.h"

#define STB_IMAGE_WRITE_IMPLEMENTATION
#include "stb_image_write.h"

int write_debug_image(const char* file_ext, const ImageData* image, const ProgramOptions* opts)
{
    char processed_filename[MAX_FILENAME_LENGTH];
    int snprintf_result;

    if (!file_ext || !image || !opts)
        return fileio_error("Null pointer passed to write_debug_image.");

    if (opts->debug_mode) {
        snprintf_result = snprintf(processed_filename, MAX_FILENAME_LENGTH, "%s_%s", opts->debug_filename, file_ext);
        if (snprintf_result < 0 || snprintf_result >= MAX_FILENAME_LENGTH) {
            return fileio_error("Could not create debug filename.");
        }
        if (!stbi_write_bmp(processed_filename, image->width, image->height, RGB_COMPONENTS, image->data))
            return fileio_perror("Error writing debug image");
    }
    return EXIT_SUCCESS;
}