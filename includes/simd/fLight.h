#pragma once

#include <simd/fScene.h>
#include <simd/vColor.h>

void vbenchmark1(fScene *scene, size_t width, size_t height);
void vbenchmark_medium(fScene *scene, size_t width, size_t height);
void vbenchmark_big(fScene *scene, size_t width, size_t height);
void vbenchmark_huge(fScene *scene, size_t width, size_t height);

void diffuse_render(fScene *scene, vRay *packed_ray, vRGB *packed_pixel_color, vHit *packed_pixel_hit_surface);
void phong_model(fScene *scene, vRay *packed_ray, vRGB *packed_pixel_color, vHit *packed_pixel_hit_surface);
void vray_sampling(fScene *scene, vRay *packed_ray, vHit *packed_pixel_hit_surface, vRGB *packed_pixel_radiance, int dmax, unsigned int *seed);

void vpath_tracing(fScene *scene, size_t n_sample, size_t n_bounce, size_t width, size_t height, fImage *img);
void vpath_tracing_omp(fScene *scene, size_t n_sample, size_t n_bounce, size_t width, size_t height, fImage *img);
