#pragma once

#include <cstdint>

namespace ts3::routing {

// TS3W.exe 0x00C97860: reports whether the routing object has any enabled routing flags.
// EA checks bit 0x100000 in the first word before checking mask 0x4000010A in the second.
struct RoutingFlags {
    bool HasRoutingFlags() const;
};

}  // namespace ts3::routing
