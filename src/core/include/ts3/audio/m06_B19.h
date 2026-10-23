#pragma once
#include "ts3/layout.h"
namespace ts3::audio {
struct B19Weather { uint32_t kind, unused, season, intensity; };
struct b19;
// Native dependency bindings; game tables and singleton are used only on x86.
struct B19Services {
    void** tables[5]{};
    void* secondary=nullptr;
    void* reference=nullptr;
    const B19Weather* weather=nullptr;
    uintptr_t (*publish)(b19*,float)=nullptr;
    uint32_t (*hash)(const char*,uint32_t,uint32_t)=nullptr;
};
extern B19Services b19Services;
struct b19 {
    void** vtable;
    void* secondary;
    void* reference;
    uint32_t state;
    uint8_t enabled, padding[3];
    float value;
    void* link;
    void* owner;
    uint32_t nameHash;
    // TS3W.exe 0x0086E0A0: Initialize the isfrozen primitive, leaving padding untouched.
    b19* InitFrozen();
    // TS3W.exe 0x0086E150: Initialize the ishail primitive, leaving padding untouched.
    b19* InitHail();
    // TS3W.exe 0x0086E220: Initialize the israin primitive, leaving padding untouched.
    b19* InitRain();
    // TS3W.exe 0x0086E320: Initialize the isseason primitive, leaving padding untouched.
    b19* InitSeason();
    // TS3W.exe 0x0086E3F0: Initialize the issnow primitive, leaving padding untouched.
    b19* InitSnow();
    // TS3W.exe 0x0086E0E0: Lazily cache the isfrozen name hash; zero is retried.
    uint32_t NameFrozen();
    // TS3W.exe 0x0086E190: Lazily cache the ishail name hash; zero is retried.
    uint32_t NameHail();
    // TS3W.exe 0x0086E260: Lazily cache the israin name hash; zero is retried.
    uint32_t NameRain();
    // TS3W.exe 0x0086E360: Lazily cache the isseason name hash; zero is retried.
    uint32_t NameSeason();
    // TS3W.exe 0x0086E430: Lazily cache the issnow name hash; zero is retried.
    uint32_t NameSnow();
    // TS3W.exe 0x0086E110: Publish the hail value through slot 14 and forward EAX.
    uintptr_t EvaluateHail(uint32_t, uint32_t);
    // TS3W.exe 0x0086E1C0: Publish the rain value through slot 14 and forward EAX.
    uintptr_t EvaluateRain(uint32_t, uint32_t);
    // TS3W.exe 0x0086E290: Publish the season value through slot 14 and forward EAX.
    uintptr_t EvaluateSeason(uint32_t, uint32_t);
    // TS3W.exe 0x0086E390: Publish the snow value through slot 14 and forward EAX.
    uintptr_t EvaluateSnow(uint32_t, uint32_t);
};
TS3_X86_OFFSET(b19, nameHash, 0x20);
TS3_X86_SIZE(b19, 0x24);
}
