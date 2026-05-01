#include <simd/fScene.h>

void set_fSphere(fSphere *sph, uint64_t c)
{
    sph->size = 0;
    sph->capacity = ((c + 3) / 4) * 4;
    sph->padding = 4;
    sph->chunked_size = sph->capacity / 4;

    sph->coord = (float *)(aligned_alloc(16, 3 * sph->capacity * sizeof(float)));
    sph->x = sph->coord;
    sph->y = sph->x + sph->capacity;
    sph->z = sph->y + sph->capacity;

    sph->albedo = (float *)(aligned_alloc(16, 3 * sph->capacity * sizeof(float)));
    sph->r = sph->albedo;
    sph->g = sph->r + sph->capacity;
    sph->b = sph->g + sph->capacity;

    sph->radius = (float *)(aligned_alloc(16, sph->capacity * sizeof(float)));

    sph->emission_power = (float *)(aligned_alloc(16, sph->capacity * sizeof(float)));
    sph->type = (float *)(aligned_alloc(16, sph->capacity * sizeof(float)));

    __vec4f rad = set1(0.5f);

    for (uint64_t i = 0; i < sph->chunked_size; ++i)
    {
        const uint32_t step = i * sph->padding;

        store(sph->x + step, &zero);
        store(sph->y + step, &zero);
        store(sph->z + step, &zero);

        store(sph->r + step, &zero);
        store(sph->g + step, &zero);
        store(sph->b + step, &zero);

        store(sph->emission_power + step, &zero);
        store(sph->type + step, &zero);

        store(sph->radius + step, &rad);
    }
}

void intersect_fsphere(fSphere *sph, const vRay *packed_ray, vHit *packed_hit, __vec4f *tmin)
{
    __vec4f center_x_min = max_limit;
    __vec4f center_y_min = max_limit;
    __vec4f center_z_min = max_limit;

    // a = dir*dir
    const __vec4f a = magnitude_(&packed_ray->dir_x, &packed_ray->dir_y, &packed_ray->dir_z);

    const __vec4f over_a = rcp(&a);

    for (uint64_t i = 0; i < sph->size; ++i)
    {
        const __vec4f r = set1(sph->radius[i]);
        const __vec4f rad = mul_(&r, &r);

        // o - c
        const __vec4f center_x = set1(sph->x[i]);
        const __vec4f center_y = set1(sph->y[i]);
        const __vec4f center_z = set1(sph->z[i]);

        const __vec4f oc_x = sub_(&packed_ray->ori_x, &center_x);
        const __vec4f oc_y = sub_(&packed_ray->ori_y, &center_y);
        const __vec4f oc_z = sub_(&packed_ray->ori_z, &center_z);

        // h = dir*oc
        const __vec4f h = dot_(&packed_ray->dir_x, &packed_ray->dir_y, &packed_ray->dir_z, &oc_x, &oc_y, &oc_z);

        // c = oc*oc - r²
        const __vec4f oc = magnitude_(&oc_x, &oc_y, &oc_z);
        const __vec4f c = sub_(&oc, &rad);

        // delta = h*h - a*c
        const __vec4f ac = mul_(&a, &c);
        const __vec4f delta = fmsub(&h, &h, &ac);
        const __vec4f mask_delta = greater(&delta, &epsilon);

        if (fuse(&mask_delta) == 0)
            continue;

        const __vec4f fake_delta = max_(&delta, &zero);

        // if(delta > ESPILON)
        const __vec4f sqrt_delta = sqrt_(&fake_delta);

        const __vec4f minus_h = sub_(&zero, &h);
        const __vec4f minus_h_sqrt_delta = sub_(&minus_h, &sqrt_delta);
        const __vec4f plus_h_sqrt_delta = add_(&minus_h, &sqrt_delta);

        const __vec4f t1 = mul_(&minus_h_sqrt_delta, &over_a);
        const __vec4f t2 = mul_(&plus_h_sqrt_delta, &over_a);

        const __vec4f test0 = greater(&t1, &epsilon);
        const __vec4f test1 = greater(&t2, &epsilon);

        __vec4f ts = blendv(&max_limit, &t2, &test1); //
        ts = blendv(&ts, &t1, &test0);
        ts = blendv(&max_limit, &ts, &mask_delta);

        const __vec4f test = or_(&test0, &test1);
        const __vec4f hit_test = and_(&mask_delta, &test);

        // Closest hit
        const __vec4f mask0 = lower(&ts, tmin);

        if (fuse(&mask0) == 0)
            continue;

        *tmin = min_(&ts, tmin);

        packed_hit->hit_isHitting = blendv(&packed_hit->hit_isHitting, &hit_test, &mask0);

        center_x_min = blendv(&center_x_min, &center_x, &mask0);
        center_y_min = blendv(&center_y_min, &center_y, &mask0);
        center_z_min = blendv(&center_z_min, &center_z, &mask0);

        const __vec4f hit_point_x = fmadd(&packed_ray->dir_x, tmin, &packed_ray->ori_x);
        const __vec4f hit_point_y = fmadd(&packed_ray->dir_y, tmin, &packed_ray->ori_y);
        const __vec4f hit_point_z = fmadd(&packed_ray->dir_z, tmin, &packed_ray->ori_z);

        __vec4f default_nx = sub_(&hit_point_x, &center_x_min);
        __vec4f default_ny = sub_(&hit_point_y, &center_y_min);
        __vec4f default_nz = sub_(&hit_point_z, &center_z_min);

        packed_hit->hit_normal_x = blendv(&packed_hit->hit_normal_x, &default_nx, &mask0);
        packed_hit->hit_normal_y = blendv(&packed_hit->hit_normal_y, &default_ny, &mask0);
        packed_hit->hit_normal_z = blendv(&packed_hit->hit_normal_z, &default_nz, &mask0);

        const __vec4f default_r = set1(sph->r[i]);
        const __vec4f default_g = set1(sph->g[i]);
        const __vec4f default_b = set1(sph->b[i]);

        packed_hit->hit_albedo_r = blendv(&packed_hit->hit_albedo_r, &default_r, &mask0);
        packed_hit->hit_albedo_g = blendv(&packed_hit->hit_albedo_g, &default_g, &mask0);
        packed_hit->hit_albedo_b = blendv(&packed_hit->hit_albedo_b, &default_b, &mask0);

        const __vec4f default_emission_power = set1(sph->emission_power[i]);
        const __vec4f default_type = set1(sph->type[i]);

        packed_hit->hit_emissive_power = blendv(&packed_hit->hit_emissive_power, &default_emission_power, &mask0);
        packed_hit->hit_mat_type = blendv(&packed_hit->hit_mat_type, &default_type, &mask0);
    }
};

