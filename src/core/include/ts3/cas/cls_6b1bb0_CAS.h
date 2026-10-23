#pragma once

#include <cstdint>
#include "ts3/layout.h"

namespace ts3::cas {

// The game-facing object whose tiny wrappers and derived initializer live at
// 0x006B1BB0, 0x006B1BD0, and 0x006B4220.
struct CASObject {
    uint32_t vtableAddress;
    // Original 0x006B1BB0: resolve key 0x2FCD2B4C and return its first dword.
    int32_t GetFirstProperty() const;

    // Original 0x006B1BD0: resolve key 0x36535688 and return its first dword.
    int32_t GetSecondProperty() const;

    // Original 0x006B4220: base initialize, then install vtable 0x00FF3DB0.
    CASObject* Initialize();
};
TS3_X86_OFFSET(CASObject, vtableAddress, 0);

}  // namespace ts3::cas
