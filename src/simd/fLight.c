#include <simd/fLight.h>

void phong_model(fScene *scene, fRay *r, fRGB *pixel_color, fHit *hit_surface, size_t x, size_t y, size_t width)
{
    const __vec4f ray_origin_x = load(r->ori_x);
    const __vec4f ray_origin_y = load(r->ori_y);
    const __vec4f ray_origin_z = load(r->ori_z);

    const __vec4f ray_direction_x = load(r->dir_x);
    const __vec4f ray_direction_y = load(r->dir_y);
    const __vec4f ray_direction_z = load(r->dir_z);

    intersect_in_scene_f(scene, &ray_origin_x, &ray_origin_y, &ray_origin_z, &ray_direction_x, &ray_direction_y, &ray_direction_z, hit_surface);

    const __vec4f lightColor_r = one;
    const __vec4f lightColor_g = one;
    const __vec4f lightColor_b = one;

    const __vec4f lightPos_x = _mm_set1_ps(0.8f);
    const __vec4f lightPos_y = _mm_set1_ps(0.8f);
    const __vec4f lightPos_z = minus_one;

    const __vec4f background_r = half;
    const __vec4f background_g = half;
    const __vec4f background_b = half;

    const __vec4f ambientStrength = zero;
    const __vec4f shininess = _mm_set1_ps(32.0f);

    __vec4f ambient_r = mul_(&ambientStrength, &lightColor_r);
    __vec4f ambient_g = mul_(&ambientStrength, &lightColor_g);
    __vec4f ambient_b = mul_(&ambientStrength, &lightColor_b);

    const __vec4f specularStrength = _mm_set1_ps(0.9f);

    __vec4f mask1 = _mm_cmpneq_ps(load(hit_surface->isHitting), zero);

    if (fuse(&mask1) == 0)
    {
        store(pixel_color->r, &background_r);
        store(pixel_color->g, &background_g);
        store(pixel_color->b, &background_b);
        return;
    }

    const __vec4f hit_point_x = load(hit_surface->hpx);
    const __vec4f hit_point_y = load(hit_surface->hpy);
    const __vec4f hit_point_z = load(hit_surface->hpz);

    __vec4f lightDirX = sub_(&lightPos_x, &hit_point_x);
    __vec4f lightDirY = sub_(&lightPos_y, &hit_point_y);
    __vec4f lightDirZ = sub_(&lightPos_z, &hit_point_z);

    norm_(&lightDirX, &lightDirY, &lightDirZ, &lightDirX, &lightDirY, &lightDirZ);

    __vec4f normalX = load(hit_surface->hnx);
    __vec4f normalY = load(hit_surface->hny);
    __vec4f normalZ = load(hit_surface->hnz);

    norm_(&normalX, &normalY, &normalZ, &normalX, &normalY, &normalZ);

    __vec4f dot__ = dot_(&normalX, &normalY, &normalZ, &ray_direction_x, &ray_direction_y, &ray_direction_z);

    __vec4f mask0 = greater(&dot__, &zero);

    __vec4f sub_0 = sub_(&zero, &normalX);
    __vec4f sub_1 = sub_(&zero, &normalY);
    __vec4f sub_2 = sub_(&zero, &normalZ);

    normalX = blendv(&normalX, &sub_0, &mask0);
    normalY = blendv(&normalY, &sub_1, &mask0);
    normalZ = blendv(&normalZ, &sub_2, &mask0);

    __vec4f dot_44 = dot_(&normalX, &normalY, &normalZ, &lightDirX, &lightDirY, &lightDirZ);

    __vec4f diff = max_(&dot_44, &zero);

    __vec4f diffuse_r = mul_(&diff, &lightColor_r);
    __vec4f diffuse_g = mul_(&diff, &lightColor_g);
    __vec4f diffuse_b = mul_(&diff, &lightColor_b);

    __vec4f viewDirX = load(r->dir_x);
    __vec4f viewDirY = load(r->dir_y);
    __vec4f viewDirZ = load(r->dir_z);

    norm_(&viewDirX, &viewDirY, &viewDirZ, &viewDirX, &viewDirY, &viewDirZ);

    __vec4f k = dot_(&normalX, &normalY, &normalZ, &lightDirX, &lightDirY, &lightDirZ);
    __vec4f dotLN_2 = mul_(&two, &k);

    __vec4f tmp0 = mul_(&normalX, &dotLN_2);
    __vec4f tmp1 = mul_(&normalY, &dotLN_2);
    __vec4f tmp2 = mul_(&normalZ, &dotLN_2);

    __vec4f reflectDirX = sub_(&tmp0, &lightDirX);
    __vec4f reflectDirY = sub_(&tmp1, &lightDirY);
    __vec4f reflectDirZ = sub_(&tmp2, &lightDirZ);

    norm_(&reflectDirX, &reflectDirY, &reflectDirZ, &reflectDirX, &reflectDirY, &reflectDirZ);

    __vec4f tmp3 = dot_(&viewDirX, &viewDirY, &viewDirZ, &reflectDirX, &reflectDirY, &reflectDirZ);
    __vec4f tmp4 = max_(&tmp3, &zero);

    __vec4f spec = powf_(&tmp4, &shininess);

    __vec4f tmp5 = mul_(&specularStrength, &spec);

    __vec4f specular_r = mul_(&tmp5, &lightColor_r);
    __vec4f specular_g = mul_(&tmp5, &lightColor_g);
    __vec4f specular_b = mul_(&tmp5, &lightColor_b);

    __vec4f tmp6 = add_(&diffuse_r, &specular_r);
    __vec4f tmp7 = add_(&diffuse_g, &specular_g);
    __vec4f tmp8 = add_(&diffuse_b, &specular_b);

    __vec4f pl_r = add_(&ambient_r, &tmp6);
    __vec4f pl_g = add_(&ambient_g, &tmp7);
    __vec4f pl_b = add_(&ambient_b, &tmp8);

    __vec4f tmp9 = max_(&zero, &pl_r);
    __vec4f tmp10 = max_(&zero, &pl_g);
    __vec4f tmp11 = max_(&zero, &pl_b);

    __vec4f phong_light_r = min_(&one, &tmp9);
    __vec4f phong_light_g = min_(&one, &tmp10);
    __vec4f phong_light_b = min_(&one, &tmp11);

    __vec4f tmp12 = load(hit_surface->hcr);
    __vec4f tmp13 = load(hit_surface->hcg);
    __vec4f tmp14 = load(hit_surface->hcb);

    __vec4f color_r = mul_(&phong_light_r, &tmp12);
    __vec4f color_g = mul_(&phong_light_g, &tmp13);
    __vec4f color_b = mul_(&phong_light_b, &tmp14);

    __vec4f tmp15 = blendv(&background_r, &color_r, &mask1);
    __vec4f tmp16 = blendv(&background_g, &color_g, &mask1);
    __vec4f tmp17 = blendv(&background_b, &color_b, &mask1);

    store(pixel_color->r, &tmp15);
    store(pixel_color->g, &tmp16);
    store(pixel_color->b, &tmp17);
}