void free_fSphere(fSphere *sph)
{
    free(sph->coord);
    free(sph->albedo);
    free(sph->radius);
    free(sph->emission_power);
    free(sph->type);
}

void set_fQuad(fQuad *q, uint64_t c)
{
    q->size = 0;
    q->capacity = ((c + 3) / 4) * 4;
    q->padding = 4;
    q->chunked_size = q->capacity / 4;
    q->Q = (float *)(aligned_alloc(16, 3 * q->capacity * sizeof(float)));
    q->qx = q->Q;
    q->qy = q->qx + q->capacity;
    q->qz = q->qy + q->capacity;

    q->u = (float *)(aligned_alloc(16, 3 * q->capacity * sizeof(float)));
    q->ux = q->u;
    q->uy = q->ux + q->capacity;
    q->uz = q->uy + q->capacity;

    q->v = (float *)(aligned_alloc(16, 3 * q->capacity * sizeof(float)));
    q->vx = q->v;
    q->vy = q->vx + q->capacity;
    q->vz = q->vy + q->capacity;

    q->n = (float *)(aligned_alloc(16, 3 * q->capacity * sizeof(float)));
    q->nx = q->n;
    q->ny = q->nx + q->capacity;
    q->nz = q->ny + q->capacity;

    q->D = (float *)(aligned_alloc(16, q->capacity * sizeof(float)));

    q->w = (float *)(aligned_alloc(16, 3 * q->capacity * sizeof(float)));
    q->wx = q->w;
    q->wy = q->wx + q->capacity;
    q->wz = q->wy + q->capacity;

    q->albedo = (float *)(aligned_alloc(16, 3 * q->capacity * sizeof(float)));
    q->r = q->albedo;
    q->g = q->r + q->capacity;
    q->b = q->g + q->capacity;

    q->emission_power = (float *)(aligned_alloc(16, q->capacity * sizeof(float)));
    q->type = (float *)(aligned_alloc(16, q->capacity * sizeof(float)));

    for (uint64_t i = 0; i < q->chunked_size; ++i)
    {
        const uint32_t step = i * q->padding;

        store(q->qx + step, &zero);
        store(q->qy + step, &zero);
        store(q->qz + step, &zero);

        store(q->ux + step, &zero);
        store(q->uy + step, &zero);
        store(q->uz + step, &zero);

        store(q->vx + step, &zero);
        store(q->vy + step, &zero);
        store(q->vz + step, &zero);

        store(q->nx + step, &zero);
        store(q->ny + step, &zero);
        store(q->nz + step, &zero);

        store(q->D + step, &zero);

        store(q->wx + step, &zero);
        store(q->wy + step, &zero);
        store(q->wz + step, &zero);

        store(q->r + step, &zero);
        store(q->g + step, &zero);
        store(q->b + step, &zero);

        store(q->emission_power + step, &zero);
        store(q->type + step, &zero);
    }
};

