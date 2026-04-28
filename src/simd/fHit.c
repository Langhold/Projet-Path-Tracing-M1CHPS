
#include <simd/fHit.h>

void set_fHit(fHit *h, uint64_t c)
{

    h->size = 0;
    h->capacity = 4;
    h->padding = 4;

    h->chunked_size = 4;

    h->hit_point = (float *)(aligned_alloc(16, 3 * h->capacity * sizeof(float)));
    h->hpx = h->hit_point;
    h->hpy = h->hpx + h->capacity;
    h->hpz = h->hpy + h->capacity;

    h->hit_normal = (float *)(aligned_alloc(16, 3 * h->capacity * sizeof(float)));
    h->hnx = h->hit_normal;
    h->hny = h->hnx + h->capacity;
    h->hnz = h->hny + h->capacity;

    h->hit_albedo = (float *)(aligned_alloc(16, 3 * h->capacity * sizeof(float)));
    h->har = h->hit_albedo;
    h->hag = h->har + h->capacity;
    h->hab = h->hag + h->capacity;

    h->isHitting = (float *)(aligned_alloc(16, h->capacity * sizeof(float)));
    h->hit_emissive_power = (float *)(aligned_alloc(16, h->capacity * sizeof(float)));
    h->hit_type = (float *)(aligned_alloc(16, h->capacity * sizeof(float)));

    __m128 zero = _mm_setzero_ps();

    {
        const uint32_t step = 0;

        store(h->hpx + step, &zero);
        store(h->hpy + step, &zero);
        store(h->hpz + step, &zero);

        store(h->hnx + step, &zero);
        store(h->hny + step, &zero);
        store(h->hnz + step, &zero);

        store(h->har + step, &zero);
        store(h->hag + step, &zero);
        store(h->hab + step, &zero);

        store(h->isHitting + step, &zero);
        store(h->hit_emissive_power + step, &zero);
        store(h->hit_type + step, &zero);
    }
};

void free_fHit(fHit *h)
{
    free(h->hit_albedo);
    free(h->hit_point);
    free(h->hit_normal);
    free(h->isHitting);
    free(h->hit_emissive_power);
    free(h->hit_type);
}