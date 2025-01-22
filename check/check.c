
/*

	code to convert r3g3b2 array to r8g8b8 bmp
        used for debugging (its not pretty code)

        MJM 2025
*/

#include <stdio.h>
#include <stdlib.h>
#include <stdint.h>
#include <string.h>
#include <ctype.h>

#define STB_IMAGE_WRITE_IMPLEMENTATION
#include "stb_image_write.h"

// Precomputed lookup table for r3g3b2 to r8g8b8 conversion
static const uint32_t r3g3b2_to_r8g8b8_lut[256] = {
    0x000000, 0x000055, 0x0000AA, 0x0000FF, 0x002100, 0x002155, 0x0021AA, 0x0021FF,
    0x004200, 0x004255, 0x0042AA, 0x0042FF, 0x006300, 0x006355, 0x0063AA, 0x0063FF,
    0x008400, 0x008455, 0x0084AA, 0x0084FF, 0x00A500, 0x00A555, 0x00A5AA, 0x00A5FF,
    0x00C600, 0x00C655, 0x00C6AA, 0x00C6FF, 0x00E700, 0x00E755, 0x00E7AA, 0x00E7FF,
    0x240000, 0x240055, 0x2400AA, 0x2400FF, 0x242100, 0x242155, 0x2421AA, 0x2421FF,
    0x244200, 0x244255, 0x2442AA, 0x2442FF, 0x246300, 0x246355, 0x2463AA, 0x2463FF,
    0x248400, 0x248455, 0x2484AA, 0x2484FF, 0x24A500, 0x24A555, 0x24A5AA, 0x24A5FF,
    0x24C600, 0x24C655, 0x24C6AA, 0x24C6FF, 0x24E700, 0x24E755, 0x24E7AA, 0x24E7FF,
    0x480000, 0x480055, 0x4800AA, 0x4800FF, 0x482100, 0x482155, 0x4821AA, 0x4821FF,
    0x484200, 0x484255, 0x4842AA, 0x4842FF, 0x486300, 0x486355, 0x4863AA, 0x4863FF,
    0x488400, 0x488455, 0x4884AA, 0x4884FF, 0x48A500, 0x48A555, 0x48A5AA, 0x48A5FF,
    0x48C600, 0x48C655, 0x48C6AA, 0x48C6FF, 0x48E700, 0x48E755, 0x48E7AA, 0x48E7FF,
    0x6C0000, 0x6C0055, 0x6C00AA, 0x6C00FF, 0x6C2100, 0x6C2155, 0x6C21AA, 0x6C21FF,
    0x6C4200, 0x6C4255, 0x6C42AA, 0x6C42FF, 0x6C6300, 0x6C6355, 0x6C63AA, 0x6C63FF,
    0x6C8400, 0x6C8455, 0x6C84AA, 0x6C84FF, 0x6CA500, 0x6CA555, 0x6CA5AA, 0x6CA5FF,
    0x6CC600, 0x6CC655, 0x6CC6AA, 0x6CC6FF, 0x6CE700, 0x6CE755, 0x6CE7AA, 0x6CE7FF,
    0x900000, 0x900055, 0x9000AA, 0x9000FF, 0x902100, 0x902155, 0x9021AA, 0x9021FF,
    0x904200, 0x904255, 0x9042AA, 0x9042FF, 0x906300, 0x906355, 0x9063AA, 0x9063FF,
    0x908400, 0x908455, 0x9084AA, 0x9084FF, 0x90A500, 0x90A555, 0x90A5AA, 0x90A5FF,
    0x90C600, 0x90C655, 0x90C6AA, 0x90C6FF, 0x90E700, 0x90E755, 0x90E7AA, 0x90E7FF,
    0xB40000, 0xB40055, 0xB400AA, 0xB400FF, 0xB42100, 0xB42155, 0xB421AA, 0xB421FF,
    0xB44200, 0xB44255, 0xB442AA, 0xB442FF, 0xB46300, 0xB46355, 0xB463AA, 0xB463FF,
    0xB48400, 0xB48455, 0xB484AA, 0xB484FF, 0xB4A500, 0xB4A555, 0xB4A5AA, 0xB4A5FF,
    0xB4C600, 0xB4C655, 0xB4C6AA, 0xB4C6FF, 0xB4E700, 0xB4E755, 0xB4E7AA, 0xB4E7FF,
    0xD80000, 0xD80055, 0xD800AA, 0xD800FF, 0xD82100, 0xD82155, 0xD821AA, 0xD821FF,
    0xD84200, 0xD84255, 0xD842AA, 0xD842FF, 0xD86300, 0xD86355, 0xD863AA, 0xD863FF,
    0xD88400, 0xD88455, 0xD884AA, 0xD884FF, 0xD8A500, 0xD8A555, 0xD8A5AA, 0xD8A5FF,
    0xD8C600, 0xD8C655, 0xD8C6AA, 0xD8C6FF, 0xD8E700, 0xD8E755, 0xD8E7AA, 0xD8E7FF,
    0xFC0000, 0xFC0055, 0xFC00AA, 0xFC00FF, 0xFC2100, 0xFC2155, 0xFC21AA, 0xFC21FF,
    0xFC4200, 0xFC4255, 0xFC42AA, 0xFC42FF, 0xFC6300, 0xFC6355, 0xFC63AA, 0xFC63FF,
    0xFC8400, 0xFC8455, 0xFC84AA, 0xFC84FF, 0xFCA500, 0xFCA555, 0xFCA5AA, 0xFCA5FF,
    0xFCC600, 0xFCC655, 0xFCC6AA, 0xFCC6FF, 0xFCE700, 0xFCE755, 0xFCE7AA, 0xFCE7FF
};

