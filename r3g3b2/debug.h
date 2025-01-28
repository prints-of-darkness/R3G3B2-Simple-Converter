
// debug.h

#ifndef DEBUG_H
#define DEBUG_H

#if defined (__cplusplus)
#define BEGIN_EXTERN_C extern "C" {
#define END_EXTERN_C }
#else
#define BEGIN_EXTERN_C
#define END_EXTERN_C
#endif

BEGIN_EXTERN_C

#include "image_typedef.h"
#include "options.h"

int write_debug_image(const char* file_ext, ImageData* image, ProgramOptions* opts);

END_EXTERN_C

#endif