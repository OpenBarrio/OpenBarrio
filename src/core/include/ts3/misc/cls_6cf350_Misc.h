#pragma once

#include <cstdint>
#include "ts3/layout.h"

namespace ts3::misc {

struct MiscRendererLayout {
    uint32_t vtable;
    uint8_t pad04[8];
    uint32_t child0C, pool, renderer, rows, columns, updateTarget, state24, state28;
    uint32_t updateCount, unknown30, boneMaskCount, unknown38;
    uint32_t boneMaskBegin, boneMaskEnd, boneMaskCapacity, boneMaskAllocator;
    uint8_t active;
    uint8_t pad4D[3];
};
TS3_X86_OFFSET(MiscRendererLayout, child0C, 0x0c);
TS3_X86_OFFSET(MiscRendererLayout, pool, 0x10);
TS3_X86_OFFSET(MiscRendererLayout, renderer, 0x14);
TS3_X86_OFFSET(MiscRendererLayout, rows, 0x18);
TS3_X86_OFFSET(MiscRendererLayout, updateCount, 0x2c);
TS3_X86_OFFSET(MiscRendererLayout, boneMaskCount, 0x34);
TS3_X86_OFFSET(MiscRendererLayout, boneMaskBegin, 0x3c);
TS3_X86_OFFSET(MiscRendererLayout, active, 0x4c);
TS3_X86_SIZE(MiscRendererLayout, 0x50);

// Mesh-pool render context. Game pointers are stored as 32-bit words in TS3W.
struct MiscRenderer {
    uint8_t bytes[0x50];

    int32_t Reset();             // 0x006CF350
    int32_t Update();            // 0x006CF380
    int32_t Destroy();           // 0x006CF860
    MiscRenderer* Construct();   // 0x006CFCA0
};

TS3_X86_SIZE(MiscRenderer, 0x50);

} // namespace ts3::misc
