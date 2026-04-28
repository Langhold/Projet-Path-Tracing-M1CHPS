#include <simd/fImage.h>

void set_fImage(fImage *img, uint64_t w, uint64_t h)
{
    img->width = w;
    img->height = h;
    img->padding = 4;
    img->capacity = (((h * w) + 3) / 4) * 4;
    img->chunked_size = img->capacity / 4;
    img->pixels = (float *)(aligned_alloc(16, 3 * img->capacity * sizeof(float)));
    img->r = img->pixels;
    img->g = img->r + img->capacity;
    img->b = img->g + img->capacity;

    for (uint64_t i = 0; i < img->chunked_size; i++)
    {
        const uint32_t step = i * img->padding;

        store(img->r + step, &zero);
        store(img->g + step, &zero);
        store(img->b + step, &zero);
    }
}

void put_pixel(fImage *img, uint64_t y, uint64_t x, const fRGB *c)
{
    const uint64_t step = y * img->width + x;

    __vec4f color_r = load(c->r);
    __vec4f color_g = load(c->g);
    __vec4f color_b = load(c->b);

    store(img->r + step, &color_r);
    store(img->g + step, &color_g);
    store(img->b + step, &color_b);
}

void create_file(fImage *img)
{
    char path[100];
    sprintf(path, "image/image_simd.ppm");

    FILE *file = fopen(path, "w");

    if (file == NULL)
    {
        fprintf(stderr, "Error: Failed to create an image file.\n");
        exit(1);
    }

    fprintf(file, "P3\n%ld %ld\n%d\n", img->width, img->height, 255);
    fflush(file);

    for (size_t i = 0; i < img->height * img->width; ++i)
    {
        img->r[i] = img->r[i] < 0.0f ? 0.0f : img->r[i];
        img->r[i] = img->r[i] > 1.0f ? 1.0f : img->r[i];

        img->g[i] = img->g[i] < 0.0f ? 0.0f : img->g[i];
        img->g[i] = img->g[i] > 1.0f ? 1.0f : img->g[i];

        img->b[i] = img->b[i] < 0.0f ? 0.0f : img->b[i];
        img->b[i] = img->b[i] > 1.0f ? 1.0f : img->b[i];

        uint8_t r = (uint8_t)(255.0f * img->r[i]);
        uint8_t g = (uint8_t)(255.0f * img->g[i]);
        uint8_t b = (uint8_t)(255.0f * img->b[i]);

        fprintf(file, "%d %d %d\n", r, g, b);
    }
    fclose(file);
}

void free_fImage(fImage *img)
{
    free(img->pixels);
}