char* get_base_filename(const char* path) {
    const char* last_slash = strrchr(path, '/');
    const char* filename = last_slash ? last_slash + 1 : path;
    const char* dot = strrchr(filename, '.');
    size_t len = dot ? (size_t)(dot - filename) : strlen(filename);
    char* base_filename = malloc(len + 1);

    if(base_filename != NULL) {
        strncpy(base_filename, filename, len);
        base_filename[len] = '\0';
        return base_filename;
    }
    
    return NULL;
}

int main(int argc, char* argv[]) {
    if (argc != 2) {
        printf("Usage: %s <image_header.h>\n", argv[0]);
        return 1;
    }

    FILE* file = fopen(argv[1], "r");
    if (!file) {
        printf("Failed to open header file: %s\n", argv[1]);
        return 1;
    }

    char* base_filename = get_base_filename(argv[1]);
    char data_name[256];
    snprintf(data_name, sizeof(data_name), "%s_data", base_filename);

    int width = 0, height = 0;
    char line[8192];
    int line_count = 0;

    while (fgets(line, sizeof(line), file)) {
        line_count++;
        if (strstr(line, ".width") && sscanf(line, "%*[^0-9]%d", &width) == 1);
        if (strstr(line, ".height") && sscanf(line, "%*[^0-9]%d", &height) == 1);
        if (width != 0 && height != 0) break;
    }

    rewind(file);
    while (fgets(line, sizeof(line), file)) {
        if (strstr(line, data_name)) break;
    }

    uint8_t* data = malloc(width * height * 3);
    if(data != NULL)
    {
        // Parse data from header file
        int pixel_count = 0;
        while (fgets(line, sizeof(line), file) && pixel_count < width * height) {
            char* token = strtok(line, ",");
            while (token && pixel_count < width * height) {
                // Skip whitespace and '0x' prefix
                while (*token && isspace(*token)) token++;
                if (strncmp(token, "0x", 2) == 0) token += 2;

                // Convert hex string to integer
                uint8_t pixel;
                if (sscanf(token, "%hhx", &pixel) == 1) {
                    uint32_t rgb = r3g3b2_to_r8g8b8_lut[pixel];
                    data[pixel_count * 3] = (rgb >> 16) & 0xFF;     // R
                    data[pixel_count * 3 + 1] = (rgb >> 8) & 0xFF;  // G
                    data[pixel_count * 3 + 2] = rgb & 0xFF;         // B
                    pixel_count++;
                }
                token = strtok(NULL, ",");
            }
        }
        fclose(file);

        char output_filename[256];
        char* input_name = strrchr(argv[1], '/');
        if (input_name == NULL) input_name = argv[1];
        else input_name++;
        snprintf(output_filename, sizeof(output_filename), "%.*s.bmp", (int)(strrchr(input_name, '.') - input_name), input_name);

        stbi_write_bmp(output_filename, width, height, 3, data);

        free(data);
        printf("Image saved as %s\n", output_filename);
    }
    else 
    {
        fclose(file);
        return 1;
    }

    return 0;
}
