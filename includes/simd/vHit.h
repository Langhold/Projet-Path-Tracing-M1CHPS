#pragma once

#include <stdint.h>
#include <simd/vSimd.h>

typedef struct vHit
{
    __vec4f hit_point_x;
    __vec4f hit_point_y;
    __vec4f hit_point_z;

    __vec4f hit_normal_x;
    __vec4f hit_normal_y;
    __vec4f hit_normal_z;

    __vec4f hit_albedo_r;
    __vec4f hit_albedo_g;
    __vec4f hit_albedo_b;

    __vec4f hit_isHitting;
    __vec4f hit_emissive_power;
    __vec4f hit_mat_type;

} vHit;

static void __attribute__((unused)) clear_hit(vHit *h)
{
    h->hit_point_x = zero;
    h->hit_point_y = zero;
    h->hit_point_z = zero;

    h->hit_normal_x = zero;
    h->hit_normal_y = zero;
    h->hit_normal_z = zero;

    h->hit_albedo_r = zero;
    h->hit_albedo_g = zero;
    h->hit_albedo_b = zero;

    h->hit_isHitting = zero;
    h->hit_emissive_power = zero;
    h->hit_mat_type = zero;
}
