/*********************************************************
 *                                                       *
 * MJM + AI 2025                                         *
 * This code is in the public domain.                    *
 * http://creativecommons.org/publicdomain/zero/1.0/     *
 *                                                       *
 *********************************************************/

#ifndef DITHER_H
#define DITHER_H

#if defined (__cplusplus)
#define BEGIN_EXTERN_C extern "C" {
#define END_EXTERN_C }
#else
#define BEGIN_EXTERN_C
#define END_EXTERN_C
#endif

BEGIN_EXTERN_C

#include "image_typedef.h"
#include "color.h"

typedef struct {
    int x_offset;
    int y_offset;
    float weight;
} ErrorDiffusionEntry;

void floydSteinbergDither(ImageData* image);
void jarvisDither(ImageData* image);
void atkinsonDither(ImageData* image);
void genericDither(ImageData* image, const ErrorDiffusionEntry* matrix, int matrix_size);
void bayer16x16Dither(ImageData* image);

END_EXTERN_C

#endif