void intersect_fquad(fQuad *q, const vRay *packed_ray, vHit *packed_hit, __vec4f *tmin)
{

    for (uint64_t i = 0; i < q->size; ++i)
    {
        const __vec4f u_x = set1(q->ux[i]);
        const __vec4f u_y = set1(q->uy[i]);
        const __vec4f u_z = set1(q->uz[i]);

        const __vec4f v_x = set1(q->vx[i]);
        const __vec4f v_y = set1(q->vy[i]);
        const __vec4f v_z = set1(q->vz[i]);

        const __vec4f Q_x = set1(q->qx[i]);
        const __vec4f Q_y = set1(q->qy[i]);
        const __vec4f Q_z = set1(q->qz[i]);

        const __vec4f n_x = set1(q->nx[i]);
        const __vec4f n_y = set1(q->ny[i]);
        const __vec4f n_z = set1(q->nz[i]);

        const __vec4f D = set1(q->D[i]);

        const __vec4f w_x = set1(q->wx[i]);
        const __vec4f w_y = set1(q->wy[i]);
        const __vec4f w_z = set1(q->wz[i]);

        const __vec4f nd = dot_(&n_x, &n_y, &n_z, &packed_ray->dir_x, &packed_ray->dir_y, &packed_ray->dir_z);

        const __vec4f abs_nd = absf_(&nd);
        const __vec4f mask0 = greater(&abs_nd, &epsilon);

        if (fuse(&mask0) == 0)
            continue;

        const __vec4f on = dot_(&packed_ray->ori_x, &packed_ray->ori_y, &packed_ray->ori_z, &n_x, &n_y, &n_z);

        const __vec4f Don = sub_(&D, &on);
        const __vec4f t = div_(&Don, &nd);

        const __vec4f mask1 = greater(&t, &epsilon);

        if (fuse(&mask1) == 0)
            continue;

        const __vec4f hp_x = fmadd(&packed_ray->dir_x, &t, &packed_ray->ori_x);
        const __vec4f hp_y = fmadd(&packed_ray->dir_y, &t, &packed_ray->ori_y);
        const __vec4f hp_z = fmadd(&packed_ray->dir_z, &t, &packed_ray->ori_z);

        const __vec4f p_x = sub_(&hp_x, &Q_x);
        const __vec4f p_y = sub_(&hp_y, &Q_y);
        const __vec4f p_z = sub_(&hp_z, &Q_z);

        const __vec4f pv_zy = mul_(&p_z, &v_y);
        const __vec4f pv_xz = mul_(&p_x, &v_z);
        const __vec4f pv_yx = mul_(&p_y, &v_x);

        const __vec4f pv_x = fmsub(&p_y, &v_z, &pv_zy);
        const __vec4f pv_y = fmsub(&p_z, &v_x, &pv_xz);
        const __vec4f pv_z = fmsub(&p_x, &v_y, &pv_yx);

        const __vec4f up_zy = mul_(&u_z, &p_y);
        const __vec4f up_xz = mul_(&u_x, &p_z);
        const __vec4f up_yx = mul_(&u_y, &p_x);

        const __vec4f up_x = fmsub(&u_y, &p_z, &up_zy);
        const __vec4f up_y = fmsub(&u_z, &p_x, &up_xz);
        const __vec4f up_z = fmsub(&u_x, &p_y, &up_yx);

        const __vec4f alpha = dot_(&w_x, &w_y, &w_z, &pv_x, &pv_y, &pv_z);
        const __vec4f beta = dot_(&w_x, &w_y, &w_z, &up_x, &up_y, &up_z);

        const __vec4f mask20 = greater(&alpha, &zero);
        const __vec4f mask21 = lower(&alpha, &one);
        const __vec4f mask22 = greater(&beta, &zero);
        const __vec4f mask23 = lower(&beta, &one);

        const __vec4f and0 = and_(&mask22, &mask23);
        const __vec4f and1 = and_(&mask21, &and0);

        const __vec4f mask2 = and_(&mask20, &and1);

        if (fuse(&mask2) == 0)
            continue;

        const __vec4f and12 = and_(&mask1, &mask2);
        const __vec4f hit_test = and_(&mask0, &and12);
        __vec4f ts = blendv(&max_limit, &t, &hit_test);

        // Closest hit
        const __vec4f mask = lower(&ts, tmin);
        *tmin = min_(&ts, tmin);

        packed_hit->hit_isHitting = blendv(&packed_hit->hit_isHitting, &hit_test, &mask);

        const __vec4f default_nx = set1(q->nx[i]);
        const __vec4f default_ny = set1(q->ny[i]);
        const __vec4f default_nz = set1(q->nz[i]);

        packed_hit->hit_normal_x = blendv(&packed_hit->hit_normal_x, &default_nx, &mask);
        packed_hit->hit_normal_y = blendv(&packed_hit->hit_normal_y, &default_ny, &mask);
        packed_hit->hit_normal_z = blendv(&packed_hit->hit_normal_z, &default_nz, &mask);

        const __vec4f default_r = set1(q->r[i]);
        const __vec4f default_g = set1(q->g[i]);
        const __vec4f default_b = set1(q->b[i]);

        packed_hit->hit_albedo_r = blendv(&packed_hit->hit_albedo_r, &default_r, &mask);
        packed_hit->hit_albedo_g = blendv(&packed_hit->hit_albedo_g, &default_g, &mask);
        packed_hit->hit_albedo_b = blendv(&packed_hit->hit_albedo_b, &default_b, &mask);

        const __vec4f default_emission_power = set1(q->emission_power[i]);
        const __vec4f default_type = set1(q->type[i]);

        packed_hit->hit_emissive_power = blendv(&packed_hit->hit_emissive_power, &default_emission_power, &mask);
        packed_hit->hit_mat_type = blendv(&packed_hit->hit_mat_type, &default_type, &mask);
    }
}

