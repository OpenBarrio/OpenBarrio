#pragma once
#include "ts3/layout.h"
#include <Windows.h>
namespace ts3::anim::M120 {
// Only the fields observed in the constructor are named; reserved bytes stay poisoned.
struct Map {
    void* buckets;
    uint32_t bucketCount, count;
    float maximumLoad, growth;
    uint32_t policy;
    const char* descriptor;
};
struct PaddedMap { Map map; uint32_t reserved; };
struct Links { void* next; void* previous; };
struct Locomotion {
    uint32_t reserved0;
    Map paths;
    uint32_t reserved20[2];
    void* handleDescriptor;
    uint32_t reserved2c;
    void* handle[4];
    Links owned;
    Locomotion* owner;
    uint32_t reserved4c;
    PaddedMap styles, effects;
    Map builders;
    Links progressions;
    const char* progressionDescriptor;
    uint32_t reservedb8;
    Map weatherEffects;
    void* lines[2];
    CRITICAL_SECTION lock;
    uint32_t reservedf8[2];
    uint8_t flags[3], reserved103;
    uint32_t state;
};
TS3_X86_SIZE(Map, 28);
TS3_X86_OFFSET(Locomotion, handle, 0x30);
TS3_X86_OFFSET(Locomotion, owned, 0x40);
TS3_X86_OFFSET(Locomotion, styles, 0x50);
TS3_X86_OFFSET(Locomotion, builders, 0x90);
TS3_X86_OFFSET(Locomotion, progressions, 0xac);
TS3_X86_OFFSET(Locomotion, weatherEffects, 0xbc);
TS3_X86_OFFSET(Locomotion, lock, 0xe0);
TS3_X86_OFFSET(Locomotion, flags, 0x100);
TS3_X86_SIZE(Locomotion, 0x108);
// TS3W.exe 0x00842700: construct locomotion registries and register optional debug controls; reserved bytes remain untouched.
Locomotion* Construct(Locomotion* self);
}

namespace ts3::anim {
// ABI adapter: register's thiscall manifest invokes a real member, not a free function.
struct m120 : M120::Locomotion {
    // TS3W.exe 0x00842700: construct this locomotion state through the typed implementation.
    m120* Construct() { return static_cast<m120*>(M120::Construct(this)); }
};
}
