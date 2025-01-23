
# RGB332 Image Converter

Convert RGB images to 8-bit RGB332 format for LT7683 TFT graphics controller compatibility.

## Overview

This C program converts standard RGB images to the 8-bit RGB332 color format, commonly used in certain TFT graphics controllers like the LT7683. It offers features such as gamma correction, contrast adjustment, and optional Floyd-Steinberg dithering to enhance image quality.

## Features

- Converts 24-bit RGB images to 8-bit RGB332 format
- Gamma correction for improved color accuracy
- Contrast adjustment capabilities
- Optional Floyd-Steinberg dithering for reduced color banding
- Debug mode for visual verification
- Generates C header files for easy integration into embedded projects

## Installation

### Prerequisites

- C compiler (e.g., GCC)
- stb_image and stb_image_write libraries (included)

### Building

Compile the program: 
Use the visual studio project

## Usage

`./rgb332 -i <input_file> -o <output_file> [options]`

### Options

- `-i <input_file>`: Specify input image file (required)
- `-o <output_file>`: Specify output header file (required)
- `-d`: Enable dithering
- `-debug`: Enable debug mode (outputs a BMP file for visual verification)
- `-g <value>`: Set gamma value (default: 1.0)
- `-c <value>`: Set contrast value (default: 0.0)
- `-h`: Display help message

### Example

`./rgb332 -i input.png -o output.h -d -g 1.0 -c 0.0`

## Output

The program generates a C header file containing:
- An array of RGB332 pixel data
- An `Image_t` struct with image metadata

## Implementation Details

### Color Conversion

The `r8g8b8_to_r3g3b2` function handles the conversion from 24-bit RGB to 8-bit RGB332, including gamma correction and contrast adjustment.

### Dithering

Floyd-Steinberg dithering is implemented in the `floydSteinbergDither` function to reduce color banding in the output image.

### Lookup Table

A pre-computed lookup table (`r3g3b2_to_r8g8b8_lut`) is used for efficient conversion from RGB332 back to RGB888 in debug mode.

## License

Please refer to the license file for more information.

## Author

MJM 2025

(This work was assisted by Perplexity AI, an AI language model)
