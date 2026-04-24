#include <simd/fRay.h>

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

void trace_ray_simd(const __vec4f *inv_w, const __vec4f *inv_h, const __vec4f *aspr_, const __vec4f *x, const __vec4f *y, const __vec4f *fov, fRay *rv)
{
    const __vec4f view_y = mul_(y, inv_h);
    const __vec4f inter = fmadd(&minus_two, &view_y, &one);
    const __vec4f ndc_y = mul_(&inter, fov);

    const __vec4f view_x = mul_(x, inv_w);
    const __vec4f inter_0 = fmsub(&two, &view_x, &one);
    const __vec4f inter_1 = mul_(&inter_0, fov);

    const __vec4f ndc_x = mul_(&inter_1, aspr_);

    store(rv->dir_x, &ndc_x);
    store(rv->dir_y, &ndc_y);
    store(rv->dir_z, &minus_one);

    store(rv->ori_x, &zero);
    store(rv->ori_y, &zero);
    store(rv->ori_z, &one);
}

void free_fRay(fRay *r)
{
    free(r->ori_coord);
    free(r->dir_coord);
}
