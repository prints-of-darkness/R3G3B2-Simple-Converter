
// constrains.h

#ifndef CONSTRAINS_H
#define CONSTRAINS_H

#if defined (__cplusplus)
#define BEGIN_EXTERN_C extern "C" {
#define END_EXTERN_C }
#else
#define BEGIN_EXTERN_C
#define END_EXTERN_C
#endif

BEGIN_EXTERN_C


#define LUT_SIZE 256
#define RGB_COMPONENTS 3
#define MAX_COLOUR_VALUE 255

#define R3G3B2_COLOR_COUNT 256 

#define RED_LEVELS 16
#define GREEN_LEVELS 16
#define BLUE_LEVELS 16

#define BAYER_SIZE 16


END_EXTERN_C

#endif