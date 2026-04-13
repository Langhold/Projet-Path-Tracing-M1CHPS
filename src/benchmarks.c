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
	
	Vector cheeks_color = {{255, 150, 150}};
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
	const float z0 = 17;
	const float fov = 50;
	
	Camera cam;
	create_camera(&cam, width, height, fov, x0, y0, z0, 0, 0);
	scene->camera = cam;
	
	Vector bg;
	create_vector_ext(&bg, 42, 230, 42);
	create_scene_ext(13, &bg, scene);
	
	const float r = 1.5;
	
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
	Primitive* p12 = malloc(sizeof(Primitive));
	Primitive* p13 = malloc(sizeof(Primitive));

	Vector raybox_color;
	create_vector_ext(&raybox_color, 33, 33, 126);
	create_box(p1, 28.5, 20, 50, 0, 2.5, 0, Lambertian, 0.9, &raybox_color, 0, 0);
	add_primitive(p1, scene);

	Vector pylone;
	create_vector_ext(&pylone, 255, 10, 10);
	create_box(p2, 0.5, 15, 0.5, -12, 0, -20, Lambertian, 0.9, &pylone, 0, 0);
	add_primitive(p2, scene);

	Vector pylone_base;
	create_vector_ext(&pylone_base, 255, 123, 123);
	create_box(p3, 4, 1, 4, -12, -7, -20, Lambertian, 0.9, &pylone_base, 0, 0);
	add_primitive(p3, scene);

	Vector pylone_up;
	create_vector_ext(&pylone_up, 255, 123, 123);
	create_box(p4, 1.7, 0.6, 1.7, -12, 7.2, -20, Lambertian, 0.9, &pylone_up, 0, 0);
	add_primitive(p4, scene);

	Vector light_bulb;
	create_vector_ext(&light_bulb, 255, 255, 255);
	create_sphere(p5, r, -12, 9, -20, Emissive, 100, &light_bulb);
	add_primitive(p5, scene);
	
	float x2 = 12, z2 = -20;
	
	create_vector_ext(&pylone, 123, 123, 255);
	create_box(p6, 0.5, 15, 0.5, x2, 0, z2, Lambertian, 0.9, &pylone, 0, 0);
	add_primitive(p6, scene);
	
	create_vector_ext(&pylone_base, 123, 123, 255);
	create_box(p7, 4, 1, 4, x2, -7, z2, Lambertian, 0.9, &pylone_base, 0, 0);
	add_primitive(p7, scene);
	
	create_vector_ext(&pylone_up, 123, 123, 255);
	create_box(p8, 1.7, 0.6, 1.7, x2, 7.2, z2, Lambertian, 0.9, &pylone_up, 0, 0);
	add_primitive(p8, scene);

	create_box(p9, 2, 2.5, 1.7, x2, 9, z2, Emissive, 80, &light_bulb, 45, 60);
	add_primitive(p9, scene);

	Vector mirror;
	create_vector_ext(&mirror, 255, 255, 255);
	create_box(p10, 3, 10, 0.1, -10, -1, -8, Specular, 1, &mirror, 125, 75);
	add_primitive(p10, scene);

	Vector big_light;
	create_vector_ext(&big_light, 200, 200, 131);
	create_sphere(p11, 4, 0, -8, 17, Emissive, 120, &big_light);
	add_primitive(p11, scene);
	

	Vector my_sphere;
	create_vector_ext(&my_sphere, 10, 255, 10);
	create_sphere(p12, r, -8, 2, -8, Lambertian, 0.92, &my_sphere);
	add_primitive(p12, scene);

	Vector my_mirror;
	create_vector_ext(&my_mirror, 255, 255, 255);
	create_sphere(p13, 2.5, 0, 0, -17, Specular, 1, &my_mirror);
	add_primitive(p13, scene);
}

void benchmark_BDPT(Scene* scene, size_t width, size_t height){

	Camera cam;
	create_camera(&cam, width, height, 50, 0, 0, 9.5, 0, 0);
	scene->camera = cam;
	
	Vector bg;
	create_vector_ext(&bg, 230, 230, 230);
	create_scene_ext(6, &bg, scene);
	
	Primitive*  p1  = malloc(sizeof(Primitive));
	Primitive*  p2  = malloc(sizeof(Primitive));
	Primitive*  p3  = malloc(sizeof(Primitive));
	Primitive*  p4  = malloc(sizeof(Primitive));
	Primitive*  p5  = malloc(sizeof(Primitive));
	Primitive*  p6  = malloc(sizeof(Primitive));

	Vector raybox_color;
	create_vector_ext(&raybox_color, 230, 230, 230);
	create_box(p1, 10, 10, 20, 0, 0, 0, Lambertian, 0.9, &raybox_color, 0, 0);
	add_primitive(p1, scene);
	
	Vector lumiere;
	create_vector_ext(&lumiere, 255, 255, 255);
	create_sphere(p2, 0.2, 0, 0, -8, Emissive, 10, &lumiere);
	add_primitive(p2, scene);

	Vector cube;
	create_vector_ext(&cube, 255, 123, 123);
	create_box(p3, 2, 2, 2, 0, 0, -4, Lambertian, 0.9, &cube, 0, 0);
	add_primitive(p3, scene);

	Vector boule1;
	create_vector_ext(&boule1, 255, 123, 123);
	create_sphere(p4, 1, 2, 2, -7, Specular, 0.9, &boule1);
	add_primitive(p4, scene);

	Vector boule2;
	create_vector_ext(&boule2, 123, 123, 255);
	create_sphere(p5, 2, -2.5, -3, -5, Lambertian, 0.9, &boule2);
	add_primitive(p5, scene);

	Vector boule3;
	create_vector_ext(&boule3, 123, 255, 123);
	create_sphere(p6, 0.8, 2.5, -3, -0, Lambertian, 0.9, &boule3);
	add_primitive(p6, scene);

}