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
#include <math.h>

#include "constrains.h"
#include "luts.h"
#include "error.h"

int initialize_luts(float gamma, float contrast, float brightness, uint8_t* gamma_lut, uint8_t* contrast_brightness_lut)
{
    if (!gamma_lut || !contrast_brightness_lut) {
        fileio_error("Null pointer passed to LUT initialization.");
        return EXIT_FAILURE;
    }

    float factor = (259.0f * (contrast + 255.0f)) / (255.0f * (259.0f - contrast));

    for (int i = 0; i < LUT_SIZE; i++) {
        float value = (float)i / (float)MAX_COLOUR_VALUE;                                                           // Normalize the input to the 0-1 range
        value = powf(value, 1.0f / gamma);                                                                          // Apply gamma correction
        value = factor * (value * brightness - 0.5f) + 0.5f;                                                        // Apply contrast and brightness adjustments
        value = fmaxf(0.0f, fminf(value, 1.0f));                                                                    // Clamp the value to ensure it stays within 0-1
        contrast_brightness_lut[i] = (uint8_t)(value * (float)MAX_COLOUR_VALUE);                                    // Scale the result back to 0-255 and store in the contrast/brightness LUT
        gamma_lut[i] = (uint8_t)(powf((float)i / (float)MAX_COLOUR_VALUE, 1.0f / gamma) * (float)MAX_COLOUR_VALUE); // Apply gamma correction and store in the gamma LUT
    }
    return EXIT_SUCCESS;
}

int process_image_with_luts(ImageData* image, const uint8_t* gamma_lut, const uint8_t* contrast_brightness_lut)
{
    if (!image || !image->data || !gamma_lut || !contrast_brightness_lut) {
        fileio_error("Null pointer passed to process_image_with_luts.");
        return EXIT_FAILURE;
    }

    for (int y = 0; y < image->height; y++) {
        for (int x = 0; x < image->width; x++) {
            int idx = (y * image->width + x) * RGB_COMPONENTS;
            image->data[idx] = contrast_brightness_lut[gamma_lut[image->data[idx]]];     // Red channel
            image->data[idx + 1] = contrast_brightness_lut[gamma_lut[image->data[idx + 1]]]; // Green channel
            image->data[idx + 2] = contrast_brightness_lut[gamma_lut[image->data[idx + 2]]]; // Blue channel
        }
    }
    return EXIT_SUCCESS;
}