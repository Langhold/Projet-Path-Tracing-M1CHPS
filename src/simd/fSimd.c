
#include <simd/fSimd.h>

__m128 three_half;
__m128 half;
__m128 epsilon;
__m128 max_limit;
__m128 epsilon_0;
__m128 epsilon_1;
__m128 zero;
__m128 one;
__m128 minus_one;
__m128 two;
__m128 minus_two;
__m128 two_pi;

void init_global_variable()
{
    three_half = _mm_set1_ps(1.5f);
    half = _mm_set1_ps(0.5f);
    epsilon = _mm_set1_ps(1e-3);
    max_limit = _mm_set1_ps(__FLT_MAX__);
    zero = _mm_setzero_ps();
    one = _mm_set1_ps(1.0f);
    minus_one = _mm_set1_ps(-1.0f);
    two = _mm_set1_ps(2.0f);
    minus_two = _mm_set1_ps(-2.0f);

    two_pi = _mm_set1_ps(2.0 * M_PI);
    epsilon_0 = _mm_set1_ps(1e-3);
    epsilon_1 = _mm_set1_ps(1e-3);
}

__vec4f load(float *data)
{
    return _mm_load_ps(data);
};

void store(float *out, const __vec4f *in)
{
    _mm_store_ps(out, *in);
};

__vec4f set1(float val)
{
    return _mm_set1_ps(val);
};

__vec4f set(float val3, float val2, float val1, float val0)
{
    return _mm_set_ps(val3, val2, val1, val0);
};

__vec4f sub_(const __vec4f *A, const __vec4f *B)
{
    return _mm_sub_ps(*A, *B);
};

__vec4f add_(const __vec4f *A, const __vec4f *B)
{
    return _mm_add_ps(*A, *B);
};

__vec4f mul_(const __vec4f *A, const __vec4f *B)
{
    return _mm_mul_ps(*A, *B);
};

__vec4f div_(const __vec4f *A, const __vec4f *B)
{
    return _mm_div_ps(*A, *B);
};

__vec4f nequal(const __vec4f *A, const __vec4f *B)
{
    return _mm_cmpneq_ps(*A, *B);
}

__vec4f equal(const __vec4f *A, const __vec4f *B)
{
    return _mm_cmpeq_ps(*A, *B);
}

__vec4f and_(const __vec4f *A, const __vec4f *B)
{
    return _mm_and_ps(*A, *B);
};

__vec4f max_(const __vec4f *A, const __vec4f *B)
{
    return _mm_max_ps(*A, *B);
};

__vec4f min_(const __vec4f *A, const __vec4f *B)
{
    return _mm_min_ps(*A, *B);
};

__vec4f or_(const __vec4f *A, const __vec4f *B)
{
    return _mm_or_ps(*A, *B);
};

__vec4f sqrt_(const __vec4f *A)
{
    return _mm_sqrt_ps(*A);
};

__vec4f absf_(const __vec4f *A)
{
    return Sleef_fabsf4_sse2(*A);
};

__vec4f _powf_(const __vec4f *A, const float exp)
{
    return Sleef_powf4_u10(*A, set1(exp));
}

__vec4f powf_(const __vec4f *A, const __vec4f *exp)
{
    return Sleef_powf4_u10(*A, *exp);
}

__vec4f lower(const __vec4f *A, const __vec4f *B)
{
    return _mm_cmplt_ps(*A, *B);
};

int fuse(const __vec4f *mask)
{
    return _mm_movemask_ps(*mask);
}

__vec4f blendv(const __vec4f *A, const __vec4f *B, const __vec4f *mask)
{
    return _mm_blendv_ps(*A, *B, *mask);
};

__vec4f greater(const __vec4f *A, const __vec4f *B)
{
    return _mm_cmpgt_ps(*A, *B);
};

__vec4f greater_or_equal(const __vec4f *A, const __vec4f *B)
{
    return _mm_cmpge_ps(*A, *B);
};

__vec4f rcp(const __vec4f *A)
{
    return _mm_rcp_ps(*A);
};

__vec4f fmadd(const __vec4f *A, const __vec4f *B, const __vec4f *C)
{
    return _mm_fmadd_ps(*A, *B, *C);
};

__vec4f fmsub(const __vec4f *A, const __vec4f *B, const __vec4f *C)
{
    return _mm_fmsub_ps(*A, *B, *C);
};

__vec4f dot_(const __vec4f *A_x, const __vec4f *A_y, const __vec4f *A_z,
             const __vec4f *B_x, const __vec4f *B_y, const __vec4f *B_z)
{
    const __m128 X = _mm_mul_ps(*A_x, *B_x);
    const __m128 Y = _mm_mul_ps(*A_y, *B_y);
    const __m128 Z = _mm_mul_ps(*A_z, *B_z);

    return _mm_add_ps(_mm_add_ps(X, Y), Z);
};

void cross_(const __vec4f *A_x, const __vec4f *A_y, const __vec4f *A_z,
            const __vec4f *B_x, const __vec4f *B_y, const __vec4f *B_z,
            __vec4f *C_x, __vec4f *C_y, __vec4f *C_z)
{
    *C_x = _mm_fmsub_ps(*A_y, *B_z, _mm_mul_ps(*A_z, *B_y));
    *C_y = _mm_fmsub_ps(*A_z, *B_x, _mm_mul_ps(*A_x, *B_z));
    *C_z = _mm_fmsub_ps(*A_x, *B_y, _mm_mul_ps(*A_y, *B_x));
};

__vec4f magnitude_(const __vec4f *x, const __vec4f *y, const __vec4f *z)
{
    return _mm_fmadd_ps(*x, *x, _mm_fmadd_ps(*y, *y, _mm_mul_ps(*z, *z)));
}

void norm_(const __vec4f *A_x, const __vec4f *A_y, const __vec4f *A_z,
           __vec4f *C_x, __vec4f *C_y, __vec4f *C_z)
{
    const __m128 a = _mm_max_ps(epsilon, _mm_fmadd_ps(*A_x, *A_x, _mm_fmadd_ps(*A_y, *A_y, _mm_mul_ps(*A_z, *A_z))));
    __m128 inv = _mm_rsqrt_ps(a);

    inv = _mm_mul_ps(inv, _mm_sub_ps(three_half,
                                     _mm_mul_ps(half,
                                                _mm_mul_ps(a, _mm_mul_ps(inv, inv)))));

    *C_x = _mm_mul_ps(*A_x, inv);
    *C_y = _mm_mul_ps(*A_y, inv);
    *C_z = _mm_mul_ps(*A_z, inv);
};

__vec4f cos_(__vec4f *A)
{
    float temp[4];

    _mm_store_ps(temp, *A);

    return set(cosf(temp[0]), cosf(temp[1]), cosf(temp[2]), cosf(temp[3]));
}

__vec4f sin_(__vec4f *A)
{
    float temp[4];

    _mm_store_ps(temp, *A);
    return set(sinf(temp[0]), sinf(temp[1]), sinf(temp[2]), sinf(temp[3]));
}

__vec4f andnot_(const __vec4f *A, const __vec4f *B)
{
    return _mm_andnot_ps(*A, *B);
}