void diffuse_render(fScene *scene, fRay *r, fRGB *pixel_color, fHit *hit_surface)
{
    const __vec4f ray_ori_x = load(r->ori_x);
    const __vec4f ray_ori_y = load(r->ori_y);
    const __vec4f ray_ori_z = load(r->ori_z);

    const __vec4f ray_dir_x = load(r->dir_x);
    const __vec4f ray_dir_y = load(r->dir_y);
    const __vec4f ray_dir_z = load(r->dir_z);

    intersect_in_scene_f(scene, &ray_ori_x, &ray_ori_y, &ray_ori_z, &ray_dir_x, &ray_dir_y, &ray_dir_z, hit_surface);

    const __vec4f lightPos_x = zero;
    const __vec4f lightPos_y = zero;
    const __vec4f lightPos_z = two;

    const __vec4f background_r = half;
    const __vec4f background_g = half;
    const __vec4f background_b = one;

    for (uint32_t i = 0; i < hit_surface->chunked_size; ++i)
    {
        const int32_t step = i * 4;

        __vec4f isHitting = load(hit_surface->isHitting + step);
        __vec4f mask1 = nequal(&isHitting, &zero);

        if (fuse(&mask1) == 0)
        {
            store(pixel_color->r + step, &background_r);
            store(pixel_color->g + step, &background_g);
            store(pixel_color->b + step, &background_b);
            continue;
        }

        const __vec4f hit_point_x = load(hit_surface->hpx + step);
        const __vec4f hit_point_y = load(hit_surface->hpy + step);
        const __vec4f hit_point_z = load(hit_surface->hpz + step);

        const __vec4f ray_direction_x = load(r->dir_x + step);
        const __vec4f ray_direction_y = load(r->dir_y + step);
        const __vec4f ray_direction_z = load(r->dir_z + step);

        __vec4f lightDirX = sub_(&lightPos_x, &hit_point_x);
        __vec4f lightDirY = sub_(&lightPos_y, &hit_point_y);
        __vec4f lightDirZ = sub_(&lightPos_z, &hit_point_z);

        norm_(&lightDirX, &lightDirY, &lightDirZ, &lightDirX, &lightDirY, &lightDirZ);

        __vec4f nX = load(hit_surface->hnx + step);
        __vec4f nY = load(hit_surface->hny + step);
        __vec4f nZ = load(hit_surface->hnz + step);

        norm_(&nX, &nY, &nZ, &nX, &nY, &nZ);

        __vec4f k = dot_(&nX, &nY, &nZ, &ray_direction_x, &ray_direction_y, &ray_direction_z);

        __vec4f mask0 = greater(&k, &zero);

        __vec4f mnx = sub_(&zero, &nX);
        __vec4f mny = sub_(&zero, &nY);
        __vec4f mnz = sub_(&zero, &nZ);

        nX = blendv(&nX, &mnx, &mask0);
        nY = blendv(&nY, &mny, &mask0);
        nZ = blendv(&nZ, &mnz, &mask0);

        __vec4f d = dot_(&nX, &nY, &nZ, &lightDirX, &lightDirY, &lightDirZ);

        __vec4f intensity = max_(&d, &zero);

        __vec4f hcr = load(hit_surface->hcr + step);
        __vec4f hcg = load(hit_surface->hcg + step);
        __vec4f hcb = load(hit_surface->hcb + step);

        __vec4f color_r = mul_(&intensity, &hcr);
        __vec4f color_g = mul_(&intensity, &hcg);
        __vec4f color_b = mul_(&intensity, &hcb);

        __vec4f color__r = blendv(&background_r, &color_r, &mask1);
        __vec4f color__g = blendv(&background_g, &color_g, &mask1);
        __vec4f color__b = blendv(&background_b, &color_b, &mask1);

        store(pixel_color->r + step, &color__r);
        store(pixel_color->g + step, &color__g);
        store(pixel_color->b + step, &color__b);
    }
}

