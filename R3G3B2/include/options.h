/*********************************************************
 *                                                       *
 * MJM + AI 2025                                         *
 * This code is in the public domain.                    *
 * http://creativecommons.org/publicdomain/zero/1.0/     *
 *                                                       *
 *********************************************************/

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

#include "options.h"

#define MAX_FILENAME_LENGTH 1024

// In options.h
typedef struct {
    char infilename[MAX_FILENAME_LENGTH];
    char outfilename[MAX_FILENAME_LENGTH];
    int dither_method;
    float gamma;
    float contrast;
    float lightness;
    bool debug_mode;
    char debug_filename[MAX_FILENAME_LENGTH];
    char palette_filename[MAX_FILENAME_LENGTH];
    bool header_output; // Flag for header output
    bool bin_output;    // Flag for binary output
} ProgramOptions;


void init_program_options(ProgramOptions* opts);
int parse_command_line_args(int argc, char* argv[], ProgramOptions* opts);

END_EXTERN_C

#endif

