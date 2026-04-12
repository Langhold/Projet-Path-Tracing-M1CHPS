#include "light.h"

void benchmark_huge(Scene* scene, size_t width, size_t height){
	
	int amount_s = 10;
	int dist = 5;
	int amount = amount_s*amount_s*amount_s;
	
	const float x0  =  dist/2 + (float) dist/5.f;
	const float y0  =  (float)dist/2.f + (float) dist/3.f;
	const float z0  =  dist/2;
	const float fov =  50;
	material_t mat[3] = {Lambertian, Emissive, Specular};
	
	Camera cam;
	create_camera(&cam, width, height, fov, x0, y0, z0, -40, -21);
	scene->camera = cam;
	
	Vector bg;
	create_vector_ext(&bg, 255, 0, 255);
	create_scene_ext(amount+1, &bg, scene);
	float ymax = dist * tanf(25.f * M_PI / 180.0f);
	
	float xmax = ymax * width / height;
	
	Vector color = {{ 255, 255, 255 }};
	for (int i = 0; i < amount_s; ++i){
		for (int j = 0; j < amount_s; ++j){
			for (int k = 0; k < amount_s; ++k){
				
				
				float u = (float)i / (amount_s-1);
				float v = (float)j / (amount_s-1);
				float w = (float)k / (amount_s-1);
				
				float x = -xmax + u * 2.f * xmax;
				float y = -ymax + v * 2.f * ymax;
				float z = -dist - xmax + w * 2.f * xmax;

				float albedo = (i%3 == 2) ? 3 : 0.9;
				Primitive* p = malloc(sizeof(Primitive));
				
				if (j%2 == 0){
					create_sphere(p, 0.15, x, y, z, mat[i%3], albedo, &color);
					add_primitive(p, scene);
				}
				else {
					create_cube(p, 0.15, x, y, z, mat[2-i%3], albedo, &color);
					add_primitive(p, scene);
				}
			}
		}
	}
	
	Vector color_bg = {{200.f,200.f,200.f}};
	Primitive* p = malloc(sizeof(Primitive));
	create_cube(p, dist*2+0.5, 0, 0, 0, Lambertian, 1, &color_bg);
	add_primitive(p, scene);
}

void benchmark_big(Scene* scene, size_t width, size_t height){
	
	int amount_s = 5;
	int dist = 5;
	int amount = amount_s*amount_s*amount_s;
	
	
	const float x0  =  (float)dist + (float)dist/5.f;
	const float y0  =  (float)dist + (float)dist/5.f;
	const float z0  =  0;
	const float fov =  50;
	material_t mat[3] = {Lambertian, Emissive, Specular};
	
	Camera cam;
	create_camera(&cam, width, height, fov, x0, y0, z0, -50, -40);
	scene->camera = cam;
	
	Vector bg;
	create_vector_ext(&bg, 255, 0, 255);
	create_scene_ext(amount+1, &bg, scene);
	float ymax = dist * tanf(25.f * M_PI / 180.0f);
	
	float xmax = ymax * width / height;
	
	Vector color = {{ 255, 255, 255 }};
	for (int i = 0; i < amount_s; ++i){
		for (int j = 0; j < amount_s; ++j){
			for (int k = 0; k < amount_s; ++k){
				
				
				float u = (float)i / (amount_s-1);
				float v = (float)j / (amount_s-1);
				float w = (float)k / (amount_s-1);
				
				float x = -xmax + u * 2.f * xmax;
				float y = -ymax + v * 2.f * ymax;
				float z = -dist - xmax + w * 2.f * xmax;

				float albedo = (i%3 == 2) ? 3 : 0.9;
				Primitive* p = malloc(sizeof(Primitive));
				
				if (j%2 == 0){
					create_sphere(p, 0.25, x, y, z, mat[i%3], albedo, &color);
					add_primitive(p, scene);
				}
				else {
					create_cube(p, 0.25, x, y, z, mat[2-i%3], albedo, &color);
					add_primitive(p, scene);
				}
			}
		}
	}
	
	Vector color_bg = {{50.f,200.f,100.f}};
	Primitive* p = malloc(sizeof(Primitive));
	create_cube(p, dist*3, 0, 0, -dist/2, Lambertian, 1, &color_bg);
	add_primitive(p, scene);
}