void frandom_Ray_demi_sphere_cosine_weighted(__vec4f *dir_x, __vec4f *dir_y, __vec4f *dir_z, const __vec4f *origin_x, const __vec4f *origin_y, const __vec4f *origin_z, const __vec4f *normal_x, const __vec4f *normal_y, const __vec4f *normal_z, unsigned int *seed)
{

    const float u01 = (float)rand_r(seed) / (float)RAND_MAX;
    const float u02 = (float)rand_r(seed) / (float)RAND_MAX;
    const float u11 = (float)rand_r(seed) / (float)RAND_MAX;
    const float u12 = (float)rand_r(seed) / (float)RAND_MAX;
    const float u21 = (float)rand_r(seed) / (float)RAND_MAX;
    const float u22 = (float)rand_r(seed) / (float)RAND_MAX;
    const float u31 = (float)rand_r(seed) / (float)RAND_MAX;
    const float u32 = (float)rand_r(seed) / (float)RAND_MAX;

    __vec4f u_n1 = set(u31, u21, u11, u01);
    __vec4f u_n2 = set(u32, u22, u12, u02);

    __vec4f atheta = sqrt_(&u_n1);
    __vec4f phi = mul_(&two_pi, &u_n2);

    // les coordonnées dans la base locale

    __vec4f x = cos_(&phi);
    __vec4f y = sin_(&phi);

    x = mul_(&x, &atheta);
    y = mul_(&y, &atheta);

    __vec4f z = sub_(&one, &u_n1);
    z = sqrt_(&z);

    // coordonnées dans la base orthonormée (up,right,forward) https://www.opengl-tutorial.org/fr/intermediate-tutorials/tutorial-13-normal-mapping/

    __vec4f basic_x = set1(0);
    __vec4f basic_y = set1(1);
    __vec4f basic_z = set1(0);

    __vec4f right_x = set1(1);
    __vec4f right_y = set1(0);
    __vec4f right_z = set1(0);

    __vec4f diff = sub_(&one, &epsilon);
    __vec4f fnormal_y = absf_(normal_y);
    __vec4f mask_normal_y = lower(&fnormal_y, &diff);

    __vec4f up_x = blendv(&right_x, &basic_x, &mask_normal_y);
    __vec4f up_y = blendv(&right_y, &basic_y, &mask_normal_y);
    __vec4f up_z = blendv(&right_z, &basic_z, &mask_normal_y);

    __vec4f tangent_x;
    __vec4f tangent_y;
    __vec4f tangent_z;

    cross_(normal_x, normal_y, normal_z, &up_x, &up_y, &up_z, &tangent_x, &tangent_y, &tangent_z);
    norm_(&tangent_x, &tangent_y, &tangent_z, &tangent_x, &tangent_y, &tangent_z);

    __vec4f bitangent_x;
    __vec4f bitangent_y;
    __vec4f bitangent_z;

    cross_(&tangent_x, &tangent_y, &tangent_z, normal_x, normal_y, normal_z, &bitangent_x, &bitangent_y, &bitangent_z);

    tangent_x = mul_(&tangent_x, &x);
    tangent_y = mul_(&tangent_y, &x);
    tangent_z = mul_(&tangent_z, &x);

    bitangent_x = mul_(&bitangent_x, &y);
    bitangent_y = mul_(&bitangent_y, &y);
    bitangent_z = mul_(&bitangent_z, &y);

    __vec4f norm_x = mul_(normal_x, &z);
    __vec4f norm_y = mul_(normal_y, &z);
    __vec4f norm_z = mul_(normal_z, &z);

    *dir_x = add_(&tangent_x, &bitangent_x);
    *dir_y = add_(&tangent_y, &bitangent_y);
    *dir_z = add_(&tangent_z, &bitangent_z);

    *dir_x = add_(dir_x, &norm_x);
    *dir_y = add_(dir_y, &norm_y);
    *dir_z = add_(dir_z, &norm_z);
}

