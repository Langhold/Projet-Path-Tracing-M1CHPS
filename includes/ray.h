#ifndef RAY_H
#define RAY_H

#include <inttypes.h>

#include "vector.h"

// Left face
#define MIN 0
// RIght face
#define MAX 1

// Back face
#define BACK 2
// Front face
#define FRONT 3

// Up face
#define UP 4
// Bottom face
#define BOTTOM 5

///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

/**
 * @brief 3D ray
 * @var position Position/Start of the ray
 * @var direction Direction/Endpoint of the ray
 */
typedef struct Ray
{
	Vector position;
	Vector direction;
	float t;
} Ray;

/**
 * @brief Camera
 * @var Up Up vector
 * @var Right Right vector
 * @var position Camera position/ray position
 * @var fov Filed of View
 * @var width Width
 * @var height Height
 * @var ray Up vector
 */
typedef struct Camera
{
	Vector up;
	Vector right;
	Vector position;
	Vector direction;
	float fov;
	size_t width;
	size_t height;
	float inv_width;
	float inv_height;
} Camera;

///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

/**
 * @brief Create a camera at some position with up, right and front vector
 * @param cam Camera to set up
 * @param width Width screen
 * @param height Height screen
 * @param fov Field of view
 * @param x0 Position x of the camera
 * @param y0 Position y of the camera
 * @param z0 Position z of the camera
 * @param pitch Camera rotation around right axis
 * @param yaw Camera rotation around up axis
 */
void create_camera(Camera *const cam, const size_t width, const size_t height, const float fov, const float x0, const float y0, const float z0, const float pitch, const float yaw);

/**
 * @brief Create 3D ray with (0,0,0) as origin and direction
 * @param ray 3D ray
 */
void create_ray_default_ext(Ray *ray);

/**
 * @brief Create 3D ray with (x0,y0,z0) as origin and (x1,y1,z1) as direction
 * @param x0 position x
 * @param y0 position y
 * @param z0 position z
 * @param x1 direction x
 * @param y1 direction y
 * @param z1 direction z
 * @param ray A 3D ray
 */
void create_ray_ext(Ray *ray, const float x0, const float y0, const float z0, const float x1, const float y1, const float z1);

void trace_ray(const size_t i, const size_t j, const Camera *c, Ray *const r);

/**
 * @brief Free ray
 * @param r Ray
 */
void free_ray(Ray *r);

///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

#endif
