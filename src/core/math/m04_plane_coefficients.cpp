#include "ts3/math/m04_plane_coefficients.h"

// These operations are deliberately not algebraically simplified. In particular
// MAXPS selects its second source on unordered/equal inputs; multiplication by
// -1 quiets sNaNs, unlike a sign-bit toggle. The half-sums can overflow.
#pragma float_control(precise, on, push)
namespace ts3::math {
namespace {
template<int Lane> __m128 Pack(__m128 a, __m128 b, __m128 c, __m128 d) noexcept {
    const __m128 ab = _mm_shuffle_ps(a, b, Lane * 0x11);
    const __m128 cd = _mm_shuffle_ps(c, d, Lane * 0x11);
    return _mm_shuffle_ps(ab, cd, 0x88);
}
}
void PreparePlaneCoefficients(scratch::ScratchClipTest689D10* value) noexcept {
    auto* v = value->vectors;
    // Snapshot every input before the disjoint output region is touched.
    const __m128 a = v[0], b = v[1], c = v[2], d = v[3], e = v[4], f = v[5];
    const __m128 o0 = _mm_shuffle_ps(v[12], v[13], 0x10);
    const __m128 o1 = _mm_shuffle_ps(v[14], v[15], 0x32);
    const __m128 o2 = _mm_shuffle_ps(v[16], v[17], 0x10);
    const __m128 o3 = _mm_shuffle_ps(v[17], v[17], 0x32);
    const __m128 offsets0 = _mm_shuffle_ps(o0, o1, 0x88);
    const __m128 offsets1 = _mm_shuffle_ps(o2, o3, 0x88);
    const __m128 packed[6] = {
        Pack<0>(a,b,c,d), Pack<0>(e,f,f,f),
        Pack<1>(a,b,c,d), Pack<1>(e,f,f,f),
        Pack<2>(a,b,c,d), Pack<2>(e,f,f,f)
    };
    const __m128 minusOne = _mm_set1_ps(-1.0f);
    const __m128 half = _mm_set1_ps(0.5f);
    __m128 absolute[6];
    for (unsigned i = 0; i != 6; ++i)
        absolute[i] = _mm_max_ps(_mm_mul_ps(packed[i], minusOne), packed[i]);
    for (unsigned i = 0; i != 6; ++i) {
        v[18+i] = packed[i];
        v[24+i] = absolute[i];
        v[30+i] = _mm_mul_ps(_mm_add_ps(absolute[i], packed[i]), half);
        v[36+i] = _mm_mul_ps(_mm_sub_ps(packed[i], absolute[i]), half);
    }
    v[42] = _mm_sub_ps(_mm_setzero_ps(), offsets0);
    v[43] = _mm_sub_ps(_mm_setzero_ps(), offsets1);
}
}
#pragma float_control(pop)
