#include "ts3/math/m04_box_union.h"
#include <cstring>
#include <intrin.h>

#pragma float_control(precise, on, push)
#pragma fenv_access(on)
namespace ts3::math {
const UnionBox* UnionBounds(UnionBox* dst, const UnionBox* a, const UnionBox* b) noexcept {
    // Copy object representations: no dereference of a fabricated larger box or
    // type-punned float lvalue, and no restrict/lifetime assumptions on overlaps.
    auto* out = reinterpret_cast<unsigned char*>(dst);
    const auto* left = reinterpret_cast<const unsigned char*>(a);
    const auto* right = reinterpret_cast<const unsigned char*>(b);
    __m128 amin, bmin;
    std::memcpy(&amin, left, 16);
    std::memcpy(&bmin, right, 16);
    const __m128 minimum = _mm_min_ps(amin, bmin);
    std::memcpy(out, &minimum, 16);
    // The original reloads maxima AFTER this store, including shifted aliases.
    _ReadWriteBarrier();
    __m128 amax, bmax;
    std::memcpy(&amax, left + 16, 16);
    std::memcpy(&bmax, right + 16, 16);
    const __m128 maximum = _mm_max_ps(amax, bmax);
    std::memcpy(out + 16, &maximum, 16);
    return a;
}
}
#pragma float_control(pop)
