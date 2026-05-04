#include "light.h"

int russian_roulette(Vector* throughput, unsigned int* seed){
	
	const float p = 0.2126f * throughput->Data[0] + 0.7152f * throughput->Data[1] + 0.0722f * throughput->Data[2];
	
	const float epsilon = rand_r(seed) / (float)RAND_MAX;

	if (epsilon > p) {
		//kill path
		return 1;
	}
	for (int i = 0; i < 3; ++i) {
		throughput->Data[i] /= p;
	}
	return 0;
}


int get_bounces(void)
{
	const char* env = getenv("BOUNCES");
	if (!env) return 26;
	else        return atoi(env);
	return 0;
}


void ray_sampling(Ray* r, const Scene* S, int dmax, Vector* radiance, unsigned int* seed, int rr)
{
	Vector throughput = {{1.f, 1.f, 1.f}};
	Ray current_ray = *r;

	for (int i = 0; i < 3; ++i) {
		radiance->Data[i] = 0.0f;
	}

	for (int d = 0; d < dmax; ++d) {

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

		const Primitive* obj = S->objects[object];
		const float albedo = obj->albedo;

		Vector offset_origin;
		Vector n_eps;
		mul_ext(&n, EPS, &n_eps);
		add_ext(&hit, &n_eps, &offset_origin);

		switch (obj->m_type) {
		case Emissive: {
			for (int i = 0; i < 3; ++i) {
				radiance->Data[i] += throughput.Data[i] * obj->color.Data[i] * albedo;
			}
			return;
		}

		case Lambertian: {
			if (dot(&n, &current_ray.direction) > 0.0f) {
				mul_ext(&n, -1.0f, &n);
				mul_ext(&n, 1e-4, &n_eps);
				add_ext(&hit, &n_eps, &offset_origin);
			}
			Ray r_new = random_Ray_demi_sphere_cosine_weighted(&offset_origin, &n, seed);
			for (int i = 0; i < 3; ++i) {
				throughput.Data[i] *= obj->color.Data[i] * albedo;
			}
			current_ray = r_new;

			if ((d > 10) && rr) {
				if (russian_roulette(&throughput, seed)) {
					return;
				}
			}
			break;
		}

		case Specular: {
			const float dotn = dot(&current_ray.direction, &n);

			Vector wo;
			wo.Data[0] = current_ray.direction.Data[0] - 2.0f * dotn * n.Data[0];
			wo.Data[1] = current_ray.direction.Data[1] - 2.0f * dotn * n.Data[1];
			wo.Data[2] = current_ray.direction.Data[2] - 2.0f * dotn * n.Data[2];

			norm_ext(&wo, &wo);

			Ray r_new;
			r_new.direction = wo;
			r_new.position  = offset_origin;

			for (int i = 0; i < 3; ++i) {
				throughput.Data[i] *= 0.9f;
			}

			if ((d > 10) && rr) {
				if (russian_roulette(&throughput, seed)) {
					return;
				}
			}

			current_ray = r_new;
			break;
		}
		}
	}
	for (int i = 0; i < 3; ++i) {
		radiance->Data[i] = 0.0f;
	}
}


void path_trace(const int x1, const int y1, const int local_y, const int width, Scene const * S, const size_t bounces, float* color_buffer, unsigned int* seed, int rr)
{
	
	Ray ray;
	trace_ray(x1, y1, &S->camera, &ray);
	
	Vector radiance;
	ray_sampling(&ray, S, (int)bounces, &radiance, seed, rr);
	
	size_t index = (local_y * width + x1) * 3;
	
	color_buffer[index+0] += radiance.Data[0];
	color_buffer[index+1] += radiance.Data[1];
	color_buffer[index+2] += radiance.Data[2];
	 
	
	return;
}


void compute_normal(Primitive* const p, Vector* n, int is_intern, int face, Vector* hit)
{
	if (!p || !n) return;

	switch (p->type) {
	case SPHERE:
		*n = get_normal_vector_sphere(hit, &p->position);
		break;
	case BBOX:
		*n = get_normal_vector_box(face, is_intern);
		break;
	case BOX: {
		OBB* box = (OBB*)p->object;
		Vector normal;
		switch (face) {
		case MIN:    normal = box->obb_right;     mul_ext(&normal, -1, &normal); break;
		case MAX:    normal = box->obb_right;                                    break;
		case BOTTOM: normal = box->obb_up;        mul_ext(&normal, -1, &normal); break;
		case UP:     normal = box->obb_up;                                       break;
		case BACK:   normal = box->obb_direction; mul_ext(&normal, -1, &normal); break;
		case FRONT:  normal = box->obb_direction;                                break;
		}

		if (is_intern)
			mul_ext(&normal, -1.0f, &normal);

		*n = normal;
		break;
	}
	}
}


