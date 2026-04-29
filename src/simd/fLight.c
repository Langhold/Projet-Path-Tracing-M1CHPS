#include <simd/fLight.h>

void phong_model(fScene *scene, vRay *packed_ray, vRGB *packed_pixel_color, vHit *packed_pixel_hit_surface)
{

    vintersect_in_scene(scene, packed_ray, packed_pixel_hit_surface);

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

    __vec4f mask1 = nequal(&packed_pixel_hit_surface->hit_isHitting, &zero);

    if (fuse(&mask1) == 0)
    {
        packed_pixel_color->r = background_r;
        packed_pixel_color->g = background_g;
        packed_pixel_color->b = background_b;
        return;
    }

    __vec4f lightDirX = sub_(&lightPos_x, &packed_pixel_hit_surface->hit_point_x);
    __vec4f lightDirY = sub_(&lightPos_y, &packed_pixel_hit_surface->hit_point_y);
    __vec4f lightDirZ = sub_(&lightPos_z, &packed_pixel_hit_surface->hit_point_z);

    norm_(&lightDirX, &lightDirY, &lightDirZ, &lightDirX, &lightDirY, &lightDirZ);

    __vec4f dot__ = dot_(&packed_pixel_hit_surface->hit_normal_x, &packed_pixel_hit_surface->hit_normal_y, &packed_pixel_hit_surface->hit_normal_z, &packed_ray->dir_x, &packed_ray->dir_y, &packed_ray->dir_z);

    __vec4f mask0 = greater(&dot__, &zero);

    __vec4f sub_0 = sub_(&zero, &packed_pixel_hit_surface->hit_normal_x);
    __vec4f sub_1 = sub_(&zero, &packed_pixel_hit_surface->hit_normal_y);
    __vec4f sub_2 = sub_(&zero, &packed_pixel_hit_surface->hit_normal_z);

    __vec4f normalX = blendv(&packed_pixel_hit_surface->hit_normal_x, &sub_0, &mask0);
    __vec4f normalY = blendv(&packed_pixel_hit_surface->hit_normal_y, &sub_1, &mask0);
    __vec4f normalZ = blendv(&packed_pixel_hit_surface->hit_normal_z, &sub_2, &mask0);

    __vec4f dot_44 = dot_(&normalX, &normalY, &normalZ, &lightDirX, &lightDirY, &lightDirZ);

    __vec4f diff = max_(&dot_44, &zero);

    __vec4f diffuse_r = mul_(&diff, &lightColor_r);
    __vec4f diffuse_g = mul_(&diff, &lightColor_g);
    __vec4f diffuse_b = mul_(&diff, &lightColor_b);

    __vec4f viewDirX;
    __vec4f viewDirY;
    __vec4f viewDirZ;

    norm_(&packed_ray->dir_x, &packed_ray->dir_y, &packed_ray->dir_z, &viewDirX, &viewDirY, &viewDirZ);

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

    __vec4f color_r = mul_(&phong_light_r, &packed_pixel_hit_surface->hit_albedo_r);
    __vec4f color_g = mul_(&phong_light_g, &packed_pixel_hit_surface->hit_albedo_g);
    __vec4f color_b = mul_(&phong_light_b, &packed_pixel_hit_surface->hit_albedo_b);

    packed_pixel_color->r = blendv(&background_r, &color_r, &mask1);
    packed_pixel_color->g = blendv(&background_g, &color_g, &mask1);
    packed_pixel_color->b = blendv(&background_b, &color_b, &mask1);
}

