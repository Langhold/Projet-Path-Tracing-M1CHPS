#ifndef scene_h
#define scene_h

#include "image.h"
#include "ray.h"


#define NULL_AABB (AABB) {{ {  FLT_MAX,  FLT_MAX,  FLT_MAX } },{ { -FLT_MAX, -FLT_MAX, -FLT_MAX }}}


typedef enum PRIM_TYPE
{
	SPHERE,
	BBOX, //the Bounding Box
	BOX //regular Box
}PRIM_TYPE;

typedef enum {
	Lambertian,
	Specular,
	Emissive
} material_t;

/**
 * @brief A 3D AABB
 * @var min Minimum coordinate
 * @var max Maximum coordinate
 * @var color color faces
 */
typedef struct AABB
{
	Vector bmin;
	Vector bmax;
}AABB;

/**
 * @brief A 3D OBB
 * @var center Center
 * @var obb_right Axis right
 * @var obb_up Axis up
 * @var obb_direction Axis direction
 * @var size Distance center to extremity
 * @var color color faces
 */
typedef struct OBB
{
	Vector center;
	Vector obb_right;
	Vector obb_up;
	Vector obb_direction;
	Vector size;
}OBB;

/**
 * @brief A 3D Sphere
 * @var radius Radius
 * @var position Position of the sphere
 */
typedef struct Sphere
{
	float radius;
}Sphere;

typedef struct Primitive
{
	void * object;
	Vector position;
	Vector color;
	PRIM_TYPE type;
	material_t m_type;
	float albedo;
}Primitive;

typedef struct Scene{
	Primitive ** objects;
	Vector * background_color;
	size_t size_objects;
	Camera camera;
}Scene;

typedef struct object_tree_t{
	AABB box;
	Primitive** objects;
	int objects_count;
	
	struct object_tree_t* right;
	struct object_tree_t* left;
} object_tree_t;


typedef struct Large_BVH_t{
	AABB box;
	int K;
	struct object_tree_t** clusters;
	
} Large_BVH_t;


/**
 * @brief Free scene
 * @param S A pointer of a scene
 */
void free_scene(Scene * S);

/**
 * @brief Free pointer objects in the scene
 * @param S A pointer of a scene
 */
void free_scene_objects(Scene * S);

/**
 * @brief Alloc a scene of n\_objects objects, nb\_lightsources light sources and with a background color
 * @param n_objects number of objects
 * @param backgroundColor a uint32_t represent the background color
 * @param s output Scene
 */
void create_scene_ext(size_t n_objects, const Vector * backgroundColor, Scene * s);


void create_sphere(Primitive* prim, const float radius, const float x, const float y, const float z, material_t m_type, float albedo, Vector *color);

void create_box(Primitive* prim, const float width, const float height, const float length, const float x, const float y, const float z, material_t m_type, float albedo, Vector *color, const float pitch, const float yaw);

void create_cube(Primitive* prim, const float width, const float x, const float y, const float z, material_t m_type, float albedo, Vector *color);

/**
 * @brief Alloc a scene of n\_objects objects, nb\_lightsources light sources and with a background color
 * @param object object to add
 * @param s output Scene
 */
void add_primitive(Primitive * object, Scene * s);



/**
 * @brief Compute the intersection of a camra ray and a AABB box
 * @param r Ray
 * @param box AABB box
 * @return If there is an intersection
 */
bool intersect_box(Ray* const r, const AABB* const box, Vector *hit, int * face, int * is_intern);

/**
 * @brief Compute the intersection of a camra ray and a OBB box
 * @param r Ray
 * @param box OBB box
 * @return If there is an intersection
 */
bool intersect_obb(Ray* const r, const OBB* const box, Vector *hit, int *face, int *is_intern);

/**
 * @brief Compute the intersection of a ray and a sphere
 * @param r Ray
 * @param position Sphere position
 * @param radius Sphere radius
 * @return Set of points
 */
bool intersect_sphere(Ray* const r, Vector *position, float radius, Vector *hit);

/**
 * @brief return a pointer of the intersection point between the ray and the closer object
 * @param r pointer of the incident ray
 * @param S pointer of the scene
 * @return index of the object
 * @return intersection:  The poter to the intersection point
 */
bool intersect_in_scene(struct Ray* r, const Scene* const S, int * object, Vector *hit, Vector *n);

/**
 * @brief Compute the sphere normal vector at a point
 * @param point pointer of the point
 * @param center sphere's center pointer
 * @return pointer to the normal vector
 */
Vector get_normal_vector_sphere(const Vector * point, const Vector *center);

/**
 * @brief Compute the sphere normal vector at a point
 * @return pointer to the normal vector
 */
Vector get_normal_vector_box(int face, int is_intern);

object_tree_t* initialize_root_tree(Scene* S);
object_tree_t* initialize_root_tree_v2(Scene* S);
void free_tree_objects(object_tree_t** root);
int intersect_in_clusters(Large_BVH_t* const tree, const Ray* r, float* closest_t, Primitive** intersected_object, int* is_intern, int* face);
Large_BVH_t* initialize_tree_clustering(const Scene* S, unsigned int* seed, const int K);

int intersect_in_tree(object_tree_t* const tree, const Ray* r, float* closest_t, Primitive** intersected_object, int* is_intern, int* face);

void free_clusters(Large_BVH_t** root);

static const float inv255 = 1 / 255.0f;

#endif /* scene_h */
