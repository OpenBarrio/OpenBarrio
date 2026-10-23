#pragma once
#include "ts3/math/float_values.h"

namespace ts3::math {
// Bounded borrowed coefficient VALUE view, not an EA object/lifetime claim and
// not an alias of any other reconstructed layout. Holes are uninterpreted.
struct alignas(16) SixPlaneView {
    Float4Value normal[6];
    unsigned char uninterpreted[0x60];
    Float4Value offset[6];
};
static_assert(sizeof(SixPlaneView) == 0x120 && alignof(SixPlaneView) == 16);
static_assert(offsetof(SixPlaneView, offset) == 0xc0);

// TS3W.exe 0x0068D200: classify a sphere against six planes; first partial returns immediately, unordered continues, full enum 0/1/2.
unsigned ClassifySphere(const SixPlaneView* planes, const Float4Value* sphere) noexcept;
}