void free_fQuad(fQuad *q)
{
    free(q->Q);
    free(q->u);

    free(q->v);
    free(q->n);
    free(q->D);

    free(q->w);

    free(q->albedo);

    free(q->emission_power);
    free(q->type);
}

void vintersect_in_scene(fScene *scene, const vRay *packed_ray, vHit *packed_hit)
{
    __vec4f tmin = max_limit;

    clear_hit(packed_hit);

    intersect_fquad(&scene->quads, packed_ray, packed_hit, &tmin);

    intersect_fsphere(&scene->spheres, packed_ray, packed_hit, &tmin);

    packed_hit->hit_point_x = fmadd(&packed_ray->dir_x, &tmin, &packed_ray->ori_x);
    packed_hit->hit_point_y = fmadd(&packed_ray->dir_y, &tmin, &packed_ray->ori_y);
    packed_hit->hit_point_z = fmadd(&packed_ray->dir_z, &tmin, &packed_ray->ori_z);

    norm_(&packed_hit->hit_normal_x, &packed_hit->hit_normal_y, &packed_hit->hit_normal_z, &packed_hit->hit_normal_x, &packed_hit->hit_normal_y, &packed_hit->hit_normal_z);
}

void add_fSphere(fSphere *sph, const float *coord, const float *color, const float radius, float emission_power, const float type)
{
    sph->x[sph->size] = coord[0];
    sph->y[sph->size] = coord[1];
    sph->z[sph->size] = coord[2];

    sph->r[sph->size] = color[0];
    sph->g[sph->size] = color[1];
    sph->b[sph->size] = color[2];

    sph->radius[sph->size] = radius;

    sph->emission_power[sph->size] = emission_power;
    sph->type[sph->size] = type;

    sph->size++;
}

