/*********************************************************
 *                                                       *
 * MJM + AI 2025                                         *
 * This code is in the public domain.                    *
 * http://creativecommons.org/publicdomain/zero/1.0/     *
 *                                                       *
 *********************************************************/
#include <stdio.h>
#include <string.h>
#include <stdlib.h>

#include "options.h"
#include "error.h"

void init_program_options(ProgramOptions* opts)
{
    if (!opts) return;
    memset(opts, 0, sizeof(ProgramOptions));
    opts->dither_method = -1;
    opts->gamma = 1.0f;
    opts->contrast = 0.0f;
    opts->lightness = 1.0f;
    opts->debug_mode = false;
    opts->header_output = false;
    opts->debug_filename[0] = '\0';
}

int parse_command_line_args(int argc, char* argv[], ProgramOptions* opts)
{
    if (!opts) {
        return fileio_error("Null pointer passed to parse_command_line_args.");
    }

    for (int i = 1; i < argc; ++i) {
        if (strcmp(argv[i], "-i") == 0) {
            if (i + 1 < argc) {
                strncpy(opts->infilename, argv[i + 1], MAX_FILENAME_LENGTH - 1);
                opts->infilename[MAX_FILENAME_LENGTH - 1] = '\0';
                i++;
            }
            else {
                return fileio_error("-i option requires an argument.");
            }
        }
        else if (strcmp(argv[i], "-o") == 0) {
            if (i + 1 < argc) {
                strncpy(opts->outfilename, argv[i + 1], MAX_FILENAME_LENGTH - 1);
                opts->outfilename[MAX_FILENAME_LENGTH - 1] = '\0';
                i++;
            }
            else {
                return fileio_error("-o option requires an argument.");
            }
        }
        else if (strcmp(argv[i], "-dm") == 0) {
            if (i + 1 < argc) {
                opts->dither_method = atoi(argv[i + 1]);
                i++;
            }
            else {
                return fileio_error("-dm option requires an argument.");
            }
        }
        else if (strcmp(argv[i], "-debug") == 0) {
            if (i + 1 < argc) {
                opts->debug_mode = true;
                strncpy(opts->debug_filename, argv[i + 1], MAX_FILENAME_LENGTH - 1);
                opts->debug_filename[MAX_FILENAME_LENGTH - 1] = '\0';
                i++;
            }
            else {
                return fileio_error("-debug option requires an argument.");
            }
        }
        else if (strcmp(argv[i], "-g") == 0) {
            if (i + 1 < argc) {
                opts->gamma = (float)atof(argv[i + 1]);
                i++;
            }
            else {
                return fileio_error("-g option requires an argument.");
            }
        }
        else if (strcmp(argv[i], "-c") == 0) {
            if (i + 1 < argc) {
                opts->contrast = (float)atof(argv[i + 1]);
                i++;
            }
            else {
                return fileio_error("-c option requires an argument.");
            }
        }
        else if (strcmp(argv[i], "-b") == 0) {
            if (opts->header_output) {
                return fileio_error("Cannot set both -b and -h");
            }
            opts->bin_output = true;
        }
        else if (strcmp(argv[i], "-h") == 0) {
            if (opts->bin_output) {
                return fileio_error("Cannot set both -b and -h");
            }
            opts->header_output = true;
        }
        else if (strcmp(argv[i], "-l") == 0) {
            if (i + 1 < argc) {
                opts->lightness = (float)atof(argv[i + 1]);
                i++;
            }
            else {
                return fileio_error("-l option requires an argument.");
            }
        }
        else if (strcmp(argv[i], "-help") == 0 || strcmp(argv[i], "-?") == 0 || strcmp(argv[i], "--help") == 0) {
            printf("Usage: R3G3B2 -i <input file> -o <output file> [-dm <method>] [-g <gamma>] [-c <contrast>] [-l <lightness>] [-h] [-b]\n");
            printf("  -i <input file>           : Specify input file\n");
            printf("  -o <output file>          : Specify output file\n");
            printf("  -dm <method>              : Set dithering method (0: Floyd-Steinberg, 1: Jarvis, 2: Atkinson, 3: Bayer 16x16)\n");
            printf("  -debug <debug_filename>   : Enable debug mode and specify debug file prefix\n");
            printf("  -g <gamma>                : Set gamma value (default: 1.0)\n");
            printf("  -c <contrast>             : Set contrast value (default: 0.0)\n");
            printf("  -l <lightness>            : Set lightness value (default: 1.0)\n");
            printf("  -h                        : Output a C header file\n");
            printf("  -b                        : Output a raw binary file\n");
            printf("  -help, -?, --help         : Display this help message\n");
            printf("Example: R3G3B2 -i tst.png -h -o tst.h -dm 0 -g 1.0 -c 0.0 -l 1.0\n");
            printf("Example: R3G3B2 -i tst.png -b -o tst.bin -dm 0 -g 1.0 -c 0.0 -l 1.0\n");
            return EXIT_FAILURE;
        }
        else {
            fprintf(stderr, "Invalid option: %s\n", argv[i]);
            return EXIT_FAILURE;
        }
    }
    return EXIT_SUCCESS;
}