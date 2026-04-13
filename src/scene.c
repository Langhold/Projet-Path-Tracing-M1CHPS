#include "scene.h"

#ifndef MAX_OBJECTS
#define MAX_OBJECTS 10
#endif

#define MAX_DEPTH 100

void free_scene(Scene* S){
	free_scene_objects(S);
	if(S->objects) free(S->objects);
	
}

void free_scene_objects(Scene * S){
	for (size_t i = 0; i<S->size_objects; ++i) {
		if(S->objects[i]) free(S->objects[i]->object);
		if(S->objects[i]) free(S->objects[i]);
	}
}

void create_scene_ext(size_t n_objects, const Vector * backgroundColor, Scene * s){
	s->objects = malloc(sizeof(Primitive*) * n_objects);
	if (!s->objects) {
		perror("Allocation failed.\n");
		exit(1);
	}
	s->lights = malloc(sizeof(Primitive*) * n_objects);
	if (!s->lights) {
		perror("Allocation failed.\n");
		exit(1);
	}
	for (size_t i = 0; i < n_objects; ++i) {
		s->objects[i] = NULL;
		s->lights[i] = NULL;
	}
	s->background_color = malloc(sizeof(Vector));
	mul_ext(backgroundColor, inv255, s->background_color);
	s->size_objects = n_objects;
	s->size_lights = 0;
}

void create_primitive_ext(void * shape, PRIM_TYPE type, float x, float y, float z, material_t m_type, float albedo, Vector *color, Primitive *prim){
	prim->type = type;
	prim->m_type = m_type;
	create_vector_ext(&prim->position, x, y, z);
	prim->albedo = albedo;
	prim->color = *color;
	for (size_t i = 0; i < 3; ++i) {
		prim->color.Data[i] = color->Data[i]*inv255;
	}
	prim->object = shape;
}

void add_primitive(Primitive * object, Scene * s){
	if (s == NULL || s->objects == NULL) return;
	for (size_t i = 0; i < s->size_objects; ++i) {
		if (s->objects[i] == NULL) {
			s->objects[i] = object;
			if (object->m_type == Emissive) {
				s->lights[s->size_lights] = object;
				s->size_lights++;
			}
			return;
		}
	}
	printf("Error: too many objects\n");
	exit(EXIT_FAILURE);
}

void create_sphere(Primitive* prim, const float radius, const float x, const float y, const float z, material_t m_type, float albedo, Vector *color)
{
	Sphere * sph = malloc(sizeof(Sphere));
	if (!sph) {
		perror("Allocation failed.\n");
		exit(1);
	}
	sph->radius = radius;
	
	prim->type = SPHERE;
	prim->m_type = m_type;
	create_vector_ext(&prim->position, x, y, z);
	prim->albedo = albedo;
	prim->color = *color;
	for (size_t i = 0; i < 3; ++i) {
		prim->color.Data[i] = color->Data[i]*inv255;
	}
	prim->object = (void *) sph;
	
}

void create_box(Primitive* prim, const float width, const float height, const float length, const float x, const float y, const float z, material_t m_type, float albedo, Vector *color, const float pitch, const float yaw)
{
	OBB* box = malloc(sizeof(OBB));
	
	create_vector_ext(&box->center, x, y, z);

	float cosA, sinA, cosB, sinB;
	Vector up;
	const float alpha=radian(pitch);
	const float beta=radian(yaw);
	sinA = sinf(alpha);
	cosA = cosf(alpha);
	sinB = sinf(beta);
	cosB = cosf(beta);

	box->obb_direction = (Vector) {{sinB, cosB*sinA, -cosA*cosB}};
	norm_ext(&box->obb_direction, &box->obb_direction);

	if (fabs(box->obb_direction.Data[0]) < 1-EPS) up = (Vector){{0.0f, 1.0f, 0.0f}};
	else up = (Vector){{1.0f, 0.0f, 0.0f}};
	
	cross_ext(&box->obb_direction, &up, &(box->obb_right));
	norm_ext(&(box->obb_right),&(box->obb_right));
	
	cross_ext(&(box->obb_right), &box->obb_direction, &(box->obb_up));
	norm_ext(&box->obb_up, &box->obb_up);

	create_vector_ext(&box->size, width*0.5, height*0.5, length*0.5);
	
	prim->type = BOX;
	prim->m_type = m_type;
	create_vector_ext(&prim->position, x, y, z);
	prim->albedo = albedo;
	prim->color = *color;
	for (size_t i = 0; i < 3; ++i) {
		prim->color.Data[i] = color->Data[i]*inv255;
	}
	prim->object = (void *) box;
}

void create_cube(Primitive* prim, const float width, const float x, const float y, const float z, material_t m_type, float albedo, Vector *color)
{
	AABB* box = malloc(sizeof(AABB));
	if (!box) {
		perror("Allocation failed.\n");
		exit(1);
	}
	
	box->bmin.Data[0] = x-width/2;
	box->bmin.Data[1] = y-width/2;
	box->bmin.Data[2] = z-width/2;
	
	box->bmax.Data[0] = x+width/2;
	box->bmax.Data[1] = y+width/2;
	box->bmax.Data[2] = z+width/2;
	
	prim->type = BBOX;
	prim->m_type = m_type;
	create_vector_ext(&prim->position, x, y, z);
	prim->albedo = albedo;
	prim->color = *color;
	for (size_t i = 0; i < 3; ++i) {
		prim->color.Data[i] = color->Data[i]*inv255;
	}
	prim->object = (void *) box;
}

