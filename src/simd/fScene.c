#include <simd/fScene.h>

void set_fSphere(fSphere *sph, uint64_t c)
{
    sph->size = 0;
    sph->capacity = ((c + 3) / 4) * 4;
    sph->padding = 4;
    sph->chunked_size = sph->capacity / 4;

    sph->coord = (float *)(aligned_alloc(16, 3 * sph->capacity * sizeof(float)));
    sph->x = sph->coord;
    sph->y = sph->coord + sph->capacity;
    sph->z = sph->y + sph->capacity;

    sph->color = (float *)(aligned_alloc(16, 3 * sph->capacity * sizeof(float)));
    sph->r = sph->color;
    sph->g = sph->color + sph->capacity;
    sph->b = sph->g + sph->capacity;

    sph->radius = (float *)(aligned_alloc(16, sph->capacity * sizeof(float)));

    sph->albedo = (float *)(aligned_alloc(16, sph->capacity * sizeof(float)));
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

        store(sph->albedo + step, &zero);
        store(sph->type + step, &zero);

        store(sph->radius + step, &rad);
    }
}

void intersect_fsphere(fSphere *sph,
                       const __vec4f *origin_x, const __vec4f *origin_y, const __vec4f *origin_z,
                       const __vec4f *dir_x, const __vec4f *dir_y, const __vec4f *dir_z,
                       __vec4f *n_x_min, __vec4f *n_y_min, __vec4f *n_z_min,
                       __vec4f *color_r_min, __vec4f *color_g_min, __vec4f *color_b_min,
                       __vec4f *albedo_min, __vec4f *type_min,
                       __vec4f *tmin, __vec4f *hit_mask)
{

    __vec4f center_x_min = max_limit;
    __vec4f center_y_min = max_limit;
    __vec4f center_z_min = max_limit;

    // a = dir*dir
    const __vec4f a = magnitude_(dir_x, dir_y, dir_z);

    const __vec4f over_a = rcp(&a);

    for (uint64_t i = 0; i < sph->size; ++i)
    {
        const __vec4f r = set1(sph->radius[i]);
        const __vec4f rad = mul_(&r, &r);

        // o - c
        const __vec4f center_x = set1(sph->x[i]);
        const __vec4f center_y = set1(sph->y[i]);
        const __vec4f center_z = set1(sph->z[i]);

        const __vec4f oc_x = sub_(origin_x, &center_x);
        const __vec4f oc_y = sub_(origin_y, &center_y);
        const __vec4f oc_z = sub_(origin_z, &center_z);

        // h = dir*oc
        const __vec4f h = dot_(dir_x, dir_y, dir_z, &oc_x, &oc_y, &oc_z);

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

        const __vec4f t1 = mul_(&minus_h_sqrt_delta, &over_a);
        const __vec4f t2 = mul_(&minus_h_sqrt_delta, &over_a);

        const __vec4f test0 = greater(&t1, &epsilon); // t10 > eps t11 > eps t12 > eps t13 > eps
        const __vec4f test1 = greater(&t2, &epsilon); // t20 > eps t21 > eps t22 > eps t23 > eps

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

        *hit_mask = blendv(hit_mask, &hit_test, &mask0);

        center_x_min = blendv(&center_x_min, &center_x, &mask0);
        center_y_min = blendv(&center_y_min, &center_y, &mask0);
        center_z_min = blendv(&center_z_min, &center_z, &mask0);

        const __vec4f hit_point_x = fmadd(dir_x, tmin, origin_x);
        const __vec4f hit_point_y = fmadd(dir_y, tmin, origin_y);
        const __vec4f hit_point_z = fmadd(dir_z, tmin, origin_z);

        const __vec4f default_nx = sub_(&hit_point_x, &center_x_min);
        const __vec4f default_ny = sub_(&hit_point_y, &center_y_min);
        const __vec4f default_nz = sub_(&hit_point_z, &center_z_min);

        *n_x_min = blendv(n_x_min, &default_nx, &mask0);
        *n_y_min = blendv(n_y_min, &default_ny, &mask0);
        *n_z_min = blendv(n_z_min, &default_nz, &mask0);

        const __vec4f default_r = set1(sph->r[i]);
        const __vec4f default_g = set1(sph->g[i]);
        const __vec4f default_b = set1(sph->b[i]);

        *color_r_min = blendv(color_r_min, &default_r, &mask0);
        *color_g_min = blendv(color_g_min, &default_g, &mask0);
        *color_b_min = blendv(color_b_min, &default_b, &mask0);

        const __vec4f default_albedo = set1(sph->albedo[i]);
        const __vec4f default_type = set1(sph->type[i]);

        *albedo_min = blendv(albedo_min, &default_albedo, &mask0);
        *type_min = blendv(type_min, &default_type, &mask0);
    }
};

