//
//  scene.c
//  Projet PPN
//
//  Created by Nolwen Dolléans on 13/11/2025.
//

#include "scene.h"

void free_scene(Scene *S)
{
	free_scene_objects(S);
	if (S->objects)
		free(S);
}

void free_scene_objects(Scene *S)
{
	for (size_t i = 0; i < S->size_objects; ++i)
	{
		if (S->objects[i])
			free(S->objects[i]->object);
		if (S->objects[i])
			free(S->objects[i]);
	}
}

void create_scene_ext(size_t n_objects, const Vector *backgroundColor, Scene *s)
{
	s->objects = malloc(sizeof(Primitive) * n_objects);
	for (size_t i = 0; i < n_objects; ++i)
	{
		s->objects[i] = NULL;
	}
	s->background_color = malloc(sizeof(Vector));
	mul_ext(backgroundColor, inv255, s->background_color);
	s->size_objects = n_objects;
}

void create_primitive_ext(void *shape, PRIM_TYPE type, float x, float y, float z, material_t m_type, float albedo, Vector *color, Primitive *prim)
{
	prim->type = type;
	prim->m_type = m_type;
	create_vector_ext(&prim->position, x, y, z);
	prim->albedo = albedo;
	prim->color = *color;
	for (size_t i = 0; i < 3; ++i)
	{
		prim->color.Data[i] = color->Data[i] * inv255;
	}
	prim->object = shape;
}

void add_primitive(Primitive *object, Scene *s)
{
	if (s == NULL || s->objects == NULL)
		return;
	for (size_t i = 0; i < s->size_objects; ++i)
	{
		if (s->objects[i] == NULL)
		{
			s->objects[i] = object;
			return;
		}
	}
	printf("Error: too many objects\n");
	exit(EXIT_FAILURE);
}

void create_sphere(Primitive *prim, const float radius, const float x, const float y, const float z, material_t m_type, float albedo, Vector *color)
{
	Sphere *sph = malloc(sizeof(Sphere));
	sph->radius = radius;

	prim->type = SPHERE;
	prim->m_type = m_type;
	create_vector_ext(&prim->position, x, y, z);
	prim->albedo = albedo;
	prim->color = *color;
	for (size_t i = 0; i < 3; ++i)
	{
		prim->color.Data[i] = color->Data[i] * inv255;
	}
	prim->object = (void *)sph;
}

void create_box(Primitive *prim, const float width, const float height, const float length, const float x, const float y, const float z, material_t m_type, float albedo, Vector *color, const float pitch, const float yaw)
{
	OBB *box = malloc(sizeof(OBB));

	create_vector_ext(&box->center, x, y, z);

	float cosA, sinA, cosB, sinB;
	Vector up;
	const float alpha = radian(pitch);
	const float beta = radian(yaw);
	sinA = sinf(alpha);
	cosA = cosf(alpha);
	sinB = sinf(beta);
	cosB = cosf(beta);

	box->obb_direction = (Vector){sinB, cosB * sinA, -cosA * cosB};

	if (fabs(box->obb_direction.Data[0]) < 1 - EPS)
		up = (Vector){0.0f, 1.0f, 0.0f};
	else
		up = (Vector){1.0f, 0.0f, 0.0f};

	cross_ext(&box->obb_direction, &up, &(box->obb_right));
	norm_ext(&(box->obb_right), &(box->obb_right));

	cross_ext(&(box->obb_right), &box->obb_direction, &(box->obb_up));
	norm_ext(&box->obb_up, &box->obb_up);

	create_vector_ext(&box->size, width * 0.5, height * 0.5, length * 0.5);

	prim->type = BOX;
	prim->m_type = m_type;
	create_vector_ext(&prim->position, x, y, z);
	prim->albedo = albedo;
	prim->color = *color;
	for (size_t i = 0; i < 3; ++i)
	{
		prim->color.Data[i] = color->Data[i] * inv255;
	}
	prim->object = (void *)box;
}

void create_cube(Primitive *prim, const float width, const float x, const float y, const float z, material_t m_type, float albedo, Vector *color)
{
	AABB *box = malloc(sizeof(AABB));

	box->bmin.Data[0] = x - width / 2;
	box->bmin.Data[1] = y - width / 2;
	box->bmin.Data[2] = z - width / 2;

	box->bmax.Data[0] = x + width / 2;
	box->bmax.Data[1] = y + width / 2;
	box->bmax.Data[2] = z + width / 2;

	prim->type = BBOX;
	prim->m_type = m_type;
	create_vector_ext(&prim->position, x, y, z);
	prim->albedo = albedo;
	prim->color = *color;
	for (size_t i = 0; i < 3; ++i)
	{
		prim->color.Data[i] = color->Data[i] * inv255;
	}
	prim->object = (void *)box;
}

