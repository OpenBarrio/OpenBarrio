#pragma once
#include "ts3/layout.h"

namespace ts3::world {
// Accessed prefix only; the qword at +8 is an identifier, not a pointer.
struct WorldCandidate82A720 {
    void* vtable;
    uint32_t reserved04;
    uint64_t identifier;
    uint8_t reserved10[0x1c];
    uint32_t flags;
};
TS3_X86_OFFSET(WorldCandidate82A720, identifier, 0x08);
TS3_X86_OFFSET(WorldCandidate82A720, flags, 0x2c);
TS3_X86_SIZE(WorldCandidate82A720, 0x30);

struct WorldGate82A720 {
    uint8_t reserved000[0x976];
    uint8_t active;
    uint8_t reserved977;
    uint8_t allowed;
    uint8_t reserved979[5];
    uint8_t gated;
    uint8_t pending;
    // TS3W.exe 0x0082A720: test candidate eligibility, clearing pending before candidate validation.
    bool Accept(WorldCandidate82A720* candidate);
};
TS3_X86_OFFSET(WorldGate82A720, active, 0x976);
TS3_X86_OFFSET(WorldGate82A720, allowed, 0x978);
TS3_X86_OFFSET(WorldGate82A720, gated, 0x97e);
TS3_X86_OFFSET(WorldGate82A720, pending, 0x97f);
TS3_X86_SIZE(WorldGate82A720, 0x980);
} // namespace ts3::world
