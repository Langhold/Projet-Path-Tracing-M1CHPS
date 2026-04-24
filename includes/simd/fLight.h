#pragma once

#include <simd/fScene.h>
#include <simd/fColor.h>

void diffuse_render(fScene *scene, fRay *r, fRGB *pixel_color, fHit *hit_surface);
void phong_model(fScene *scene, fRay *r, fRGB *pixel_color, fHit *hit_surface, size_t x, size_t y, size_t width);
void fray_sampling(fScene *scene, fRay *r, const float *background_color, fRGB *radiance, fHit *hit_surface, int dmax, unsigned int *seed);
void fpath_tracing(fScene *scene, size_t n_sample, size_t n_bounce, size_t width, size_t height, fImage *img);
void fBenchmark_mouse(fScene *scene, size_t width, size_t height);