bool intersect_sphere(Ray *const r, Vector *center, float radius, Vector *hit)
{
	Vector oc;
	sub_ext(&r->position, center, &oc);

	const float A = dot(&r->direction, &r->direction);
	const float B = 2.0f * dot(&r->direction, &oc);
	const float C = dot(&oc, &oc) - radius * radius;

	const float delta = B * B - 4.0f * A * C;

	if (delta < 0.0f)
		return false;

	float sqrt_delta = sqrtf(delta);
	float t0 = (-B - sqrt_delta) / (2.0f * A);
	float t1 = (-B + sqrt_delta) / (2.0f * A);

	float t = -1.0f;

	if (t0 > EPS)
		t = t0;
	else if (t1 > EPS)
		t = t1;
	else
		return false; // return false if each intersection point is negative

	linear_ext(&r->position, &r->direction, t, hit);
	return true;
}

bool intersect_box(Ray *const r, const AABB *const box, Vector *hit, int *face, int *is_intern)
{

	float tmin = -INFINITY;
	float tmax = INFINITY;
	int enterAxis = -1;
	int exitAxis = -1;

	for (int i = 0; i < 3; ++i)
	{
		float *origin = &r->position.Data[i];
		float *direction = &r->direction.Data[i];
		float minA = box->bmin.Data[i];
		float maxA = box->bmax.Data[i];

		if (fabsf(*direction) < EPS)
		{
			if (*origin < minA || *origin > maxA)
				return false;
			continue;
		}

		float invD = 1.0f / *direction;
		float t1 = (minA - *origin) * invD;
		float t2 = (maxA - *origin) * invD;
		int axisEnterCandidate = i;
		int axisExitCandidate = i;

		float low = fminf(t1, t2);
		float high = fmaxf(t1, t2);

		if (low > tmin)
		{
			tmin = low;
			enterAxis = axisEnterCandidate;
		}
		if (high < tmax)
		{
			tmax = high;
			exitAxis = axisExitCandidate;
		}

		if (tmin > tmax)
			return false;
	}

	float tHit;
	bool inside = false;

	// Si l'origine est à l'intérieur de la box
	if (tmin < EPS && tmax > EPS)
	{
		tHit = tmax;
		*is_intern = 1;
	}
	else if (tmin > EPS)
	{
		tHit = tmin;
		*is_intern = 0;
	}
	else
	{
		return false;
	}

	linear_ext(&r->position, &r->direction, tHit, hit);

	int axis = (*is_intern == 1) ? exitAxis : enterAxis;
	float dir = r->direction.Data[axis];
	switch (axis)
	{
	case 0:
		*face = (dir > 0) ? (inside ? MAX : MIN) : (inside ? MIN : MAX);
		break;
	case 1:
		*face = (dir > 0) ? (inside ? UP : BOTTOM) : (inside ? BOTTOM : UP);
		break;
	case 2:
		*face = (dir > 0) ? (inside ? FRONT : BACK) : (inside ? BACK : FRONT);
		break;
	}

	return true;
}

bool intersect_obb(Ray *const r, const OBB *const box, Vector *hit, int *face, int *is_intern)
{

	Vector o;
	sub_ext(&r->position, &box->center, &o);

	Vector rotated_origin;
	Vector rotated_dir;
	rotated_origin.Data[0] = dot(&o, &box->obb_right);
	rotated_origin.Data[1] = dot(&o, &box->obb_up);
	rotated_origin.Data[2] = dot(&o, &box->obb_direction);
	rotated_dir.Data[0] = dot(&r->direction, &box->obb_right);
	rotated_dir.Data[1] = dot(&r->direction, &box->obb_up);
	rotated_dir.Data[2] = dot(&r->direction, &box->obb_direction);

	Ray rotated_ray;
	rotated_ray.position = rotated_origin;
	rotated_ray.direction = rotated_dir;

	// Nous reutilisons les proprietees de l'AABB sur la rotated basis
	AABB rotated_box;
	Vector rotated_hit;
	int rotated_face;
	int rotated_inside;

	rotated_box.bmin.Data[0] = -box->size.Data[0];
	rotated_box.bmin.Data[1] = -box->size.Data[1];
	rotated_box.bmin.Data[2] = -box->size.Data[2];
	rotated_box.bmax.Data[0] = box->size.Data[0];
	rotated_box.bmax.Data[1] = box->size.Data[1];
	rotated_box.bmax.Data[2] = box->size.Data[2];

	if (!intersect_box(&rotated_ray, &rotated_box, &rotated_hit, &rotated_face, &rotated_inside))
		return false;

	// Nous recontruisons le point d'intersection sur les trois coordonnees en partant de la position initiale du cube
	Vector box_hit = box->center;
	linear_ext(&box_hit, &box->obb_right, rotated_hit.Data[0], &box_hit);
	linear_ext(&box_hit, &box->obb_up, rotated_hit.Data[1], &box_hit);
	linear_ext(&box_hit, &box->obb_direction, rotated_hit.Data[2], &box_hit);
	*hit = box_hit;
	*face = rotated_face;
	*is_intern = rotated_inside;

	return true;
}

