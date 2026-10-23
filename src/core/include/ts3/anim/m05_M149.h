#pragma once
#include "ts3/layout.h"
#include <immintrin.h>
namespace ts3::anim::m149 {
struct alignas(16) Event {
    void* vtable;
    unsigned char opaque[16-sizeof(void*)];
    __m128 value;
};
struct Strings { void* begin; void* end; void* capacity; const char* label; };
struct alignas(16) Vehicle {
    void* vtable;
    void* interface4;
    unsigned char opaque8[12];
    void* host;
    void* interface18;
    // Align the SIMD fields natively; the x86 padding is the observed +1c field.
    uint32_t opaque1c;
    __m128 velocity, rotation;
    Event cruise, decelerate;
    uint32_t eventState;
    Strings cruiseName, decelerateName;
    uint32_t argument;
    float maxSpeed, speed, acceleration, valueB4, distance, valueBC, valueC0;
    uint32_t mode, valueC8, flags;
};
TS3_X86_OFFSET(Vehicle, host, 0x14);
TS3_X86_OFFSET(Vehicle, interface18, 0x18);
TS3_X86_OFFSET(Vehicle, velocity, 0x20);
TS3_X86_OFFSET(Vehicle, decelerate, 0x60);
TS3_X86_OFFSET(Vehicle, speed, 0xac);
TS3_X86_OFFSET(Vehicle, flags, 0xcc);
TS3_X86_SIZE(Vehicle, 0xd0);
// TS3W.exe 0x00859980: Aim velocity toward a target, keeping two-step rsqrt, sign-bit angle clamps and path-dependent EAX.
uintptr_t Aim(Vehicle*, const __m128*);
// TS3W.exe 0x0085A100: Construct the vehicle controller and attach its two embedded event interfaces.
Vehicle* Construct(Vehicle*, void* host, uint32_t argument);
// TS3W.exe 0x0085A260: Destroy the controller and optionally free it, returning this even after free.
Vehicle* Delete(Vehicle*, uint32_t flags);
// TS3W.exe 0x0085A280: Advance along the retained route, keeping sign-bit speed clamps, low-byte predicates and release ordering.
uintptr_t Update(Vehicle*, float elapsed);
// TS3W.exe 0x0085A7D0: Restore the animated-body base vtable without touching any other bytes.
void InitBase(void*);
// TS3W.exe 0x0085A7F0: Initialize an event interface, leaving the opaque middle bytes untouched.
Event* InitEvent(Event*);
}
#if defined(_M_IX86)
namespace ts3::anim::m149::hooks {
uintptr_t __fastcall Aim(Vehicle*,void*,const __m128*);
Vehicle* __fastcall Construct(Vehicle*,void*,void*,uint32_t);
Vehicle* __fastcall Delete(Vehicle*,void*,uint32_t);
uintptr_t __fastcall Update(Vehicle*,void*,float);
void __fastcall InitBase(void*,void*);
Event* __fastcall InitEvent(Event*,void*);
}
#endif
