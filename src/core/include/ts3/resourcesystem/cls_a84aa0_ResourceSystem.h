#pragma once
#include <cstdint>
#include "ts3/layout.h"

namespace ts3::resourcesystem {
// Unnamed class at vtable 0x010621B0: three dimensions and an EASTL vector<uint16_t>.
// Allocation/resize remain in the original dependency unit; this is not an owning C++
// container. The allocator is the game's pointer-sized allocator token.
struct Grid16Samples {
    uint16_t* begin;
    uint16_t* end;
    uint16_t* capacity;
    void* allocator;
};
struct Grid16 {
    void* const* vtable;
    int32_t width, height, depth;
    Grid16Samples samples;
    // 0xA84AA0, __thiscall, ret 12; no defined return value.
    void ShiftDepth(int32_t delta, bool atEnd, bool dimensionsOnly);
    // 0xA84B80, tail call to the vector's resize, ret 4 in the dependency.
    void ResizeSamples(uint32_t count);
    // 0xAC0810, copy constructor, __thiscall, ret 4; returns this.
    Grid16* ConstructCopy(const Grid16* source);
};
TS3_X86_OFFSET(Grid16, width, 0x04);
TS3_X86_OFFSET(Grid16, height, 0x08);
TS3_X86_OFFSET(Grid16, depth, 0x0C);
TS3_X86_OFFSET(Grid16, samples, 0x10);
TS3_X86_OFFSET(Grid16Samples, allocator, 0x0C);
TS3_X86_SIZE(Grid16Samples, 16);
TS3_X86_SIZE(Grid16, 32);
}