void ray_sampling_tree(Ray* const r, object_tree_t* const scene, int dmax,
					   Vector* radiance, unsigned int* seed, Vector* bg_color)
{
	Vector throughput = {{1.f, 1.f, 1.f}};
	Ray current_ray = *r;

	for (int i = 0; i < 3; ++i)
		radiance->Data[i] = 0.0f;

	int is_intern = 0, face = -1;
	for (int d = 0; d < dmax; ++d) {
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

		switch (obj->m_type) {
		case Emissive: {
			for (int i = 0; i < 3; ++i) {
				radiance->Data[i] += throughput.Data[i] * obj->color.Data[i] * albedo;
			}
			return;
		}

		case Lambertian: {
			Ray r_new = random_Ray_demi_sphere_cosine_weighted(&offset_origin, &n, seed);
			for (int i = 0; i < 3; ++i) {
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

		case Specular: {
			float dotn = dot(&current_ray.direction, &n);
			if (dotn > 0.f) abort();

			Vector wo;
			wo.Data[0] = current_ray.direction.Data[0] - 2.0f * dotn * n.Data[0];
			wo.Data[1] = current_ray.direction.Data[1] - 2.0f * dotn * n.Data[1];
			wo.Data[2] = current_ray.direction.Data[2] - 2.0f * dotn * n.Data[2];

			norm_ext(&wo, &wo);

			Ray r_new;
			r_new.direction = wo;
			r_new.position  = offset_origin;

			for (int i = 0; i < 3; ++i) {
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
	for (int i = 0; i < 3; ++i) {
		radiance->Data[i] = 0.0f;
	}
}


void path_trace_tree(const int x1, const int y1, const int local_y, const int width,
					 Scene const* S, const size_t bounces, Vector* color_buffer,
					 unsigned int* seed, object_tree_t* const tree)
{
	 Ray ray;
	trace_ray(x1, y1, &S->camera, &ray);

	 Vector radiance;
	ray_sampling_tree(&ray, tree, (int)bounces, &radiance, seed, S->background_color);

	size_t index = (local_y * width + x1) * 3;

	color_buffer->Data[0] += radiance.Data[0];
	color_buffer->Data[1] += radiance.Data[1];
	color_buffer->Data[2] += radiance.Data[2];

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


void path_trace_clusters(const int x1, const int y1, const int local_y, const int width,
						 Scene const* S, const size_t bounces, Vector* color_buffer,
						 unsigned int* seed, Large_BVH_t* const tree)
{
	 Ray ray;
	trace_ray(x1, y1, &S->camera, &ray);

	 Vector radiance;
	ray_sampling_clusters(&ray, tree, (int)bounces, &radiance, seed, S->background_color);

	const size_t index = (local_y * width + x1) * 3;

	color_buffer->Data[0] += radiance.Data[0];
	color_buffer->Data[1] += radiance.Data[1];
	color_buffer->Data[2] += radiance.Data[2];

	return;
}


void ray_sampling_clusters_no_light(Ray* const r, Large_BVH_t* const scene, int dmax, Vector * radiance, unsigned int* seed, Vector* bg_color){
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

static float compute_pdf(float pdf, const Vector* normal, const Vector* from, const Vector* to){
	if (pdf <= 0.f) return 0.f;
	Vector w;
	sub_ext(to, from, &w);
	float d2 = fabsf(dot(&w, &w));
	if (d2 < 1e-20f) return 0.f;
	norm_ext(&w, &w);
	float cos = fabsf(dot(normal, &w));
	return pdf * cos / d2;
}

void compute_vertex(Vector * color, Vertex *camera_path, int camera_path_length, Vertex *light_path, int light_path_length, Large_BVH_t* const tree) {
	if (camera_path_length <= 0 || light_path_length <= 0) {
    	return;
	}
	Ray r;
	Vector r_light;
	Vector bsdf_camera = (Vector){{0.0f,0.0f,0.0f}};
	Vector bsdf_light = (Vector){{0.0f,0.0f,0.0f}};
	Vector neg_wi = (Vector){{0.f,0.0f,0.0f}};
	float dist2 = 0.0f, cos_camera = 0.0f, cos_light = 0.0f, G = 0.0f;
	float cur_cos = 0.0f, r1 = 1.0f, sum_mis = 1.0f, weight = 0.0f;
	int c,l,i;
	float t = FLT_MAX;
	Primitive* obj = NULL;
	int is_intern = 0, face = -1;

	for (c=0; c<camera_path_length; ++c) {

		if (camera_path[c].object->m_type == Specular) continue;

		if (c == camera_path_length-1 && camera_path[c].object->albedo > 1) {
			color->Data[0] += camera_path[c].throughput.Data[0];
			color->Data[1] += camera_path[c].throughput.Data[1];
			color->Data[2] += camera_path[c].throughput.Data[2];
			continue;
		}

		if (c > 0) {
			cur_cos = fabsf(dot(&camera_path[c].normal, &camera_path[c-1].direction))/M_PI;
			camera_path[c].pdf_fwd = compute_pdf(cur_cos, &camera_path[c-1].normal, &camera_path[c-1].position, &camera_path[c].position);
			mul_ext(&camera_path[c].direction, -1.0f, &neg_wi);
			cur_cos = fabsf(dot(&camera_path[c].normal, &neg_wi)) / M_PI;
			camera_path[c-1].pdf_rev = compute_pdf(cur_cos, &camera_path[c-1].normal, &camera_path[c].position, &camera_path[c-1].position);
		}

		if (camera_path[c].object->m_type == Specular) {
			bsdf_camera.Data[0] = 1.0f;
			bsdf_camera.Data[1] = 1.0f;
			bsdf_camera.Data[2] = 1.0f;	
		}
		else {
			bsdf_camera.Data[0] = camera_path[c].object->albedo * camera_path[c].object->color.Data[0] / M_PI;
			bsdf_camera.Data[1] = camera_path[c].object->albedo * camera_path[c].object->color.Data[1] / M_PI;
			bsdf_camera.Data[2] = camera_path[c].object->albedo * camera_path[c].object->color.Data[2] / M_PI;
		}
		r.position = camera_path[c].position;

		for (l=0; l<light_path_length; ++l) {
	
			if (light_path[l].object->m_type == Specular) continue;

			sub_ext(&light_path[l].position, &camera_path[c].position, &r.direction);
			dist2 = fabsf(dot(&r.direction, &r.direction));
			if (dist2 < 1e-20f) dist2=1e-20f;
			norm_ext(&r.direction, &r.direction);
			intersect_in_clusters(tree, &r, &t, &obj, &is_intern, &face);

			if (light_path[l].object != obj) {
				continue;
			}
			else {
				if (l > 0 && light_path[l].object->albedo > 1.0f) continue;
				mul_ext(&r.direction, -1.0f, &r_light);
				if (light_path[l].object->m_type == Specular) {
					bsdf_light.Data[0] = 1.0f;
					bsdf_light.Data[1] = 1.0f;
					bsdf_light.Data[2] = 1.0f;
				}
				else {
					bsdf_light.Data[0] = light_path[l].object->albedo * light_path[l].object->color.Data[0] / M_PI;
					bsdf_light.Data[1] = light_path[l].object->albedo * light_path[l].object->color.Data[1] / M_PI;
					bsdf_light.Data[2] = light_path[l].object->albedo * light_path[l].object->color.Data[2] / M_PI;
				}

				if (l>0) {
					cur_cos = fabsf(dot(&light_path[l-1].normal, &light_path[l-1].wo)) / M_PI;
					light_path[l].pdf_fwd = compute_pdf(cur_cos, &light_path[l].normal, &light_path[l-1].position, &light_path[l].position);
					mul_ext(&light_path[l].direction, -1.0f, &neg_wi);
					cur_cos = fabsf(dot(&light_path[l].normal, &neg_wi)) / M_PI;
					light_path[l-1].pdf_rev = compute_pdf(cur_cos, &light_path[l-1].normal, &light_path[l].position, &light_path[l-1].position);

					cur_cos = fabsf(dot(&light_path[l].normal, &r_light)) / M_PI;
					camera_path[c].pdf_rev = compute_pdf(cur_cos, &light_path[l].normal, &light_path[l].position, &camera_path[c].position);
					cur_cos = fabsf(dot(&camera_path[c].normal, &r.direction)) / M_PI;
					light_path[l].pdf_rev = compute_pdf(cur_cos, &camera_path[c].normal, &camera_path[c].position, &light_path[l].position);
				}
				else { // l==0
					cur_cos = fabsf(dot(&camera_path[c].normal, &r_light));
					camera_path[c].pdf_rev = light_path[0].pdf_fwd * cur_cos / dist2 ;
				}

				r1 = 1.0f;
				sum_mis = 1.0f;
				for (i=c; i>=0; --i) { //camera part of mis_weight
					if (camera_path[i].pdf_fwd == 0.f) break;
					r1 *= camera_path[i].pdf_rev / camera_path[i].pdf_fwd ;
					if (camera_path[i].object->m_type != Specular) sum_mis += r1 ;
				}
				r1 = 1.0f;
				for (i=l; i>=0; --i) { //light part of mis_weight
					if (light_path[i].pdf_fwd == 0.f) break;
					r1 *= light_path[i].pdf_rev / light_path[i].pdf_fwd ;
					if (light_path[i].object->m_type != Specular) sum_mis += r1 ;
				}

				weight = 1.0f/sum_mis;

				cos_camera = fabsf(dot(&camera_path[c].normal, &r.direction)) ;
				cos_light = fabsf(dot(&light_path[l].normal, &r_light)) ;
				G = cos_camera * cos_light / dist2;

				// printf("G=%f, cos_camera=%f, cos_light=%f, dist2=%f\n", G, cos_camera, cos_light, dist2);

				// printf("cam_thpt=%f, l_thpt=%f, bsdf_cam=%f, bsdf_l=%f, G=%f, weight=%f\n", camera_path[c].throughput.Data[0], light_path[l].throughput.Data[0], bsdf_camera.Data[0], bsdf_light.Data[0], G, weight);
				for (i=0; i<3; ++i) {
					color->Data[i] += camera_path[c].throughput.Data[i] * light_path[l].throughput.Data[i] * bsdf_camera.Data[i] * bsdf_light.Data[i] * G * weight;
				}
			}
		}

	}
	return;
}

void ray_sampling_clusters_bdpt(Ray* const r, Large_BVH_t* const scene, int dmax, Vertex * path, unsigned int* seed, int * path_length, Vector* bg_color){
	Vector throughput = (path[0].is_light == 0) ? (Vector){{1.f, 1.f, 1.f}} : path[0].throughput;
	Ray current_ray = *r;
	
	int is_intern = 0, face = -1, start = (path->is_light == 0) ? 0 : 1;
	for (int d = start; d<dmax; ++d) {
		Vector n;
		Vector hit;
		float t = FLT_MAX;
		Primitive* obj = NULL;
		
		if (!intersect_in_clusters(scene, &current_ray, &t, &obj, &is_intern, &face) || !obj) {
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
					path[d].position = hit;
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

				if (d > 10) {
					if (russian_roulette(&throughput, seed)) {
						(*path_length)--;
						return;
					}
				}
				path[d].direction = current_ray.direction;
				path[d].wo = r_new.direction;

				path[d].position = hit;
				path[d].throughput = throughput;
				path[d].normal = n;
				norm_ext(&path[d].normal, &path[d].normal);
				path[d].object = obj;


				current_ray = r_new;
				
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
				
				path[d].direction = current_ray.direction;
				path[d].wo = r_new.direction;

				path[d].position = r_new.position;
				path[d].throughput.Data[0] = throughput.Data[0];
				path[d].throughput.Data[1] = throughput.Data[1];
				path[d].throughput.Data[2] = throughput.Data[2];
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


void path_trace_t(const int x1, const int y1, Scene const * S, const size_t bounces, Vector * pixel_color, unsigned int* seed, Large_BVH_t* const tree){
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
		ray_sampling_clusters_bdpt(&ray, tree, (int)bounces, camera_path, seed, &camera_path_length, S->background_color);
		ray_sampling_clusters_bdpt(&light_ray, tree, (int)bounces, light_path, seed, &light_path_length, S->background_color);

		camera_path[0].pdf_fwd = compute_pdf(1.0f, &camera_path[0].normal, &S->camera.position, &camera_path[0].position);

		Vector color = {{0.0f,0.0f,0.0f}};
		compute_vertex(&color, camera_path, camera_path_length, light_path, light_path_length, tree);
				
		pixel_color->Data[0] += color.Data[0];
		pixel_color->Data[1] += color.Data[1];
		pixel_color->Data[2] += color.Data[2];

		return;
	}
	else {
		Ray ray;
		trace_ray(x1, y1, &S->camera, &ray);
		
		Vector radiance;
		ray_sampling_clusters_no_light(&ray, tree, (int)bounces, &radiance, seed, S->background_color);
		
		pixel_color->Data[0] += radiance.Data[0];
		pixel_color->Data[1] += radiance.Data[1];
		pixel_color->Data[2] += radiance.Data[2];
		
		return;
	}
}

void path_trace_original(const int x1, const int y1, Scene const * S, const size_t bounces, Vector * pixel_color, unsigned int* seed, Large_BVH_t* const tree) {
	Ray ray;
	trace_ray(x1, y1, &S->camera, &ray);
	
	Vector radiance;
	ray_sampling_clusters_no_light(&ray, tree, (int)bounces, &radiance, seed, S->background_color);
			
	pixel_color->Data[0] += radiance.Data[0];
	pixel_color->Data[1] += radiance.Data[1];
	pixel_color->Data[2] += radiance.Data[2];
	
	return;
}
