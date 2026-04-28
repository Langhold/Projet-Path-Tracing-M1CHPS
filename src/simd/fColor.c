#include <simd/fColor.h>

#define ESPILON 1e-5

void set_fRGB(fRGB *rgb, uint64_t c)
{
    rgb->size = 0;
    rgb->capacity = c;
    rgb->padding = 4;

    rgb->chunked_size = rgb->capacity / 4;

    rgb->color = (float *)(aligned_alloc(16, 3 * rgb->capacity * sizeof(float)));
    rgb->r = rgb->color;
    rgb->g = rgb->r + rgb->capacity;
    rgb->b = rgb->g + rgb->capacity;

    store(rgb->r, &zero);
    store(rgb->g, &zero);
    store(rgb->b, &zero);
}

void free_fRGB(fRGB *rgb)
{
    free(rgb->color);
}