void diffuse_render(fScene *scene, vRay *packed_ray, vRGB *packed_pixel_color, vHit *packed_pixel_hit_surface)
{
    vintersect_in_scene(scene, packed_ray, packed_pixel_hit_surface);

    const __vec4f lightPos_x = zero;
    const __vec4f lightPos_y = zero;
    const __vec4f lightPos_z = two;

    const __vec4f background_r = half;
    const __vec4f background_g = half;
    const __vec4f background_b = one;

    __vec4f mask1 = nequal(&packed_pixel_hit_surface->hit_isHitting, &zero);

    if (fuse(&mask1) == 0)
    {
        packed_pixel_color->r = background_r;
        packed_pixel_color->g = background_g;
        packed_pixel_color->b = background_b;
        return;
    }

    __vec4f lightDirX = sub_(&lightPos_x, &packed_pixel_hit_surface->hit_point_x);
    __vec4f lightDirY = sub_(&lightPos_y, &packed_pixel_hit_surface->hit_point_y);
    __vec4f lightDirZ = sub_(&lightPos_z, &packed_pixel_hit_surface->hit_point_z);

    norm_(&lightDirX, &lightDirY, &lightDirZ, &lightDirX, &lightDirY, &lightDirZ);

    __vec4f nX;
    __vec4f nY;
    __vec4f nZ;

    norm_(&packed_pixel_hit_surface->hit_normal_x, &packed_pixel_hit_surface->hit_normal_y, &packed_pixel_hit_surface->hit_normal_z, &nX, &nY, &nZ);

    __vec4f k = dot_(&nX, &nY, &nZ, &packed_ray->dir_x, &packed_ray->dir_y, &packed_ray->dir_z);

    __vec4f mask0 = greater(&k, &zero);

    __vec4f mnx = sub_(&zero, &nX);
    __vec4f mny = sub_(&zero, &nY);
    __vec4f mnz = sub_(&zero, &nZ);

    nX = blendv(&nX, &mnx, &mask0);
    nY = blendv(&nY, &mny, &mask0);
    nZ = blendv(&nZ, &mnz, &mask0);

    __vec4f d = dot_(&nX, &nY, &nZ, &lightDirX, &lightDirY, &lightDirZ);

    __vec4f intensity = max_(&d, &zero);

    __vec4f color_r = mul_(&intensity, &packed_pixel_hit_surface->hit_albedo_r);
    __vec4f color_g = mul_(&intensity, &packed_pixel_hit_surface->hit_albedo_g);
    __vec4f color_b = mul_(&intensity, &packed_pixel_hit_surface->hit_albedo_b);

    packed_pixel_color->r = blendv(&background_r, &color_r, &mask1);
    packed_pixel_color->g = blendv(&background_g, &color_g, &mask1);
    packed_pixel_color->b = blendv(&background_b, &color_b, &mask1);
}

void vrandom_Ray_demi_sphere_cosine_weighted(__vec4f *dir_x, __vec4f *dir_y, __vec4f *dir_z, const __vec4f *normal_x, const __vec4f *normal_y, const __vec4f *normal_z, unsigned int *seed)
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

    __vec4f diff = sub_(&one, &epsilon_0);
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

