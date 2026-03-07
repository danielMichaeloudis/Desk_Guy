#ifndef BMP_READER_H
#define BMP_READER_H

#include <stdint.h>
#include <stdio.h>

#define BMP_WIDTH 412
#define BMP_HEIGHT 412

typedef struct
{
    uint16_t pixels[BMP_HEIGHT * BMP_WIDTH];
} Bitmap;

int bmp_load(const char *filename, Bitmap *out);

#endif