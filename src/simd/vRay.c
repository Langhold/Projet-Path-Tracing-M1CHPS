#include <simd/vRay.h>

void vcreate_camera(vCamera *const cam, const float x0, const float y0, const float z0, const float pitch, const float yaw)
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

void vtrace_ray(vCamera *cam, vRay *ray, const __vec4f *inv_w, const __vec4f *inv_h, const __vec4f *aspr_, const __vec4f *x, const __vec4f *y, const __vec4f *fov)
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

    ray->ori_x = cam->position_x;
    ray->ori_y = cam->position_y;
    ray->ori_z = cam->position_z;

    ray->dir_x = add_(&pixel_x_right_x, &pixel_y_up_x);
    ray->dir_y = add_(&pixel_x_right_y, &pixel_y_up_y);
    ray->dir_z = add_(&pixel_x_right_z, &pixel_y_up_z);

    ray->dir_x = add_(&cam->direction_x, &ray->dir_x);
    ray->dir_y = add_(&cam->direction_y, &ray->dir_y);
    ray->dir_z = add_(&cam->direction_z, &ray->dir_z);
}
