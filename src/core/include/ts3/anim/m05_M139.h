#pragma once
#include <cstdint>
#include "ts3/anim/m05_layouts.h"
namespace ts3::anim::m139 {
struct Node { Node* next; Node* previous; void* controller; };
struct Body {
    uint8_t head[0x14]; void* canvas;
    void* secondary; Node controllers; // sentinel's third word is the allocation descriptor
    uint8_t gap28[0x18]; void* factory;
    uint8_t gap44[0x38]; int32_t styleIndex;
    uint8_t gap80[4]; int32_t styleType;
    uint8_t gap88[0x28]; void* slaveHost; void** slaves;
    uint8_t gapb8[0xc]; void* walker;
    int32_t slaveCount; uint8_t slaveMotion; uint8_t gapcd[0x23];
    uint8_t statef0[0x1c]; void* argument;
    uint8_t gap110[0x14]; void* registry;
    uint8_t gap128[0x28]; float speed;
    uint8_t gap154[0x18];
    int32_t lower,current,upper;
    uint8_t gap178[5]; uint8_t pending, gap17e, blocked;
    uint8_t gap180[2]; uint8_t dirty,force, gap184,disabled;
    uint8_t gap186[2]; void* begin; void* end; void* capacity;
    uint8_t gap194[0xb4]; uint32_t limit,count;
    uint8_t flag250,flag251,flag252,flag253;
};
// The 28-byte x86 request passed by value to 00849D40; pointers widen natively.
struct RootRequest {
    void* registry; float distance,angle,heading,weight;
    uint8_t flag0,flag1,padding[2]; void* owned;
};
struct ControllerResult { void* pointer; uint8_t flag; };
struct SlaveVector { void** begin; void** end; void** capacity; const void* descriptor; };
// TS3W.exe 0x00851710: build root-motion controllers with original SIMD refinement, route rejection, and cleanup ordering.
uint8_t RootMotion(Body* self,uint8_t force,uint8_t snap);
// TS3W.exe 0x00852520: detach pending controllers and dispatch the next root-motion state; preserves full EAX from callees.
uint32_t Advance(Body* self);
#if defined(_M_IX86)
uint32_t __fastcall HookAdvance(Body* self,void*);
uint8_t __fastcall HookRootMotion(Body* self,void*,uint8_t force,uint8_t snap);
#endif
// TS3W.exe 0x008526E0: destroy the animated body, optionally free on flags bit zero, and return the original pointer.
Body* Delete(Body* self,uint32_t flags);
}
