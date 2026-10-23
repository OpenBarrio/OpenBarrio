#include "ts3/math/m04_sign.h"
#include <xmmintrin.h>

namespace ts3::math {
float FloatSign(float value) noexcept {
    const __m128 x = _mm_set_ss(value);
    const __m128 zero = _mm_setzero_ps();
    // Quiet equality has UCOMISS's masked invalid/denormal behavior, but
    // (unlike testing ZF alone) does not mistake unordered for zero.
    if (_mm_movemask_ps(_mm_cmpeq_ss(x, zero)) & 1) return 0.0f;
    // Signaling ordered comparison: even a quiet NaN accrues invalid here.
    if (_mm_comigt_ss(x, zero)) return 1.0f;
    return -1.0f;
}
}
