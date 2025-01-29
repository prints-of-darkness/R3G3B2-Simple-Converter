/*********************************************************
 *                                                       *
 * MJM + AI 2025                                         *
 * This code is in the public domain.                    *
 * http://creativecommons.org/publicdomain/zero/1.0/     *
 *                                                       *
 *********************************************************/

#ifndef IMAGE_TYPEDEF_H
#define IMAGE_TYPEDEF_H

#if defined (__cplusplus)
#define BEGIN_EXTERN_C extern "C" {
#define END_EXTERN_C }
#else
#define BEGIN_EXTERN_C
#define END_EXTERN_C
#endif

BEGIN_EXTERN_C

#include <stdint.h>

typedef struct {
    uint8_t* data;
    int width;
    int height;
} ImageData;

typedef struct {
    uint16_t width;
    uint16_t height;
    uint16_t format_id;
} ImageMetadata;

END_EXTERN_C

#endif