void fray_sampling(fScene *scene, fRay *r, const float *background_color, fRGB *radiance, fHit *hit_surface, int dmax, unsigned int *seed)
{
    __vec4f throughput_r = one;
    __vec4f throughput_g = one;
    __vec4f throughput_b = one;

    __vec4f current_ray_ori_x = load(r->ori_x);
    __vec4f current_ray_ori_y = load(r->ori_y);
    __vec4f current_ray_ori_z = load(r->ori_z);

    __vec4f current_ray_dir_x = load(r->dir_x);
    __vec4f current_ray_dir_y = load(r->dir_y);
    __vec4f current_ray_dir_z = load(r->dir_z);

    __vec4f background_color_r = set1(background_color[0]);
    __vec4f background_color_g = set1(background_color[1]);
    __vec4f background_color_b = set1(background_color[2]);

    __vec4f radiance_r = set1(0.0f);
    __vec4f radiance_g = set1(0.0f);
    __vec4f radiance_b = set1(0.0f);

    __vec4f mask_emissive = set1(Emissive);
    __vec4f mask_lambertian = set1(Lambertian);
    __vec4f mask_specular = set1(Specular);

    __vec4f mask_is_active = set1(-1);
    __vec4f mask_is_not_active_and_emissive = set1(0.0f);
    __vec4f zero_nine = set1(0.9f);

    for (int d = 0; d < dmax; ++d)
    {
        intersect_in_scene_f(scene, &current_ray_ori_x, &current_ray_ori_y, &current_ray_ori_z, &current_ray_dir_x, &current_ray_dir_y, &current_ray_dir_z, hit_surface);
        __vec4f mask_is_hitting = load(hit_surface->isHitting);

        // if ray active and hit miss
        __vec4f mask_is_miss = andnot_(&mask_is_hitting, &mask_is_active);
        __vec4f color_is_miss_r = mul_(&background_color_r, &mask_is_miss);
        __vec4f color_is_miss_g = mul_(&background_color_g, &mask_is_miss);
        __vec4f color_is_miss_b = mul_(&background_color_b, &mask_is_miss);

        color_is_miss_r = mul_(&throughput_r, &color_is_miss_r);
        color_is_miss_g = mul_(&throughput_g, &color_is_miss_g);
        color_is_miss_b = mul_(&throughput_b, &color_is_miss_b);

        radiance_r = add_(&radiance_r, &color_is_miss_r);
        radiance_g = add_(&radiance_g, &color_is_miss_g);
        radiance_b = add_(&radiance_b, &color_is_miss_b);

        // Mask out unactive rays
        mask_is_active = and_(&mask_is_active, &mask_is_hitting);

        // normal at hit surface
        __vec4f hit_surface_nx = load(hit_surface->hnx);
        __vec4f hit_surface_ny = load(hit_surface->hny);
        __vec4f hit_surface_nz = load(hit_surface->hnz);

        // dot product of n and the current ray
        __vec4f dot_normal_ray_dir = dot_(&hit_surface_nx, &hit_surface_ny, &hit_surface_nz, &current_ray_dir_x, &current_ray_dir_y, &current_ray_dir_z);

        // mask for the if (dot(n,r) > 0)
        __vec4f mask_is_dot_greater_than_zero = greater(&dot_normal_ray_dir, &zero);

        __vec4f sub_0 = sub_(&zero, &hit_surface_nx);
        __vec4f sub_1 = sub_(&zero, &hit_surface_ny);
        __vec4f sub_2 = sub_(&zero, &hit_surface_nz);

        // if the normal at hit surface is behind the surface
        hit_surface_nx = blendv(&hit_surface_nx, &sub_0, &mask_is_dot_greater_than_zero);
        hit_surface_ny = blendv(&hit_surface_ny, &sub_1, &mask_is_dot_greater_than_zero);
        hit_surface_nz = blendv(&hit_surface_nz, &sub_2, &mask_is_dot_greater_than_zero);

        // hit surface point
        __vec4f hit_surface_px = load(hit_surface->hpx);
        __vec4f hit_surface_py = load(hit_surface->hpy);
        __vec4f hit_surface_pz = load(hit_surface->hpz);

        // albedo at hit surface
        __vec4f albedo = load(hit_surface->albedo);

        __vec4f hit_surface_nx_eps = mul_(&hit_surface_nx, &epsilon);
        __vec4f hit_surface_ny_eps = mul_(&hit_surface_ny, &epsilon);
        __vec4f hit_surface_nz_eps = mul_(&hit_surface_nz, &epsilon);

        // offset origin
        __vec4f offset_origin_x = add_(&hit_surface_px, &hit_surface_nx_eps);
        __vec4f offset_origin_y = add_(&hit_surface_py, &hit_surface_ny_eps);
        __vec4f offset_origin_z = add_(&hit_surface_pz, &hit_surface_nz_eps);

        __vec4f obj_color_r = load(hit_surface->hcr);
        __vec4f obj_color_g = load(hit_surface->hcg);
        __vec4f obj_color_b = load(hit_surface->hcb);

        __vec4f type = load(hit_surface->type);

        __vec4f mask_is_emissive = equal(&type, &mask_emissive);
        __vec4f mask_is_lambertian = equal(&type, &mask_lambertian);
        __vec4f mask_is_specular = equal(&type, &mask_specular);

        // Emissive
        __vec4f albedo_with_color_r = mul_(&albedo, &obj_color_r);
        __vec4f albedo_with_color_g = mul_(&albedo, &obj_color_g);
        __vec4f albedo_with_color_b = mul_(&albedo, &obj_color_b);

        __vec4f radiance_emissive_r = mul_(&throughput_r, &obj_color_r);
        __vec4f radiance_emissive_g = mul_(&throughput_g, &obj_color_g);
        __vec4f radiance_emissive_b = mul_(&throughput_b, &obj_color_b);

        // Lambertian

        __vec4f albedo_with_color_r_0 = mul_(&albedo, &obj_color_r);
        __vec4f albedo_with_color_g_0 = mul_(&albedo, &obj_color_g);
        __vec4f albedo_with_color_b_0 = mul_(&albedo, &obj_color_b);

        __vec4f inv_pi = set1(0.31830988618f);

        albedo_with_color_r_0 = mul_(&albedo_with_color_r_0, &inv_pi);
        albedo_with_color_g_0 = mul_(&albedo_with_color_g_0, &inv_pi);
        albedo_with_color_b_0 = mul_(&albedo_with_color_b_0, &inv_pi);

        __vec4f lambertian_throughput_r = mul_(&throughput_r, &albedo_with_color_r_0);
        __vec4f lambertian_throughput_g = mul_(&throughput_g, &albedo_with_color_g_0);
        __vec4f lambertian_throughput_b = mul_(&throughput_b, &albedo_with_color_b_0);
        // Specular
        __vec4f dot_normal_ray_dir_1 = dot_(&hit_surface_nx, &hit_surface_ny, &hit_surface_nz, &current_ray_dir_x, &current_ray_dir_y, &current_ray_dir_z);
        __vec4f two_dot = mul_(&two, &dot_normal_ray_dir_1);

        __vec4f mul_x = mul_(&two_dot, &hit_surface_nx);
        __vec4f mul_y = mul_(&two_dot, &hit_surface_ny);
        __vec4f mul_z = mul_(&two_dot, &hit_surface_nz);

        __vec4f specular_throughput_r = mul_(&throughput_r, &zero_nine);
        __vec4f specular_throughput_g = mul_(&throughput_g, &zero_nine);
        __vec4f specular_throughput_b = mul_(&throughput_b, &zero_nine);

        // Emissive (no bounce)

        // Lambertian bounce
        __vec4f lambertian_bounce_dir_x;
        __vec4f lambertian_bounce_dir_y;
        __vec4f lambertian_bounce_dir_z;

        frandom_Ray_demi_sphere_cosine_weighted(&lambertian_bounce_dir_x, &lambertian_bounce_dir_y, &lambertian_bounce_dir_z, &offset_origin_x, &offset_origin_y, &offset_origin_z, &hit_surface_nx, &hit_surface_ny, &hit_surface_nz, seed);

        // Specular bounce
        __vec4f specular_bounce_dir_x = sub_(&current_ray_dir_x, &mul_x);
        __vec4f specular_bounce_dir_y = sub_(&current_ray_dir_y, &mul_y);
        __vec4f specular_bounce_dir_z = sub_(&current_ray_dir_z, &mul_z);
        norm_(&specular_bounce_dir_x, &specular_bounce_dir_y, &specular_bounce_dir_z, &specular_bounce_dir_x, &specular_bounce_dir_y, &specular_bounce_dir_z);

        // Which bounce

        // if material type is emissive so no bounce
        __vec4f new_bounce_x = zero;
        __vec4f new_bounce_y = zero;
        __vec4f new_bounce_z = zero;

        // if material type is lambertian, lambertian bounce
        new_bounce_x = blendv(&new_bounce_x, &lambertian_bounce_dir_x, &mask_is_lambertian);
        new_bounce_y = blendv(&new_bounce_y, &lambertian_bounce_dir_y, &mask_is_lambertian);
        new_bounce_z = blendv(&new_bounce_z, &lambertian_bounce_dir_z, &mask_is_lambertian);

        throughput_r = blendv(&throughput_r, &lambertian_throughput_r, &mask_is_lambertian);
        throughput_g = blendv(&throughput_g, &lambertian_throughput_g, &mask_is_lambertian);
        throughput_b = blendv(&throughput_b, &lambertian_throughput_b, &mask_is_lambertian);

        // if material type is specular, specular bounce
        new_bounce_x = blendv(&new_bounce_x, &specular_bounce_dir_x, &mask_is_specular);
        new_bounce_y = blendv(&new_bounce_y, &specular_bounce_dir_y, &mask_is_specular);
        new_bounce_z = blendv(&new_bounce_z, &specular_bounce_dir_z, &mask_is_specular);

        throughput_r = blendv(&throughput_r, &specular_throughput_r, &mask_is_specular);
        throughput_g = blendv(&throughput_g, &specular_throughput_g, &mask_is_specular);
        throughput_b = blendv(&throughput_b, &specular_throughput_b, &mask_is_specular);

        // if some ray are not active anymore because they hitted a light source
        mask_is_not_active_and_emissive = and_(&mask_is_active, &mask_is_emissive);

        __vec4f accum_rad_r = blendv(&zero, &radiance_emissive_r, &mask_is_not_active_and_emissive);
        __vec4f accum_rad_g = blendv(&zero, &radiance_emissive_g, &mask_is_not_active_and_emissive);
        __vec4f accum_rad_b = blendv(&zero, &radiance_emissive_b, &mask_is_not_active_and_emissive);

        radiance_r = add_(&radiance_r, &accum_rad_r);
        radiance_g = add_(&radiance_g, &accum_rad_g);
        radiance_b = add_(&radiance_b, &accum_rad_b);

        mask_is_active = blendv(&mask_is_active, &zero, &mask_is_emissive);

        current_ray_ori_x = offset_origin_x;
        current_ray_ori_y = offset_origin_y;
        current_ray_ori_z = offset_origin_z;

        current_ray_dir_x = new_bounce_x;
        current_ray_dir_y = new_bounce_y;
        current_ray_dir_z = new_bounce_z;

        if (fuse(&mask_is_active) == 0)
        {
            break;
        }
    }
    store(radiance->r, &radiance_r);
    store(radiance->g, &radiance_g);
    store(radiance->b, &radiance_b);
}

