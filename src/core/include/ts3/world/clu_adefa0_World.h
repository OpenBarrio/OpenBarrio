#pragma once
#include <cstddef>
#include <cstdint>
#include "ts3/layout.h"

namespace ts3::world {
// 0x00ADEFA0: an EASTL-style vector range; its game ABI stores begin/end at 0/4 on x86.
struct WorldRecordVector {
    char* begin;
    char* end;
    // Copy 28-byte records and advance end; __thiscall ret 8.
    char* CopyRecords(char* destination, const char* source);
};
TS3_X86_OFFSET(WorldRecordVector, end, 4);
TS3_X86_SIZE(WorldRecordVector, 8);

struct alignas(16) WorldTransformState {
    std::byte reserved[0x1f0];
    float basisX[4];       // 0x1f0
    float basisY[4];       // 0x200
    float basisZ[4];       // 0x210
    float translation[4];  // 0x220

    // 0x00AE0990: transform a 4-float vector; returns the input pointer.
    float* __thiscall TransformVector(const float* input, float* output) const;
};
static_assert(offsetof(WorldTransformState, basisX) == 0x1f0);
static_assert(offsetof(WorldTransformState, basisY) == 0x200);
static_assert(offsetof(WorldTransformState, basisZ) == 0x210);
static_assert(offsetof(WorldTransformState, translation) == 0x220);
static_assert(sizeof(WorldTransformState) == 0x230);
}
