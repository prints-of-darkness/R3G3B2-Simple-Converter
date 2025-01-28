
#include <stdio.h>
#include <stdlib.h>
#include <stdint.h>
#include <math.h>

#include "luts.h"
#include "constrains.h"
#include "dither.h"
#include "fileio.h"
#include "debug.h"
#include "image_process.h"

int process_image(ProgramOptions* opts)
{
    ImageData image = { 0 };
    uint8_t gamma_lut[LUT_SIZE] = { 0 };
    uint8_t contrast_brightness_lut[LUT_SIZE] = { 0 };
    DitherFunc dither_function = NULL;
    char array_name[MAX_FILENAME_LENGTH] = { 0 };

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

    if (load_image(opts->infilename, &image) != EXIT_SUCCESS) {
        return EXIT_FAILURE;
    }

    initialize_luts(opts->gamma, opts->contrast, opts->brightness, gamma_lut, contrast_brightness_lut);
    process_image_with_luts(&image, gamma_lut, contrast_brightness_lut);

    if (write_debug_image("processed.bmp", &image, opts) != EXIT_SUCCESS) {
        free_image_memory(&image);
        return EXIT_FAILURE;
    }

    switch (opts->dither_method) {
    case 0:  dither_function = floydSteinbergDither; break;
    case 1:  dither_function = jarvisDither;         break;
    case 2:  dither_function = atkinsonDither;       break;
    case 3:  dither_function = bayer16x16Dither;     break;
    default: dither_function = NULL;
    }

    if (dither_function != NULL) {
        dither_function(&image);
    }

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