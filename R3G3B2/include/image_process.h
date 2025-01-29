/*********************************************************
 *                                                       *
 * MJM + AI 2025                                         *
 * This code is in the public domain.                    *
 * http://creativecommons.org/publicdomain/zero/1.0/     *
 *                                                       *
 *********************************************************/

#ifndef IMAGE_PROCESS_H
#define IMAGE_PROCESS_H

#if defined (__cplusplus)
#define BEGIN_EXTERN_C extern "C" {
#define END_EXTERN_C }
#else
#define BEGIN_EXTERN_C
#define END_EXTERN_C
#endif

BEGIN_EXTERN_C

#include "options.h"
#include "image_typedef.h"

typedef void (*DitherFunc)(ImageData* image);

int process_image(ProgramOptions* opts);
char* trim_filename_copy(const char* filename, char* dest, size_t dest_size);

END_EXTERN_C

#endif
