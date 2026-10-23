#include "ts3/lighting/clu_79b840_Lighting.h"
#include "ts3/layout.h"

namespace ts3::lighting {
// 0x0079B840: ECX is the receiver, ret 12. The decompiler loses ECX.
void* LightingLookup::FindByName(const char* name, std::uint32_t key, std::uint32_t fallback)
{
    const auto hash = CallGameFunction<std::uint32_t, 0x007A0CE0>(name);
    // 0x0079B790 may update the returned resource; it is not compare-safe.
    // Forward the full fallback word, although the dependency tests only its low byte.
    return CallGameMethod<void*, 0x0079B790>(this, hash, key, fallback);
}
}
