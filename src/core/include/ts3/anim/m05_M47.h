#pragma once
#include "ts3/anim/m05_layouts.h"
namespace ts3::anim::m47 {
struct Ref { void* vtable; int32_t refs; };
struct Scratch {
    Controller base;
    void* listener;
    Ref* source;
    Ref* controller;
    Ref* curve;
    float weight;
    uint32_t value3c, value40;
    uint8_t flag44, flag45, flag46;
};
TS3_X86_OFFSET(Scratch,curve,0x34);
TS3_X86_OFFSET(Scratch,flag46,0x46);
struct Event { void* sender; Ref* resource; void* host; uint32_t a,b; uint8_t x,y,z; };
TS3_X86_SIZE(Event,0x18);
// TS3W.exe 0x0056E4C0: construct and retain a synchronization controller.
Scratch* Construct(Scratch*,void*,Ref*,Ref*,Ref*,uint32_t,uint32_t,float,uint8_t,uint8_t,uint8_t);
// TS3W.exe 0x0056E580: detach listener and release curve, controller, source in order.
uintptr_t Destroy(Scratch*);
// TS3W.exe 0x0056E6E0: relay an event from the +28 listener, retaining its resource.
uintptr_t Relay(void*,const Event*,uint32_t);
// TS3W.exe 0x0056E7A0: destroy and optionally free, returning the freed this pointer.
Scratch* Delete(Scratch*,uint32_t);
// TS3W.exe 0x0056E7C0: sample either side of a curve; empty curves return four zero lanes.
Float4Value* Sample(Scratch*,Float4Value*,uint32_t,uint32_t,float);
// TS3W.exe 0x0056E8E0: estimate an event crossing from four samples, retaining SSE ordering.
float Crossing(Scratch*,uint32_t,uint32_t,float,float);
}

#if defined(_M_IX86)
namespace ts3::anim::m47_hooks {
using namespace m47;
Scratch* __fastcall Construct(Scratch* s,void*,void* host,Ref* source,Ref* curve,Ref* controller,uint32_t a,uint32_t b,float weight,uint8_t x,uint8_t y,uint8_t z);
uintptr_t __fastcall Destroy(Scratch* s,void*);
uintptr_t __fastcall Relay(void* s,void*,const Event* e,uint32_t unused);
Scratch* __fastcall Delete(Scratch* s,void*,uint32_t flags);
Float4Value* __fastcall Sample(Scratch* s,void*,Float4Value* out,uint32_t a,uint32_t b,float t);
float __fastcall Crossing(Scratch* s,void*,uint32_t a,uint32_t b,float t,float step);
}
#endif
