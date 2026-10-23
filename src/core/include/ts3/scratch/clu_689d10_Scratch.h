#pragma once

#include <cstddef>
#include <cstdint>
#include <xmmintrin.h>

#include "ts3/layout.h"

namespace ts3::scratch {

// 0x00689D10 walks 44-byte records and releases the two non-null handles.
struct ScratchReleaseRecord689D10 {
    uint8_t prefix[12];
    uint32_t handle0C;
    uint8_t middle[12];
    uint32_t handle1C;
    uint8_t suffix[12];
};

int32_t __stdcall ReleaseScratchResources689D10(ScratchReleaseRecord689D10* begin,
                                                 ScratchReleaseRecord689D10* end);

// 0x0068BB40 zeros the first 32 bytes using two aligned SSE stores.
struct alignas(16) ScratchBase689D10 {
    uint8_t bytes[32];
    void Reset();
};

struct alignas(16) Float4Pair689D10 {
    __m128 first;
    __m128 second;
};

// The two four-lane bounds and twelve four-lane coefficient vectors at 0x0068C660.
struct alignas(16) ScratchClipTest689D10 {
    __m128 vectors[44];
    bool Contains(const Float4Pair689D10* value) const;
};

struct alignas(16) ScratchRecord689D10 {
    uint8_t bytes[64];
};

struct ScratchRecordVector689D10 {
    ScratchRecord689D10* begin;
    ScratchRecord689D10* end;
    ScratchRecord689D10* capacity;
    uint32_t allocator;

    // 0x006F7AB0 changes the logical vector length, constructing default records if needed.
    ScratchRecord689D10* Resize(uint32_t count);
};

TS3_X86_SIZE(ScratchReleaseRecord689D10, 44);
TS3_X86_OFFSET(ScratchReleaseRecord689D10, handle0C, 12);
TS3_X86_OFFSET(ScratchReleaseRecord689D10, handle1C, 28);
TS3_X86_SIZE(ScratchBase689D10, 32);
TS3_X86_SIZE(Float4Pair689D10, 32);
TS3_X86_SIZE(ScratchClipTest689D10, 704);
TS3_X86_SIZE(ScratchRecord689D10, 64);
TS3_X86_OFFSET(ScratchRecordVector689D10, end, 4);
TS3_X86_OFFSET(ScratchRecordVector689D10, capacity, 8);
TS3_X86_OFFSET(ScratchRecordVector689D10, allocator, 12);
TS3_X86_SIZE(ScratchRecordVector689D10, 16);

}  // namespace ts3::scratch
