#pragma once
#include "ts3/layout.h"
namespace ts3::anim::m143 {
struct Body {
    void* vtable; void* secondary; int32_t refs; unsigned char pad0[8];
    void* host; void* interfaceTable; void* context; void* first;
    unsigned char pad24[40];
    void* six; void* seven; void* change; void* same; void* fromSix; void* fromOther; void* two; void* three;
    unsigned char pad6c[12]; void* condition;
    unsigned char flags[4]; unsigned char flag80,flag81,flag82,flag83,flag84;
    unsigned char pad85[27]; unsigned char request[16]; int32_t mode;
    unsigned char padb4[16]; int32_t count; unsigned char padc8[8];
    alignas(16) float start[4]; unsigned char pade0[16]; float finish[4];
    unsigned char pad100[16]; uint32_t keys[4]; unsigned char reset[36];
    void* active; int32_t state; void* source; int32_t remaining;
};
TS3_X86_OFFSET(Body,first,0x20); TS3_X86_OFFSET(Body,six,0x4c);
TS3_X86_OFFSET(Body,request,0xa0); TS3_X86_OFFSET(Body,start,0xd0);
TS3_X86_OFFSET(Body,active,0x144); TS3_X86_OFFSET(Body,remaining,0x150);
struct Brain { void* vtable; void* secondary; unsigned char pad8[16]; void* interfaceTable; unsigned char pad1c[180]; uint32_t value; };
TS3_X86_OFFSET(Brain,value,0xd0);
// TS3W.exe 0x00854D30: Select a stairs animation state, retaining recursive fallback and cleanup order.
uintptr_t State(Body*,int32_t);
// TS3W.exe 0x008550E0: Handle a stairs animation callback with a temporary body reference.
uintptr_t Update(void* interfaceSelf,void* const*,int32_t);
// TS3W.exe 0x008555C0: Destroy a stairs body and optionally free it, returning the freed address.
void* Destroy(void*,uint32_t);
// TS3W.exe 0x008555E0: Prepare a stairs request and start the initial animation.
uint8_t Start(Body*,void*);
// TS3W.exe 0x008557A0: Construct the off-screen brain and install its three interface tables.
Brain* Construct(Brain*,void*,void*);
// TS3W.exe 0x008558A0: Adjust the secondary interface by one pointer and forward deleting destruction.
void* DestroySecondary(void*,uint32_t);
#if defined(_M_IX86)
namespace hooks {
uintptr_t __fastcall State(Body*,void*,int32_t);
uintptr_t __fastcall Update(void*,void*,void* const*,int32_t);
void* __fastcall Destroy(void*,void*,uint32_t);
uint8_t __fastcall Start(Body*,void*,void*);
Brain* __fastcall Construct(Brain*,void*,void*,void*);
void* __fastcall DestroySecondary(void*,void*,uint32_t);
}
#endif
}
