#pragma once
#include "ts3/layout.h"

namespace ts3::lotrenderer {
// Bounded views of the three objects read by 0x0096B820; unknown bytes stay opaque.
struct LotRendererLeaf96B820 {
    std::byte unknown00[0x0c];
    void* result;
};
struct LotRendererMiddle96B820 {
    std::byte unknown00[0x8c];
    LotRendererLeaf96B820* leaf;
};
struct LotRendererRoot96B820 {
    std::byte unknown00[0x14];
    LotRendererMiddle96B820* middle;
};
TS3_X86_OFFSET(LotRendererRoot96B820, middle, 0x14);
TS3_X86_SIZE(LotRendererRoot96B820, 0x18);
TS3_X86_OFFSET(LotRendererMiddle96B820, leaf, 0x8c);
TS3_X86_SIZE(LotRendererMiddle96B820, 0x90);
TS3_X86_OFFSET(LotRendererLeaf96B820, result, 0x0c);
TS3_X86_SIZE(LotRendererLeaf96B820, 0x10);
// TS3W.exe 0x0096B820: return global->middle->leaf->result, or null at a missing link.
void* GetRenderer96B820();
} // namespace ts3::lotrenderer
