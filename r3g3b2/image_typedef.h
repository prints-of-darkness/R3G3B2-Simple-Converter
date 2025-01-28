
// image_typedef.h

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
    uint8_t r;
    uint8_t g;
    uint8_t b;
} RGBColor;

typedef struct {
    uint8_t* data;
    int width;
    int height;
} ImageData;

END_EXTERN_C

#endif
