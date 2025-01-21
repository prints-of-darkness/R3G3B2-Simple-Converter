
/*

    rgb332.cpp

	Converts a RGB image to 8-bit RGB332.

    MJM 2025

*/

#include <cstdio>
#include <cstdint>
#include <cstdlib>
#include <cstring>
#include <iostream>
#include <string>

#define STB_IMAGE_IMPLEMENTATION
#include "stb_image.h"

unsigned char rgbTo332(unsigned char red, unsigned char green, unsigned char blue);

int main(int argc, char* argv[])
{
    int i;
    FILE* fp;
    std::string infilename;
    std::string outfilename;

    for (i = 1; i < argc; i++) {
        if (strcmp(argv[i], "-f") == 0 && i + 1 < argc) {
            infilename = argv[i + 1];
            printf("Input file: %s\n", infilename.c_str());
            i++;
        }
        else if (strcmp(argv[i], "-o") == 0 && i + 1 < argc) {
			outfilename = argv[i + 1];
            printf("Output file: %s\n", outfilename.c_str());
            i++;
        }
        else if (strcmp(argv[i], "-v") == 0) {
            printf("Verbose mode enabled.\n");
        }
        else {
            printf("Unknown option: %s\n", argv[i]);
        }
    }

    if(infilename.empty()) {
		printf("No input file specified.\n");
		return 1;
	}
    if (outfilename.empty()) {
        printf("No output file specified.\n");
		return 1;
    }

    int x,y,n;
    unsigned char *data = stbi_load(infilename.c_str(), &x, &y, &n, 3);
    if (data != NULL) {
        // ... process data if not NULL ...
        // ... x = width, y = height, n = # 8-bit components per pixel ...
        // ... replace '0' with '1'..'4' to force that many components per pixel
        // ... but 'n' will always be the number that it would have been if you said 0
        
        fp = fopen(outfilename.c_str(), "w");
        std::string array_name = outfilename;
        array_name.erase(array_name.find_last_of("."));
        
        if (fp != NULL) {
            fprintf(fp, "#include <stdint.h>\n");
            fprintf(fp, "// heigth: %d\n", y);
			fprintf(fp, "// width: %d\n", x);
            fprintf(fp, "// extern const uint8_t %s[];\n", array_name.c_str());
            fprintf(fp, "const uint8_t %s[%d] = {\n", array_name.c_str(), x*y);
            for (int _y = 0; _y < y; _y++) {
                for (int _x = 0; _x < x; _x++) {
                    unsigned char* pixel = data + (_y * x + _x) * 3;
                    uint8_t pixel332 = rgbTo332(pixel[0], pixel[1], pixel[2]);
                    fprintf(fp, "0x%.2X, ", pixel332);
                }
                fprintf(fp, "\n");
            }
            fprintf(fp, "};\n");
            fclose(fp);
        }
        else {
            printf("Failed to open output file.\n");
        }

        stbi_image_free(data);
    }
    else {
		printf("Failed to load image.\n");
		return 1;
	}
    
    return 0;
}

//https://basedwa.re/drummyfish/less_retarded_wiki/src/commit/70c10acfc52d3e3877b3772afba4dfd26bfe6a9b/rgb332.md
unsigned char rgbTo332(unsigned char red, unsigned char green, unsigned char blue)
{
    return ((red / 32) << 5) | ((green / 32) << 2) | (blue / 64);
}