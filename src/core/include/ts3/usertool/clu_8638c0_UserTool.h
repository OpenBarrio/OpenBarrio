#pragma once
#include "ts3/layout.h"

namespace ts3::usertool {
// Accessed prefix only; the first eight bytes and +12 are opaque to this unit.
struct UserToolValue {
    uint8_t opaque00[8];
    uint8_t flags;
    uint8_t opaque09;
    uint16_t type;
    uint8_t opaque0c[4];
    union Payload {
        void* indirect;
        uint8_t inlineBytes[4];
    } payload;

    // TS3W.exe 0x008638C0: return inline/indirect storage for tags 16/19, otherwise the game's default singleton.
    void* GetValue() const;
};
TS3_X86_OFFSET(UserToolValue, flags, 8);
TS3_X86_OFFSET(UserToolValue, type, 10);
TS3_X86_OFFSET(UserToolValue, payload, 16);
TS3_X86_SIZE(UserToolValue, 20);
} // namespace ts3::usertool
