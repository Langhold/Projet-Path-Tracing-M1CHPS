//
//  BRDF.h
//  Projet PPN
//
//  Created by Nolwen Dolléans on 12/11/2025.
//

#ifndef light_h
#define light_h

#include "scene.h"

void benchmark1(Scene *scene, size_t width, size_t height);
void benchmark_medium(Scene *scene, size_t width, size_t height);
void benchmark_huge(Scene *scene, size_t width, size_t height);

/**
 * @brief  recursive path-tracing algorithm to compute a single sample with Lambertian materials.
 * @param r incident ray
 * @param S pointer of the current scene
 * @param d current number of bounces
 * @param dmax maximum bounces number
 * @param radiance color of the pixels at the e object
 */
void ray_sampling(Ray *r, const Scene *S, int d, int dmax, Vector *radiance);

/**
 * @brief Trace 3D ray from the camera with Path Tracing with N samples
 * @param pixel_x input: index x of the pixel
 * @param pixel_y input: index y of the pixel
 * @param S input: the scene of objects
 * @param N input: number of samples
 * @param color output: the color of the pixel
 */
void path_trace(const int width, const int height, Scene const *S, const size_t bounces, const size_t N, float *color_buffer);

int get_bounces(void);

#endif /* light_h */
