#pragma once

#include <inttypes.h>
#include <memory.h>
#include <immintrin.h>
#include <stdlib.h>

#include <simd/fSimd.h>

#define ESPILON 1e-5

typedef struct fRGB
{
    __attribute__((aligned(16))) float *color;
    __attribute__((aligned(16))) float *r;
    __attribute__((aligned(16))) float *g;
    __attribute__((aligned(16))) float *b;
    uint64_t size;
    uint64_t capacity;
    uint64_t padding;
    uint64_t chunked_size;
} fRGB;

void set_fRGB(fRGB *rgb, uint64_t c);
void free_fRGB(fRGB *rgb);