#pragma once

#include <cstdint>
#include "ts3/layout.h"

namespace ts3::animation {

// 5-dword rectangle read by 0x0060FFB0: a leading unused dword, then x, y, w, h.
// The caller (e.g. 0x00691AE0) interprets the four values as floats, but the
// function itself only performs integer (dword) adds on the raw bits.
struct Rect {
    uint32_t pad;  // 0x00 (never read by GetBounds)
    uint32_t x;    // 0x04
    uint32_t y;    // 0x08
    uint32_t w;    // 0x0C
    uint32_t h;    // 0x10
};
TS3_X86_SIZE(Rect, 0x14);

// The 4-dword box written by GetBounds: {x, y, x+w, y+h}.
struct Box {
    uint32_t x0;  // 0x00
    uint32_t y0;  // 0x04
    uint32_t x1;  // 0x08
    uint32_t y1;  // 0x0C
};
TS3_X86_SIZE(Box, 0x10);

// Owner object: the rectangle pointer lives at offset 0xE0.
struct BoundsOwner {
    uint8_t pad[0xE0];
    const Rect* rect;  // 0xE0

    // Original 0x0060FFB0 (thiscall, ret 4): write the bounding box of the rect at
    // *(this+0xE0) into `out` and return `out`.
    Box* GetBounds(Box* out);
};
TS3_X86_OFFSET(BoundsOwner, rect, 0xE0);
TS3_X86_SIZE(BoundsOwner, 0xE4);

}  // namespace ts3::animation
