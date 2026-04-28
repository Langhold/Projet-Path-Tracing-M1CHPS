#include <simd/fRay.h>

void create_fcamera(fCamera *const cam, const float x0, const float y0, const float z0, const float pitch, const float yaw)
{
    float cosA, sinA, cosB, sinB;
    float up[3] = {0.0f};

    float cam_up[3] = {0.0f};
    float cam_right[3] = {0.0f};
    float cam_direction[3] = {0.0f};
    float cam_position[3] = {0.0f};

    const float alpha = radian(pitch);
    const float beta = radian(yaw);
    sinA = sinf(alpha);
    cosA = cosf(alpha);
    sinB = sinf(beta);
    cosB = cosf(beta);

    cam_direction[0] = sinB;
    cam_direction[1] = cosB * sinA;
    cam_direction[2] = -cosA * cosB;

    if (fabsf(cam_direction[0]) < (1 - EPS))
        up[1] = 1.0f;
    else
        up[0] = 1.0f;

    cam_right[0] = cam_direction[1] * up[2] - cam_direction[2] * up[1];
    cam_right[1] = cam_direction[2] * up[0] - cam_direction[0] * up[2];
    cam_right[2] = cam_direction[0] * up[1] - cam_direction[1] * up[0];

    const float dot = cam_right[0] * cam_right[0] + cam_right[1] * cam_right[1] + cam_right[2] * cam_right[2];
    const float len = sqrtf(dot);

    cam_right[0] = cam_right[0] / len;
    cam_right[1] = cam_right[1] / len;
    cam_right[2] = cam_right[2] / len;

    cam_up[0] = cam_right[1] * cam_direction[2] - cam_right[2] * cam_direction[1];
    cam_up[1] = cam_right[2] * cam_direction[0] - cam_right[0] * cam_direction[2];
    cam_up[2] = cam_right[0] * cam_direction[1] - cam_right[1] * cam_direction[0];

    cam_position[0] = x0;
    cam_position[1] = y0;
    cam_position[2] = z0;

    cam->position_x = set1(cam_position[0]);
    cam->position_y = set1(cam_position[1]);
    cam->position_z = set1(cam_position[2]);

    cam->direction_x = set1(cam_direction[0]);
    cam->direction_y = set1(cam_direction[1]);
    cam->direction_z = set1(cam_direction[2]);

    cam->right_x = set1(cam_right[0]);
    cam->right_y = set1(cam_right[1]);
    cam->right_z = set1(cam_right[2]);

    cam->up_x = set1(cam_up[0]);
    cam->up_y = set1(cam_up[1]);
    cam->up_z = set1(cam_up[2]);
}

void set_fray(fRay *ray, const uint64_t c)
{
    ray->capacity = c;
    ray->padding = 4;
    ray->chunked_size = 4;

    ray->ori_coord = (float *)(aligned_alloc(16, 3 * ray->capacity * sizeof(float)));
    ray->ori_x = ray->ori_coord;
    ray->ori_y = ray->ori_x + ray->capacity;
    ray->ori_z = ray->ori_y + ray->capacity;

    ray->dir_coord = (float *)(aligned_alloc(16, 3 * ray->capacity * sizeof(float)));
    ray->dir_x = ray->dir_coord;
    ray->dir_y = ray->dir_x + ray->capacity;
    ray->dir_z = ray->dir_y + ray->capacity;

    const uint32_t step = 0;

    store(ray->ori_x, &zero);
    store(ray->ori_y, &zero);
    store(ray->ori_z, &zero);

    store(ray->dir_x, &zero);
    store(ray->dir_y, &zero);
    store(ray->dir_z, &zero);
}

void trace_ray_simd(fCamera *cam, const __vec4f *inv_w, const __vec4f *inv_h, const __vec4f *aspr_, const __vec4f *x, const __vec4f *y, const __vec4f *fov, fRay *rv)
{
    const __vec4f view_y = mul_(y, inv_h);
    const __vec4f inter = fmadd(&minus_two, &view_y, &one);

    const __vec4f view_x = mul_(x, inv_w);
    const __vec4f inter_0 = fmsub(&two, &view_x, &one);
    const __vec4f inter_1 = mul_(&inter_0, fov);

    const __vec4f Pixel_x = mul_(&inter_1, aspr_);
    const __vec4f Pixel_y = mul_(&inter, fov);

    const __vec4f pixel_x_right_x = mul_(&Pixel_x, &cam->right_x);
    const __vec4f pixel_x_right_y = mul_(&Pixel_x, &cam->right_y);
    const __vec4f pixel_x_right_z = mul_(&Pixel_x, &cam->right_z);

    const __vec4f pixel_y_up_x = mul_(&Pixel_y, &cam->up_x);
    const __vec4f pixel_y_up_y = mul_(&Pixel_y, &cam->up_y);
    const __vec4f pixel_y_up_z = mul_(&Pixel_y, &cam->up_z);

    __vec4f ray_direction_x = add_(&pixel_x_right_x, &pixel_y_up_x);
    __vec4f ray_direction_y = add_(&pixel_x_right_y, &pixel_y_up_y);
    __vec4f ray_direction_z = add_(&pixel_x_right_z, &pixel_y_up_z);

    ray_direction_x = add_(&cam->direction_x, &ray_direction_x);
    ray_direction_y = add_(&cam->direction_y, &ray_direction_y);
    ray_direction_z = add_(&cam->direction_z, &ray_direction_z);

    store(rv->dir_x, &ray_direction_x);
    store(rv->dir_y, &ray_direction_y);
    store(rv->dir_z, &ray_direction_z);

    store(rv->ori_x, &cam->position_x);
    store(rv->ori_y, &cam->position_y);
    store(rv->ori_z, &cam->position_z);
}

void free_fRay(fRay *r)
{
    free(r->ori_coord);
    free(r->dir_coord);
}
