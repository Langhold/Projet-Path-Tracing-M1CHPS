
#include <simd/fHit.h>

void set_fHit(fHit *h, uint64_t c)
{

    h->size = 0;
    h->capacity = 4;
    h->padding = 4;

    h->chunked_size = 4;

    h->hit_point = (float *)(aligned_alloc(16, 3 * h->capacity * sizeof(float)));
    h->hpx = h->hit_point;
    h->hpy = h->hit_point + h->capacity;
    h->hpz = h->hit_point + 2 * h->capacity;

    h->hit_normal = (float *)(aligned_alloc(16, 3 * h->capacity * sizeof(float)));
    h->hnx = h->hit_normal;
    h->hny = h->hit_normal + h->capacity;
    h->hnz = h->hit_normal + 2 * h->capacity;

    h->hit_color = (float *)(aligned_alloc(16, 3 * h->capacity * sizeof(float)));
    h->hcr = h->hit_color;
    h->hcg = h->hit_color + h->capacity;
    h->hcb = h->hit_color + 2 * h->capacity;

    h->isHitting = (float *)(aligned_alloc(16, h->capacity * sizeof(float)));
    h->albedo = (float *)(aligned_alloc(16, h->capacity * sizeof(float)));
    h->type = (float *)(aligned_alloc(16, h->capacity * sizeof(float)));

    __m128 zero = _mm_setzero_ps();

    {
        const uint32_t step = 0;

        store(h->hpx + step, &zero);
        store(h->hpy + step, &zero);
        store(h->hpz + step, &zero);

        store(h->hnx + step, &zero);
        store(h->hny + step, &zero);
        store(h->hnz + step, &zero);

        store(h->hcr + step, &zero);
        store(h->hcg + step, &zero);
        store(h->hcb + step, &zero);

        store(h->isHitting + step, &zero);
        store(h->albedo + step, &zero);
        store(h->type + step, &zero);
    }
};

void free_fHit(fHit *h)
{
    free(h->hit_color);
    free(h->hit_point);
    free(h->hit_normal);
    free(h->isHitting);
    free(h->albedo);
    free(h->type);
}