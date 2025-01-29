
# r3g3b2 Image Converter

## Overview

`r3g3b2` is a command-line utility designed to convert RGB images into an 8-bit RGB332 format. This specific format is commonly used with TFT graphics controllers, such as the LT7683. In addition to color space conversion, the program offers several dithering options to improve the visual quality of the reduced color palette. Furthermore, it provides adjustments for gamma correction, contrast, and brightness.

## Features

-   **RGB to RGB332 Conversion:** Converts standard 24-bit RGB images to an 8-bit RGB332 format.
-   **Dithering Algorithms:** Includes four dithering methods to mitigate color banding artifacts:
    -   Floyd-Steinberg
    -   Jarvis
    -   Atkinson
    -   Bayer 16x16
-   **Gamma Correction:** Allows for adjusting the gamma value to fine-tune image brightness and contrast.
-   **Contrast and Brightness Control:** Provides independent controls for adjusting image contrast and brightness.
-   **Broad Image Format Support:** Leverages the `stb_image` library for loading various common image formats (e.g., PNG, JPG, BMP).
-   **C Header Output:** Generates a C-compatible header file containing the converted image data as a static array, ideal for embedded systems.
-   **Debug Output (Optional):** The program can generate intermediate and final processed images in BMP format for debugging purposes by using the `-debug` flag.
-   **Command-Line Interface:** The program's behavior is fully controlled through command-line arguments, allowing for flexibility and batch processing.

## Compilation

To compile the code, you will need a C compiler that supports the C99 standard or later. Use the following command:

	gcc r3g3b2.c -o r3g3b2 -lm
    
The -lm flag is essential, as it links the math library, which is required for gamma correction. This will create an executable named r3g3b2.

## Usage

The program is executed from the command line using the following structure:

	r3g3b2 -i <input file> -o <output file> [-dm <method>] [-g <gamma>] [-c <contrast>] [-b <brightness>] [-debug <debug_filename>] [-h]

### Options

-   -i <input file>: Specifies the path to the input image file.
    
-   -o <output file>: Specifies the path to the output C header file.
    
-   -dm <method>: Selects the dithering method:
    
    -   0: Floyd-Steinberg
        
    -   1: Jarvis
        
    -   2: Atkinson
        
    -   3: Bayer 16x16
        
    -   -1: No dithering (default)
        
-   -g <gamma>: Sets the gamma correction value (default: 1.0).
    
-   -c <contrast>: Sets the contrast adjustment value (default: 0.0).
    
-   -b <brightness>: Sets the brightness adjustment value (default: 1.0).
    
-   -debug <debug_filename>: Enables debug mode, using <debug_filename> as the prefix for debug output BMP files.
    
-   -h: Displays the help message and exits.
    

### Examples

1.  **Convert input.png to output.h using Floyd-Steinberg dithering:**
    
		./r3g3b2 -i input.png -o output.h -dm 0
    
-   **Convert image.jpg to image_out.h with a gamma of 2.2 and increased contrast:**
    
		./r3g3b2 -i image.jpg -o image_out.h -g 2.2 -c 50
	
-   **Convert my_pic.bmp to my_pic.h without any dithering:**
    
		./r3g3b2 -i my_pic.bmp -o my_pic.h
          
-   **Convert my_pic.bmp to my_pic.h with debug mode enabled:**
    
		./r3g3b2 -i my_pic.bmp -o my_pic.h -debug debug_out
    
    This command generates a debug BMP image named debug_out_processed.bmp (showing the image after gamma, contrast, and brightness adjustments) and another named debug_out_final.bmp (showing the final result after dithering, if selected).
    
## Code Structure

The code is organized for readability and maintainability, featuring the following modules:

-   **Data Structures:** Defines the ProgramOptions struct to manage command-line arguments and the ImageData struct to store image data.
    
-   **Function Prototypes:** Declares all functions used in the program.
    
-   **Helper Functions:** Includes error handling functions and filename trimming.
    
-   **LUT and Image Processing:** Provides functions for generating and applying look-up tables for gamma correction, contrast, and brightness adjustments. Also contains functions for color quantization.
    
-   **Dithering Algorithms:** Implements the various dithering techniques.
    
-   **File IO:** Includes functions for image loading using stb_image, writing the converted image as a C header file, and memory management.
    
-   **Command Line Processing:** Parses command-line arguments and initializes the ProgramOptions struct.
    
-   **Main Function:** The entry point of the program, handles the execution flow.
    

## Notes

-   The program uses the stb_image and stb_image_write single-header libraries for image I/O.
    
-   Error handling is done through custom functions like fileio_error and fileio_perror.
    
-   The generated C header file includes the necessary definitions for integrating the generated image array with embedded systems, including image_types.h, which has been included at the top of the generated header for convenience.
    
-   It's important to remember to add image_types.h to your embedded project for the output file to compile and run correctly.
    

## Dependencies

-   stb_image.h (included)
    
-   stb_image_write.h (included)
    
-   Math library (linked with -lm)
    

## Attribution

This code was a collaborative effort by **MJM** and **AI** in 2025.
