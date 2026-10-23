#pragma once
#include "ts3/layout.h"

namespace ts3::lot {
struct LotFlags {
    void* vtable;
    uint32_t unknown04;
    void* next;
    void* previous;
    void* owner;
    uint32_t unknown14;
    void* childrenNext;
    void* childrenPrevious;
    uint32_t payload[4];
    uint32_t flags;
    uint32_t clearFlags;
    uint32_t setFlags;

    // TS3W.exe 0x006FAB50: update five flag bits and notify the owning tree; only the low enable byte is tested.
    uintptr_t UpdateFlags(uint32_t mask, uint8_t enable);
};
TS3_X86_OFFSET(LotFlags, owner, 0x10);
TS3_X86_OFFSET(LotFlags, payload, 0x20);
TS3_X86_OFFSET(LotFlags, flags, 0x30);
TS3_X86_OFFSET(LotFlags, clearFlags, 0x34);
TS3_X86_OFFSET(LotFlags, setFlags, 0x38);
TS3_X86_SIZE(LotFlags, 0x3c);
}
