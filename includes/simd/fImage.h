#ifndef IMAGE_H
#define IMAGE_H

#include <simd/vSimd.h>
#include <simd/vColor.h>
#include <stdlib.h>
#include <stdio.h>

#define RED 0x0F00
#define GRN 0x00F0
#define BLU 0x000F

///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
typedef struct fImage
{
    __attribute__((aligned(16))) float *pixels;
    __attribute__((aligned(16))) float *r;
    __attribute__((aligned(16))) float *g;
    __attribute__((aligned(16))) float *b;

    uint64_t width;
    uint64_t height;

    uint64_t size;
    uint64_t capacity;
    uint64_t padding;
    uint64_t chunked_size;
} fImage;

void set_fImage(fImage *img, uint64_t w, uint64_t h);

void put_pixel(fImage *img, uint64_t y, uint64_t x, const vRGB *color);

void create_file(fImage *img);

void free_fImage(fImage *img);

#endif