void benchmark1(Scene* scene, size_t width, size_t height){
	
	const float x0  =  0.0;
	const float y0  =  0.0;
	const float z0  =  0.0;
	const float fov =  50;
	
	Camera cam;
	create_camera(&cam, width, height, fov, x0, y0, z0, 0, 0);
	scene->camera = cam;
	
	Vector bg;
	create_vector_ext(&bg, 230, 230, 230);
	create_scene_ext(11, &bg, scene);
	
	Primitive*  p1  = malloc(sizeof(Primitive));
	Primitive*  p2  = malloc(sizeof(Primitive));
	Primitive*  p3  = malloc(sizeof(Primitive));
	Primitive*  p4  = malloc(sizeof(Primitive));
	Primitive*  p5  = malloc(sizeof(Primitive));
	Primitive*  p6  = malloc(sizeof(Primitive));
	Primitive*  p7  = malloc(sizeof(Primitive));
	Primitive*  p8  = malloc(sizeof(Primitive));
	Primitive*  p9  = malloc(sizeof(Primitive));
	Primitive*  p10 = malloc(sizeof(Primitive));
	Primitive*  p11 = malloc(sizeof(Primitive));
	
	Vector head  = {{255, 220, 180}};
	Vector black = {{60, 60.0, 60.0}};
	Vector white = {{255.0, 255.0, 255.0}};
	create_sphere(p1, 0.5, 0, 0, -1.5, Lambertian, 1, &head);
	add_primitive(p1, scene);
	
	create_sphere(p2, 0.30, -0.45, 0.45, -1.6, Lambertian, 0.9, &black);
	add_primitive(p2, scene);
	
	create_sphere(p3, 0.30,  0.45, 0.45, -1.6, Lambertian, 0.9, &black);
	add_primitive(p3, scene);
	
	create_sphere(p4, 0.10, -0.18, 0.12, -1.03, Lambertian, 0.9, &white);
	add_primitive(p4, scene);
	
	create_sphere(p5, 0.06, -0.18, 0.10, -0.99, Lambertian, 0.9, &black);
	add_primitive(p5, scene);
	
	create_sphere(p6, 0.06, 0.18, 0.10, -0.99, Lambertian, 0.9, &black);
	add_primitive(p6, scene);
	
	create_sphere(p7, 0.10, 0.18, 0.12, -1.03, Lambertian, 0.9, &white);
	add_primitive(p7, scene);
	
	create_sphere(p8, 0.10, 0.0, -0.02, -0.98, Lambertian, 0.9, &black);
	add_primitive(p8, scene);
	
	Vector cheeks_color = {255, 150, 150};
	create_sphere(p9, 0.07, -0.30, -0.05, -1.09, Lambertian, 0.9, &cheeks_color);
	add_primitive(p9, scene);
	
	create_sphere(p10, 0.07, 0.30, -0.05, -1.09, Lambertian, 0.9, &cheeks_color);
	add_primitive(p10, scene);
	
	Vector raybox_color;
	create_vector_ext(&raybox_color, 230, 230, 230);
	create_box(p11, 10, 0.1, 10, 0, -0.51, 0.0, Lambertian, 0.9, &raybox_color, 0, 0);
	add_primitive(p11, scene);
}

void benchmark_medium(Scene* scene, size_t width, size_t height){
	
	const float x0 = 0;
	const float y0 = 0;
	const float z0 = 1.7;
	const float fov = 50;
	
	Camera cam;
	create_camera(&cam, width, height, fov, x0, y0, z0, 0, 0);
	scene->camera = cam;
	
	Vector bg;
	create_vector_ext(&bg, 42, 230, 42);
	create_scene_ext(15, &bg, scene);
	
	const float r = 0.15;
	
	Primitive*  p1 = malloc(sizeof(Primitive));
	Primitive*  p2 = malloc(sizeof(Primitive));
	Primitive*  p3 = malloc(sizeof(Primitive));
	Primitive*  p4 = malloc(sizeof(Primitive));
	Primitive*  p5 = malloc(sizeof(Primitive));
	Primitive*  p6 = malloc(sizeof(Primitive));
	Primitive*  p7 = malloc(sizeof(Primitive));
	Primitive*  p8 = malloc(sizeof(Primitive));
	Primitive*  p9 = malloc(sizeof(Primitive));
	Primitive* p10 = malloc(sizeof(Primitive));
	Primitive* p11 = malloc(sizeof(Primitive));

	Vector raybox_color;
	create_vector_ext(&raybox_color, 230, 230, 230);
	create_box(p1, 2.85, 2, 5, 0, 0.25, 0, Lambertian, 0.9, &raybox_color, 0, 0);
	add_primitive(p1, scene);

	Vector pylone;
	create_vector_ext(&pylone, 255, 123, 123);
	create_box(p2, 0.05, 1.5, 0.05, -1.2, 0, -2, Lambertian, 0.9, &pylone, 0, 0);
	add_primitive(p2, scene);

	Vector pylone_base;
	create_vector_ext(&pylone_base, 255, 123, 123);
	create_box(p3, 0.4, 0.1, 0.4, -1.2, -0.7, -2, Lambertian, 0.9, &pylone_base, 0, 0);
	add_primitive(p3, scene);

	Vector pylone_up;
	create_vector_ext(&pylone_up, 255, 123, 123);
	create_box(p4, 0.17, 0.06, 0.17, -1.2, 0.72, -2, Lambertian, 0.9, &pylone_up, 0, 0);
	add_primitive(p4, scene);

	Vector light_bulb;
	create_vector_ext(&light_bulb, 255, 255, 255);
	create_sphere(p5, r, -1.2, 0.9, -2, Emissive, 5, &light_bulb);
	add_primitive(p5, scene);
	
	float x2 = 1.2, z2 = -2;
	
	create_vector_ext(&pylone, 123, 123, 255);
	create_box(p6, 0.05, 1.5, 0.05, x2, 0, z2, Lambertian, 0.9, &pylone, 0, 0);
	add_primitive(p6, scene);
	
	create_vector_ext(&pylone_base, 123, 123, 255);
	create_box(p7, 0.4, 0.1, 0.4, x2, -0.7, z2, Lambertian, 0.9, &pylone_base, 0, 0);
	add_primitive(p7, scene);
	
	create_vector_ext(&pylone_up, 123, 123, 255);
	create_box(p8, 0.17, 0.06, 0.17, x2, 0.72, z2, Lambertian, 0.9, &pylone_up, 0, 0);
	add_primitive(p8, scene);

	create_sphere(p9, r, x2, 0.9, z2, Emissive, 5, &light_bulb);
	add_primitive(p9, scene);

	Vector mirror;
	create_vector_ext(&mirror, 255, 255, 255);
	//create_box(p10, 0.3, 1, 0.01, -1.3, -0.6, -1.3, Specular, 0.92, &mirror, 125, 75);
	create_box(p10, 0.01, 2, 5, -2.85/2, 0.25, 0, Specular, 0.92, &mirror, 0, 0);
	add_primitive(p10, scene);
	
	create_box(p11, 1, 0.01, 0.3, -1.2, -0.3, -2.1, Specular, 0.92, &mirror, -10, 90);
	add_primitive(p11, scene);
}
