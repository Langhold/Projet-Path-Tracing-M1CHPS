#ifndef VECTOR_H
#define VECTOR_H

#include "tools.h"

/**
 * @brief Single precision 3D Vector (AoS version)
 * @var Data Componants values
 */
typedef struct Vector
{
    float Data[3];
}Vector;

//////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////


/**
 * @brief Create a single precision null 3D vector (AoS version)
 * @param u A single precision 3D vector
 */
static inline void create_vector_default_ext(Vector* u)
{
	u->Data[0] = 0.0f;
	u->Data[1] = 0.0f;
	u->Data[2] = 0.0f;
}


/**
 * @brief Create a random single precision 3D vector (AoS version)
 * @param u A single precision 3D vector
 */
void create_vector_random_default_ext(Vector* u);

/**
 * @brief Create a random single precision 3D vector (AoS version)
 * @param u A pointer to an empty single precision 3D vector
 * @param min Minimum componant value
 * @param max Maximum componant value
 */
void create_vector_random_ext(Vector* u, const float min, const float max);
/**
 * @brief Create a single precision 3D vector (AoS version)
 * @param u A single precision 3D vector pointer
 * @param x x componant
 * @param y y componant
 * @param z z componant
 */
static inline void create_vector_ext(Vector* u, const float x, const float y, const float z)
{
	u->Data[0] = x;
	u->Data[1] = y;
	u->Data[2] = z;
}
//////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////


/**
 * @brief Retrun a bool: true if the vector is null, false else
 * @param v A single precision 3D vector
 * @return bool
 */
static inline bool is_null(const Vector * v){
	for(int i = 0; i<3; ++i){
		if(v->Data[i] <= EPS) return false;
	}
	return true;
}

/**
 * @brief Sum of two single precision 3D vector
 * @param u A single precision 3D vector
 * @param v A single precision 3D vector
 * @param w A single precision 3D vector
 */
static inline void add_ext(const Vector* const u, const Vector* const v, Vector* w)
{
	w->Data[0] = u->Data[0] + v->Data[0];
	w->Data[1] = u->Data[1] + v->Data[1];
	w->Data[2] = u->Data[2] + v->Data[2];
}

/**
 * @brief Substraction of two single precision 3D vector
 * @param u A single precision 3D vector
 * @param v A single precision 3D vector
 * @param w A single precision 3D vector
 */
static inline void sub_ext(const Vector* const u, const Vector* const v, Vector* w)
{
	w->Data[0] = u->Data[0] - v->Data[0];
	w->Data[1] = u->Data[1] - v->Data[1];
	w->Data[2] = u->Data[2] - v->Data[2];
}

/**
 * @brief Multiplication of a single precision 3D vector by a single precsion scalar
 * @param u A single precision 3D vector
 * @param k A single precision scalar
 * @param w A single precision 3D vector
 */
static inline void mul_ext(const Vector* const u, const float k, Vector* w)
{
	w->Data[0] = u->Data[0] * k;
	w->Data[1] = u->Data[1] * k;
	w->Data[2] = u->Data[2] * k;
}

/**
 * @brief Multiplication of a single precision 3D vector by a single precsion scalar
 * @param u A single precision 3D vector
 * @param v A single precision 3D vector
 * @param k A single precision scalar
 * @param w A single precision 3D vector
 */
static inline void linear_ext(const Vector* const u, const Vector* const v, const float k, Vector* w)
{
	w->Data[0] = u->Data[0] + v->Data[0] * k;
	w->Data[1] = u->Data[1] + v->Data[1] * k;
	w->Data[2] = u->Data[2] + v->Data[2] * k;
}

/**
 * @brief Dot product of two single precision 3D vector
 * @param u A single precision 3D vector
 * @param v A single precision 3D vector
 * @return A single precision  scalar
 */
static inline float dot(const Vector* const u, const Vector* const v)
{
	const float scalar = u->Data[0] * v->Data[0] + u->Data[1] * v->Data[1] + u->Data[2] * v->Data[2];

	return scalar;
}


/**
 * @brief Cross product of two single precision 3D vector
 * @param u A single precision 3D vector
 * @param v A single precision 3D vector
 * @param w A single precision 3D vector
 */
static inline void cross_ext(const Vector* const u, const Vector* const v, Vector* w)
{
	w->Data[0] = u->Data[1] * v->Data[2] - u->Data[2] * v->Data[1];
	w->Data[1] = u->Data[2] * v->Data[0] - u->Data[0] * v->Data[2];
	w->Data[2] = u->Data[0] * v->Data[1] - u->Data[1] * v->Data[0];
}

//////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

/**
 * @brief Single precision Length of single precsion 3D vector
 * @param u A single precision 3D vector
 * @return Single precision scalar 
 */
static inline float length(const Vector* const u)
{
	const float square = dot(u,u);
	const float len = sqrtf(square);

	return len;
}

/**
 * @brief Normalize a single precsion 3D vector
 * @param u A single single precision 3D vector
 * @param w A single single precision 3D vector
 */
static inline void norm_ext(const Vector* const u, Vector* w)
{
	const float len = length(u);

	w->Data[0] = u->Data[0] / len;
	w->Data[1] = u->Data[1] / len;
	w->Data[2] = u->Data[2] / len;
}

//////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

/**
 * @brief Viewport transformation
 * @param u A single single precision 3D vector
 * @param width Width of the screen
 * @param height Height of the screen
 * @param near z minimum
 * @param far z maximum
 * @return A single precision normalized 3D vector
 */
Vector* viewport(const Vector* const u, const float width, const float height, const float near, const float far);

/**
 * @brief Viewport transformation
 * @param u A single single precision 3D vector
 * @param w A single single precision 3D vector
 */
void viewport_ext(const Vector* const u, Vector* w);

//////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

/**
 * @brief Display single precision 3D vector
 * @param u A single single precision 3D vector
 */
void display_vector(const Vector* const u);

/**
 * @brief Free single precision 3D vector
 * @param u A single single precision 3D vector
 */
void free_vector(Vector* u);

static inline float dist(const Vector* u, const Vector* v){
	Vector w;
	sub_ext(u, v, &w);
	
	return fabsf(length(&w));
}
#endif
