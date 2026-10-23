#include "ts3/math/m04_plane_classifier.h"
#include <xmmintrin.h>

#pragma float_control(precise, on, push)
namespace ts3::math {
unsigned ClassifySphere(const SixPlaneView* planes, const Float4Value* sphere) noexcept {
    const __m128 s = _mm_load_ps(sphere->lane);
    const __m128 radius = _mm_shuffle_ps(s, s, 0xff);
    for (unsigned i = 0; i != 6; ++i) {
        // Volatile full-vector checkpoints keep otherwise-unused upper-lane
        // arithmetic observable (including its MXCSR exceptions) on both widths.
        const __m128 product = _mm_mul_ps(_mm_load_ps(planes->normal[i].lane), s);
        const __m128 zy = _mm_add_ps(_mm_shuffle_ps(product, product, 2),
                                       _mm_shuffle_ps(product, product, 1));
        const __m128 sum = _mm_add_ps(zy, product);
        volatile __m128 observeZy = zy;
        volatile __m128 observeSum = sum;
        const __m128 xyz = sum;
        const __m128 distance = _mm_add_ps(_mm_shuffle_ps(xyz, xyz, 0),
                                             _mm_load_ps(planes->offset[i].lane));
        volatile __m128 observeDistance = distance;
        // COMISS/JA, not a quiet CMPPS or an unordered-accepting inverse.
        if (_mm_comigt_ss(radius, distance)) {
            const __m128 negativeRadius = _mm_sub_ps(_mm_setzero_ps(), radius);
            volatile __m128 observeNegative = negativeRadius;
            if (_mm_comigt_ss(negativeRadius, distance)) return 0;
            return 2; // EA: do NOT inspect any later plane after a partial.
        }
    }
    return 1;
}
}
#pragma float_control(pop)
