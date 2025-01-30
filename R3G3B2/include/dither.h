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

int floydSteinbergDither(ImageData* image);
int jarvisDither(ImageData* image);
int atkinsonDither(ImageData* image);
int bayer16x16Dither(ImageData* image);

int noDither(ImageData* image);

END_EXTERN_C

#endif