void vray_sampling(fScene *scene, vRay *packed_ray, vHit *packed_hit, vRGB *packed_radiance, const float *background_color, int dmax, unsigned int *seed)
{
    __vec4f throughput_r = one;
    __vec4f throughput_g = one;
    __vec4f throughput_b = one;

    vRay current_packed_ray = *packed_ray;

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

        vintersect_in_scene(scene, &current_packed_ray, packed_hit);
        __vec4f mask_is_hitting = packed_hit->hit_isHitting;

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

        // Mask out unactive rays if a rays was previously active but there is no hit
        mask_is_active = and_(&mask_is_active, &mask_is_hitting);

        if (fuse(&mask_is_active) == 0)
            break;

        // dot product of n and the current ray
        __vec4f dot_normal_ray_dir = dot_(&packed_hit->hit_normal_x, &packed_hit->hit_normal_y, &packed_hit->hit_normal_z, &current_packed_ray.dir_x, &current_packed_ray.dir_y, &current_packed_ray.dir_z);

        // mask for the if (dot(n,r) > 0)
        __vec4f mask_is_dot_greater_than_zero = greater(&dot_normal_ray_dir, &zero);

        __vec4f sub_0 = sub_(&zero, &packed_hit->hit_normal_x);
        __vec4f sub_1 = sub_(&zero, &packed_hit->hit_normal_y);
        __vec4f sub_2 = sub_(&zero, &packed_hit->hit_normal_z);

        // if the normal at hit surface is behind the surface
        __vec4f hit_surface_nx = blendv(&packed_hit->hit_normal_x, &sub_0, &mask_is_dot_greater_than_zero);
        __vec4f hit_surface_ny = blendv(&packed_hit->hit_normal_y, &sub_1, &mask_is_dot_greater_than_zero);
        __vec4f hit_surface_nz = blendv(&packed_hit->hit_normal_z, &sub_2, &mask_is_dot_greater_than_zero);

        __vec4f hit_surface_nx_eps = mul_(&hit_surface_nx, &epsilon_1);
        __vec4f hit_surface_ny_eps = mul_(&hit_surface_ny, &epsilon_1);
        __vec4f hit_surface_nz_eps = mul_(&hit_surface_nz, &epsilon_1);

        // offset origin
        __vec4f offset_origin_x = add_(&packed_hit->hit_point_x, &hit_surface_nx_eps);
        __vec4f offset_origin_y = add_(&packed_hit->hit_point_y, &hit_surface_ny_eps);
        __vec4f offset_origin_z = add_(&packed_hit->hit_point_z, &hit_surface_nz_eps);

        __vec4f mask_is_emissive = equal(&packed_hit->hit_mat_type, &mask_emissive);
        __vec4f mask_is_lambertian = equal(&packed_hit->hit_mat_type, &mask_lambertian);
        __vec4f mask_is_specular = equal(&packed_hit->hit_mat_type, &mask_specular);

        // Emissive (no bounce)

        // Lambertian bounce
        __vec4f lambertian_bounce_dir_x;
        __vec4f lambertian_bounce_dir_y;
        __vec4f lambertian_bounce_dir_z;

        vrandom_Ray_demi_sphere_cosine_weighted(&lambertian_bounce_dir_x, &lambertian_bounce_dir_y, &lambertian_bounce_dir_z, &hit_surface_nx, &hit_surface_ny, &hit_surface_nz, seed);

        // Specular bounce

        __vec4f two_dot = mul_(&two, &dot_normal_ray_dir);

        __vec4f mul_x = mul_(&two_dot, &hit_surface_nx);
        __vec4f mul_y = mul_(&two_dot, &hit_surface_ny);
        __vec4f mul_z = mul_(&two_dot, &hit_surface_nz);

        __vec4f specular_bounce_dir_x = sub_(&current_packed_ray.dir_x, &mul_x);
        __vec4f specular_bounce_dir_y = sub_(&current_packed_ray.dir_y, &mul_y);
        __vec4f specular_bounce_dir_z = sub_(&current_packed_ray.dir_z, &mul_z);

        // Emissive
        __vec4f albedo_with_emission_r = mul_(&packed_hit->hit_emissive_power, &packed_hit->hit_albedo_r);
        __vec4f albedo_with_emission_g = mul_(&packed_hit->hit_emissive_power, &packed_hit->hit_albedo_g);
        __vec4f albedo_with_emission_b = mul_(&packed_hit->hit_emissive_power, &packed_hit->hit_albedo_b);

        __vec4f emission_throughput_r = mul_(&throughput_r, &albedo_with_emission_r);
        __vec4f emission_throughput_g = mul_(&throughput_g, &albedo_with_emission_g);
        __vec4f emission_throughput_b = mul_(&throughput_b, &albedo_with_emission_b);

        emission_throughput_r = blendv(&zero, &emission_throughput_r, &mask_is_emissive);
        emission_throughput_g = blendv(&zero, &emission_throughput_g, &mask_is_emissive);
        emission_throughput_b = blendv(&zero, &emission_throughput_b, &mask_is_emissive);

        // Lambertian

        __vec4f lambertian_throughput_r = mul_(&packed_hit->hit_albedo_r, &throughput_r);
        __vec4f lambertian_throughput_g = mul_(&packed_hit->hit_albedo_g, &throughput_g);
        __vec4f lambertian_throughput_b = mul_(&packed_hit->hit_albedo_b, &throughput_b);

        // Specular

        __vec4f specular_throughput_r = mul_(&throughput_r, &packed_hit->hit_albedo_r);
        __vec4f specular_throughput_g = mul_(&throughput_g, &packed_hit->hit_albedo_g);
        __vec4f specular_throughput_b = mul_(&throughput_b, &packed_hit->hit_albedo_b);

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
        mask_is_active = blendv(&mask_is_active, &zero, &mask_is_emissive);

        throughput_r = and_(&throughput_r, &mask_is_active);
        throughput_g = and_(&throughput_g, &mask_is_active);
        throughput_b = and_(&throughput_b, &mask_is_active);

        radiance_r = add_(&radiance_r, &emission_throughput_r);
        radiance_g = add_(&radiance_g, &emission_throughput_g);
        radiance_b = add_(&radiance_b, &emission_throughput_b);

        current_packed_ray.ori_x = blendv(&current_packed_ray.ori_x, &offset_origin_x, &mask_is_active);
        current_packed_ray.ori_y = blendv(&current_packed_ray.ori_y, &offset_origin_y, &mask_is_active);
        current_packed_ray.ori_z = blendv(&current_packed_ray.ori_z, &offset_origin_z, &mask_is_active);

        current_packed_ray.dir_x = blendv(&current_packed_ray.dir_x, &new_bounce_x, &mask_is_active);
        current_packed_ray.dir_y = blendv(&current_packed_ray.dir_y, &new_bounce_y, &mask_is_active);
        current_packed_ray.dir_z = blendv(&current_packed_ray.dir_z, &new_bounce_z, &mask_is_active);
    }

    packed_radiance->r = radiance_r;
    packed_radiance->g = radiance_g;
    packed_radiance->b = radiance_b;
}

