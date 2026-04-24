#pragma once

#include <stdint.h>
#include <simd/fSimd.h>

typedef struct fHit
{
    __attribute__((aligned(16))) float *hit_point;
    __attribute__((aligned(16))) float *hpx;
    __attribute__((aligned(16))) float *hpy;
    __attribute__((aligned(16))) float *hpz;

    __attribute__((aligned(16))) float *hit_normal;
    __attribute__((aligned(16))) float *hnx;
    __attribute__((aligned(16))) float *hny;
    __attribute__((aligned(16))) float *hnz;

    __attribute__((aligned(16))) float *hit_color;
    __attribute__((aligned(16))) float *hcr;
    __attribute__((aligned(16))) float *hcg;
    __attribute__((aligned(16))) float *hcb;

    __attribute__((aligned(16))) float *isHitting;
    __attribute__((aligned(16))) float *albedo;
    __attribute__((aligned(16))) float *type;

    uint64_t size;
    uint64_t capacity;
    uint64_t padding;
    uint64_t chunked_size;
} fHit;

void set_fHit(fHit *h, uint64_t c);
void free_fHit(fHit *h);