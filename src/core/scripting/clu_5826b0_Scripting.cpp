#include "ts3/scripting/clu_5826b0_Scripting.h"
#include "ts3/layout.h"

namespace ts3::scripting {
// 0x005826B0: ECX retains owner for the thiscall dependency; it is not cdecl.
void* FindSelectedBinding(const void* owner)
{
    if (!owner) return nullptr;
#if defined(_M_IX86)
    const auto mask = *reinterpret_cast<const uint32_t*>(0x011E92A8);
    return CallGameMethod<void*, 0x00935770>(owner, mask);
#else
    AbortNotRewritten("FindSelectedBinding game selection mask");
    return nullptr;
#endif
}
} // namespace ts3::scripting
