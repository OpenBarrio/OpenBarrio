#pragma once
#include "ts3/layout.h"
#include <xmmintrin.h>
namespace ts3::audio {
struct b21;
struct alignas(16) B21Vector { float x,y,z,w; };
struct B21Points { B21Vector *begin,*end,*capacity; };
struct B21Lots { void **begin,**end; };
struct B21Cell { float x,z; int32_t level; };
// Native bindings for external interfaces, not alternate implementations of this unit.
struct B21Services {
    void** tables[3]{}; void* secondary=nullptr; void* reference=nullptr;
    uint32_t (*hash)(const char*,uint32_t,uint32_t)=nullptr;
    void* (*system)()=nullptr;
    uintptr_t (*resolve)(void*,uint32_t,uint32_t,void**,uint32_t)=nullptr;
    uintptr_t (*read)(void*,unsigned,uint32_t,void*)=nullptr;
    uintptr_t (*release)(void*)=nullptr;
    uintptr_t (*configure)(b21*)=nullptr;
    uintptr_t (*publish)(b21*,float)=nullptr;
    void* world=nullptr;
    const float* (*position)(void*,int)=nullptr;
    bool (*underground)(void*)=nullptr;
    B21Lots (*lots)(void*)=nullptr;
    void* (*rooms)(void*)=nullptr;
    void (*sphere)(void*,B21Vector*)=nullptr;
    void (*bounds)(void*,B21Vector*,B21Vector*)=nullptr;
    void (*local)(void*,const B21Vector*,B21Vector*)=nullptr;
    void (*dimensions)(void*,int32_t*,int32_t*)=nullptr;
    int32_t (*level)(void*,const B21Vector*)=nullptr;
    uint16_t (*room)(void*,B21Cell*,int,int)=nullptr;
    void* (*lotAt)(void*,float,float)=nullptr;
    void* (*roomMap)(void*,int)=nullptr;
    uint32_t (*roomValue)(void*,uint16_t)=nullptr;
    void (*grow)(B21Points*,const B21Vector*)=nullptr;
};
extern B21Services b21Services;
struct b21 {
    void** vtable; void* secondary; void* reference;
    uint32_t state; uint8_t enabled,padding[3]; float value;
    void* link; void* owner; uint32_t nameHash;
    uint32_t tick; int32_t period; float radius, distanceSquared;
    // TS3W.exe 0x0086EA60: Initialize ocean; padding and the outside-only tail remain untouched.
    b21* InitOcean();
    // TS3W.exe 0x0086EAC0: Lazily cache the ocean hash; a zero hash is retried.
    uint32_t NameOcean();
    // TS3W.exe 0x0086EB80: Load outside configuration; retain release EAX high bytes and base AL.
    uintptr_t ConfigureOutside();
    // TS3W.exe 0x0086EC40: Load outsidelot configuration; retain release EAX high bytes and base AL.
    uintptr_t ConfigureOutsideLot();
    // TS3W.exe 0x0086ECD0: Initialize outside with a 10000 squared-distance threshold.
    b21* InitOutside();
    // TS3W.exe 0x0086ED40: Lazily cache the outside hash.
    uint32_t NameOutside();
    // TS3W.exe 0x0086ED70: Initialize outsidelot without writing its unused tail.
    b21* InitOutsideLot();
    // TS3W.exe 0x0086EDD0: Lazily cache the outsidelot hash.
    uint32_t NameOutsideLot();
    // TS3W.exe 0x0086F120: Sample outside coverage on signed tick cadence; overlapping lots count repeatedly.
    uintptr_t EvaluateOutside(uint32_t,uint32_t);
    // TS3W.exe 0x0086F480: Sample lot coverage on signed tick cadence without clamping overlapping coverage.
    uintptr_t EvaluateOutsideLot(uint32_t,uint32_t);
    // TS3W.exe 0x0086F6E0: Publish the room value; the no-world path leaves entry EAX unchanged on x86.
    uintptr_t EvaluateRoom(uint32_t,uint32_t);
};
TS3_X86_OFFSET(b21,nameHash,0x20);
TS3_X86_OFFSET(b21,distanceSquared,0x30);
// TS3W.exe 0x0086EAF0: Test the signed four-lane squared separation with EA's shuffle reduction.
bool B21Intersects(const B21Vector* a,const B21Vector* b);
struct B21Sphere : B21Vector {
    // TS3W.exe 0x0086EAF0: ABI view for the ECX vector passed to the sphere test.
    bool Intersects(const void* other) const { return B21Intersects(this,static_cast<const B21Vector*>(other)); }
};
// TS3W.exe 0x0086EEE0: Append center and two six-point rings; W deliberately follows the sine offset.
B21Vector* B21Sample(const B21Vector*,float,B21Points*);
}
