// luts.h

#ifndef LUTS_H
#define LUTS_H

#if defined (__cplusplus)
#define BEGIN_EXTERN_C extern "C" {
#define END_EXTERN_C }
#else
#define BEGIN_EXTERN_C
#define END_EXTERN_C
#endif

BEGIN_EXTERN_C

#include "image_typedef.h"

void initialize_luts(float gamma, float contrast, float brightness, uint8_t* gamma_lut, uint8_t* contrast_brightness_lut);
void process_image_with_luts(ImageData* image, const uint8_t* gamma_lut, const uint8_t* contrast_brightness_lut);

END_EXTERN_C

#endif

