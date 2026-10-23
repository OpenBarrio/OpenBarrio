#pragma once

#include <cstdint>

namespace ts3::world {

// TS3W.exe 0x009DC550: returns 1 when the object's pointer field at +0x1c does not hold
// the field's own address (the self-pointer sentinel), else 0. Reads one dword, no writes.
struct WorldSelfRef9dc550 {
    uint8_t bytes[0x20];

    bool IsRefNotSelf() const;
};

}  // namespace ts3::world
