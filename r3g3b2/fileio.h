
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

char* trim_filename_copy(const char* filename, char* dest, size_t dest_size);

int write_image_data_to_file(const char* filename, const char* array_name, const ImageData* image);

int write_c_header(FILE* fp, const char* array_name);

int write_image_struct(FILE* fp, const char* array_name, const ImageData* image);

int write_image_data(FILE* fp, const char* array_name, const ImageData* image);
int write_c_footer(FILE* fp, const char* array_name);
int process_image(ProgramOptions* opts);


END_EXTERN_C

#endif