void vpath_tracing(fScene *scene, size_t n_sample, size_t n_bounce, size_t width, size_t height, fImage *img)
{

    vRGB packed_radiance;
    vRGB packed_accumulated_radiance;

    vRay packed_ray;
    vHit packed_hit;

    clear_rgb(&packed_radiance);
    clear_rgb(&packed_accumulated_radiance);

    clear_hit(&packed_hit);
    clear_ray(&packed_ray);

    const float background_color[3] = {0.1, 0.1, 0.5};

    __vec4f inv_n = set1(1.0f / n_sample);

    for (uint64_t y = 0; y < height; ++y)
    {
        const __vec4f y_s = _mm_set1_ps(y);
        for (uint64_t x = 0; x < width; x += 4)
        {
            const __vec4f x_s = set(x + 3, x + 2, x + 1, x);
            vtrace_ray(&scene->cam, &packed_ray, &scene->inv_w, &scene->inv_h, &scene->aspr_, &x_s, &y_s, &scene->fov);

            unsigned int seed = (y * width + x);
            for (uint64_t s = 0; s < n_sample; ++s)
            {
                vray_sampling(scene, &packed_ray, &packed_hit, &packed_radiance, background_color, n_bounce, &seed);
                vadd_(&packed_accumulated_radiance, &packed_radiance);
            }

            vscale_(&packed_accumulated_radiance, &inv_n);

            put_pixel(img, y, x, &packed_accumulated_radiance);

            clear_rgb(&packed_radiance);
            clear_rgb(&packed_accumulated_radiance);
        }
    }
}

void vpath_tracing_omp(fScene *scene, size_t n_sample, size_t n_bounce, size_t width, size_t height, fImage *img)
{

    const float background_color[3] = {0.1, 0.1, 0.5};

    __vec4f inv_n = set1(1.0f / n_sample);

#pragma omp parallel for schedule(dynamic, 4)
    for (uint64_t y = 0; y < height; ++y)
    {
        vRGB packed_radiance;
        vRGB packed_accumulated_radiance;

        vRay packed_ray;
        vHit packed_hit;

        clear_rgb(&packed_radiance);
        clear_rgb(&packed_accumulated_radiance);

        clear_hit(&packed_hit);
        clear_ray(&packed_ray);
        const __vec4f y_s = _mm_set1_ps(y);
        for (uint64_t x = 0; x < width; x += 4)
        {
            const __vec4f x_s = set(x + 3, x + 2, x + 1, x);
            vtrace_ray(&scene->cam, &packed_ray, &scene->inv_w, &scene->inv_h, &scene->aspr_, &x_s, &y_s, &scene->fov);

            unsigned int seed = (y * width + x);
            for (uint64_t s = 0; s < n_sample; ++s)
            {
                vray_sampling(scene, &packed_ray, &packed_hit, &packed_radiance, background_color, n_bounce, &seed);
                vadd_(&packed_accumulated_radiance, &packed_radiance);
            }

            vscale_(&packed_accumulated_radiance, &inv_n);

            put_pixel(img, y, x, &packed_accumulated_radiance);

            clear_rgb(&packed_radiance);
            clear_rgb(&packed_accumulated_radiance);
        }
    }
}
