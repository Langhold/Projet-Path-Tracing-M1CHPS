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

void path_trace(const int x1, const int y1, const int local_y, const int width, Scene const * S, const size_t bounces, float* color_buffer, unsigned int* seed){
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

void ray_sampling_t(Ray* const r, object_tree_t* const scene, int dmax, Vertex * path, unsigned int* seed, int * path_length, Vector* bg_color){
	Vector throughput = (path[0].is_light == 0) ? (Vector){{1.f, 1.f, 1.f}} : path[0].throughput;
	Ray current_ray = *r;
	
	int is_intern = 0, face = -1, start = (path->is_light == 0) ? 0 : 1;
	for (int d = start; d<dmax; ++d) {
		Vector n;
		Vector hit;
		float t = FLT_MAX;
		Primitive* obj = NULL;
		
		if (!intersect_in_tree(scene, &current_ray, &t, &obj, &is_intern, &face) || !obj) {
			for (int i = 0; i < 3; ++i) {
				path[d].throughput.Data[i] *= bg_color->Data[i];
			}
			return;
		}
		*path_length = d+1;
		
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
				if (path[d].is_light == 0) {
					path[d].position = current_ray.position;
					path[d].direction = current_ray.direction;
					path[d].throughput.Data[0] = throughput.Data[0] * obj->color.Data[0] * obj->albedo;
					path[d].throughput.Data[1] = throughput.Data[1] * obj->color.Data[1] * obj->albedo;
					path[d].throughput.Data[2] = throughput.Data[2] * obj->color.Data[2] * obj->albedo;
					path[d].normal = n;
					path[d].object = obj;
					path[d].wo = path[d].normal;
				}
				else {
					(*path_length)--;
				}
				return;
			}
				
			case Lambertian:
			{
				Ray r_new = random_Ray_demi_sphere_cosine_weighted(&offset_origin, &n, seed);
				for (int i = 0; i < 3; ++i){
					throughput.Data[i] *= obj->color.Data[i] * albedo;
				}
				path[d].direction = current_ray.direction;
				path[d].wo = r_new.direction;

				path[d].position = r_new.position;
				path[d].throughput = throughput;
				path[d].normal = n;
				norm_ext(&path[d].normal, &path[d].normal);
				path[d].object = obj;


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
				
				Vector wo;
				wo.Data[0] = current_ray.direction.Data[0] - 2.0f * dotn * n.Data[0];
				wo.Data[1] = current_ray.direction.Data[1] - 2.0f * dotn * n.Data[1];
				wo.Data[2] = current_ray.direction.Data[2] - 2.0f * dotn * n.Data[2];
				
				norm_ext(&wo, &wo);
				
				Ray r_new;
				r_new.direction = wo;
				r_new.position = offset_origin;
				
				float cosTheta = fabsf(dot(&current_ray.direction, &n));
				float fresnel = albedo + (1.0f - albedo) * powf(1.0f - cosTheta, 5.0f);

				for (int i = 0; i < 3; ++i){
					throughput.Data[i] *= fresnel * obj->color.Data[i];
				}
				path[d].direction = current_ray.direction;
				path[d].wo = r_new.direction;

				path[d].position = r_new.position;
				path[d].throughput.Data[0] *= throughput.Data[0];
				path[d].throughput.Data[1] *= throughput.Data[1];
				path[d].throughput.Data[2] *= throughput.Data[2];
				path[d].normal = n;
				norm_ext(&path[d].normal, &path[d].normal);
				path[d].object = obj;
				
				current_ray = r_new;
				if (d > 10) {
					if (russian_roulette(&throughput, seed)) {
						return;
					}
				}
				
				break;
			}
		}
	}
}

