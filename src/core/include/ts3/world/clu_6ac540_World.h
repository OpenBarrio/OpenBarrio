#pragma once

#include <cstdint>

#include "ts3/layout.h"

namespace ts3::world {

// Original 0x006C2990 copies the first 0x34 bytes and the four dwords at
// 0x40..0x4c; the gap at 0x34..0x3f is intentionally not part of this record.
struct alignas(16) WorldFragment6ac540 {
    uint8_t bytes[0x50];

    WorldFragment6ac540* CopyFrom(const WorldFragment6ac540* source);
};
TS3_X86_SIZE(WorldFragment6ac540, 0x50);

struct WorldPredicateState6ac540 {
    uint8_t bytes[0x40];

    bool IsPending(uint8_t testSecondFlag) const;
};
TS3_X86_SIZE(WorldPredicateState6ac540, 0x40);

// The global object at 0x011D08D0 is at least 0x148 bytes and 16-byte aligned.
struct alignas(16) WorldState6ac540 {
    uint8_t bytes[0x148];

    void* Update(int32_t selector);
};

// 0x006AC540: cdecl global state initializer.
int32_t InitializeWorldState6ac540();

// 0x006AFDA0: thiscall renderer update flag setter.
struct WorldLightRigState6ac540 {
    uint8_t bytes[0xDE];

    int32_t SetUpdateLightRigs(uint8_t enabled);
};

// 0x006E97B0: cdecl getter for the current world object.
uint32_t GetWorldObject6ac540();

// Script bridge entry points (0x00786BD0 and 0x00786C70).
void* GetNumDaysUntilFullMoonImpl6ac540(int32_t selector);
int32_t SetUpdateLightRigsImpl6ac540(uint8_t enabled);

}  // namespace ts3::world
