#pragma once

#include <inttypes.h>
#include <memory.h>
#include <immintrin.h>
#include <stdlib.h>

#include <simd/vSimd.h>

#define ESPILON 1e-5

typedef struct vRGB
{
    __vec4f r;
    __vec4f g;
    __vec4f b;

} vRGB;

static inline void clear_rgb(vRGB *rgb)
{
    rgb->r = zero;
    rgb->g = zero;
    rgb->b = zero;
};

static inline void vadd_(vRGB *a, vRGB *b)
{
    a->r = _mm_add_ps(a->r, b->r);
    a->g = _mm_add_ps(a->g, b->g);
    a->b = _mm_add_ps(a->b, b->b);
}

static inline void vscale_(vRGB *a, __vec4f *b)
{
    a->r = _mm_mul_ps(a->r, *b);
    a->g = _mm_mul_ps(a->g, *b);
    a->b = _mm_mul_ps(a->b, *b);
}
