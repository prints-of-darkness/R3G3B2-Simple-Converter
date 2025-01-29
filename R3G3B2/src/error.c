#include <stdio.h>
#include <stdlib.h>
#include "error.h"

int fileio_error(const char* message) {
    fprintf(stderr, "Error: %s\n", message);
    return EXIT_FAILURE;
}

int fileio_perror(const char* message) {
    perror(message);
    return EXIT_FAILURE;
}