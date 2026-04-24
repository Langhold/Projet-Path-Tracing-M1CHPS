#pragma once

#include <immintrin.h>
#include <sleef.h>
#include <stdlib.h>
#include <math.h>
#include <tools.h>
#define EPSILON 1e-5

typedef __m128 __vec4f;

extern __m128 three_half;
extern __m128 half;
extern __m128 epsilon;
extern __m128 max_limit;
extern __m128 zero;
extern __m128 one;
extern __m128 minus_one;
extern __m128 two;
extern __m128 minus_two;
extern __m128 two_pi;

void init_global_variable();

__vec4f load(float *data);

void store(float *out, const __vec4f *in);

__vec4f set1(float val);

__vec4f set(float val3, float val2, float val1, float val0);

__vec4f sub_(const __vec4f *A, const __vec4f *B);

__vec4f add_(const __vec4f *A, const __vec4f *B);

__vec4f mul_(const __vec4f *A, const __vec4f *B);

__vec4f div_(const __vec4f *A, const __vec4f *B);

__vec4f nequal(const __vec4f *A, const __vec4f *B);

__vec4f equal(const __vec4f *A, const __vec4f *B);

__vec4f and_(const __vec4f *A, const __vec4f *B);
__vec4f andnot_(const __vec4f *A, const __vec4f *B);

__vec4f max_(const __vec4f *A, const __vec4f *B);

__vec4f min_(const __vec4f *A, const __vec4f *B);

__vec4f or_(const __vec4f *A, const __vec4f *B);

__vec4f sqrt_(const __vec4f *A);

__vec4f absf_(const __vec4f *A);

__vec4f _powf_(const __vec4f *A, const float exp);

__vec4f powf_(const __vec4f *A, const __vec4f *exp);

__vec4f lower(const __vec4f *A, const __vec4f *B);

int fuse(const __vec4f *mask);

__vec4f blendv(const __vec4f *A, const __vec4f *B, const __vec4f *mask);

__vec4f greater(const __vec4f *A, const __vec4f *B);

__vec4f rcp(const __vec4f *A);

__vec4f fmadd(const __vec4f *A, const __vec4f *B, const __vec4f *C);

__vec4f fmsub(const __vec4f *A, const __vec4f *B, const __vec4f *C);

__vec4f dot_(const __vec4f *A_x, const __vec4f *A_y, const __vec4f *A_z,
             const __vec4f *B_x, const __vec4f *B_y, const __vec4f *B_z);

void cross_(const __vec4f *A_x, const __vec4f *A_y, const __vec4f *A_z,
            const __vec4f *B_x, const __vec4f *B_y, const __vec4f *B_z,
            __vec4f *C_x, __vec4f *C_y, __vec4f *C_z);

__vec4f magnitude_(const __vec4f *x, const __vec4f *y, const __vec4f *z);

void norm_(const __vec4f *A_x, const __vec4f *A_y, const __vec4f *A_z,
           __vec4f *C_x, __vec4f *C_y, __vec4f *C_z);

__vec4f cos_(__vec4f *A);
__vec4f sin_(__vec4f *A);