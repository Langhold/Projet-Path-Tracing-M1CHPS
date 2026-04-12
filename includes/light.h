#ifndef light_h
#define light_h


#include "scene.h"
#include <omp.h>


void benchmark1(Scene* scene, size_t width, size_t height);
void benchmark_medium(Scene* scene, size_t width, size_t height);
void benchmark_huge(Scene* scene, size_t width, size_t height);
void benchmark_big(Scene* scene, size_t width, size_t height);

/**
 * @brief  recursive path-tracing algorithm to compute a single sample with Lambertian materials.
 * @param r incident ray
 * @param S pointer of the current scene
 * @param d current number of bounces
 * @param dmax maximum bounces number
 * @param radiance color of the pixels at the e object
 */
void ray_sampling(Ray * r, const Scene * S, int dmax, Vector * radiance, unsigned int* seed);

/**
 * @brief Trace 3D ray from the camera with Path Tracing with N samples
 * @param pixel_x input: index x of the pixel
 * @param pixel_y input: index y of the pixel
 * @param S input: the scene of objects
 * @param N input: number of samples
 * @param color output: the color of the pixel
 */
void path_trace(const int x1, const int y1, const int local_y, const int width, Scene const * S, const size_t bounces, float* color_buffer, unsigned int* seed);
void path_trace_t(const int x1, const int y1, const int local_y, const int width, Scene const * S, const size_t bounces, float* color_buffer, unsigned int* seed, object_tree_t* const tree);
void path_trace_clusters(const int x1, const int y1, const int local_y, const int width, Scene const * S, const size_t bounces, float* color_buffer, unsigned int* seed, Large_BVH_t* const tree);

int get_bounces(void);

static Vector black;
static Vector white;


#endif /* light_h */
