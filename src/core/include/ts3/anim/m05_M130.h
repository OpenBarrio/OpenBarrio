#pragma once
#include "ts3/layout.h"
namespace ts3::anim::m130 {
struct Reference { void** vtable; volatile int32_t refs; };
struct Style {
    Reference* controllers[20];
    uint32_t values[5];
    uint8_t flags[6],padding[2];
    const char *begin,*end,*capacity,*descriptor;
};
struct ProgressionEntry { unsigned char opaque[24]; uint32_t key; };
struct Progression {
    ProgressionEntry *begin,*end,*capacity;
    unsigned char opaque[176];
};
struct Body {
    unsigned char prefix[20]; void* host;
    unsigned char opaque18[100]; uint32_t mode;
    unsigned char opaque80[4]; uint32_t kind;
    unsigned char opaque88[40]; Style* styles; void** slaves;
    unsigned char opaqueB8[16]; int32_t count; uint8_t direct;
    unsigned char opaqueCD[87]; void* primary;
    unsigned char opaque128[96]; ProgressionEntry *begin,*end;
    unsigned char opaque190[180]; Progression* progression;
    uint32_t current,target;
};
struct Request {
    void* slave; float time;
    uintptr_t arg6,arg7,arg8,arg9;
    Reference* nameSpace; uint8_t option;
};
TS3_X86_SIZE(Style,124);
TS3_X86_SIZE(Progression,188);
TS3_X86_OFFSET(Body,host,0x14);
TS3_X86_OFFSET(Body,mode,0x7c);
TS3_X86_OFFSET(Body,kind,0x84);
TS3_X86_OFFSET(Body,styles,0xb0);
TS3_X86_OFFSET(Body,slaves,0xb4);
TS3_X86_OFFSET(Body,count,0xc8);
TS3_X86_OFFSET(Body,primary,0x124);
TS3_X86_OFFSET(Body,begin,0x188);
TS3_X86_OFFSET(Body,progression,0x244);
TS3_X86_OFFSET(Request,nameSpace,24);
// TS3W.exe 0x00849D40: creates each slave's controller and optional root-motion wrapper, rolling back prior outputs on failure; retains EA null-dereference and clamp quirks.
bool Build(Body*,Reference*** outputs,int action,void* slave,float time,uintptr_t arg6,uintptr_t arg7,uintptr_t arg8,uintptr_t arg9,Reference* nameSpace,bool option);
// No-active-slave convenience entry used by the original subset fixture.
bool RunNoActive(Body*,Reference*);
#if defined(_M_IX86)
bool __fastcall HookBuild(Body*,void*,Reference***,int,void*,float,uintptr_t,uintptr_t,uintptr_t,uintptr_t,Reference*,bool);
#endif
}
