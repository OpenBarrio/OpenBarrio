#include "ts3/lighting/clu_c5b9b0_Lighting.h"

#include <cstring>
#include "ts3/layout.h"

namespace ts3::lighting {
namespace {
// Only the observed prefix is known; it contains no interpreted pointers.
struct LightingFlagsPrefix {
    std::uint8_t opaque[0x90];
    std::uint32_t flags;
};
TS3_X86_OFFSET(LightingFlagsPrefix, flags, 0x90);
TS3_X86_SIZE(LightingFlagsPrefix, 0x94);
} // namespace

// 0x00C5B9B0: cdecl, ret (caller pops the object pointer), no callees or writes.
std::int32_t __cdecl HasLightingFlags(const void* object)
{
    std::uint32_t flags;
    std::memcpy(&flags, static_cast<const std::uint8_t*>(object) +
                        offsetof(LightingFlagsPrefix, flags), sizeof(flags));
    return (flags & 0xE400u) != 0 ? 1 : 0;
}

} // namespace ts3::lighting
