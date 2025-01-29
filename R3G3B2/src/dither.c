/*********************************************************
 *                                                       *
 * MJM + AI 2025                                         *
 * This code is in the public domain.                    *
 * http://creativecommons.org/publicdomain/zero/1.0/     *
 *                                                       *
 *********************************************************/
#include <stdint.h>
#include <stdio.h>
#include <stdlib.h>
#include <math.h>

#include "constrains.h"
#include "dither.h"
#include "error.h"

static const uint8_t BAYER_MATRIX_16X16[BAYER_SIZE][BAYER_SIZE] = {
    {0,  128, 32, 160,  8, 136, 40, 168,  2, 130, 34, 162, 10, 138, 42, 170},
    {192, 64, 224, 96, 200, 72, 232, 104, 194, 66, 226, 98, 202, 74, 234, 106},
    {48, 176, 16, 144, 56, 184, 24, 152, 50, 178, 18, 146, 58, 186, 26, 154},
    {240, 112, 208, 80, 248, 120, 216, 88, 242, 114, 210, 82, 250, 122, 218, 90},
    {12, 140, 44, 172,  4, 132, 36, 164, 14, 142, 46, 174,  6, 134, 38, 166},
    {204, 76, 236, 108, 196, 68, 228, 100, 206, 78, 238, 110, 198, 70, 230, 102},
    {60, 188, 28, 156, 52, 180, 20, 148, 62, 190, 30, 158, 54, 182, 22, 150},
    {252, 124, 220, 92, 244, 116, 212, 84, 254, 126, 222, 94, 246, 118, 214, 86},
    {3,  131, 35, 163, 11, 139, 43, 171,  1, 129, 33, 161,  9, 137, 41, 169},
    {195, 67, 227, 99, 203, 75, 235, 107, 193, 65, 225, 97, 201, 73, 233, 105},
    {51, 179, 19, 147, 59, 187, 27, 155, 49, 177, 17, 145, 57, 185, 25, 153},
    {243, 115, 211, 83, 251, 123, 219, 91, 241, 113, 209, 81, 249, 121, 217, 89},
    {15, 143, 47, 175,  7, 135, 39, 167, 13, 141, 45, 173,  5, 133, 37, 165},
    {207, 79, 239, 111, 199, 71, 231, 103, 205, 77, 237, 109, 197, 69, 229, 101},
    {63, 191, 31, 159, 55, 183, 23, 151, 61, 189, 29, 157, 53, 181, 21, 149},
    {255, 127, 223, 95, 247, 119, 215, 87, 253, 125, 221, 93, 245, 117, 213, 85}
};

static void genericDither(ImageData* image, const ErrorDiffusionEntry* matrix, int matrix_size)
{
    if (!image || !image->data) {
        fileio_error("Null pointer passed to genericDither.");
        return;
    }

    const int width = image->width;
    const int height = image->height;

    for (int y = 0; y < height; y++) {
        for (int x = 0; x < width; x++) {
            int idx = (y * width + x) * RGB_COMPONENTS;
            uint8_t oldR = image->data[idx];
            uint8_t oldG = image->data[idx + 1];
            uint8_t oldB = image->data[idx + 2];

            uint8_t newR = oldR;
            uint8_t newG = oldG;
            uint8_t newB = oldB;

            quantize_pixel_with_map_reduced(&newR, &newG, &newB);

            image->data[idx] = newR;
            image->data[idx + 1] = newG;
            image->data[idx + 2] = newB;

            float errorR = (float)(oldR - newR);
            float errorG = (float)(oldG - newG);
            float errorB = (float)(oldB - newB);

            for (int i = 0; i < matrix_size; i++) {
                int nx = x + matrix[i].x_offset;
                int ny = y + matrix[i].y_offset;

                if (nx >= 0 && nx < width && ny >= 0 && ny < height) {
                    int adj_idx = (ny * width + nx) * RGB_COMPONENTS;
                    image->data[adj_idx] = (uint8_t)fmin(MAX_COLOUR_VALUE, fmax(0, image->data[adj_idx] + errorR * matrix[i].weight));
                    image->data[adj_idx + 1] = (uint8_t)fmin(MAX_COLOUR_VALUE, fmax(0, image->data[adj_idx + 1] + errorG * matrix[i].weight));
                    image->data[adj_idx + 2] = (uint8_t)fmin(MAX_COLOUR_VALUE, fmax(0, image->data[adj_idx + 2] + errorB * matrix[i].weight));
                }
            }
        }
    }
}