void fpath_tracing(fScene *scene, size_t n_sample, size_t n_bounce, size_t width, size_t height, fImage *img)
{
    fRGB batch_radiance;
    set_fRGB(&batch_radiance, 4);

    fHit batch_hit;
    set_fHit(&batch_hit, 4);
    const float background_color[3] = {0.1, 0.1, 0.5};

    fRay batch_ray;
    set_fray(&batch_ray, 4);

    unsigned int seed = time(NULL);

    for (uint64_t y = 0; y < height; ++y)
    {
        const __vec4f y_s = _mm_set1_ps(y);
        for (uint64_t x = 0; x < width; x += 4)
        {
            const __vec4f x_s = set(x + 3, x + 2, x + 1, x);
            trace_ray_simd(&scene->cam, &scene->inv_w, &scene->inv_h, &scene->aspr_, &x_s, &y_s, &scene->fov, &batch_ray);

            float accumulate_r[4] = {0}, accumulate_g[4] = {0}, accumulate_b[4] = {0};

            for (uint64_t s = 0; s < n_sample; ++s)
            {
                fray_sampling(scene, &batch_ray, background_color, &batch_radiance, &batch_hit, n_bounce, &seed);
                for (int i = 0; i < 4; i++)
                {
                    accumulate_r[i] += batch_radiance.r[i];
                    accumulate_g[i] += batch_radiance.g[i];
                    accumulate_b[i] += batch_radiance.b[i];
                }
            }
            const float inv_n = 1.0f / n_sample;
            for (int i = 0; i < 4; i++)
            {
                batch_radiance.r[i] = accumulate_r[i] * inv_n;
                batch_radiance.g[i] = accumulate_g[i] * inv_n;
                batch_radiance.b[i] = accumulate_b[i] * inv_n;
            }

            put_pixel(img, y, x, &batch_radiance);
        }
    }

    free_fRay(&batch_ray);
    free_fHit(&batch_hit);
    free_fRGB(&batch_radiance);
}

