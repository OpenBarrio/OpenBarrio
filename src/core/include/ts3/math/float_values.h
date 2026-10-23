#pragma once

#include "ts3/layout.h"
#include <cstddef>
#include <type_traits>

namespace ts3::math {

// Owned mathematical VALUE storage used by the narrowly released SIMD kernels.
// These are not reconstructed EA classes or claims about callers' allocation sizes.
struct alignas(16) Float4Value { float lane[4]; };
struct alignas(16) SrtValue {
    Float4Value translation;
    Float4Value rotation;
    Float4Value scale;
};
struct alignas(16) Matrix44Value { Float4Value vector[4]; };

static_assert(sizeof(Float4Value) == 16 && alignof(Float4Value) == 16);
static_assert(sizeof(SrtValue) == 48 && alignof(SrtValue) == 16);
static_assert(sizeof(Matrix44Value) == 64 && alignof(Matrix44Value) == 16);
static_assert(std::is_standard_layout_v<SrtValue>);
static_assert(offsetof(SrtValue, translation) == 0);
static_assert(offsetof(SrtValue, rotation) == 0x10);
static_assert(offsetof(SrtValue, scale) == 0x20);
TS3_X86_OFFSET(SrtValue, translation, 0);
TS3_X86_OFFSET(SrtValue, rotation, 0x10);
TS3_X86_OFFSET(SrtValue, scale, 0x20);

} // namespace ts3::math
