#include <simd/fLight.h>

void vbenchmark_big(fScene *scene, size_t width, size_t height)
{
}

void vbenchmark1(fScene *scene, size_t width, size_t height)
{

    float position[3] = {0.0f, -0.2f, 0.9f};

    const float background_color[3] = {0.90196078431f, 0.90196078431f, 0.90196078431f};
    set_fScene(scene, background_color, position, 50.f, 10, 0, 10, 5, width, height);

    const float beige[3] = {198.0f / 255.0f, 146.0f / 255.0f, 148.0f / 255.0f};
    const float black[3] = {0.1f, 0.1f, 0.1f};
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

    add_fSphere(&scene->spheres, head, color_h, 0.5, 1, vLambertian);
    add_fSphere(&scene->spheres, left_ear, black, 0.30, 1, vLambertian);
    add_fSphere(&scene->spheres, right_ear, black, 0.30, 1, vLambertian);
    add_fSphere(&scene->spheres, p0, white, 0.10, 1, vLambertian);
    add_fSphere(&scene->spheres, p1, black, 0.06, 1, vLambertian);

    add_fSphere(&scene->spheres, p2, white, 0.10, 1, vLambertian);
    add_fSphere(&scene->spheres, p3, black, 0.06, 1, vLambertian);
    add_fSphere(&scene->spheres, p4, black, 0.10, 1, vLambertian);
    add_fSphere(&scene->spheres, p5, beige, 0.07, 1, vLambertian);
    add_fSphere(&scene->spheres, p6, beige, 0.07, 1, vLambertian);

    const float min[3] = {-5.0f, -3.0f, -5.0f};
    const float max[3] = {5.0f, 3.0f, 5.0f};

    const float dx[3] = {max[0] - min[0], 0.0f, 0.0f};
    const float dy[3] = {0.0f, max[1] - min[1], 0.0f};
    const float dz[3] = {0.0f, 0.0f, max[2] - min[2]};

    const float mdx[3] = {-max[0] + min[0], 0.0f, 0.0f};
    const float mdy[3] = {0.0f, -max[1] + min[1], 0.0f};
    const float mdz[3] = {0.0f, 0.0f, -max[2] + min[2]};

    const float color_0[3] = {0.9019f, 0.9019f, 0.9019f};

    add_fQuad(&scene->quads, min, dy, dz, color_0, 0.0, vLambertian);
    add_fQuad(&scene->quads, max, mdy, mdz, color_0, 0.0, vLambertian);
    add_fQuad(&scene->quads, min, dx, dz, color_0, 0.0, vLambertian);
    add_fQuad(&scene->quads, max, mdx, mdz, white, 2.0f, vEmissive);
    add_fQuad(&scene->quads, min, dx, dy, color_0, 0.9, vLambertian);
}

void vbenchmark_medium(fScene *scene, size_t width, size_t height)
{

    const float x0 = 0;
    const float y0 = 0;
    const float z0 = 17;
    const float fov = 50;
    float position[3] = {x0, y0, z0};

    const float background_color[3] = {0.1647f, 0.90196078431f, 0.1647f};

    set_fScene(scene, background_color, position, fov, 0, 0, 4, 54, width, height);
    const float r = 1.5;

    ///////////////////////////////////////////////////////////////////////////////////////////////////////////////////
    const float raybox_color[3] = {0.1294f, 0.1294f, 0.4941f};
    const float raybox_positions[3] = {0, 2.5, 0};
    add_fSquare(&scene->quads, 28.5, 20, 50, raybox_positions, raybox_color, 0, vLambertian);

    const float pylone_color[3] = {1.f, 0.0392f, 0.0392f};
    const float pylone_positions[3] = {-12, 0, -20};
    add_fSquare(&scene->quads, 0.5, 15, 0.5, pylone_positions, pylone_color, 0, vLambertian);

    const float pylone_base_color[3] = {1.f, 0.0392f, 0.0392f};
    const float pylone_base_positions[3] = {-12, -7., -20};
    add_fSquare(&scene->quads, 4, 1, 4, pylone_base_positions, pylone_base_color, 0, vLambertian);

    const float pylone_up_color[3] = {1.f, 0.0392f, 0.0392f};
    const float pylone_up_positions[3] = {-12, 7.2, -20};
    add_fSquare(&scene->quads, 1.7, 0.6, 1.7, pylone_up_positions, pylone_up_color, 0, vLambertian);

    float x2 = 12, z2 = -20;

    const float light_color[3] = {1.0f, 1.0f, 1.0f};

    const float color_0[3] = {0.482f, 0.482f, 1.0f};
    const float position_0[3] = {x2, 0, z2};
    add_fSquare(&scene->quads, 0.5, 15, 0.5, position_0, color_0, 0, vLambertian);

    const float position_1[3] = {x2, -7, z2};
    add_fSquare(&scene->quads, 4, 1, 4, position_1, color_0, 0, vLambertian);

    const float position_2[3] = {x2, 7.2, z2};
    add_fSquare(&scene->quads, 1.7, 0.6, 1.7, position_2, color_0, 0, vLambertian);

    const float position_3[3] = {x2, 9, z2};
    add_fSquare(&scene->quads, 2, 2.5, 1.7, position_3, light_color, 80, vEmissive);

    const float mirror_position[3] = {-10, -1, -8};
    add_fSquare(&scene->quads, 3, 10, 0.1, mirror_position, light_color, 0.0, vSpecular);

    //////////////////////////////////////////////////////////////////////////////////////////////////////

    const float light_bulb_color[3] = {1.0, 1.0, 1.0};
    const float light_bulb_position[3] = {-12, 9, -20};
    add_fSphere(&scene->spheres, light_bulb_position, light_bulb_color, r, 100, vEmissive);

    const float big_light_color[3] = {0.7936f, 0.7936f, 0.3921f};
    const float big_light_position[3] = {0, -8, 17};
    add_fSphere(&scene->spheres, big_light_position, big_light_color, 4, 120, vEmissive);

    const float my_sphere_color[3] = {0.03921f, 1.0f, 0.03921f};
    const float my_sphere_position[3] = {-8, 2, -8};
    add_fSphere(&scene->spheres, my_sphere_position, my_sphere_color, r, 0.92, vLambertian);

    const float my_mirror_color[3] = {1.0, 1.0, 1.0};
    const float my_mirror_position[3] = {0, 0, -17};
    add_fSphere(&scene->spheres, my_mirror_position, my_mirror_color, 2.5, 1, vSpecular);
}

