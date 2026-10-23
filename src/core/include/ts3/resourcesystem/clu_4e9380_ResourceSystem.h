#pragma once
#include <cstdint>
#include "ts3/layout.h"
#include "ts3/resourcesystem/clu_a1a290_ResourceSystem.h"

namespace ts3::resourcesystem {
struct ResourceSlot4e9380 {
    uint32_t key;
    void* resource;
};
TS3_X86_OFFSET(ResourceSlot4e9380, resource, 4);
TS3_X86_SIZE(ResourceSlot4e9380, 8);

struct ResourceState4e9380 {
    uint8_t enabled;
    uint8_t gap01[3];
    uint32_t busy;
    uint32_t nesting;
    ResourceSlot4e9380 slots[33];
    uint8_t gap114[4];
    GameLock lock; // Dependency-owned storage; accessed only by the game bridge.
    uint8_t gap144[4];
    uint32_t interval;
    uint8_t active;
    uint8_t gap14d[3];
    void* current;
    uint8_t currentFlag;
    uint8_t tail155[3];

    // TS3W.exe 0x004E9380: initializes resource slots and constructs the embedded lock; preserves padding.
    ResourceState4e9380* Construct();
};
TS3_X86_OFFSET(ResourceState4e9380, busy, 4);
TS3_X86_OFFSET(ResourceState4e9380, nesting, 8);
TS3_X86_OFFSET(ResourceState4e9380, slots, 0x0c);
TS3_X86_OFFSET(ResourceState4e9380, lock, 0x118);
TS3_X86_OFFSET(ResourceState4e9380, interval, 0x148);
TS3_X86_OFFSET(ResourceState4e9380, active, 0x14c);
TS3_X86_OFFSET(ResourceState4e9380, current, 0x150);
TS3_X86_OFFSET(ResourceState4e9380, currentFlag, 0x154);
TS3_X86_SIZE(ResourceState4e9380, 0x158);
} // namespace ts3::resourcesystem