//Original Path Tracing ray_sampling function : only if there is no light sources
void ray_sampling_no_light(Ray* const r, object_tree_t* const scene, int dmax, Vector * radiance, unsigned int* seed, Vector* bg_color){
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
				radiance->Data[i] += throughput.Data[i] * (bg_color->Data[i]);
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
					radiance->Data[i] += throughput.Data[i] * (obj->color.Data[i]) * albedo;
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

void compute_vertex(Vector * color, Vertex *camera_path, int camera_path_length, Vertex *light_path, int light_path_length, Scene const * S) {
	if (camera_path_length <= 0 || light_path_length <= 0) {
    	return;
	}
	Ray r;
	Vector r_light, hit, n;
	Vector bsdf_camera = (Vector){{0.0f,0.0f,0.0f}};
	Vector bsdf_light = (Vector){{0.0f,0.0f,0.0f}};
	float dist2 = 0.0f, cos_camera = 0.0f, cos_light = 0.0f, G = 0.0f;
	float pdf_fwd = 0.0f, pdf_rev = 0.0f, r1 = 1.0f, r2 = 1.0f, weight = 0.0f;
	int c,l,i;
	int object = -1;
	float sum_camera = 1.0f, sum_light = 0.0f;

	for (c=0; c<camera_path_length; ++c) {
		bsdf_camera.Data[0] = camera_path[c].object->albedo * camera_path[c].object->color.Data[0] / M_PI;
		bsdf_camera.Data[1] = camera_path[c].object->albedo * camera_path[c].object->color.Data[1] / M_PI;
		bsdf_camera.Data[2] = camera_path[c].object->albedo * camera_path[c].object->color.Data[2] / M_PI;
		r.position = camera_path[c].position;
		r1 = 1.0f;
		sum_camera = 1.0f;
		for (i=c-1; i>=0; --i) {
			pdf_fwd = fabsf(dot(&camera_path[i].normal, &camera_path[i].direction))/M_PI ;
			pdf_rev = fabsf(dot(&camera_path[i].normal, &camera_path[i].wo))/M_PI ;
			r1 *= pdf_rev / pdf_fwd;
			sum_camera += r1;
		}
		if (c == camera_path_length-1 && camera_path[c].object->albedo > 1) {
			color->Data[0] += camera_path[c].throughput.Data[0] / sum_camera;
			color->Data[1] += camera_path[c].throughput.Data[1] / sum_camera;
			color->Data[2] += camera_path[c].throughput.Data[2] / sum_camera;
			continue;
		}

		for (l=0; l<light_path_length; ++l) {
			sub_ext(&light_path[l].position, &camera_path[c].position, &r.direction);
			dist2 = dot(&r.direction, &r.direction);
			if (dist2 < 0.0001) continue;
			norm_ext(&r.direction, &r.direction);
			intersect_in_scene(&r, S, &object, &hit, &n);
			if (fabsf(hit.Data[0] - light_path[l].position.Data[0]) > 1e-8f || fabsf(hit.Data[1] - light_path[l].position.Data[1]) > 1e-8f || fabsf(hit.Data[2] - light_path[l].position.Data[2]) > 1e-8f) {
				continue;
			}
			else {
				if (l > 0 && light_path[l].object->albedo > 1.0f) continue;
				mul_ext(&r.direction, -1.0f, &r_light);
				bsdf_light.Data[0] = light_path[l].object->albedo * light_path[l].object->color.Data[0] / M_PI;
				bsdf_light.Data[1] = light_path[l].object->albedo * light_path[l].object->color.Data[1] / M_PI;
				bsdf_light.Data[2] = light_path[l].object->albedo * light_path[l].object->color.Data[2] / M_PI;

				cos_camera = fabsf(dot(&camera_path[c].normal, &r.direction)) ;
				cos_light = fabsf(dot(&light_path[l].normal, &r_light)) ;
				G = cos_camera * cos_light / dist2;
				
				sum_light = sum_camera;
				r2 = 1.0f;
				for (i=l-1; i>=0; --i) {
					pdf_fwd = fabsf(dot(&light_path[i].normal, &light_path[i].direction))/M_PI ;
					pdf_rev = fabsf(dot(&light_path[i].normal, &light_path[i].wo))/M_PI ;
					r2 *= pdf_rev / pdf_fwd;
					sum_light += r2;
				}

				weight = 1.0f/sum_light;

				// printf("cam_thpt=%f, l_thpt=%f, bsdf_cam=%f, bsdf_l=%f, G=%f, weight=%f\n", camera_path[c].throughput.Data[0], light_path[l].throughput.Data[0], bsdf_camera.Data[0], bsdf_light.Data[0], G, weight);

				for (i=0; i<3; ++i) {
					color->Data[i] += camera_path[c].throughput.Data[i] * light_path[l].throughput.Data[i] * bsdf_camera.Data[i] * bsdf_light.Data[i] * G * weight;
				}
			}
		}
	}
	return;
}

void path_trace_t(const int x1, const int y1, Scene const * S, const size_t bounces, Vector * pixel_color, unsigned int* seed, object_tree_t* const tree){
	if (S->size_lights > 0) {
		Ray ray, light_ray;

		Vertex camera_path[bounces];
		Vertex light_path[bounces];
		for (size_t d=0; d<bounces; ++d) {
			camera_path[d].is_light = 0;
			light_path[d].is_light = 1;
		}

		trace_ray(x1, y1, &S->camera, &ray);
		trace_light_ray(S->size_lights, S->lights, light_path, seed);

		for (size_t i=0; i<3; ++i) {
			light_ray.direction.Data[i] = light_path[0].direction.Data[i] ;
			light_ray.position.Data[i] = light_path[0].position.Data[i] ;
		}
		
		int camera_path_length = 0, light_path_length = 0;
		ray_sampling_t(&ray, tree, (int)bounces, camera_path, seed, &camera_path_length, S->background_color);
		ray_sampling_t(&light_ray, tree, (int)bounces, light_path, seed, &light_path_length, S->background_color);

		Vector color = {{0.0f,0.0f,0.0f}};
		compute_vertex(&color, camera_path, camera_path_length, light_path, light_path_length, S);
				
		pixel_color->Data[0] += color.Data[0];
		pixel_color->Data[1] += color.Data[1];
		pixel_color->Data[2] += color.Data[2];

		return;
	}
	else {
		Ray ray;
		trace_ray(x1, y1, &S->camera, &ray);
		
		Vector radiance;
		ray_sampling_no_light(&ray, tree, (int)bounces, &radiance, seed, S->background_color);
				
		pixel_color->Data[0] += radiance.Data[0];
		pixel_color->Data[1] += radiance.Data[1];
		pixel_color->Data[2] += radiance.Data[2];
		
		return;
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