bool intersect_sphere(Ray* const r, Vector *center, float radius, Vector *hit)
{
	Vector oc;
	sub_ext(&r->position, center, &oc);
	

	const float A = dot(&r->direction, &r->direction);
	const float B = 2.0f * dot(&r->direction, &oc);
	const float C = dot(&oc, &oc) - radius * radius;

	const float delta = B*B - 4.0f*A*C;

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

bool intersect_box(Ray* const r, const AABB* const box, Vector *hit, int *face, int *is_intern) {

	float tmin = -INFINITY;
	float tmax = INFINITY;
	int enterAxis = -1;
	int exitAxis = -1;

	for (int i = 0; i < 3; ++i) {
		float *origin = &r->position.Data[i];
		float *direction = &r->direction.Data[i];
		float minA = box->bmin.Data[i];
		float maxA = box->bmax.Data[i];

		if (fabsf(*direction) < EPS) {
			if (*origin < minA || *origin > maxA) return false;
			continue;
		}

		float invD = 1.0f / *direction;
		float t1 = (minA - *origin) * invD;
		float t2 = (maxA - *origin) * invD;
		int axisEnterCandidate = i;
		int axisExitCandidate  = i;

		float low = fminf(t1, t2);
		float high = fmaxf(t1, t2);
		
		if (low > tmin) {
			tmin = low;
			enterAxis = axisEnterCandidate;
		}
		if (high < tmax) {
			tmax = high;
			exitAxis  = axisExitCandidate;
		}
		
		if (tmin > tmax) return false;
	}

	float tHit;

	// Si l'origine est à l'intérieur de la box
	if (tmin < EPS && tmax > EPS) {
		tHit = tmax;
		if (is_intern) *is_intern = 1;
	}
	else if (tmin > EPS) {
		tHit = tmin;
		if (is_intern) *is_intern = 0;
	}
	else {
		return false;
	}

	linear_ext(&r->position, &r->direction, tHit, hit);

	int axis = (is_intern && *is_intern == 1) ? exitAxis : enterAxis;
	float dir = r->direction.Data[axis];
	switch (axis) {
		case 0: *face = (dir > 0) ? MIN : MAX; break;
		case 1: *face = (dir > 0) ? BOTTOM : UP; break;
		case 2: *face = (dir > 0) ? BACK : FRONT; break;
	}
	
	return true;
}

bool intersect_obb(Ray* const r, const OBB* const box, Vector *hit, int *face, int *is_intern) {

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
	
	if (!intersect_box(&rotated_ray, &rotated_box, &rotated_hit, &rotated_face, &rotated_inside)) return false;

	//Nous recontruisons le point d'intersection sur les trois coordonnees en partant de la position initiale du cube
	Vector box_hit = box->center;
	linear_ext(&box_hit, &box->obb_right, rotated_hit.Data[0], &box_hit);
	linear_ext(&box_hit, &box->obb_up, rotated_hit.Data[1], &box_hit);
	linear_ext(&box_hit, &box->obb_direction, rotated_hit.Data[2], &box_hit);
	*hit = box_hit;
	*face = rotated_face;
	*is_intern = rotated_inside;

	return true;
}

Vector get_normal_vector_sphere(const Vector * point, const Vector * center){
	Vector n;
	sub_ext(point, center, &n);
	norm_ext(&n,&n);
	return n;
}


Vector get_normal_vector_box(int face, int is_intern){
	if (face<0 || face>5) exit(EXIT_FAILURE);

	Vector n = {0};

	switch (face) {
		case MIN:    n.Data[0] = -1; break;  // MIN = x min => normal vers -X
		case MAX:    n.Data[0] =  1; break;  // MAX = x max => normal vers +X
		case BOTTOM: n.Data[1] = -1; break;  // Y min
		case UP:     n.Data[1] =  1; break;  // Y max
		case BACK:   n.Data[2] = -1; break;  // Z min
		case FRONT:  n.Data[2] =  1; break;  // Z max
	}

	if(is_intern) mul_ext(&n, -1.0, &n);    // inversion si l'intérieur de la box

	return n;
}


bool intersect_in_scene(struct Ray* r, const Scene* const S, int *object, Vector *hit, Vector *n){
	if(S->objects == NULL) return false;
	int object_index = -1;
	
	const Vector *origin = &r->position;
	Vector best_hit;
	double closest_t = 1e30;
	
	for (size_t i = 0; i < S->size_objects; ++i) {
		if(S->objects[i] == NULL) continue;
		
		switch (S->objects[i]->type) {
			case SPHERE:{
				const Sphere *sp = (Sphere *)S->objects[i]->object;
				
				if (!intersect_sphere(r,&S->objects[i]->position, sp->radius, hit))
					continue;
				Vector diff;
				sub_ext(hit, origin, &diff);
				double t2 = dot(&diff, &diff);
				
				if (t2 < closest_t) {
					closest_t = t2;
					object_index = (int)i;
					best_hit = *hit;
					*n = get_normal_vector_sphere(hit, &S->objects[i]->position);
				}
				break;
			}
				
			case BBOX: {
				AABB *box = (AABB *)S->objects[i]->object;
				int face;
				int is_intern;

				if (!intersect_box(r, box, hit, &face, &is_intern))
					continue;

				Vector diff;
				sub_ext(hit, origin, &diff);
				double t2 = dot(&diff, &diff);

				if (t2 < closest_t) {
					closest_t = t2;
					object_index = (int) i;
					best_hit = *hit;
					*n = get_normal_vector_box(face, is_intern);
				}
				break;
			}
			case BOX: {
				OBB *box = (OBB *)S->objects[i]->object;
				int face;
				int is_intern;

				if (!intersect_obb(r, box, hit, &face, &is_intern))
					continue;

				Vector diff;
				sub_ext(hit, origin, &diff);
				double t2 = dot(&diff, &diff);

				Vector normal;
				if (t2 < closest_t) {
					closest_t = t2;
					object_index = (int) i;
					best_hit = *hit;
					switch (face) {
					case MIN:    normal = box->obb_right;   mul_ext(&normal, -1, &normal); break;
					case MAX:    normal = box->obb_right;   break;
					case BOTTOM: normal = box->obb_up;      mul_ext(&normal, -1, &normal); break;
					case UP:     normal = box->obb_up;      break;
					case BACK:   normal = box->obb_direction; mul_ext(&normal, -1, &normal); break;
					case FRONT:  normal = box->obb_direction; break;
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



void centeroid_aabb(AABB const* box, Vector* centeroid){
	if (!box) return;
	if (!centeroid) return;
	
	*centeroid = (Vector){{(box->bmax.Data[0] + box->bmin.Data[0]) * 0.5f,
		(box->bmax.Data[1] + box->bmin.Data[1]) * 0.5f,
		(box->bmax.Data[2] + box->bmin.Data[2]) * 0.5f}};
}

AABB compute_aabb_sphere(Primitive* p){
	if (!p) exit(1);
	if (p->type != SPHERE) exit(1);
	Sphere s = *(Sphere *)p->object;
	
	AABB box;
	box.bmin.Data[0] = p->position.Data[0] - s.radius;
	box.bmin.Data[1] = p->position.Data[1] - s.radius;
	box.bmin.Data[2] = p->position.Data[2] - s.radius;
	
	box.bmax.Data[0] = p->position.Data[0] + s.radius;
	box.bmax.Data[1] = p->position.Data[1] + s.radius;
	box.bmax.Data[2] = p->position.Data[2] + s.radius;
	
	return box;
}

AABB compute_aabb_bbox(Primitive* p){
	if (!p) exit(1);
	if (p->type != BOX) exit(1);
	OBB* b = (OBB *)p->object;
	
	AABB box;
	
	box.bmin.Data[0] = p->position.Data[0] - b->size.Data[0] * fabsf(b->obb_right.Data[0]) - b->size.Data[1] * fabsf(b->obb_up.Data[0]) - b->size.Data[2] * fabsf(b->obb_direction.Data[0]);
	box.bmin.Data[1] = p->position.Data[1] - b->size.Data[0] * fabsf(b->obb_right.Data[1]) - b->size.Data[1] * fabsf(b->obb_up.Data[1]) - b->size.Data[2] * fabsf(b->obb_direction.Data[1]);
	box.bmin.Data[2] = p->position.Data[2] - b->size.Data[0] * fabsf(b->obb_right.Data[2]) - b->size.Data[1] * fabsf(b->obb_up.Data[2]) - b->size.Data[2] * fabsf(b->obb_direction.Data[2]);
	box.bmax.Data[0] = p->position.Data[0] + b->size.Data[0] * fabsf(b->obb_right.Data[0]) + b->size.Data[1] * fabsf(b->obb_up.Data[0]) + b->size.Data[2] * fabsf(b->obb_direction.Data[0]);
	box.bmax.Data[1] = p->position.Data[1] + b->size.Data[0] * fabsf(b->obb_right.Data[1]) + b->size.Data[1] * fabsf(b->obb_up.Data[1]) + b->size.Data[2] * fabsf(b->obb_direction.Data[1]);
	box.bmax.Data[2] = p->position.Data[2] + b->size.Data[0] * fabsf(b->obb_right.Data[2]) + b->size.Data[1] * fabsf(b->obb_up.Data[2]) + b->size.Data[2] * fabsf(b->obb_direction.Data[2]);
	
	
	return box;
}

AABB compute_hitbox(Primitive* p){
	if(!p) return NULL_AABB;
	switch (p->type) {
		case SPHERE:
			return compute_aabb_sphere(p);
			break;
		case BBOX:
			return *(AABB *) p->object;
			break;
		case BOX:
			return compute_aabb_bbox(p);
			break;
	}
	return NULL_AABB;
}


void add_node(object_tree_t* node, int const side){
	if (!node) return;
	if (side != 0 && side != 1) return;
	
	object_tree_t* node_new = malloc(sizeof(object_tree_t));
	if (!node_new) {
		perror("Allocation failed.\n");
		exit(1);
	}
	
	if (!side) node->left = node_new;
	else node->right = node_new;
	
	node_new->box = NULL_AABB;
	node_new->objects = malloc(sizeof(Primitive*) * MAX_OBJECTS);
	if (!node_new->objects) {
		perror("Allocation failed.\n");
		exit(1);
	}
	node_new->left = NULL;
	node_new->right = NULL;
	node_new->objects_count = 0;
	
	
}

void update_aabb_from_children(object_tree_t* node) {
	if (!node) return;

	node->box = NULL_AABB;

	if (node->left) {
		for (int i = 0; i < 3; i++) {
			node->box.bmin.Data[i] = fminf(node->box.bmin.Data[i], node->left->box.bmin.Data[i]);
			node->box.bmax.Data[i] = fmaxf(node->box.bmax.Data[i], node->left->box.bmax.Data[i]);
		}
	}
	if (node->right) {
		for (int i = 0; i < 3; i++) {
			node->box.bmin.Data[i] = fminf(node->box.bmin.Data[i], node->right->box.bmin.Data[i]);
			node->box.bmax.Data[i] = fmaxf(node->box.bmax.Data[i], node->right->box.bmax.Data[i]);
		}
	}

	// Inclure les objets restants dans ce node (si c’est une feuille)
	for (int i = 0; i < node->objects_count; i++) {
		AABB obj_box = compute_hitbox(node->objects[i]);
		for (int j = 0; j < 3; j++) {
			node->box.bmin.Data[j] = fminf(node->box.bmin.Data[j], obj_box.bmin.Data[j]);
			node->box.bmax.Data[j] = fmaxf(node->box.bmax.Data[j], obj_box.bmax.Data[j]);
		}
	}
}


void add_object_to_node(object_tree_t* node, Primitive* p, int depth) {
	if (!node || !p) return;

	AABB p_hitbox = compute_hitbox(p);

	if ((!node->left && !node->right) && (node->objects_count < MAX_OBJECTS || depth >= MAX_DEPTH)) {
		if (!node->objects) {
			node->objects = malloc(sizeof(Primitive*) * MAX_OBJECTS);
			if (!node->objects) { perror("Allocation failed.\n"); exit(1); }
		}
		node->objects[node->objects_count++] = p;

		for (int i = 0; i < 3; i++) {
			node->box.bmin.Data[i] = fminf(node->box.bmin.Data[i], p_hitbox.bmin.Data[i]);
			node->box.bmax.Data[i] = fmaxf(node->box.bmax.Data[i], p_hitbox.bmax.Data[i]);
		}
		return;
	}

	if (!node->left) add_node(node, 0);
	if (!node->right) add_node(node, 1);

	float size[3] = {
		node->box.bmax.Data[0] - node->box.bmin.Data[0],
		node->box.bmax.Data[1] - node->box.bmin.Data[1],
		node->box.bmax.Data[2] - node->box.bmin.Data[2]
	};
	int axis = 0;
	if (size[1] > size[0] && size[1] >= size[2]) axis = 1;
	else if (size[2] > size[0] && size[2] > size[1]) axis = 2;

	float split = (node->box.bmin.Data[axis] + node->box.bmax.Data[axis]) * 0.5f;

	
	Vector centroid;
	centeroid_aabb(&p_hitbox, &centroid);
	float min = p_hitbox.bmin.Data[axis];
	float max = p_hitbox.bmax.Data[axis];

	
	if (max <= split) {
		add_object_to_node(node->left, p, depth + 1);
	}
	else if (min >= split) {
		add_object_to_node(node->right, p, depth + 1);
	}
	else {
		add_object_to_node(node->left, p, depth + 1);
		add_object_to_node(node->right, p, depth + 1);
	}

	update_aabb_from_children(node);
}

typedef struct obj_arrange{
	Primitive* p;
	float coord;
	AABB hitbox;
}obj_arrange;

int compare_float(const void* a, const void* b) {
	float fa = *(float*)a;
	float fb = *(float*)b;
	return (fa > fb) - (fa < fb);
}

int compare(const void* a, const void* b) {
	const obj_arrange* oa = (const obj_arrange*)a;
	const obj_arrange* ob = (const obj_arrange*)b;

	if (oa->coord < ob->coord) return -1;
	if (oa->coord > ob->coord) return 1;
	return 0;
}

int overlaps_with_split(AABB* hb, int axis, float split) {
	return (hb->bmin.Data[axis] < split && hb->bmax.Data[axis] >= split);
}

void add_object_to_node_v2(object_tree_t* node, Primitive** p, int object_count, int depth) {
	if (!node || !p) return;

	for (int j = 0; j < object_count; ++j) {
		AABB p_hitbox = compute_hitbox(p[j]);
		for (int i = 0; i < 3; i++) {
			node->box.bmin.Data[i] = fminf(node->box.bmin.Data[i], p_hitbox.bmin.Data[i]);
			node->box.bmax.Data[i] = fmaxf(node->box.bmax.Data[i], p_hitbox.bmax.Data[i]);
		}
	}

	if (object_count <= MAX_OBJECTS) {
		node->objects_count = 0;
		for (int i = 0; i < object_count; ++i) {
			if (p[i] && p[i]->object)
				node->objects[node->objects_count++] = p[i];
		}
		return;
	}

	add_node(node, 0);
	add_node(node, 1);

	float size[3] = {
		node->box.bmax.Data[0] - node->box.bmin.Data[0],
		node->box.bmax.Data[1] - node->box.bmin.Data[1],
		node->box.bmax.Data[2] - node->box.bmin.Data[2]
	};
	int axis = 0;
	if (size[1] > size[0] && size[1] >= size[2]) axis = 1;
	else if (size[2] > size[0] && size[2] > size[1]) axis = 2;

	obj_arrange* references = malloc(sizeof(obj_arrange) * object_count);
	for (int i = 0; i < object_count; ++i) {
		AABB p_hitbox = compute_hitbox(p[i]);
		references[i].p = p[i];
		references[i].coord = (p_hitbox.bmin.Data[axis] + p_hitbox.bmax.Data[axis]) * 0.5f;
		references[i].hitbox = p_hitbox;
	}
	qsort(references, object_count, sizeof(obj_arrange), &compare);

	float split = references[object_count / 2].coord;

	int n_left = 0, n_right = 0;
	for (int i = 0; i < object_count; ++i) {
		Vector centroid;
		centeroid_aabb(&references[i].hitbox, &centroid);

		// Si l’objet chevauche le split, il peut aller dans les deux enfants
		if ((centroid.Data[axis] < split)) {
			n_left++;
		}
		else  {
			n_right++;
		}
	}
	
	float ratio_min = fminf(n_left, n_right) / (float) object_count;
	float ratio_max = fmaxf(n_left, n_right) / (float) object_count;
	if (ratio_min < 0.2f || (ratio_max > 0.9f)) {
		n_left = object_count / 2;
		n_right = object_count / 2 + object_count % 2;
	}

	Primitive** buffer_left = malloc(sizeof(Primitive*) * n_left);
	Primitive** buffer_right = malloc(sizeof(Primitive*) * n_right);
	if (ratio_min < 0.2f || (ratio_max > 0.9f)) {
		int i = 0;
		int l = 0, r = 0;
		for (;i < object_count/2; ++i) {
			buffer_left[l++] = references[i].p;
		}
		for (;i < object_count; ++i) {
			buffer_right[r++] = references[i].p;
		}
		
		add_object_to_node_v2(node->left, buffer_left, n_left, depth + 1);
		add_object_to_node_v2(node->right, buffer_right, n_right, depth + 1);

		update_aabb_from_children(node);

		free(buffer_left);
		free(buffer_right);
		free(references);
		return;
	}
	int l = 0, r = 0;
	
	for (int i = 0; i < object_count; ++i) {
		Vector centroid;
		centeroid_aabb(&references[i].hitbox, &centroid);
		
		if ((centroid.Data[axis] < split)) {
			buffer_left[l++] = references[i].p;
		}
		
		else  {
			buffer_right[r++] = references[i].p;
		}
	}
	

	if (depth > MAX_DEPTH)
		abort();
	add_object_to_node_v2(node->left, buffer_left, n_left, depth + 1);
	add_object_to_node_v2(node->right, buffer_right, n_right, depth + 1);

	update_aabb_from_children(node);

	free(buffer_left);
	free(buffer_right);
	free(references);
}

void print_tree(object_tree_t* node, int depth) {
	if (!node) return;

	// Affichage des objets si c’est une feuille
	if (node->objects_count > 0 && node->objects) {
		for (int i = 0; i < node->objects_count; i++) {
			Primitive* p = node->objects[i];
			if (!p) continue;
		}
	}

	// Appel récursif sur les enfants
	if (node->left) print_tree(node->left, depth + 1);
	if (node->right) print_tree(node->right, depth + 1);
}

// Wrapper pour appeler facilement
void print_tree_root(object_tree_t* root) {
	print_tree(root, 0);
}

object_tree_t* initialize_root_tree(Scene* S) {
	if (S == NULL) return NULL;
	object_tree_t* root = malloc(sizeof(object_tree_t));
	if (!root) {
		perror("Tree malloc error.\n");
		exit(1);
	}
	
	root->box = NULL_AABB;
	root->left = NULL;
	root->right = NULL;
	root->objects = malloc(sizeof(Primitive*) * MAX_OBJECTS);
	if (!root->objects) {
		perror("Tree malloc error.\n");
		exit(1);
	}
	root->objects_count = 0;

	for (size_t i = 0; i < S->size_objects; ++i) {
		Primitive* p = S->objects[i];
		if (p == NULL) {
			continue;
		}
		if ((p->type == SPHERE && !p->object)) {
			fprintf(stderr, "Scene contains invalid sphere at index %zu\n", i);
			exit(1);
		}
		add_object_to_node(root, p, 0);
		
	}
	print_tree_root(root);
	return root;
}

object_tree_t* initialize_root_tree_v2(Scene* S) {
	if (S == NULL) return NULL;
	object_tree_t* root = malloc(sizeof(object_tree_t));
	if (!root) {
		perror("Tree malloc error.\n");
		exit(1);
	}
	root->box = NULL_AABB;
	root->left = NULL;
	root->right = NULL;
	root->objects = malloc(sizeof(Primitive*) * MAX_OBJECTS);
	if (!root->objects) {
		perror("Tree malloc error.\n");
		exit(1);
	}
	root->objects_count = 0;

	add_object_to_node_v2(root, S->objects, S->size_objects, 0);
	
	print_tree_root(root);
	return root;
}

void free_tree_objects(object_tree_t** root) {
	if (!root || !*root) return;
	free_tree_objects(&(*root)->left);
	free_tree_objects(&(*root)->right);
	if ((*root)->objects) free((*root)->objects);
	free(*root);
	*root = NULL;
}

int check_intersection_box(AABB* box, Vector* hit){
	{
		if (hit->Data[0] < box->bmin.Data[0] || hit->Data[0] > box->bmax.Data[0]) return 0;
		if (hit->Data[1] < box->bmin.Data[1] || hit->Data[1] > box->bmax.Data[1]) return 0;
		if (hit->Data[2] < box->bmin.Data[2] || hit->Data[2] > box->bmax.Data[2]) return 0;
		
		return 1;
	}
}

/*float intersect_box_t(const Ray* r, const AABB* box, int* is_intern, int* face) {
	float tmin = -FLT_MAX;
	float tmax = FLT_MAX;
	int enterAxis = -1, exitAxis = -1;

	for (int i = 0; i < 3; ++i) {
		float origin = r->position.Data[i];
		float dir = r->direction.Data[i];
		float minA = box->bmin.Data[i];
		float maxA = box->bmax.Data[i];

		if (fabsf(dir) < EPS) {
			// Rayon parallèle à l'axe
			if (origin < minA - EPS || origin > maxA + EPS)
				return -1.f;
			continue;
		}

		float invD = 1.0f / dir;
		float t1 = (minA - origin) * invD;
		float t2 = (maxA - origin) * invD;
		float low = fminf(t1, t2);
		float high = fmaxf(t1, t2);

		if (low > tmin) {
			tmin = low;
			enterAxis = i;
		}
		if (high < tmax) {
			tmax = high;
			exitAxis = i;
		}

		if (tmin > tmax + EPS)
			return -1.f;
	}

	if (tmax < EPS) return -1.f;

	if (is_intern) *is_intern = (tmin < EPS && tmax > EPS) ? 1 : 0;

	if (face) {
		int axis = (is_intern && *is_intern) ? exitAxis : enterAxis;
		float dir = r->direction.Data[axis];
		switch (axis) {
			case 0: *face = (dir > 0) ? MIN : MAX; break;
			case 1: *face = (dir > 0) ? BOTTOM : UP; break;
			case 2: *face = (dir > 0) ? BACK : FRONT; break;
		}
	}

	return (tmin > EPS) ? tmin : tmax;
}*/

float intersect_box_t(const Ray* r, const AABB* box, int* is_intern, int* face) {
	float tmin = -FLT_MAX;
	float tmax = FLT_MAX;
	int enterAxis = -1, exitAxis = -1;

	for (int i = 0; i < 3; ++i) {
		float origin = r->position.Data[i];
		float dir = r->direction.Data[i];
		float minA = box->bmin.Data[i];
		float maxA = box->bmax.Data[i];

		if (fabsf(dir) < EPS) {
			// Rayon parallèle à l'axe
			if (origin < minA - EPS || origin > maxA + EPS)
				return -1.f; // Pas d'intersection
			continue; // Le rayon reste dans la tranche pour cet axe
		}

		float invD = 1.0f / dir;
		float t1 = (minA - origin) * invD;
		float t2 = (maxA - origin) * invD;

		float low = fminf(t1, t2);
		float high = fmaxf(t1, t2);

		if (low > tmin) {
			tmin = low;
			enterAxis = i; // Axe par lequel le rayon entre
		}
		if (high < tmax) {
			tmax = high;
			exitAxis = i; // Axe par lequel le rayon sort
		}
		if (tmax < 0.f) return -1.f;
		if (tmin > tmax + EPS) return -1.f; // Pas d'intersection
	}

	if (tmax < EPS) return -1.f; // Intersection derrière le rayon

	// Déterminer si le rayon commence à l'intérieur
	if (is_intern) *is_intern = (tmin < EPS && tmax > EPS) ? 1 : 0;

	// Choisir la face correcte
	if (face) {
		int axis = (is_intern && *is_intern) ? exitAxis : enterAxis;
		float dir = r->direction.Data[axis];
		switch (axis) {
			case 0: *face = (dir > 0) ? MIN : MAX; break;
			case 1: *face = (dir > 0) ? BOTTOM : UP; break;
			case 2: *face = (dir > 0) ? BACK : FRONT; break;
		}
	}

	// Retourner la première intersection positive
	return (tmin > EPS) ? tmin : tmax;
}

float intersect_sphere_t(const Ray* r, Vector* center, float radius) {
	Vector oc;
	sub_ext(&r->position, center, &oc);

	float B = 2.0f * dot(&r->direction, &oc);
	float C = dot(&oc, &oc) - radius * radius;
	float delta = B*B - 4.0f*C;

	if (delta < 0.0f) return -1.f;

	float sqrt_delta = sqrtf(delta);
	float t0 = (-B - sqrt_delta) * 0.5f;
	float t1 = (-B + sqrt_delta) * 0.5f;

	if (t0 > EPS) return t0;
	if (t1 > EPS) return t1;
	return -1.f;
}

float intersect_obb_t(const Ray* r, const OBB* box, int* is_intern, int* face) {
	// Transformer le rayon dans la base locale de l'OBB
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

	AABB local_box;
	local_box.bmin.Data[0] = -box->size.Data[0];
	local_box.bmin.Data[1] = -box->size.Data[1];
	local_box.bmin.Data[2] = -box->size.Data[2];
	local_box.bmax.Data[0] =  box->size.Data[0];
	local_box.bmax.Data[1] =  box->size.Data[1];
	local_box.bmax.Data[2] =  box->size.Data[2];

	return intersect_box_t(&rotated_ray, &local_box, is_intern, face);
}

int intersect_in_tree(object_tree_t* const tree, const Ray* r, float* closest_t, Primitive** intersected_object, int* is_intern, int* face) {
	if (!tree || !closest_t) return 0;

	int hit = 0;
	
	int node_intern = 0, node_face = -1;
	float tbox = intersect_box_t(r, &tree->box, &node_intern, &node_face);

	if (tbox < 0.f) return 0;
	

	for (int i = 0; i < tree->objects_count; ++i) {
		Primitive* p = tree->objects[i];
		if (!p) continue;

		float t = -1.f;
		int obj_intern = 0, obj_face = -1;

		switch (p->type) {
			case SPHERE:
				t = intersect_sphere_t(r, &p->position, ((Sphere*)p->object)->radius);
				break;

			case BBOX:
				t = intersect_box_t(r, (AABB*)p->object, &obj_intern, &obj_face);
				break;

			case BOX:
				t = intersect_obb_t(r, (OBB*)p->object, &obj_intern, &obj_face);
				break;
		}

		if (t > EPS && t < *closest_t) {
			*closest_t = t;
			*intersected_object = p;

			if (is_intern) *is_intern = obj_intern;
			if (face) *face = obj_face;

			hit = 1;
		}
	}
	if (tree->objects_count > 0.f) return hit;
	
	float tleft = -1.f, tright = -1.f;

	tleft = intersect_box_t(r, &tree->left->box, NULL, NULL);

	tright = intersect_box_t(r, &tree->right->box, NULL, NULL);

	if (tleft < EPS) tleft = -1.f;
	if (tright < EPS) tright = -1.f;
	
	// aucun hit dans les noeuds suivants
	if (tleft < 0.f && tright < 0.f)
		return hit;

	object_tree_t* first = NULL;
	object_tree_t* second = NULL;
	float tfirst, tsecond;

	if (tleft >= 0.f && (tleft < tright || tright < 0.f)) {
		first = tree->left;
		second = tree->right;
		tfirst = tleft;
		tsecond = tright;
	} else {
		first = tree->right;
		second = tree->left;
		tfirst = tright;
		tsecond = tleft;
	}

	if (first && tfirst >= 0.f/* && 1e-1*tfirst < *closest_t*/)
		hit |= intersect_in_tree(first, r, closest_t, intersected_object, is_intern, face);

	if (second && tsecond >= 0.f/* && 1e-1*tsecond < *closest_t */)
		hit |= intersect_in_tree(second, r, closest_t, intersected_object, is_intern, face);
	return hit;
}

Ray random_Ray_demi_sphere_cosine_weighted(const Vector * origin, const Vector * normal, unsigned int* seed){
	
	const float u1 = rand_r(seed) / (float)RAND_MAX;
	const float u2 = rand_r(seed) / (float)RAND_MAX;
	const float atheta = sqrtf(u1);
	const float phi = 2*M_PI*u2;
	//les coordonnées dans la base locale
	float x, y;
	y = sinf(phi);
	x = cosf(phi);
	x *= atheta;
	y *= atheta;
	const float z = sqrtf(1 - u1);
	
	//coordonnées dans la base orthonormée (up,right,forward) https://www.opengl-tutorial.org/fr/intermediate-tutorials/tutorial-13-normal-mapping/
	Vector up;
	Vector norm;
	Ray ray;
	ray.position = *origin;
	
	if (fabs((*normal).Data[1])<1.0f-EPS){
		create_vector_ext(&up, 0, 1, 0);
	}
	else{
		create_vector_ext(&up, 1, 0, 0);
	}
	
	Vector tangent;
	cross_ext(normal, &up, &tangent);
    norm_ext(&tangent, &tangent);
	
	Vector bitangent;
	cross_ext(&tangent, normal, &bitangent);
	
	mul_ext(&tangent, x, &tangent);
	mul_ext(&bitangent, y, &bitangent);
	mul_ext(normal, z, &norm);
	
	add_ext(&tangent, &bitangent, &ray.direction);
	add_ext(&ray.direction, &norm, &ray.direction);
	
	return ray;
}

void trace_light_ray(size_t size_lights, Primitive ** light, Vertex * light_path, unsigned int* seed) {
	//Choose a random light source depending on intensity (albedo)
	if (size_lights <= 0) {
		return;
	}
	float sum_intensity = 0;
	for (size_t i=0; i<size_lights; ++i) {
		sum_intensity += light[i]->albedo;
	}
	float P[size_lights];
	float sum = 0;
	for (size_t i=0; i<size_lights; ++i) {
		sum += light[i]->albedo;
		P[i] = sum / sum_intensity;
	}
	float p = rand_r(seed) / (float)RAND_MAX;
	size_t chosen = 0;
	for (size_t i=0; i<size_lights; ++i) {
		if (p <= P[i]) {
			chosen = i;
			break;
		}
	}

	light_path[0].object = light[chosen];
	light_path[0].object->albedo = light[chosen]->albedo;
	light_path[0].object->color = light[chosen]->color;
	light_path[0].object->m_type = light[chosen]->m_type;
	light_path[0].object->object = light[chosen]->object;
	light_path[0].object->position = light[chosen]->position;
	light_path[0].object->type = light[chosen]->type;

	light_path[0].position = light[chosen]->position; //Centre de l'objet
	light_path[0].throughput.Data[0] = light[chosen]->color.Data[0] * sum_intensity;
	light_path[0].throughput.Data[1] = light[chosen]->color.Data[1] * sum_intensity;
	light_path[0].throughput.Data[2] = light[chosen]->color.Data[2] * sum_intensity;

	switch (light[chosen]->type) {
		case SPHERE: {
			const float u1 = rand_r(seed) / (float)RAND_MAX;
			const float u2 = rand_r(seed) / (float)RAND_MAX;
			const float phi = 2.0f*M_PI*u2;
			const float z = 2.0f*u1-1.0f;
			const float r = sqrtf(1.0f-z*z);

			Sphere *sph = (Sphere *)light[chosen]->object;
			light_path[0].position.Data[0] += r * cosf(phi) * sph->radius ;
			light_path[0].position.Data[1] += r * sinf(phi) * sph->radius ;
			light_path[0].position.Data[2] += z * sph->radius ;
			light_path[0].throughput.Data[0] /= 4*M_PI*sph->radius*sph->radius;
			light_path[0].throughput.Data[1] /= 4*M_PI*sph->radius*sph->radius;
			light_path[0].throughput.Data[2] /= 4*M_PI*sph->radius*sph->radius;

			sub_ext(&light_path[0].position, &light[chosen]->position, &light_path[0].normal);
			norm_ext(&light_path[0].normal, &light_path[0].normal);
			break;
		}
		case BOX :
		case BBOX: {
			OBB *box = (OBB *)light[chosen]->object;
			//random face
			int face = (int)(rand_r(seed) % 6) ;
			//random position on the face
			float w = (rand_r(seed) / (float)RAND_MAX) * 2.0f * box->size.Data[0] - box->size.Data[0];
			float l = (rand_r(seed) / (float)RAND_MAX) * 2.0f * box->size.Data[1] - box->size.Data[1];
			float h = (rand_r(seed) / (float)RAND_MAX) * 2.0f * box->size.Data[2] - box->size.Data[2];
			switch (face) {
				case 0: {
					//MIN				
					light_path[0].position.Data[0] += box->obb_direction.Data[0] * box->size.Data[2] + box->obb_right.Data[0]*l + box->obb_up.Data[0]*h;
					light_path[0].position.Data[1] += box->obb_direction.Data[1] * box->size.Data[2] + box->obb_right.Data[1] * l + box->obb_up.Data[1]*h ;
					light_path[0].position.Data[2] += box->obb_direction.Data[2] * box->size.Data[2] + box->obb_right.Data[2] * l + box->obb_up.Data[2]*h ;
	
					light_path[0].normal = box->obb_direction;
					light_path[0].throughput.Data[0] /= 4*box->size.Data[1]*box->size.Data[2]*6 ;
					light_path[0].throughput.Data[1] /= 4*box->size.Data[1]*box->size.Data[2]*6 ;
					light_path[0].throughput.Data[2] /= 4*box->size.Data[1]*box->size.Data[2]*6 ;
					break;
				}
				case 1: {
					//MAX
					light_path[0].position.Data[0] += -box->obb_direction.Data[0] * box->size.Data[2] + box->obb_right.Data[0]*l + box->obb_up.Data[0]*h ;
					light_path[0].position.Data[1] += -box->obb_direction.Data[1] * box->size.Data[2] + box->obb_right.Data[1]*l + box->obb_up.Data[1]*h ;
					light_path[0].position.Data[2] += -box->obb_direction.Data[2] * box->size.Data[2] + box->obb_right.Data[2]*l + box->obb_up.Data[2]*h ;

					mul_ext(&box->obb_direction, -1.0f, &light_path[0].normal);
					light_path[0].throughput.Data[0] /= 4*box->size.Data[1]*box->size.Data[2]*6 ;
					light_path[0].throughput.Data[1] /= 4*box->size.Data[1]*box->size.Data[2]*6 ;
					light_path[0].throughput.Data[2] /= 4*box->size.Data[1]*box->size.Data[2]*6 ;
					break;
				}
				case 2: {
					//BOTTOM
					light_path[0].position.Data[0] += box->obb_up.Data[0] * box->size.Data[1] + box->obb_direction.Data[0]*w + box->obb_right.Data[0]*l ;
					light_path[0].position.Data[1] += box->obb_up.Data[1] * box->size.Data[1] + box->obb_direction.Data[1]*w + box->obb_right.Data[1]*l ;
					light_path[0].position.Data[2] += box->obb_up.Data[2] * box->size.Data[1] + box->obb_direction.Data[2]*w + box->obb_right.Data[2]*l ;

					light_path[0].normal = box->obb_up;
					light_path[0].throughput.Data[0] /= 4*box->size.Data[0]*box->size.Data[2]*6 ;
					light_path[0].throughput.Data[1] /= 4*box->size.Data[0]*box->size.Data[2]*6 ;
					light_path[0].throughput.Data[2] /= 4*box->size.Data[0]*box->size.Data[2]*6 ;
					break;
				}
				case 3: {
					//UP
					light_path[0].position.Data[0] += -box->obb_up.Data[0] * box->size.Data[1] + box->obb_direction.Data[0]*w + box->obb_right.Data[0]*l ;
					light_path[0].position.Data[1] += -box->obb_up.Data[1] * box->size.Data[1] + box->obb_direction.Data[1]*w + box->obb_right.Data[1]*l ;
					light_path[0].position.Data[2] += -box->obb_up.Data[2] * box->size.Data[1] + box->obb_direction.Data[2]*w + box->obb_right.Data[2]*l ;

					mul_ext(&box->obb_up, -1.0f, &light_path[0].normal);
					light_path[0].throughput.Data[0] /= 4*box->size.Data[0]*box->size.Data[2]*6 ;
					light_path[0].throughput.Data[1] /= 4*box->size.Data[0]*box->size.Data[2]*6 ;
					light_path[0].throughput.Data[2] /= 4*box->size.Data[0]*box->size.Data[2]*6 ;
					break;
				}
				case 4: {
					//RIGHT
					light_path[0].position.Data[0] += box->obb_right.Data[0] * box->size.Data[0] + box->obb_direction.Data[0]*w + box->obb_up.Data[0]*h ;
					light_path[0].position.Data[1] += box->obb_right.Data[1] * box->size.Data[0] + box->obb_direction.Data[1]*w + box->obb_up.Data[1]*h ;
					light_path[0].position.Data[2] += box->obb_right.Data[2] * box->size.Data[0] + box->obb_direction.Data[2]*w + box->obb_up.Data[2]*h ;

					light_path[0].normal = box->obb_right;
					light_path[0].throughput.Data[0] /= 4*box->size.Data[0]*box->size.Data[1]*6 ;
					light_path[0].throughput.Data[1] /= 4*box->size.Data[0]*box->size.Data[1]*6 ;
					light_path[0].throughput.Data[2] /= 4*box->size.Data[0]*box->size.Data[1]*6 ;
					break;
				}
				case 5: {
					//LEFT
					light_path[0].position.Data[0] += -box->obb_right.Data[0] * box->size.Data[0] + box->obb_direction.Data[0]*w + box->obb_up.Data[0]*h ;
					light_path[0].position.Data[1] += -box->obb_right.Data[1] * box->size.Data[0] + box->obb_direction.Data[1]*w + box->obb_up.Data[1]*h ;
					light_path[0].position.Data[2] += -box->obb_right.Data[2] * box->size.Data[0] + box->obb_direction.Data[2]*w + box->obb_up.Data[2]*h ;
					
					mul_ext(&box->obb_right, -1.0f, &light_path[0].normal);
					light_path[0].throughput.Data[0] /= 4*box->size.Data[0]*box->size.Data[1]*6 ;
					light_path[0].throughput.Data[1] /= 4*box->size.Data[0]*box->size.Data[1]*6 ;
					light_path[0].throughput.Data[2] /= 4*box->size.Data[0]*box->size.Data[1]*6 ;
					break;
				}
				norm_ext(&light_path[0].normal, &light_path[0].normal);
			}
			break;
		}
	}

	Ray r_new = random_Ray_demi_sphere_cosine_weighted(&light_path[0].position, &light_path[0].normal, seed);
	light_path[0].throughput.Data[0] *= M_PI / dot(&r_new.direction, &light_path[0].normal);
	light_path[0].throughput.Data[1] *= M_PI / dot(&r_new.direction, &light_path[0].normal);
	light_path[0].throughput.Data[2] *= M_PI / dot(&r_new.direction, &light_path[0].normal);
	light_path[0].direction = r_new.direction;
	light_path[0].is_light = 1;
	light_path[0].wo = r_new.direction;
}