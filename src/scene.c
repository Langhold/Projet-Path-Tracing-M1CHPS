#include "scene.h"

#ifndef MAX_OBJECTS
#define MAX_OBJECTS 10
#endif

#define MAX_DEPTH 100
#define MAX_ITER 10

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
	for (size_t i = 0; i < n_objects; ++i) {
		s->objects[i] = NULL;
	}
	s->background_color = malloc(sizeof(Vector));
	mul_ext(backgroundColor, inv255, s->background_color);
	s->size_objects = n_objects;
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


/*###################################################################################################*/
/*###################################################################################################*/
/*###################################################################################################*/
/*###################################################################################################*/
/*###################################################################################################*/



void centeroid_aabb(AABB const* box, Vector* centeroid){
	if (!box) return;
	if (!centeroid) return;
	
	*centeroid = (Vector){{(box->bmax.Data[0] + box->bmin.Data[0]) * 0.5f,
		(box->bmax.Data[1] + box->bmin.Data[1]) * 0.5f,
		(box->bmax.Data[2] + box->bmin.Data[2]) * 0.5f}};
}

AABB compute_aabb_sphere(const Primitive* p){
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

AABB compute_aabb_bbox(const Primitive* p){
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

AABB compute_hitbox(const Primitive* p){
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

	// Indentation selon la profondeur
	for (int i = 0; i < depth; i++) printf("  ");

	// Affichage du noeud
	printf("Node (depth %d): AABB [min=(%.2f, %.2f, %.2f) max=(%.2f, %.2f, %.2f)]\n",
		   depth,
		   node->box.bmin.Data[0], node->box.bmin.Data[1], node->box.bmin.Data[2],
		   node->box.bmax.Data[0], node->box.bmax.Data[1], node->box.bmax.Data[2]);

	// Affichage des objets si c’est une feuille
	if (node->objects_count > 0 && node->objects) {
		for (int i = 0; i < node->objects_count; i++) {
			Primitive* p = node->objects[i];
			for (int j = 0; j < depth; j++) printf("  ");
			if (!p) continue;
			printf("  Object %d: type=%d, position=(%.2f, %.2f, %.2f)\n",
				   i, p->type, p->position.Data[0], p->position.Data[1], p->position.Data[2]);
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

	if (!is_intern && !face) return (tmin > EPS) ? tmin : tmax;
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

float intersect_AABB_t(const Ray* r, const AABB* box) {
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
	
	float tbox = intersect_AABB_t(r, &tree->box);

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
	tleft = intersect_AABB_t(r, &tree->left->box);
	tright = intersect_AABB_t(r, &tree->right->box);

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

static inline float dist2(const Vector* a, const Vector* b) {
	float dx = a->Data[0] - b->Data[0];
	float dy = a->Data[1] - b->Data[1];
	float dz = a->Data[2] - b->Data[2];
	return dx*dx + dy*dy + dz*dz;
}


/*###################################################################################################*/
/*###################################################################################################*/
/*###################################################################################################*/
/*###################################################################################################*/
/*###################################################################################################*/


typedef struct cluster_t{
	int count;
	Vector centroid;
	Vector sum;
	Primitive** objects;
	AABB box;
} cluster_t;


AABB compute_aabb_cluster(const cluster_t* cluster){
	AABB hb = NULL_AABB;
	for (int i = 0; i<cluster->count; ++i) {
		Primitive* current_object = cluster->objects[i];
		AABB obj_hb = compute_hitbox(current_object);
		hb.bmin.Data[0] = fminf(obj_hb.bmin.Data[0], hb.bmin.Data[0]);
		hb.bmin.Data[1] = fminf(obj_hb.bmin.Data[1], hb.bmin.Data[1]);
		hb.bmin.Data[2] = fminf(obj_hb.bmin.Data[2], hb.bmin.Data[2]);
		hb.bmax.Data[0] = fmaxf(obj_hb.bmax.Data[0], hb.bmax.Data[0]);
		hb.bmax.Data[1] = fmaxf(obj_hb.bmax.Data[1], hb.bmax.Data[1]);
		hb.bmax.Data[2] = fmaxf(obj_hb.bmax.Data[2], hb.bmax.Data[2]);
	}
	return hb;
}

void kmeans(int K, int N, const Primitive** Objects, cluster_t* clusters, unsigned int* seed){
	if (!clusters || !Objects || N == 0 || K <= 0) return;

	Vector centroids[N];
	unsigned int labels[N], all_index[K];
	
	for (int i = 0; i < N; ++i) {
		AABB box = compute_hitbox(Objects[i]);
		centeroid_aabb(&box, &centroids[i]);
	}

	// Initialise le centroid du cluster comme étant celui d'un objets aléatoire de la scene qui n'a pas encore servi pour un autre
	for (int k = 0; k < K; ++k) {
		clusters[k].objects = malloc(N * sizeof(Primitive*));
		clusters[k].count = 0;

		int unique = 0;

		while (!unique) {
			unsigned int index = rand_r(seed) % N;
			Vector candidate = centroids[index];
			all_index[k] = index;
			unique = 1;

			for (int j = 0; j < k; ++j) {
				if (all_index[j] == index) {
					unique = 0;
					break;
				}
			}

			if (unique) {
				clusters[k].centroid = candidate;
			}
		}
	}


	for (int iter = 0; iter < MAX_ITER; ++iter) {

		for (int i = 0; i < N; ++i) {

			float best_dist = 1e30f;
			int best_k = 0;

			for (int k = 0; k < K; ++k) {
				float d = dist2(&centroids[i], &clusters[k].centroid);

				if (d < best_dist) {
					best_dist = d;
					best_k = k;
				}
			}

			labels[i] = best_k;
		}

		for (int k = 0; k < K; ++k) {
			clusters[k].sum = (Vector){{0.f, 0.f, 0.f}};
			clusters[k].count = 0;
		}

		for (int i = 0; i < N; ++i) {
			int k = labels[i];

			clusters[k].sum.Data[0] += centroids[i].Data[0];
			clusters[k].sum.Data[1] += centroids[i].Data[1];
			clusters[k].sum.Data[2] += centroids[i].Data[2];

			clusters[k].objects[clusters[k].count++] = (Primitive*)Objects[i];
		}

		for (int k = 0; k < K; ++k) {

			if (clusters[k].count > 0) {
				clusters[k].centroid.Data[0] = clusters[k].sum.Data[0] / clusters[k].count;
				clusters[k].centroid.Data[1] = clusters[k].sum.Data[1] / clusters[k].count;
				clusters[k].centroid.Data[2] = clusters[k].sum.Data[2] / clusters[k].count;
			} else {
				clusters[k].centroid = centroids[rand_r(seed) % N];
			}
		}
	}
	for (int k = 0; k < K; ++k) {
		clusters[k].box = compute_aabb_cluster(&clusters[k]);
	}
}

Large_BVH_t* initialize_root_tree_clustering(const Primitive** Objects, const int N, const int K, unsigned int* seed) {
	if (!Objects || K<=0) return NULL;
	cluster_t clusters[K];
	
	Large_BVH_t* root = malloc(sizeof(Large_BVH_t));
	if(!root){
		perror("Allocation failed for tree\n");
		abort();
	}
	root->clusters = calloc(K, sizeof(object_tree_t*));
	if(!root->clusters){
		perror("Allocation failed clusters for tree\n");
		abort();
	}
	kmeans(K, N, Objects, clusters, seed);
	AABB box = NULL_AABB;
	for (int k = 0; k<K; ++k) {
		root->clusters[k] = malloc(sizeof(object_tree_t));
		root->clusters[k]->box = NULL_AABB;
		root->clusters[k]->left = NULL;
		root->clusters[k]->right = NULL;
		root->clusters[k]->objects = malloc(sizeof(Primitive*) * MAX_OBJECTS);
		if (!root->clusters[k]->objects) {
			perror("Tree malloc error.\n");
			exit(1);
		}
		root->clusters[k]->objects_count = 0;
		
		add_object_to_node_v2(root->clusters[k], clusters[k].objects, clusters[k].count, 1);
		box.bmin.Data[0] = fminf(box.bmin.Data[0],clusters[k].box.bmin.Data[0]);
		box.bmin.Data[1] = fminf(box.bmin.Data[1],clusters[k].box.bmin.Data[1]);
		box.bmin.Data[2] = fminf(box.bmin.Data[2],clusters[k].box.bmin.Data[2]);
		box.bmax.Data[0] = fmaxf(box.bmax.Data[0],clusters[k].box.bmax.Data[0]);
		box.bmax.Data[1] = fmaxf(box.bmax.Data[1],clusters[k].box.bmax.Data[1]);
		box.bmax.Data[2] = fmaxf(box.bmax.Data[2],clusters[k].box.bmax.Data[2]);
		free(clusters[k].objects);
	}
	root->box = box;
	root->K = K;
	
	return root;
}


Large_BVH_t* initialize_tree_clustering(const Scene* S, unsigned int* seed, const int K){
	return initialize_root_tree_clustering((const Primitive**)S->objects, (const int)S->size_objects, K, seed);
}

int intersect_in_clusters(Large_BVH_t* const tree, const Ray* r, float* closest_t, Primitive** intersected_object, int* is_intern, int* face) {
	if (!tree || !closest_t) return 0;

	int hit = 0;
	float tbox = intersect_box_t(r, &tree->box, NULL, NULL);

	if (tbox < 0.f) return 0;
	
	for (int k = 0; k < tree->K; ++k) {
		float tcluster = intersect_box_t(r, &tree->clusters[k]->box, NULL, NULL);

		if (tcluster < 0.f)
			continue;

		hit |= intersect_in_tree(tree->clusters[k], r, closest_t, intersected_object, is_intern, face);
	}
	
	return hit;
}

void free_clusters(Large_BVH_t** root){
	if (!root) return;
	for (int i = 0; i<(*root)->K; ++i) {
		if ((*root)->clusters[i]) free_tree_objects(&(*root)->clusters[i]);
	}
}
