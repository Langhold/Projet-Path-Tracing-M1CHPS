#pragma once

#include <inttypes.h>
#include <simd/vSimd.h>

typedef struct vCamera
{
    __vec4f up_x;
    __vec4f up_y;
    __vec4f up_z;

    __vec4f right_x;
    __vec4f right_y;
    __vec4f right_z;

    __vec4f direction_x;
    __vec4f direction_y;
    __vec4f direction_z;

    __vec4f position_x;
    __vec4f position_y;
    __vec4f position_z;

} vCamera;

void vcreate_camera(vCamera *const cam, const float x0, const float y0, const float z0, const float pitch, const float yaw);

/**
 * @brief 3D vectorized ray with a soa structure in mind
 * @var position Position/Start of the ray
 * @var direction Direction/Endpoint of the ray
 */
typedef struct vRay
{
    __vec4f ori_x;
    __vec4f ori_y;
    __vec4f ori_z;

    __vec4f dir_x;
    __vec4f dir_y;
    __vec4f dir_z;

} vRay;

static inline void clear_ray(vRay *ray)
{
    ray->ori_x = zero;
    ray->ori_y = zero;
    ray->ori_z = zero;

    ray->dir_x = zero;
    ray->dir_y = zero;
    ray->dir_z = zero;
};

void vtrace_ray(vCamera *cam, vRay *ray, const __vec4f *inv_w, const __vec4f *inv_h, const __vec4f *aspr_, const __vec4f *x, const __vec4f *y, const __vec4f *fov);