void free_fSphere(fSphere *sph)
{
    free(sph->coord);
    free(sph->color);
    free(sph->radius);
    free(sph->albedo);
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

    q->color = (float *)(aligned_alloc(16, 3 * q->capacity * sizeof(float)));
    q->r = q->color;
    q->g = q->r + q->capacity;
    q->b = q->g + q->capacity;

    q->albedo = (float *)(aligned_alloc(16, q->capacity * sizeof(float)));
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

        store(q->albedo + step, &zero);
        store(q->type + step, &zero);
    }
};

void intersect_fquad(fQuad *q,
                     const __vec4f *origin_x, const __vec4f *origin_y, const __vec4f *origin_z,
                     const __vec4f *dir_x, const __vec4f *dir_y, const __vec4f *dir_z,
                     __vec4f *n_x_min, __vec4f *n_y_min, __vec4f *n_z_min,
                     __vec4f *color_r_min, __vec4f *color_g_min, __vec4f *color_b_min,
                     __vec4f *albedo_min, __vec4f *type_min,
                     __vec4f *tmin, __vec4f *hit_mask)
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

        const __vec4f nd = dot_(&n_x, &n_y, &n_z, dir_x, dir_y, dir_z);

        const __vec4f abs_nd = absf_(&nd);
        const __vec4f mask0 = greater(&abs_nd, &epsilon);

        if (fuse(&mask0) == 0)
            continue;

        const __vec4f on = dot_(origin_x, origin_y, origin_z, &n_x, &n_y, &n_z);

        const __vec4f Don = sub_(&D, &on);
        const __vec4f t = div_(&Don, &nd);

        const __vec4f mask1 = greater(&t, &epsilon);

        if (fuse(&mask1) == 0)
            continue;

        const __vec4f hp_x = fmadd(dir_x, &t, origin_x);
        const __vec4f hp_y = fmadd(dir_y, &t, origin_y);
        const __vec4f hp_z = fmadd(dir_z, &t, origin_z);

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

        *hit_mask = blendv(hit_mask, &hit_test, &mask);

        const __vec4f default_nx = set1(q->nx[i]);
        const __vec4f default_ny = set1(q->ny[i]);
        const __vec4f default_nz = set1(q->nz[i]);

        *n_x_min = blendv(n_x_min, &default_nx, &mask);
        *n_y_min = blendv(n_y_min, &default_ny, &mask);
        *n_z_min = blendv(n_z_min, &default_nz, &mask);

        const __vec4f default_r = set1(q->r[i]);
        const __vec4f default_g = set1(q->g[i]);
        const __vec4f default_b = set1(q->b[i]);

        *color_r_min = blendv(color_r_min, &default_r, &mask);
        *color_g_min = blendv(color_g_min, &default_g, &mask);
        *color_b_min = blendv(color_b_min, &default_b, &mask);

        const __vec4f default_albedo = set1(q->albedo[i]);
        const __vec4f default_type = set1(q->type[i]);

        *albedo_min = blendv(albedo_min, &default_albedo, &mask0);
        *type_min = blendv(type_min, &default_type, &mask0);
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

    free(q->color);

    free(q->albedo);
    free(q->type);
}

