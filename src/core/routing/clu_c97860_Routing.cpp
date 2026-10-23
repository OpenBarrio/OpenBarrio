#include "ts3/routing/clu_c97860_Routing.h"

#include <cstdint>
#include <cstring>

namespace ts3::routing {

bool RoutingFlags::HasRoutingFlags() const
{
    const auto* bytes = reinterpret_cast<const uint8_t*>(this);
    uint32_t first = 0;
    uint32_t second = 0;
    std::memcpy(&first, bytes + 0x98, sizeof(first));
    std::memcpy(&second, bytes + 0xD0, sizeof(second));
    return (first & 0x00100000u) == 0 && (second & 0x4000010Au) != 0;
}

}  // namespace ts3::routing
