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
    opts->brightness = 1.0f;
    opts->debug_mode = false;
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
            if (i + 1 < argc) {
                opts->brightness = (float)atof(argv[i + 1]);
                i++;
            }
            else {
                return fileio_error("-b option requires an argument.");
            }
        }
        else if (strcmp(argv[i], "-h") == 0) {
            printf("Usage: r3g3b2 -i <input file> -o <output file> [-dm <method>] [-g <gamma>] [-c <contrast>] [-b <brightness>]\n");
            printf("  -i <input file>           : Specify input file\n");
            printf("  -o <output file>          : Specify output file\n");
            printf("  -dm <method>              : Set dithering method (0: Floyd-Steinberg, 1: Jarvis, 2: Atkinson, 3: Bayer 16x16)\n");
            printf("  -debug <debug_filename>   : Enable debug mode and specify debug file prefix\n");
            printf("  -g <gamma>                : Set gamma value (default: 1.0)\n");
            printf("  -c <contrast>             : Set contrast value (default: 0.0)\n");
            printf("  -b <brightness>           : Set brightness value (default: 1.0)\n");
            printf("  -h                        : Display this help message\n");
            printf("Example: r3g3b2 -i tst.png -o tst.h -dm 0 -g 1.0 -c 0.0 -b 1.0\n");
            return EXIT_FAILURE;
        }
        else {
            fprintf(stderr, "Invalid option: %s\n", argv[i]);
            return EXIT_FAILURE;
        }
    }
    return EXIT_SUCCESS;
}