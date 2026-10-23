#pragma once

#include <cstdint>
#include "ts3/layout.h"

namespace ts3::lotrenderer {

// Source data for one update: the first dword is a pointer to a 0x44-byte block that the
// original copies by value and hands to 0x00516AA0.
struct CyclingRenderTarget {
    const uint8_t* data;
};

// Cycling renderer (vtable 0x0101B954); functions at 0x007E3D40, 0x007E3D70, 0x007E3DA0.
// Only the fields these routines touch are named. +0x10..+0x27 is an embedded stopwatch
// (initialized by 0x004F35B0, read by 0x004F33C0): +0x10/+0x14 start tick (low/high),
// +0x18/+0x1C zeroed, +0x20 mode (1 = rdtsc, otherwise QueryPerformanceCounter).
struct CyclingRenderer {
    uint32_t vtableAddress;
    float cycleSeconds;
    uint32_t current;
    uint32_t pad0c;
    uint32_t startLow;
    uint32_t startHigh;
    uint32_t pad18;
    uint32_t pad1c;
    uint32_t timerMode;
    float timerScale;
    uint32_t lockWord;

    // Original 0x007E3D40 (vtable slot, ret 4): answers this for two interface ids.
    CyclingRenderer* QueryInterface(int32_t interfaceId);

    // Original 0x007E3D70 (ret 0): installs the vtable, sets current = 1, initializes the
    // embedded stopwatch (5, 1) and clears the lock word.
    CyclingRenderer* Construct();

    // Original 0x007E3DA0 (ret 0x10, 4th stack argument unused; vtable slot).
    void Update(CyclingRenderTarget* target, void* const* targets, uint32_t count, uint32_t unused);
};
TS3_X86_OFFSET(CyclingRenderer, current, 8);
TS3_X86_OFFSET(CyclingRenderer, startLow, 0x10);
TS3_X86_OFFSET(CyclingRenderer, timerMode, 0x20);
TS3_X86_OFFSET(CyclingRenderer, lockWord, 0x28);

}  // namespace ts3::lotrenderer