void intersect_in_scene_f(fScene *scene, const __vec4f *origin_x, const __vec4f *origin_y, const __vec4f *origin_z, const __vec4f *dir_x, const __vec4f *dir_y, const __vec4f *dir_z, fHit *hits)
{

    __vec4f tmin = max_limit;

    __vec4f color_r_min = zero;
    __vec4f color_g_min = zero;
    __vec4f color_b_min = zero;

    __vec4f albedo_min = zero;
    __vec4f type_min = zero;

    __vec4f hit_mask = zero;

    __vec4f n_x_min = max_limit;
    __vec4f n_y_min = max_limit;
    __vec4f n_z_min = max_limit;

    intersect_fquad(&scene->quads, origin_x,
                    origin_y, origin_z,
                    dir_x, dir_y, dir_z,
                    &n_x_min, &n_y_min, &n_z_min,
                    &color_r_min, &color_g_min, &color_b_min,
                    &albedo_min, &type_min,
                    &tmin, &hit_mask);

    intersect_fsphere(&scene->spheres, origin_x,
                      origin_y, origin_z,
                      dir_x, dir_y, dir_z,
                      &n_x_min, &n_y_min, &n_z_min,
                      &color_r_min, &color_g_min, &color_b_min,
                      &albedo_min, &type_min,
                      &tmin, &hit_mask);

    store(hits->isHitting, &hit_mask);

    __vec4f hit_point_x = fmadd(dir_x, &tmin, origin_x);
    __vec4f hit_point_y = fmadd(dir_y, &tmin, origin_y);
    __vec4f hit_point_z = fmadd(dir_z, &tmin, origin_z);

    store(hits->hpx, &hit_point_x);
    store(hits->hpy, &hit_point_y);
    store(hits->hpz, &hit_point_z);

    store(hits->hnx, &n_x_min);
    store(hits->hny, &n_y_min);
    store(hits->hnz, &n_z_min);

    store(hits->hcr, &color_r_min);
    store(hits->hcg, &color_g_min);
    store(hits->hcb, &color_b_min);

    store(hits->albedo, &albedo_min);
    store(hits->type, &type_min);
}

void add_fSphere(fSphere *sph, const float *coord, const float *color, const float radius, float albedo, const float type)
{
    sph->x[sph->size] = coord[0];
    sph->y[sph->size] = coord[1];
    sph->z[sph->size] = coord[2];

    sph->r[sph->size] = color[0];
    sph->g[sph->size] = color[1];
    sph->b[sph->size] = color[2];

    sph->radius[sph->size] = radius;

    sph->albedo[sph->size] = albedo;
    sph->type[sph->size] = type;

    sph->size++;
}

void add_fSphere_(fSphere *sph, float x, float y, float z, float r, float g, float b, const float radius, float albedo, const float type)
{
    sph->r[sph->size] = r;
    sph->g[sph->size] = g;
    sph->b[sph->size] = b;

    sph->x[sph->size] = x;
    sph->y[sph->size] = y;
    sph->z[sph->size] = z;

    sph->radius[sph->size] = radius;

    sph->albedo[sph->size] = albedo;
    sph->type[sph->size] = type;

    sph->size++;
}

void add_fQuad(fQuad *q, const float *Q, const float *u, const float *v, const float *c, float albedo, float type)
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

    q->albedo[q->size] = albedo;
    q->type[q->size] = type;

    float normal[3];
    float normal_2;

    float normal_norm[3];
    float Q_n[3];
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

void set_fScene(fScene *scene, const float *cam_position, float degree, float pitch, float yaw, size_t sphere_capacity, size_t quad_capacity, size_t width, size_t height)
{

    create_fcamera(&scene->cam, cam_position[0], cam_position[1], cam_position[2], pitch, yaw);

    const float inv_width = 1.0f / width;
    const float inv_height = 1.0f / height;
    const float aspr = (float)(width) / height;

    scene->fov = set1(tan(radian(degree * 0.5f)));
    scene->aspr_ = set1(aspr);
    scene->inv_w = set1(inv_width);
    scene->inv_h = set1(inv_height);

    set_fSphere(&scene->spheres, sphere_capacity);
    set_fQuad(&scene->quads, quad_capacity);
}

void free_fScene(fScene *scene)
{
    free_fSphere(&scene->spheres);
    free_fQuad(&scene->quads);
}