Vector get_normal_vector_sphere(const Vector *point, const Vector *center)
{
	Vector n;
	sub_ext(point, center, &n);
	norm_ext(&n, &n);
	return n;
}

Vector get_normal_vector_box(int *face, int is_intern)
{
	if (*face < 0 || *face > 5)
		exit(EXIT_FAILURE);

	Vector n = {0};

	switch (*face)
	{
	case MIN:
		n.Data[0] = 1;
		break;
	case MAX:
		n.Data[0] = -1;
		break;
	case BOTTOM:
		n.Data[1] = 1;
		break;
	case UP:
		n.Data[1] = -1;
		break;
	case BACK:
		n.Data[2] = 1;
		break;
	case FRONT:
		n.Data[2] = -1;
		break;
	}

	if (is_intern)
		mul_ext(&n, -1.0, &n);

	return n;
}

bool intersect_in_scene(struct Ray *r, const Scene *const S, int *object, Vector *hit, Vector *n)
{
	if (S->objects == NULL)
		return false;
	int object_index = -1;

	const Vector *origin = &r->position;
	Vector best_hit;
	double closest_t = 1e30;

	for (size_t i = 0; i < S->size_objects; ++i)
	{
		if (S->objects[i] == NULL)
			continue;

		switch (S->objects[i]->type)
		{
		case SPHERE:
		{
			const Sphere *sp = (Sphere *)S->objects[i]->object;

			if (!intersect_sphere(r, &S->objects[i]->position, sp->radius, hit))
				continue;
			Vector diff;
			sub_ext(hit, origin, &diff);
			double t2 = dot(&diff, &diff);

			if (t2 < closest_t)
			{
				closest_t = t2;
				object_index = (int)i;
				best_hit = *hit;
				*n = get_normal_vector_sphere(hit, &S->objects[i]->position);
			}
			break;
		}

		case BBOX:
		{
			AABB *box = (AABB *)S->objects[i]->object;
			int face;
			int is_intern;

			if (!intersect_box(r, box, hit, &face, &is_intern))
				continue;

			Vector diff;
			sub_ext(hit, origin, &diff);
			double t2 = dot(&diff, &diff);

			if (t2 < closest_t)
			{
				closest_t = t2;
				object_index = (int)i;
				best_hit = *hit;
				*n = get_normal_vector_box(&face, is_intern);
			}
			break;
		}
		case BOX:
		{
			OBB *box = (OBB *)S->objects[i]->object;
			int face;
			int is_intern;

			if (!intersect_obb(r, box, hit, &face, &is_intern))
				continue;

			Vector diff;
			sub_ext(hit, origin, &diff);
			double t2 = dot(&diff, &diff);

			Vector normal;
			if (t2 < closest_t)
			{
				closest_t = t2;
				object_index = (int)i;
				best_hit = *hit;
				switch (face)
				{
				case MIN:
					normal = box->obb_right;
					mul_ext(&normal, -1, &normal);
					break;
				case MAX:
					normal = box->obb_right;
					break;
				case BOTTOM:
					normal = box->obb_up;
					mul_ext(&normal, -1, &normal);
					break;
				case UP:
					normal = box->obb_up;
					break;
				case BACK:
					normal = box->obb_direction;
					mul_ext(&normal, -1, &normal);
					break;
				case FRONT:
					normal = box->obb_direction;
					break;
				}

				if (is_intern)
					mul_ext(&normal, -1.0f, &normal);

				*n = normal;
			}
			break;
		}
		}
	}

	if (object_index == -1)
		return false;

	*object = object_index;
	*hit = best_hit;
	return true;
}