void fBenchmark_mouse(fScene *scene, size_t width, size_t height)
{

    float position[3] = {0.0f, -0.2f, 0.9f};

    set_fScene(scene, position, 50.f, 10, 0, 10, 5, width, height);

    const float beige[3] = {198.0f / 255.0f, 146.0f / 255.0f, 148.0f / 255.0f};
    const float black[3] = {0.0f, 0.0f, 0.0f};
    const float white[3] = {1.0f, 1.0f, 1.0f};
    const float color_h[3] = {255.0f / 255.0f, 220.0f / 255.0f, 180.0f / 255.0f};

    const float head[3] = {0, 0, -1.5};

    const float left_ear[3] = {-0.45, 0.45, -1.6};
    const float right_ear[3] = {0.45, 0.45, -1.6};

    const float p0[3] = {-0.18, 0.12, -1.03};
    const float p1[3] = {-0.18, 0.10, -0.98};
    const float p2[3] = {0.18, 0.12, -1.03};
    const float p3[3] = {0.18, 0.10, -0.98};
    const float p4[3] = {0.0, -0.02, -0.99};
    const float p5[3] = {-0.30, -0.05, -1.05};
    const float p6[3] = {0.30, -0.05, -1.05};

    add_fSphere(&scene->spheres, head, color_h, 0.5, 1, Lambertian);
    add_fSphere(&scene->spheres, left_ear, black, 0.30, 1, Lambertian);
    add_fSphere(&scene->spheres, right_ear, black, 0.30, 1, Lambertian);
    add_fSphere(&scene->spheres, p0, white, 0.10, 1, Lambertian);
    add_fSphere(&scene->spheres, p1, black, 0.06, 1, Lambertian);

    add_fSphere(&scene->spheres, p2, white, 0.10, 1, Lambertian);
    add_fSphere(&scene->spheres, p3, black, 0.06, 1, Lambertian);
    add_fSphere(&scene->spheres, p4, black, 0.10, 1, Lambertian);
    add_fSphere(&scene->spheres, p5, beige, 0.07, 1, Lambertian);
    add_fSphere(&scene->spheres, p6, beige, 0.07, 1, Lambertian);

    const float min[3] = {-5.0f, -3.0f, -5.0f};
    const float max[3] = {5.0f, 3.0f, 5.0f};

    const float dx[3] = {max[0] - min[0], 0.0f, 0.0f};
    const float dy[3] = {0.0f, max[1] - min[1], 0.0f};
    const float dz[3] = {0.0f, 0.0f, max[2] - min[2]};

    const float mdx[3] = {-max[0] + min[0], 0.0f, 0.0f};
    const float mdy[3] = {0.0f, -max[1] + min[1], 0.0f};
    const float mdz[3] = {0.0f, 0.0f, -max[2] + min[2]};

    const float red[3] = {1.0f, 0.0f, 0.0};
    const float green[3] = {0.0f, 1.0f, 0.0};
    const float blue[3] = {0.0f, 0.0f, 1.0};
    const float white_light[3] = {10.0f, 10.0f, 10.0f};
    const float color_0[3] = {0.9019f, 0.9019f, 0.9019f};
    const float orange[3] = {0.92f, 0.92f, 0.92f};

    add_fQuad(&scene->quads, min, dy, dz, color_0, 0.9, Lambertian);
    add_fQuad(&scene->quads, max, mdy, mdz, color_0, 0.9, Lambertian);
    add_fQuad(&scene->quads, min, dx, dz, color_0, 0.9, Lambertian);
    add_fQuad(&scene->quads, max, mdx, mdz, white_light, 10.0f, Emissive);
    add_fQuad(&scene->quads, min, dx, dy, color_0, 0.9, Lambertian);
}