void add_fSphere_(fSphere *sph, float x, float y, float z, float r, float g, float b, const float radius, float emission_power, const float type)
{
    sph->r[sph->size] = r;
    sph->g[sph->size] = g;
    sph->b[sph->size] = b;

    sph->x[sph->size] = x;
    sph->y[sph->size] = y;
    sph->z[sph->size] = z;

    sph->radius[sph->size] = radius;

    sph->emission_power[sph->size] = emission_power;
    sph->type[sph->size] = type;

    sph->size++;
}

void add_fQuad(fQuad *q, const float *Q, const float *u, const float *v, const float *c, float emission_power, float type)
{
    q->r[q->size] = c[0];
    q->g[q->size] = c[1];
    q->b[q->size] = c[2];

    q->qx[q->size] = Q[0];
    q->qy[q->size] = Q[1];
    q->qz[q->size] = Q[2];

    q->ux[q->size] = u[0];
    q->uy[q->size] = u[1];
    q->uz[q->size] = u[2];

    q->vx[q->size] = v[0];
    q->vy[q->size] = v[1];
    q->vz[q->size] = v[2];

    q->emission_power[q->size] = emission_power;
    q->type[q->size] = type;

    float normal[3];

    float normal_norm[3];
    float w_[3];

    normal[0] = u[1] * v[2] - u[2] * v[1];
    normal[1] = u[2] * v[0] - u[0] * v[2];
    normal[2] = u[0] * v[1] - u[1] * v[0];

    const float dot = (normal[0] * normal[0]) + (normal[1] * normal[1]) + (normal[2] * normal[2]);
    const float len = sqrtf(dot);

    normal_norm[0] = normal[0] / len;
    normal_norm[1] = normal[1] / len;
    normal_norm[2] = normal[2] / len;

    q->nx[q->size] = normal_norm[0];
    q->ny[q->size] = normal_norm[1];
    q->nz[q->size] = normal_norm[2];

    q->D[q->size] = (Q[0] * normal_norm[0]) + (Q[1] * normal_norm[1]) + (Q[2] * normal_norm[2]);
    const float k = 1.0f / (dot);
    w_[0] = normal[0] * k;
    w_[1] = normal[1] * k;
    w_[2] = normal[2] * k;

    q->wx[q->size] = w_[0];
    q->wy[q->size] = w_[1];
    q->wz[q->size] = w_[2];

    q->size++;
}

void add_cornel_box(fQuad *q, const float width, const float height, const float depth, const float *position, float emission_power, float type)
{
    const float min[3] = {(-width / 2.0f) + position[0], (-height / 2.0f) + position[1], (-depth / 2.0f) + position[2]};
    const float max[3] = {(width / 2.0f) + position[0], (height / 2.0f) + position[1], (depth / 2.0f) + position[2]};

    const float dx[3] = {max[0] - min[0], 0.0f, 0.0f};
    const float dy[3] = {0.0f, max[1] - min[1], 0.0f};
    const float dz[3] = {0.0f, 0.0f, max[2] - min[2]};

    const float mdx[3] = {-max[0] + min[0], 0.0f, 0.0f};
    const float mdy[3] = {0.0f, -max[1] + min[1], 0.0f};
    const float mdz[3] = {0.0f, 0.0f, -max[2] + min[2]};

    const float white_light[3] = {10.0f, 10.0f, 10.0f};
    const float color_0[3] = {0.9019f, 0.9019f, 0.9019f};

    add_fQuad(q, min, dy, dz, color_0, emission_power, type);
    add_fQuad(q, max, mdy, mdz, color_0, emission_power, type);
    add_fQuad(q, min, dx, dz, color_0, emission_power, type);
    add_fQuad(q, max, mdx, mdz, white_light, 10.0f, vEmissive);
    add_fQuad(q, min, dx, dy, color_0, emission_power, type);
}

