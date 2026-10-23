#include "ts3/lotrenderer/clu_96b820_LotRenderer.h"

namespace ts3::lotrenderer {
// Original ABI: cdecl, no arguments, plain ret; EAX is a pointer (see callers).
void* GetRenderer96B820()
{
#if defined(_M_IX86)
    auto* root = *reinterpret_cast<LotRendererRoot96B820* const*>(uintptr_t{0x011E9400});
#else
    // The 64-bit runtime has no mapped TS3W.exe global yet.
    AbortNotRewritten("LotRenderer global 0x011E9400");
    LotRendererRoot96B820* root = nullptr;
#endif
    if (!root)
        return nullptr;
    auto* middle = root->middle;
    if (!middle)
        return nullptr;
    auto* leaf = middle->leaf;
    if (!leaf)
        return nullptr;
    return leaf->result;
}
} // namespace ts3::lotrenderer