void floydSteinbergDither(ImageData* image)
{
    const ErrorDiffusionEntry matrix[] = {
        { 1, 0, 7.0f / 16.0f },
        {-1, 1, 3.0f / 16.0f },
        { 0, 1, 5.0f / 16.0f },
        { 1, 1, 1.0f / 16.0f }
    };
    genericDither(image, matrix, sizeof(matrix) / sizeof(matrix[0]));
}

void jarvisDither(ImageData* image)
{
    const ErrorDiffusionEntry matrix[] = {
    { 1, 0, 7.0f / 48.0f }, { 2, 0, 5.0f / 48.0f },
    {-2, 1, 3.0f / 48.0f }, {-1, 1, 5.0f / 48.0f }, { 0, 1, 7.0f / 48.0f }, { 1, 1, 5.0f / 48.0f }, { 2, 1, 3.0f / 48.0f },
    {-2, 2, 1.0f / 48.0f }, {-1, 2, 3.0f / 48.0f }, { 0, 2, 5.0f / 48.0f }, { 1, 2, 3.0f / 48.0f }, { 2, 2, 1.0f / 48.0f }
    };
    genericDither(image, matrix, sizeof(matrix) / sizeof(matrix[0]));
}

void atkinsonDither(ImageData* image)
{
    const ErrorDiffusionEntry matrix[] = {
    { 1, 0, 1.0f / 8.0f }, { 2, 0, 1.0f / 8.0f },
    {-1, 1, 1.0f / 8.0f }, { 0, 1, 1.0f / 8.0f }, { 1, 1, 1.0f / 8.0f },
    { 0, 2, 1.0f / 8.0f }
    };
    genericDither(image, matrix, sizeof(matrix) / sizeof(matrix[0]));
}

void bayer16x16Dither(ImageData* image)
{
    if (!image || !image->data) {
        fileio_error("Null pointer passed to bayer16x16Dither.");
        return;
    }

    const int width = image->width;
    const int height = image->height;

    for (int y = 0; y < height; y++) {
        for (int x = 0; x < width; x++) {
            int idx = (y * width + x) * RGB_COMPONENTS;

            int bayer_threshold = BAYER_MATRIX_16X16[y % BAYER_SIZE][x % BAYER_SIZE];
            float normalized_bayer = (float)(bayer_threshold - 128);

            int r = image->data[idx];
            int g = image->data[idx + 1];
            int b = image->data[idx + 2];

            r = (int)round((float)r + (normalized_bayer / 8.0f));
            g = (int)round((float)g + (normalized_bayer / 8.0f));
            b = (int)round((float)b + (normalized_bayer / 8.0f));

            r = (r > MAX_COLOUR_VALUE) ? MAX_COLOUR_VALUE : (r < 0) ? 0 : r;
            g = (g > MAX_COLOUR_VALUE) ? MAX_COLOUR_VALUE : (g < 0) ? 0 : g;
            b = (b > MAX_COLOUR_VALUE) ? MAX_COLOUR_VALUE : (b < 0) ? 0 : b;

            quantize_pixel_with_map_reduced((uint8_t*)&r, (uint8_t*)&g, (uint8_t*)&b);

            image->data[idx] = r;
            image->data[idx + 1] = g;
            image->data[idx + 2] = b;
        }
    }
}

void noDither(ImageData* image)
{
    if (!image || !image->data) {
        fileio_error("Null pointer passed to noDither.");
        return;
    }

    const int width = image->width;
    const int height = image->height;

    for (int y = 0; y < height; y++) {
        for (int x = 0; x < width; x++) {
            int idx = (y * width + x) * RGB_COMPONENTS;

            int r = image->data[idx];
            int g = image->data[idx + 1];
            int b = image->data[idx + 2];

            quantize_pixel_with_map_reduced((uint8_t*)&r, (uint8_t*)&g, (uint8_t*)&b);

            image->data[idx] = r;
            image->data[idx + 1] = g;
            image->data[idx + 2] = b;
        }
    }
}
