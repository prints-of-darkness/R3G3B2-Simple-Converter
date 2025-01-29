// error.h

#ifndef ERROR_H
#define ERROR_H

#if defined(__cplusplus)
#define BEGIN_EXTERN_C extern "C" {
#define END_EXTERN_C }
#else
#define BEGIN_EXTERN_C
#define END_EXTERN_C
#endif

BEGIN_EXTERN_C

#include <stdio.h>
#include <stdlib.h>

// Function to report a general error and return EXIT_FAILURE
int fileio_error(const char* message);

// Function to report a system error using perror and return EXIT_FAILURE
int fileio_perror(const char* message);

END_EXTERN_C

#endif