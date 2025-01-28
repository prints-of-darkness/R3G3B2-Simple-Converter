
// options.h

#ifndef OPTIONS_H
#define OPTIONS_H

#if defined (__cplusplus)
#define BEGIN_EXTERN_C extern "C" {
#define END_EXTERN_C }
#else
#define BEGIN_EXTERN_C
#define END_EXTERN_C
#endif

BEGIN_EXTERN_C

#include <stdbool.h>
#include <stdio.h>
#include <string.h>
#include <stdlib.h>

#define MAX_FILENAME_LENGTH 1024

typedef struct {
    char infilename[MAX_FILENAME_LENGTH];
    char outfilename[MAX_FILENAME_LENGTH];
    char debug_filename[MAX_FILENAME_LENGTH];
    int dither_method;
    float gamma;
    float contrast;
    float brightness;
    bool debug_mode;
} ProgramOptions;

void init_program_options(ProgramOptions* opts);
int parse_command_line_args(int argc, char* argv[], ProgramOptions* opts);

END_EXTERN_C

#endif

