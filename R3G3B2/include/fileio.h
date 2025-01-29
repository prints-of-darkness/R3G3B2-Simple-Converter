
// fileio.h

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
int write_image_data_to_file(const char* filename, const char* array_name, const ImageData* image);

END_EXTERN_C

#endif

