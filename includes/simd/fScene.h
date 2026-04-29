#pragma once

#include <stdint.h>
#include <simd/vHit.h>
#include <simd/fImage.h>
#include <simd/vRay.h>

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
    __attribute__((aligned(16))) float *albedo;
    __attribute__((aligned(16))) float *r;
    __attribute__((aligned(16))) float *g;
    __attribute__((aligned(16))) float *b;
    __attribute__((aligned(16))) float *radius;
    __attribute__((aligned(16))) float *type;
    __attribute__((aligned(16))) float *emission_power;

    uint64_t size;
    uint64_t capacity;
    uint64_t padding;
    uint64_t chunked_size;

} fSphere;

void set_fSphere(fSphere *sph, uint64_t c);
void add_fSphere(fSphere *sph, const float *coord, const float *albedo, const float radius, float emission_power, const float type);
void add_fSphere_(fSphere *sph, float x, float y, float z, float r, float g, float b, const float radius, float emission_power, const float type);

void intersect_fsphere(fSphere *sph, const vRay *packed_ray, vHit *packed_hit, __vec4f *tmin);

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

    __attribute__((aligned(16))) float *albedo;

    __attribute__((aligned(16))) float *r;
    __attribute__((aligned(16))) float *g;
    __attribute__((aligned(16))) float *b;

    __attribute__((aligned(16))) float *type;
    __attribute__((aligned(16))) float *emission_power;

    uint64_t size;
    uint64_t capacity;
    uint64_t padding;
    uint64_t chunked_size;

} fQuad;

void set_fQuad(fQuad *q, uint64_t c);
void add_fQuad(fQuad *q, const float *Q, const float *u, const float *v, const float *c, float emission_power, float type);
void intersect_fquad(fQuad *q, const vRay *packed_ray, vHit *packed_hit, __vec4f *tmin);
void free_fQuad(fQuad *q);

////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

typedef struct fScene
{
    vCamera cam;
    fSphere spheres;
    fQuad quads;
    __vec4f fov;
    __vec4f aspr_;
    __vec4f inv_w;
    __vec4f inv_h;
} fScene;

void set_fScene(fScene *scene, const float *cam_position, float degree, float pitch, float yaw, size_t sphere_capacity, size_t quad_capacity, size_t width, size_t height);

void add_fSquare(fQuad *q, const float width, const float height, const float depth, const float *position, const float *albedo, float emission_power, float type);
void add_cornel_box(fQuad *q, const float width, const float height, const float depth, const float *position, float emission_power, float type);

void vintersect_in_scene(fScene *scene, const vRay *packed_ray, vHit *packed_hit);
void free_fScene(fScene *scene);
