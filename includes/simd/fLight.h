#pragma once

#include <simd/fScene.h>
#include <simd/fColor.h>

void diffuse_render(fRay *r, fSphere *sph, fQuad *q, fRGB *pixel_color, fHit *hit_surface);
void phong_model(fRay *r, fSphere *sph, fQuad *q, fRGB *pixel_color, fHit *hit_surface, size_t x, size_t y, size_t width);
void fray_sampling(fRay *r, fSphere *sph, fQuad *q, const float *background_color, fRGB *radiance, fHit *hit_surface, int dmax, unsigned int *seed);

void fBenchmark_mouse(fSphere *sph, fQuad *cornel_box);
