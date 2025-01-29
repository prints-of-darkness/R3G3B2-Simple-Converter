/*********************************************************
 *                                                       *
 * MJM + AI 2025                                         *
 * This code is in the public domain.                    *
 * http://creativecommons.org/publicdomain/zero/1.0/     *
 *                                                       *
 *********************************************************/

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

typedef struct {
    uint8_t r;
    uint8_t g;
    uint8_t b;
} RGBColor;

uint8_t reduceBits(uint8_t value, int max_value);
uint8_t mapColorToR3G3B2_Reduced(uint8_t r, uint8_t g, uint8_t b);
void quantize_pixel_with_map_reduced(uint8_t* r, uint8_t* g, uint8_t* b);
uint8_t rgbToRgb332(uint8_t r, uint8_t g, uint8_t b);

END_EXTERN_C

#endif