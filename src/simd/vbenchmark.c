#include <simd/fLight.h>

void vbenchmark1(fScene *scene, size_t width, size_t height)
{

    float position[3] = {0.0f, -0.2f, 0.9f};

    set_fScene(scene, position, 50.f, 10, 0, 10, 5, width, height);

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
    const float color_0[3] = {0.9019f, 0.9019f, 0.9019f};
    const float orange[3] = {0.92f, 0.92f, 0.92f};

    add_fQuad(&scene->quads, min, dy, dz, color_0, 0.0, Lambertian);
    add_fQuad(&scene->quads, max, mdy, mdz, color_0, 0.0, Lambertian);
    add_fQuad(&scene->quads, min, dx, dz, color_0, 0.0, Lambertian);
    add_fQuad(&scene->quads, max, mdx, mdz, white, 2.0f, Emissive);
    add_fQuad(&scene->quads, min, dx, dy, color_0, 0.9, Lambertian);
}

void vbenchmark_medium(fScene *scene, size_t width, size_t height)
{
    const float position[3] = {0.0f, 0.0f, 1.1};
    const float head[3] = {0, 0, -1.5};

    const float min[3] = {-10.0f, -6.0f, -10.0f};
    const float max[3] = {10.0f, 6.0f, 10.0f};

    const float dx[3] = {max[0] - min[0], 0.0f, 0.0f};
    const float dy[3] = {0.0f, max[1] - min[1], 0.0f};
    const float dz[3] = {0.0f, 0.0f, max[2] - min[2]};

    const float mdx[3] = {-max[0] + min[0], 0.0f, 0.0f};
    const float mdy[3] = {0.0f, -max[1] + min[1], 0.0f};
    const float mdz[3] = {0.0f, 0.0f, -max[2] + min[2]};

    const float red[3] = {1.0f, 0.0f, 0.0};
    const float green[3] = {0.0f, 1.0f, 0.0};
    const float blue[3] = {0.0f, 0.0f, 1.0};
    const float white_light[3] = {5.0f, 5.0f, 5.0f};
    const float white[3] = {1.0f, 1.0f, 1.0f};

    const float color_0[3] = {0.9019f, 0.9019f, 0.9019f};
    const float orange[3] = {0.92f, 0.92f, 0.92f};

    set_fScene(scene, position, 50.f, 10, 0, 1, 12, width, height);

    add_fQuad(&scene->quads, min, dy, dz, color_0, 0.0, Lambertian);
    add_fQuad(&scene->quads, max, mdy, mdz, color_0, 0.0, Lambertian);

    add_fQuad(&scene->quads, min, dx, dz, color_0, 0.0, Lambertian);
    add_fQuad(&scene->quads, max, mdx, mdz, white, 0.2f, Emissive);

    add_fQuad(&scene->quads, min, dx, dy, color_0, 0.0, Lambertian);
    add_fQuad(&scene->quads, max, mdx, mdy, color_0, 0.0, Lambertian);

    add_fSphere(&scene->spheres, head, white, 0.5f, 0.0, Specular);

    const float cube_position[3] = {-0.8f, 0.5f, -1.2f};
    add_fSquare(&scene->quads, 0.5, 0.5, 0.5, cube_position, red, 20.f, Emissive);
}

void vbenchmark_huge(fScene *scene, size_t width, size_t height)
{

    float position[3] = {0.0f, 0.0f, 1.0};
    fMaterial_t mat[3] = {Lambertian, Emissive, Specular};

    int amount = 2500;
    int amount_s = (int)sqrtf((float)amount);

    int dist = 10;
    float ymax = dist * tanf(25.f * M_PI / 180.0f);

    float xmax = ymax * width / height;

    const float color[3] = {1.0f, 1.0f, 1.0f};

    const size_t num_sph = (amount_s * amount_s) / 2;
    const size_t num_qua = ((amount_s * amount_s) / 2) * 6 + 5;

    set_fScene(scene, position, 50.f, 0, 0., num_sph, num_qua, width, height);

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

    const float red[3] = {1.0f, 0.0f, 0.0};
    const float green[3] = {0.0f, 1.0f, 0.0};
    const float blue[3] = {0.0f, 0.0f, 1.0};
    const float white[3] = {10.0f, 10.0f, 10.0f};
    const float color_0[3] = {0.9019f, 0.9019f, 0.9019f};
    const float orange[3] = {0.92f, 0.92f, 0.92f};

    add_fQuad(&scene->quads, min, dy, dz, color_0, 0.0, Lambertian);
    add_fQuad(&scene->quads, max, mdy, mdz, color_0, 0.0, Lambertian);

    add_fQuad(&scene->quads, min, dx, dz, color_0, 0.0, Lambertian);
    add_fQuad(&scene->quads, max, mdx, mdz, white, 0.8, Emissive);

    add_fQuad(&scene->quads, min, dx, dy, color_0, 0.0, Lambertian);
    add_fQuad(&scene->quads, max, mdx, mdy, color_0, 0.0, Lambertian);
}
