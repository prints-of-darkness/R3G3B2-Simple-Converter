/*********************************************************
 *                                                       *
 * MJM + AI 2025                                         *
 * This code is in the public domain.                    *
 * http://creativecommons.org/publicdomain/zero/1.0/     *
 *                                                       *
 *********************************************************/

#ifndef FILEIO_H
#define FILEIO_H

#if defined (__cplusplus)
#define BEGIN_EXTERN_C extern "C" {
#define END_EXTERN_C }
#else
#define BEGIN_EXTERN_C
#define END_EXTERN_C
#endif

BEGIN_EXTERN_C

#include <stdio.h>

#include "options.h"
#include "image_typedef.h"

void free_image_memory(ImageData* image);
int load_image(const char* filename, ImageData* image);
int write_image_data_to_file(const char* filename, const char* array_name, const ImageData* image, bool header_output, bool bin_output);

END_EXTERN_C

#endif

