
# R3G3B2 Image Converter

## Overview

`R3G3B2` is a command-line utility designed to convert RGB images into an 8-bit RGB332 format. This specific format is commonly used with TFT graphics controllers, such as the LT7683. In addition to color space conversion, the program offers several dithering options to improve the visual quality of the reduced color palette. Furthermore, it provides adjustments for gamma correction, contrast, and lightness.

## Features

-   **RGB to RGB332 Conversion:** Converts standard 24-bit RGB images to an 8-bit RGB332 format.
-   **Dithering Algorithms:** Includes four dithering methods to mitigate color banding artifacts:
    -   Floyd-Steinberg
    -   Jarvis
    -   Atkinson
    -   Bayer 16x16
-   **Gamma Correction:** Allows for adjusting the gamma value to fine-tune image brightness and contrast.
-   **Contrast and Lightness Control:** Provides independent controls for adjusting image contrast and lightness.
-   **Broad Image Format Support:** Leverages the `stb_image` library for loading various common image formats (e.g., PNG, JPG, BMP).
-   **C Header Output:** Generates a C-compatible header file containing the converted image data as a static array, ideal for embedded systems. This includes a copy of the image_types.h for convenience.
-  **Binary Output:** Can also output the converted image data as a raw binary file with a small header of meta data.
-   **Debug Output (Optional):** The program can generate intermediate and final processed images in BMP format for debugging purposes by using the `-debug` flag.
-   **Command-Line Interface:** The program's behavior is fully controlled through command-line arguments, allowing for flexibility and batch processing.

## Compilation

To compile the code, you will need a C compiler that supports the C99 standard or later. Use the following command:

    gcc r3g3b2.c -o R3G3B2 -lm
    
The `-lm` flag is essential, as it links the math library, which is required for gamma correction. This will create an executable named `R3G3B2`.

## Usage

The program is executed from the command line using the following structure:

    R3G3B2 -i <input file> -o <output file> [-dm <method>] [-g <gamma>] [-c <contrast>] [-l <lightness>] [-h | -b] [-debug <debug_filename>]

### Options

-   `-i <input file>`: Specifies the path to the input image file.
    
-   `-o <output file>`: Specifies the path to the output file.
    
-   `-dm <method>`: Selects the dithering method:
    
    -   `0`: Floyd-Steinberg
        
    -   `1`: Jarvis
        
    -   `2`: Atkinson
        
    -   `3`: Bayer 16x16
        
    -   `-1` or not specified: No dithering (default)
        
-   `-g <gamma>`: Sets the gamma correction value (default: 1.0).
    
-   `-c <contrast>`: Sets the contrast adjustment value (default: 0.0).
    
-   `-l <lightness>`: Sets the lightness adjustment value (default: 1.0).

-  `-h`: Output a C header file. Cannot be used with `-b`.
    
-  `-b`: Output a raw binary file. Cannot be used with `-h`.

-   `-debug <debug_filename>`: Enables debug mode, using `<debug_filename>` as the prefix for debug output BMP files.

- `-help`, `-?`, `--help`: Displays the help message and exits.

### Examples

1.  **Convert input.png to output.h using Floyd-Steinberg dithering:**
    
        ./R3G3B2 -i input.png -h -o output.h -dm 0
    
2.  **Convert image.jpg to image_out.h with a gamma of 2.2, increased contrast, and reduced lightness:**
    
        ./R3G3B2 -i image.jpg -h -o image_out.h -g 2.2 -c 50 -l 0.8
	
3.  **Convert my_pic.bmp to my_pic.bin without any dithering, output as a binary file:**
    
        ./R3G3B2 -i my_pic.bmp -b -o my_pic.bin
          
4.  **Convert my_pic.bmp to my_pic.h with debug mode enabled:**
    
        ./R3G3B2 -i my_pic.bmp -h -o my_pic.h -debug debug_out
    
    This command generates a debug BMP image named `debug_out_processed.bmp` (showing the image after gamma, contrast, and lightness adjustments) and another named `debug_out_final.bmp` (showing the final result after dithering, if selected).

5. **Convert input.png to output.bin with Jarvis dithering, output as a binary file.**

        ./R3G3B2 -i input.png -b -o output.bin -dm 1

## Code Structure

The code is organized for readability and maintainability, featuring the following modules:

-   **Data Structures:** Defines the `ProgramOptions` struct to manage command-line arguments and the `ImageData` struct to store image data.
    
-   **Function Prototypes:** Declares all functions used in the program.
    
-   **Helper Functions:** Includes error handling functions, filename trimming, and color reduction.
    
-   **LUT and Image Processing:** Provides functions for generating and applying look-up tables for gamma correction, contrast, and brightness adjustments. Also contains functions for color quantization.
    
-   **Dithering Algorithms:** Implements the various dithering techniques.
    
-   **File IO:** Includes functions for image loading using `stb_image`, writing the converted image as a C header file or a raw binary file, and memory management.
    
-   **Command Line Processing:** Parses command-line arguments and initializes the `ProgramOptions` struct.
    
-   **Main Function:** The entry point of the program, handles the execution flow.

## Notes

-   The program uses the `stb_image` and `stb_image_write` single-header libraries for image I/O.
    
-   Error handling is done through custom functions like `fileio_error` and `fileio_perror`.
    
-   The generated C header file includes the necessary definitions for integrating the generated image array with embedded systems, including `image_types.h`, which has been included at the top of the generated header for convenience.
    
-   It's important to remember to add `image_types.h` to your embedded project for the output file to compile and run correctly.
    
-   The binary output will include a small binary header with metadata about the image before the raw image data, refer to `fileio.h` for the struct definition.

## Dependencies

-   `stb_image.h` (included)
    
-   `stb_image_write.h` (included)
    
-   Math library (linked with `-lm`)

## Attribution

This code was a collaborative effort by **MJM** and **AI** in 2025.
