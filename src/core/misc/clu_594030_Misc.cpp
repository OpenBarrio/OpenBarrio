#include "ts3/misc/clu_594030_Misc.h"
#include "ts3/layout.h"

namespace ts3::misc {
// 0x00594030 (__cdecl): no bounds checking, allocation, or call edges.
int32_t SetGlobalPair([[maybe_unused]] int32_t index, [[maybe_unused]] uint32_t first, [[maybe_unused]] uint32_t second)
{
#if defined(_M_IX86)
    // Effective addresses wrap at 32 bits, including negative/oversized indices.
    const uint32_t address = 0x011CDD30u + static_cast<uint32_t>(index) * 8u;
    *reinterpret_cast<uint32_t*>(static_cast<uintptr_t>(address)) = first;
    *reinterpret_cast<uint32_t*>(static_cast<uintptr_t>(address + 4u)) = second;
    return index;
#else
    NotRewritten("SetGlobalPair requires the game global table");
#endif
}
} // namespace ts3::misc
