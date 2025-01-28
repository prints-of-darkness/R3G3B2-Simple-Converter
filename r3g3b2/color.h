
// color.h

#ifndef COLOR_H
#define COLOR_H

#if defined (__cplusplus)
#define BEGIN_EXTERN_C extern "C" {
#define END_EXTERN_C }
#else
#define BEGIN_EXTERN_C
#define END_EXTERN_C
#endif

BEGIN_EXTERN_C

#include "image_typedef.h"

uint8_t reduceBits(uint8_t value, int max_value);

float colorDistance(const RGBColor* color1, const RGBColor* color2);

uint8_t mapColorToR3G3B2_Reduced(uint8_t r, uint8_t g, uint8_t b);

void quantize_pixel_with_map_reduced(uint8_t* r, uint8_t* g, uint8_t* b);

uint8_t rgbToRgb332(uint8_t r, uint8_t g, uint8_t b);

END_EXTERN_C

#endif