#include "ray.h"



void create_camera(Camera * const cam, const size_t width, const size_t height, const float fov, const float x0, const float y0, const float z0, const float pitch, const float yaw)
{
	float cosA, sinA, cosB, sinB;
	Vector up;
	const float alpha=radian(pitch);
	const float beta=radian(yaw);
	sinA = sinf(alpha);
	cosA = cosf(alpha);
	sinB = sinf(beta);
	cosB = cosf(beta);

	cam->direction = (Vector) {{sinB, cosB*sinA, -cosA*cosB}};
	
	if (fabs(cam->direction.Data[0]) < 1-EPS) up = (Vector){{0.0f, 1.0f, 0.0f}};
	else up = (Vector){{1.0f, 0.0f, 0.0f}};
	
	cross_ext(&cam->direction, &up, &(cam->right));
	norm_ext(&(cam->right),&(cam->right));
	
	cross_ext(&(cam->right), &cam->direction, &(cam->up));
	
	create_vector_ext(&cam->position, x0, y0, z0);
	cam->width = width;
	cam->height = height;
	cam->fov = tanf(radian(fov * 0.5));

	cam->inv_height = 1.0f/height;
	cam->inv_width = 1.0f/width;

}

void create_ray_default_ext(Ray * ray)
{
    create_vector_default_ext(&ray->position);
    create_vector_ext(&ray->direction,0.0f, 0.0f, 0.0f);
}

void create_ray_ext(Ray * ray, const float x0, const float y0, const float z0, const float x1, const float y1, const float z1)
{
    create_vector_ext(&ray->position,x0, y0, z0);
    create_vector_ext(&ray->direction,x1, y1, z1);
	norm_ext(&ray->direction,&ray->direction);

}

void trace_ray(const size_t i, const size_t j, const Camera *cam, Ray* const r)
{
   

    float Pixel_x = (2.0f*i + 1.0f - cam->width) * cam->inv_height * cam->fov;
    float Pixel_y = (cam->height - 2.0f*j - 1.0f) * cam->inv_height * cam->fov;
	
	
	r->position = cam->position;

	r->direction.Data[0] = cam->direction.Data[0] + Pixel_x * cam->right.Data[0] + Pixel_y * cam->up.Data[0];
	r->direction.Data[1] = cam->direction.Data[1] + Pixel_x * cam->right.Data[1] + Pixel_y * cam->up.Data[1];
	r->direction.Data[2] = cam->direction.Data[2] + Pixel_x * cam->right.Data[2] + Pixel_y * cam->up.Data[2];

	norm_ext(&r->direction, &r->direction);
}

void free_ray(Ray* r)
{
    if(r) free(r);
    else fprintf(stdout, "No need to free memory.\n");
}
