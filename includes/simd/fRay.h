#ifndef RAY_H
#define RAY_H

#include <inttypes.h>
#include <simd/fSimd.h>
/**
 * @brief 3D ray with a soa structure
 * @var position Position/Start of the ray
 * @var direction Direction/Endpoint of the ray
 */
typedef struct fRay
{
    float *ori_coord;
    float *ori_x;
    float *ori_y;
    float *ori_z;

    float *dir_coord;
    float *dir_x;
    float *dir_y;
    float *dir_z;

    uint64_t size;
    uint64_t capacity;
    uint64_t padding;
    uint64_t chunked_size;
    float t;
} fRay;

void set_fray(fRay *ray, const uint64_t c);

void trace_ray_simd(const __vec4f *inv_w, const __vec4f *inv_h, const __vec4f *aspr_, const __vec4f *x, const __vec4f *y, const __vec4f *fov, fRay *rv);
void free_fRay(fRay *ray);

#endif
