#pragma once
#include <cstdint>

namespace ts3::lighting {
// Opaque receiver: this wrapper accesses no object fields itself.
class LightingLookup {
public:
    // TS3W.exe 0x0079B840: hash a name and forward to the lighting lookup, preserving all argument bits.
    void* FindByName(const char* name, std::uint32_t key, std::uint32_t fallback);
};
}