void add_fSquare(fQuad *q, const float width, const float height, const float depth, const float *position, const float *color, float emission_power, float type)
{

    const float min[3] = {(-width / 2.0f) + position[0], (-height / 2.0f) + position[1], (-depth / 2.0f) + position[2]};
    const float max[3] = {(width / 2.0f) + position[0], (height / 2.0f) + position[1], (depth / 2.0f) + position[2]};

    const float dx[3] = {max[0] - min[0], 0.0f, 0.0f};
    const float dy[3] = {0.0f, max[1] - min[1], 0.0f};
    const float dz[3] = {0.0f, 0.0f, max[2] - min[2]};

    const float mdx[3] = {-max[0] + min[0], 0.0f, 0.0f};
    const float mdy[3] = {0.0f, -max[1] + min[1], 0.0f};
    const float mdz[3] = {0.0f, 0.0f, -max[2] + min[2]};

    add_fQuad(q, min, dy, dz, color, emission_power, type);
    add_fQuad(q, max, mdy, mdz, color, emission_power, type);

    add_fQuad(q, min, dx, dz, color, emission_power, type);
    add_fQuad(q, max, mdx, mdz, color, emission_power, type);

    add_fQuad(q, min, dx, dy, color, emission_power, type);
    add_fQuad(q, max, mdx, mdy, color, emission_power, type);
}

void rotate(float point[3], float pitch, float yaw)
{
    float px = point[0];
    float py = cosf(pitch) * point[1] - sinf(pitch) * point[2];
    float pz = sinf(pitch) * point[1] + cosf(pitch) * point[2];

    float yx = cosf(yaw) * px + sinf(yaw) * pz;
    float yy = py;
    float yz = -sinf(yaw) * px + cosf(yaw) * pz;

    point[0] = yx;
    point[1] = yy;
    point[2] = yz;
}

void translate(float *point, const float *positions)
{
    point[0] += positions[0];
    point[1] += positions[1];
    point[2] += positions[2];
}

void add_fSquare_with_rotation(fQuad *q, const float width, const float height, const float depth, const float *position, const float *color, float emission_power, float type, float pitch, float yaw)
{

    float min[3] = {(-width / 2.0f), (-height / 2.0f), (-depth / 2.0f)};
    float max[3] = {(width / 2.0f), (height / 2.0f), (depth / 2.0f)};

    float dx[3] = {width * 0.5f, 0.0f, 0.0f};
    float dy[3] = {0.0f, height * 0.5f, 0.0f};
    float dz[3] = {0.0f, 0.0f, depth * 0.5f};

    float mdx[3] = {-dx[0], 0.0f, 0.0f};
    float mdy[3] = {0.0f, -dy[1], 0.0f};
    float mdz[3] = {0.0f, 0.0f, -dz[2]};

    rotate(dx, pitch, yaw);
    rotate(dy, pitch, yaw);
    rotate(dz, pitch, yaw);
    rotate(mdx, pitch, yaw);
    rotate(mdy, pitch, yaw);
    rotate(mdz, pitch, yaw);
    rotate(min, pitch, yaw);
    rotate(max, pitch, yaw);

    translate(min, position);
    translate(max, position);

    add_fQuad(q, min, dy, dz, color, emission_power, type);
    add_fQuad(q, max, mdy, mdz, color, emission_power, type);

    add_fQuad(q, min, dx, dz, color, emission_power, type);
    add_fQuad(q, max, mdx, mdz, color, emission_power, type);

    add_fQuad(q, min, dx, dy, color, emission_power, type);
    add_fQuad(q, max, mdx, mdy, color, emission_power, type);
}

void set_fScene(fScene *scene, const float *background_color, const float *cam_position, float degree, float pitch, float yaw, size_t sphere_capacity, size_t quad_capacity, size_t width, size_t height)
{

    vcreate_camera(&scene->cam, cam_position[0], cam_position[1], cam_position[2], pitch, yaw);

    const float inv_width = 1.0f / width;
    const float inv_height = 1.0f / height;
    const float aspr = (float)(width) / height;

    scene->fov = set1(tan(radian(degree * 0.5f)));
    scene->aspr_ = set1(aspr);
    scene->inv_w = set1(inv_width);
    scene->inv_h = set1(inv_height);
    scene->background_color_r = set1(background_color[0]);
    scene->background_color_g = set1(background_color[1]);
    scene->background_color_b = set1(background_color[2]);

    set_fSphere(&scene->spheres, sphere_capacity);
    set_fQuad(&scene->quads, quad_capacity);
}

void free_fScene(fScene *scene)
{
    free_fSphere(&scene->spheres);
    free_fQuad(&scene->quads);
}