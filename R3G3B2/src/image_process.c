/*********************************************************
 *                                                       *
 * MJM + AI 2025                                         *
 * This code is in the public domain.                    *
 * http://creativecommons.org/publicdomain/zero/1.0/     *
 *                                                       *
 *********************************************************/
#include <stdio.h>
#include <stdlib.h>
#include <stdint.h>
#include <string.h>
#include <math.h>

#include "luts.h"
#include "constrains.h"
#include "dither.h"
#include "fileio.h"
#include "debug.h"
#include "image_process.h"
#include "error.h"

static char* trim_filename_copy(const char* filename, char* dest, size_t dest_size)
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

int process_image(ProgramOptions* opts)
{
    if (!opts) {
        return fileio_error("Null pointer passed to process_image.");
    }

    if (opts->infilename[0] == '\0') {
        return fileio_error("No input file specified.");
    }

    if (opts->outfilename[0] == '\0') {
        return fileio_error("No output file specified.");
    }

    ImageData image = { 0 };
    if (load_image(opts->infilename, &image) != EXIT_SUCCESS) {
        return EXIT_FAILURE;
    }

    uint8_t gamma_lut[LUT_SIZE];
    uint8_t contrast_brightness_lut[LUT_SIZE];
    initialize_luts(opts->gamma, opts->contrast, opts->brightness, gamma_lut, contrast_brightness_lut);
    process_image_with_luts(&image, gamma_lut, contrast_brightness_lut);

    if (write_debug_image("processed.bmp", &image, opts) != EXIT_SUCCESS) {
        free_image_memory(&image);
        return EXIT_FAILURE;
    }

    DitherFunc dither_function = NULL;
    switch (opts->dither_method) {
    case 0:  dither_function = floydSteinbergDither; break;
    case 1:  dither_function = jarvisDither;         break;
    case 2:  dither_function = atkinsonDither;       break;
    case 3:  dither_function = bayer16x16Dither;     break;
    default:
        // Leave dither_function as NULL if an invalid method is selected
        break;
    }

    if (dither_function) {
        dither_function(&image);
    }

    char array_name[MAX_FILENAME_LENGTH];
    trim_filename_copy(opts->outfilename, array_name, MAX_FILENAME_LENGTH);
    if (write_image_data_to_file(opts->outfilename, array_name, &image) != EXIT_SUCCESS) {
        free_image_memory(&image);
        return EXIT_FAILURE;
    }

    if (write_debug_image("final.bmp", &image, opts) != EXIT_SUCCESS) {
        free_image_memory(&image);
        return EXIT_FAILURE;
    }

    free_image_memory(&image);
    return EXIT_SUCCESS;
}
