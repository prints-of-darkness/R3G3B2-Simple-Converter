# r3g3b2 Image Converter

## Overview

`r3g3b2.c` is a command-line utility designed to convert RGB images into 8-bit RGB332 format. This format is specifically needed for certain TFT graphics controllers, such as the LT7683. The program also includes several dithering options to improve the visual quality of the reduced color space. Additionally, it offers gamma correction, contrast, and brightness adjustments.

## Features

-   **RGB to RGB332 Conversion:** Converts standard 24-bit RGB images into 8-bit RGB332 format.
-   **Dithering:** Supports four dithering methods to minimize color banding:
    -   Floyd-Steinberg
    -   Jarvis
    -   Atkinson
    -   Bayer 16x16
-   **Gamma Correction:** Allows for gamma adjustment.
-   **Contrast and Brightness Control:** Enables adjustment of contrast and brightness levels.
-   **Image Loading:** Uses the `stb_image` library to load various image formats (e.g., PNG, JPG, BMP).
-   **Output to C Header File:** Writes the converted image data as a C array within a header file.
-   **Optional Debug Output:**  (when compiled with `-DDEBUG_BUILD`) Can write intermediate and final processed images to BMP for visual debugging.
-   **Command-Line Interface:** Provides options to set the input, output file, dithering method, gamma, contrast, and brightness values.

## Compilation

To compile the code, use a C compiler that supports C99 or later:

		gcc r3g3b2.c -o r3g3b2 -lm
    
To enable debug mode, add the DEBUG_BUILD define

		gcc r3g3b2.c -o r3g3b2 -lm -DDEBUG_BUILD
    
This will generate an executable named r3g3b2. The -lm flag links the math library, which is required for the gamma correction.

## Usage

		r3g3b2 -i <input file> -o <output file> [-dm <method>] [-g <gamma>] [-c <contrast>] [-b <brightness>] [-h]
    

### Options

-   -i <input file>: Specifies the input image file path.
    
-   -o <output file>: Specifies the output header file path.
    
-   -dm <method>: Sets the dithering method:
    
    -   0: Floyd-Steinberg
        
    -   1: Jarvis
        
    -   2: Atkinson
        
    -   3: Bayer 16x16
        
    -   -1: No Dithering (default)
        
-   -g <gamma>: Sets the gamma correction value (default: 1.0).
    
-   -c <contrast>: Sets the contrast adjustment value (default: 0.0).
    
-   -b <brightness>: Sets the brightness adjustment value (default: 1.0).
    
-   -h: Displays the help message.
    
-   -debug <debug_filename>: (when compiled with -DDEBUG_BUILD) Enables debug mode and specifies debug file prefix.
    

### Examples

1.  **Convert input.png to output.h using Floyd-Steinberg dithering:**
    
		./r3g3b2 -i input.png -o output.h -dm 0

    
-   **Convert image.jpg to image_out.h with a gamma of 2.2 and increased contrast:**
    
		./r3g3b2 -i image.jpg -o image_out.h -g 2.2 -c 50
          
2.  **Convert my_pic.bmp to my_pic.h with no dithering:**
    
		./r3g3b2 -i my_pic.bmp -o my_pic.h

1.  **Convert my_pic.bmp to my_pic.h with debug mode enabled:**
    

		./r3g3b2 -i my_pic.bmp -o my_pic.h -debug debug_out

This example will also output a bmp image, named debug_out_processed.bmp, which shows what the image looks like after the gamma, contrast, and brightness LUT has been applied, and a bmp image, named debug_out_final.bmp, which shows what the image looks like after dithering has been applied (if selected)

## Code Structure

The code is organized into the following sections:

-   **Data Structures:** Defines ProgramOptions and ImageData structs.
    
-   **Function Prototypes:** Declares the functions used in the program.
    
-   **Helper Functions:** Provides utility functions like error handling.
    
-   **LUT and Image Processing Functions:** Implements look-up tables, image processing, and color conversions.
    
-   **Dithering Algorithm Functions:** Implements various dithering techniques.
    
-   **File IO Functions:** Implements image loading, C header output, and memory management.
    
-   **Command Line Processing:** Parses arguments and initializes the program.
    
-   **Main Function:** Entry point for the program.
    

## Notes

-   The program utilizes the stb_image and stb_image_write (for debug) libraries for image I/O.
    
-   Memory allocation errors are handled with a custom mem_alloc_failed function.
    
-   The output C header file includes the definitions needed to use the generated image array on an embedded device.
    
-   Remember to add the image_types.h to the project that the output header file is being imported to, this has been generated at the top of the header file for convenience.

## Dependencies

-   stb_image.h (included) - single header library for image loading.
    
-   stb_image_write.h (included) - single header library for debug image writing.
    
-   Math library (linked with -lm).
    

## Attribution

This code was created as a joint effort by **MJM** and **AI** in 2025.
