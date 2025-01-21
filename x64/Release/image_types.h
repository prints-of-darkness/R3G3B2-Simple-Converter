/**/
//Contents of image_types.h:

#ifndef IMAGE_TYPES_H
#define IMAGE_TYPES_H

#include <stdint.h>

#define RGB332_FORMAT_ID 0x332

typedef struct {
    const uint8_t* data;
    uint16_t width;
    uint16_t height;
    uint16_t format_id;
} Image_t;

#endif // IMAGE_TYPES_H
/**/