void vbenchmark_huge(fScene *scene, size_t width, size_t height)
{

    float position[3] = {0.0f, 0.0f, 1.0};
    fMaterial_t mat[3] = {vLambertian, vEmissive, vSpecular};

    int amount = 2500;
    int amount_s = (int)sqrtf((float)amount);

    int dist = 10;
    float ymax = dist * tanf(25.f * M_PI / 180.0f);

    float xmax = ymax * width / height;

    const float color[3] = {1.0f, 1.0f, 1.0f};

    const size_t num_sph = (amount_s * amount_s) / 2;
    const size_t num_qua = ((amount_s * amount_s) / 2) * 6 + 5;

    const float background_color[3] = {0.16470588235f, 0.90196078431f, 0.16470588235f};

    set_fScene(scene, background_color, position, 50.f, 0, 0., num_sph, num_qua, width, height);

    for (int i = 0; i < amount_s; ++i)
    {
        for (int j = 0; j < amount_s; ++j)
        {

            float u = (float)i / (amount_s - 1);
            float v = (float)j / (amount_s - 1);

            float x = -xmax + u * 2.f * xmax;
            float y = -ymax + v * 2.f * ymax;
            float coord[3] = {x, y, -dist};
            if (j % 2 == 0)
            {
                add_fSphere(&scene->spheres, coord, color, 0.075, 3, mat[i % 3 - 1]);
            }
            else
            {
                add_fSquare(&scene->quads, 0.075, 0.075, 0.075, coord, color, 3.f, mat[i % 3 - 1]);
            }
        }
    }

    const float min[3] = {-10.0f, -6.0f, -10.0f};
    const float max[3] = {10.0f, 6.0f, 10.0f};

    const float dx[3] = {max[0] - min[0], 0.0f, 0.0f};
    const float dy[3] = {0.0f, max[1] - min[1], 0.0f};
    const float dz[3] = {0.0f, 0.0f, max[2] - min[2]};

    const float mdx[3] = {-max[0] + min[0], 0.0f, 0.0f};
    const float mdy[3] = {0.0f, -max[1] + min[1], 0.0f};
    const float mdz[3] = {0.0f, 0.0f, -max[2] + min[2]};

    const float white[3] = {10.0f, 10.0f, 10.0f};
    const float color_0[3] = {0.9019f, 0.9019f, 0.9019f};

    add_fQuad(&scene->quads, min, dy, dz, color_0, 0.0, vLambertian);
    add_fQuad(&scene->quads, max, mdy, mdz, color_0, 0.0, vLambertian);

    add_fQuad(&scene->quads, min, dx, dz, color_0, 0.0, vLambertian);
    add_fQuad(&scene->quads, max, mdx, mdz, white, 0.8, vEmissive);

    add_fQuad(&scene->quads, min, dx, dy, color_0, 0.0, vLambertian);
    add_fQuad(&scene->quads, max, mdx, mdy, color_0, 0.0, vLambertian);
}
