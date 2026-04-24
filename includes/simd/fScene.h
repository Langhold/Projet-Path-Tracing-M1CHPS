#pragma once

#include <stdint.h>
#include <simd/fHit.h>
#include <simd/fImage.h>
#include <simd/fRay.h>

typedef enum
{
    Lambertian,
    Specular,
    Emissive
} fMaterial_t;

typedef struct fSphere
{
    __attribute__((aligned(16))) float *coord;
    __attribute__((aligned(16))) float *x;
    __attribute__((aligned(16))) float *y;
    __attribute__((aligned(16))) float *z;
    __attribute__((aligned(16))) float *color;
    __attribute__((aligned(16))) float *r;
    __attribute__((aligned(16))) float *g;
    __attribute__((aligned(16))) float *b;
    __attribute__((aligned(16))) float *radius;
    __attribute__((aligned(16))) float *type;
    __attribute__((aligned(16))) float *albedo;

    uint64_t size;
    uint64_t capacity;
    uint64_t padding;
    uint64_t chunked_size;

} fSphere;

void set_fSphere(fSphere *sph, uint64_t c);
void add_fSphere(fSphere *sph, const float *coord, const float *color, const float radius, float albedo, const float type);
void add_fSphere_(fSphere *sph, float x, float y, float z, float r, float g, float b, const float radius, float albedo, const float type);

void intersect_fsphere(fSphere *sph,
                       const __vec4f *origin_x, const __vec4f *origin_y, const __vec4f *origin_z,
                       const __vec4f *dir_x, const __vec4f *dir_y, const __vec4f *dir_z,
                       __vec4f *n_x_min, __vec4f *n_y_min, __vec4f *n_z_min,
                       __vec4f *color_r_min, __vec4f *color_g_min, __vec4f *color_b_min,
                       __vec4f *albedo_min, __vec4f *type_min,
                       __vec4f *tmin, __vec4f *hit_mask);

void free_fSphere(fSphere *sph);

////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

typedef struct fQuad
{
    __attribute__((aligned(16))) float *Q;
    __attribute__((aligned(16))) float *qx;
    __attribute__((aligned(16))) float *qy;
    __attribute__((aligned(16))) float *qz;

    __attribute__((aligned(16))) float *u;
    __attribute__((aligned(16))) float *ux;
    __attribute__((aligned(16))) float *uy;
    __attribute__((aligned(16))) float *uz;

    __attribute__((aligned(16))) float *v;
    __attribute__((aligned(16))) float *vx;
    __attribute__((aligned(16))) float *vy;
    __attribute__((aligned(16))) float *vz;

    __attribute__((aligned(16))) float *n;
    __attribute__((aligned(16))) float *nx;
    __attribute__((aligned(16))) float *ny;
    __attribute__((aligned(16))) float *nz;

    __attribute__((aligned(16))) float *D;

    __attribute__((aligned(16))) float *w;
    __attribute__((aligned(16))) float *wx;
    __attribute__((aligned(16))) float *wy;
    __attribute__((aligned(16))) float *wz;

    __attribute__((aligned(16))) float *color;

    __attribute__((aligned(16))) float *r;
    __attribute__((aligned(16))) float *g;
    __attribute__((aligned(16))) float *b;

    __attribute__((aligned(16))) float *type;
    __attribute__((aligned(16))) float *albedo;

    uint64_t size;
    uint64_t capacity;
    uint64_t padding;
    uint64_t chunked_size;

} fQuad;

void set_fQuad(fQuad *q, uint64_t c);
void add_fQuad(fQuad *q, const float *Q, const float *u, const float *v, const float *c, float albedo, float type);
void intersect_fquad(fQuad *q,
                     const __vec4f *origin_x, const __vec4f *origin_y, const __vec4f *origin_z,
                     const __vec4f *dir_x, const __vec4f *dir_y, const __vec4f *dir_z,
                     __vec4f *n_x_min, __vec4f *n_y_min, __vec4f *n_z_min,
                     __vec4f *color_r_min, __vec4f *color_g_min, __vec4f *color_b_min,
                     __vec4f *albedo_min, __vec4f *type_min,
                     __vec4f *tmin, __vec4f *hit_mask);
void free_fQuad(fQuad *q);

////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

typedef struct fScene
{
    fCamera cam;
    fSphere spheres;
    fQuad quads;
    __vec4f fov;
    __vec4f aspr_;
    __vec4f inv_w;
    __vec4f inv_h;
} fScene;

void set_fScene(fScene *scene, const float *cam_position, float degree, float pitch, float yaw, size_t sphere_capacity, size_t quad_capacity, size_t width, size_t height);

void intersect_in_scene_f(fScene *scene, const __vec4f *origin_x, const __vec4f *origin_y, const __vec4f *origin_z, const __vec4f *dir_x, const __vec4f *dir_y, const __vec4f *dir_z, fHit *hits);
void free_fScene(fScene *scene);
