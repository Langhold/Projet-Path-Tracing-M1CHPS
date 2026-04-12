#include "light.h"




int russian_roulette(Vector* throughput, unsigned int* seed){
	
	const float p = 0.2126f * throughput->Data[0] + 0.7152f * throughput->Data[1] + 0.0722f * throughput->Data[2];
	
	const float epsilon = rand_r(seed) / (float)RAND_MAX;
	
	if (epsilon > p) {
		//kill path
		return 1;
	}
	for (int i = 0; i<3; ++i) {
		throughput->Data[i] /= p;
	}
	return 0;
	
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

void ray_sampling(Ray *r, const Scene * S, int dmax, Vector * radiance, unsigned int* seed){
	Vector throughput = {{1.f, 1.f, 1.f}};
	Ray current_ray = *r;
	
	for (int i = 0; i < 3; ++i){
			radiance->Data[i] = 0.0f;
	}
	
	for (int d = 0; d<dmax; ++d) {
		
		Vector n;
		Vector hit;
		int object = -1;
		if (!intersect_in_scene(&current_ray, S, &object, &hit, &n)) {
			for (int i = 0; i < 3; ++i) {
				radiance->Data[i] += throughput.Data[i] * S->background_color->Data[i];
			}
			return;
		}
		
		if (dot(&n, &current_ray.direction) > 0.0f) {
			mul_ext(&n, -1.0f, &n);
		}
		
		const Primitive *obj = S->objects[object];
		const float albedo = obj->albedo;
		
		
		Vector offset_origin;
		Vector n_eps;
		mul_ext(&n, EPS, &n_eps);
		add_ext(&hit, &n_eps, &offset_origin);
		
		
		switch (obj->m_type){
			case Emissive:{
				for (int i = 0; i < 3; ++i){
					radiance->Data[i] += throughput.Data[i] * obj->color.Data[i] * albedo;
				}
				return;
			}
				
			case Lambertian:
			{
				
				if (dot(&n, &current_ray.direction) > 0.0f) {
					mul_ext(&n, -1.0f, &n);
					mul_ext(&n, 1e-4, &n_eps);
					add_ext(&hit, &n_eps, &offset_origin);
				}
				Ray r_new = random_Ray_demi_sphere_cosine_weighted(&offset_origin, &n, seed);
				for (int i = 0; i < 3; ++i){
					throughput.Data[i] *= obj->color.Data[i] * albedo;
				}
				current_ray = r_new;
				
				
				
				if (d > 10) {
					if (russian_roulette(&throughput, seed)) {
						return;
					}
				}
				
				break;
			}
			case Specular:{
				const float dotn = dot(&current_ray.direction, &n);
				
				
				Vector wo;
				wo.Data[0] = current_ray.direction.Data[0] - 2.0f * dotn * n.Data[0];
				wo.Data[1] = current_ray.direction.Data[1] - 2.0f * dotn * n.Data[1];
				wo.Data[2] = current_ray.direction.Data[2] - 2.0f * dotn * n.Data[2];
				
				norm_ext(&wo, &wo);
				
				Ray r_new;
				r_new.direction = wo;
				r_new.position = offset_origin;
				
				
				for (int i = 0; i < 3; ++i){
					throughput.Data[i] *= 0.9f;
				}
				
				if (d > 10) {
					if (russian_roulette(&throughput, seed)) {
						return;
					}
				}
				
				current_ray = r_new;
				break;
			}
		}
	}
	for (int i = 0; i < 3; ++i){
			radiance->Data[i] = 0.0f;
	}
}

void path_trace(const int x1, const int y1, const int local_y, const int width, Scene const * S, const size_t bounces, float* color_buffer, unsigned int* seed)
{
	
	Ray ray;
	trace_ray(x1, y1, &S->camera, &ray);
	
	Vector radiance;
	ray_sampling(&ray, S, (int)bounces, &radiance, seed);
	
	size_t index = (local_y * width + x1) * 3;
	
	color_buffer[index+0] += radiance.Data[0];
	color_buffer[index+1] += radiance.Data[1];
	color_buffer[index+2] += radiance.Data[2];
	 
	
	return;
}

int get_bounces(void){
	const char * env = getenv("BOUNCES");
	if(!env) return 26;
	else return atoi(env);
	return 0;
}



void compute_normal(Primitive* const p, Vector* n, int is_intern, int face, Vector* hit){
	if (!p || !n) return;
	
	
	switch (p->type) {
		case SPHERE:
			*n = get_normal_vector_sphere(hit, &p->position);
			break;
		case BBOX:
			*n = get_normal_vector_box(face, is_intern);
			break;
		case BOX:
		{
			OBB *box = (OBB *)p->object;
			Vector normal;
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
			break;
		}
	}
}

void ray_sampling_t(Ray* const r, object_tree_t* const scene, int dmax, Vector * radiance, unsigned int* seed, Vector* bg_color){
	Vector throughput = {{1.f, 1.f, 1.f}};
	Ray current_ray = *r;
	
	for (int i = 0; i < 3; ++i)
			radiance->Data[i] = 0.0f;
	
	int is_intern = 0, face = -1;
	for (int d = 0; d<dmax; ++d) {
		Vector n;
		Vector hit;
		float t = FLT_MAX;
		Primitive* obj = NULL;
		
		if (!intersect_in_tree(scene, &current_ray, &t, &obj, &is_intern, &face) || !obj) {
			for (int i = 0; i < 3; ++i) {
				radiance->Data[i] += throughput.Data[i] * bg_color->Data[i];
			}
			return;
		}
		
		linear_ext(&current_ray.position, &current_ray.direction, t, &hit);
		compute_normal(obj, &n, is_intern, face, &hit);
		
		if (dot(&n, &current_ray.direction) > 0.0f) {
			mul_ext(&n, -1.0f, &n);
		}
		const float albedo = obj->albedo;
		
		
		Vector offset_origin;
		Vector n_eps;
		mul_ext(&n, EPS, &n_eps);
		add_ext(&hit, &n_eps, &offset_origin);
		
		
		
		
		switch (obj->m_type){
			case Emissive:{
				for (int i = 0; i < 3; ++i){
					radiance->Data[i] += throughput.Data[i] * obj->color.Data[i] * albedo;
				}
				return;
			}
				
			case Lambertian:
			{
				Ray r_new = random_Ray_demi_sphere_cosine_weighted(&offset_origin, &n, seed);
				for (int i = 0; i < 3; ++i){
					throughput.Data[i] *= obj->color.Data[i] * albedo;
				}
				current_ray = r_new;
				
				
				if (d > 10) {
					if (russian_roulette(&throughput, seed)) {
						return;
					}
				}
				 
				break;
			}
			case Specular:{
				float dotn = dot(&current_ray.direction, &n);
				if (dotn > 0.f) abort();
				
				
				Vector wo;
				wo.Data[0] = current_ray.direction.Data[0] - 2.0f * dotn * n.Data[0];
				wo.Data[1] = current_ray.direction.Data[1] - 2.0f * dotn * n.Data[1];
				wo.Data[2] = current_ray.direction.Data[2] - 2.0f * dotn * n.Data[2];
				
				norm_ext(&wo, &wo);
				
				Ray r_new;
				r_new.direction = wo;
				r_new.position = offset_origin;
				

				for (int i = 0; i < 3; ++i){
					throughput.Data[i] *= albedo;
				}
				
				if (d > 10) {
					if (russian_roulette(&throughput, seed)) {
						return;
					}
				}
				
				current_ray = r_new;
				break;
			}
		}
	}
	for (int i = 0; i < 3; ++i){
			radiance->Data[i] = 0.0f;
	}
}

void path_trace_t(const int x1, const int y1, const int local_y, const int width, Scene const * S, const size_t bounces, float* color_buffer, unsigned int* seed, object_tree_t* const tree)
{
	
	static Ray ray;
	trace_ray(x1, y1, &S->camera, &ray);
	
	static Vector radiance;
	ray_sampling_t(&ray, tree, (int)bounces, &radiance, seed, S->background_color);
	
	size_t index = (local_y * width + x1) * 3;
	
	color_buffer[index+0] += radiance.Data[0];
	color_buffer[index+1] += radiance.Data[1];
	color_buffer[index+2] += radiance.Data[2];
	 
	
	return;
}


void ray_sampling_clusters(Ray* const r, Large_BVH_t* const scene, int dmax, Vector * radiance, unsigned int* seed, Vector* bg_color){
	Vector throughput = {{1.f, 1.f, 1.f}};
	Ray current_ray = *r;
	
	for (int i = 0; i < 3; ++i)
			radiance->Data[i] = 0.0f;
	
	int is_intern = 0, face = -1;
	for (int d = 0; d<dmax; ++d) {
		Vector n;
		Vector hit;
		float t = FLT_MAX;
		Primitive* obj = NULL;
		
		if (!intersect_in_clusters(scene, &current_ray, &t, &obj, &is_intern, &face) || !obj) {
			for (int i = 0; i < 3; ++i) {
				radiance->Data[i] += throughput.Data[i] * bg_color->Data[i];
			}
			return;
		}
		
		linear_ext(&current_ray.position, &current_ray.direction, t, &hit);
		compute_normal(obj, &n, is_intern, face, &hit);
		
		if (dot(&n, &current_ray.direction) > 0.0f) {
			mul_ext(&n, -1.0f, &n);
		}
		const float albedo = obj->albedo;
		
		
		Vector offset_origin;
		Vector n_eps;
		mul_ext(&n, EPS, &n_eps);
		add_ext(&hit, &n_eps, &offset_origin);
		
		
		
		
		switch (obj->m_type){
			case Emissive:{
				for (int i = 0; i < 3; ++i){
					radiance->Data[i] += throughput.Data[i] * obj->color.Data[i] * albedo;
				}
				return;
			}
				
			case Lambertian:
			{
				Ray r_new = random_Ray_demi_sphere_cosine_weighted(&offset_origin, &n, seed);
				for (int i = 0; i < 3; ++i){
					throughput.Data[i] *= obj->color.Data[i] * albedo;
				}
				current_ray = r_new;
				
				
				if (d > 10) {
					if (russian_roulette(&throughput, seed)) {
						return;
					}
				}
				 
				break;
			}
			case Specular:{
				float dotn = dot(&current_ray.direction, &n);
				if (dotn > 0.f) abort();
				
				
				Vector wo;
				wo.Data[0] = current_ray.direction.Data[0] - 2.0f * dotn * n.Data[0];
				wo.Data[1] = current_ray.direction.Data[1] - 2.0f * dotn * n.Data[1];
				wo.Data[2] = current_ray.direction.Data[2] - 2.0f * dotn * n.Data[2];
				
				norm_ext(&wo, &wo);
				
				Ray r_new;
				r_new.direction = wo;
				r_new.position = offset_origin;
				

				for (int i = 0; i < 3; ++i){
					throughput.Data[i] *= albedo;
				}
				
				if (d > 10) {
					if (russian_roulette(&throughput, seed)) {
						return;
					}
				}
				
				current_ray = r_new;
				break;
			}
		}
	}
	for (int i = 0; i < 3; ++i){
			radiance->Data[i] = 0.0f;
	}
}

void path_trace_clusters(const int x1, const int y1, const int local_y, const int width, Scene const * S, const size_t bounces, float* color_buffer, unsigned int* seed, Large_BVH_t* const tree) {
	
	static Ray ray;
	trace_ray(x1, y1, &S->camera, &ray);
	
	static Vector radiance;
	ray_sampling_clusters(&ray, tree, (int)bounces, &radiance, seed, S->background_color);
	
	const size_t index = (local_y * width + x1) * 3;
	
	color_buffer[index+0] += radiance.Data[0];
	color_buffer[index+1] += radiance.Data[1];
	color_buffer[index+2] += radiance.Data[2];
	 
	
	return;
}
