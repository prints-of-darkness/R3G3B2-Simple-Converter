
// dither.h

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

#include <stdint.h>
#include <stdio.h>
#include <stdlib.h>
#include <math.h>

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

