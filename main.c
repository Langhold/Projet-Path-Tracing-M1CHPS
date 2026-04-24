#include <simd/fImage.h>
#include <simd/fLight.h>
#include <simd/fScene.h>
#include <simd/fRay.h>
#include <time.h>

int main(int argc, char **argv)
{
    srand(time(NULL));

    (void)argc;
    (void)argv;
    init_global_variable();

    uint64_t w = 800;
    uint64_t h = 600;

    fImage img;
    set_fImage(&img, w, h);

    fRGB p;
    set_fRGB(&p, 4);

    fHit hp;
    set_fHit(&hp, 4);

    fSphere sph;
    fQuad cornel_box;

    fBenchmark_mouse(&sph, &cornel_box);

    const float inv_width = 1.0f / img.width;
    const float inv_height = 1.0f / img.height;
    const float aspr = (float)(img.width) / img.height;
    const float degree = 60.f;
    const __m128 fov = _mm_set1_ps(tan(radian(degree * 0.5f)));
    const __m128 aspr_ = _mm_set1_ps(aspr);

    const __m128 inv_w = _mm_set1_ps(inv_width);
    const __m128 inv_h = _mm_set1_ps(inv_height);

    const float background_color[3] = {0.1, 0.1, 0.5};

    fRay batch_ray;
    set_fray(&batch_ray, 4);

    const uint64_t n_sample = 100;

    unsigned int seed = time(NULL);

    for (uint64_t y_ = 0; y_ < img.height; ++y_)
    {
        const __m128 y_s = _mm_set1_ps(y_);
        for (uint64_t x_ = 0; x_ < img.width; x_ += 4)
        {
            __vec4f x_s = set(x_ + 3, x_ + 2, x_ + 1, x_);
            trace_ray_simd(&inv_w, &inv_h, &aspr_, &x_s, &y_s, &fov, &batch_ray);

            float acc_r[4] = {0}, acc_g[4] = {0}, acc_b[4] = {0};

            for (uint64_t s = 0; s < n_sample; ++s)
            {
                fray_sampling(&batch_ray, &sph, &cornel_box, background_color, &p, &hp, 5, &seed);
                for (int i = 0; i < 4; i++)
                {
                    acc_r[i] += p.r[i];
                    acc_g[i] += p.g[i];
                    acc_b[i] += p.b[i];
                }
            }
            const float inv_n = 1.0f / n_sample;
            for (int i = 0; i < 4; i++)
            {
                p.r[i] = acc_r[i] * inv_n;
                p.g[i] = acc_g[i] * inv_n;
                p.b[i] = acc_b[i] * inv_n;
            }

            put_pixel(&img, y_, x_, &p);
        }
    }

    create_file(&img);
    free_fQuad(&cornel_box);
    free_fSphere(&sph);
    free_fImage(&img);
    free_fRay(&batch_ray);
    free_fHit(&hp);
    free_fRGB(&p);
}