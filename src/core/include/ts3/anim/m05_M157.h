#pragma once
#include "ts3/layout.h"
namespace ts3::anim::m157 {
struct RangeState {
 void* next; void* previous; uint32_t descriptor; float weight;
 int32_t ids[3]; uint32_t zero[2]; uint8_t flags[8];
};
struct Body {
 void* table; void* host; void* interfaceTable;
 uint8_t opaque0c[0x48-0x0c]; uint32_t keys[4]; uint8_t opaque58[0x10];
 RangeState range; uint32_t mode; void* owned;
 uint8_t opaque9c[0x20]; uint32_t flags; float loopSound;
 uint8_t opaquec4[7]; uint8_t pending;
 uint8_t opaquecc[0x20]; uint8_t disabled0,disabled1;
 uint8_t opaqueee[10]; int32_t state; uint8_t opaquefc[0x10]; void* media;
};
struct StyleOwner { void* table; void* secondary; uint8_t opaque[16]; uint8_t style[128]; };
TS3_X86_OFFSET(Body,range,0x68);
TS3_X86_OFFSET(Body,mode,0x94);
TS3_X86_OFFSET(Body,owned,0x98);
TS3_X86_OFFSET(Body,flags,0xbc);
TS3_X86_OFFSET(Body,pending,0xcb);
TS3_X86_OFFSET(Body,state,0xf8);
TS3_X86_OFFSET(Body,media,0x10c);
// TS3W.exe 0x00953530: Construct the style-owning locomotion interface.
StyleOwner* Construct(StyleOwner*,void*);
// TS3W.exe 0x009558F0: Resolve the loop-sound scalar, defaulting to five.
double LoopSound(Body*);
// TS3W.exe 0x00955FB0: Replace the stairs body when a requested style requires it.
int Stairs(Body*,void*);
// TS3W.exe 0x00956100: Try the current controller and release its temporary reference.
bool TryController(Body*,void*);
// TS3W.exe 0x009564A0: Apply style keys and their media/controller flags.
uintptr_t Apply(Body*,const uint32_t*,uint8_t,uint8_t);
// TS3W.exe 0x009566C0: Change state on the secondary interface, preserving reset order.
intptr_t SetState(void*,int,int,int);
}
#if defined(_M_IX86)
namespace ts3::anim::m157::hooks {
StyleOwner* __fastcall Construct(StyleOwner*,void*,void*);
double __fastcall LoopSound(Body*,void*);
int __fastcall Stairs(Body*,void*,void*);
bool __fastcall TryController(Body*,void*,void*);
uintptr_t __fastcall Apply(Body*,void*,const uint32_t*,uint8_t,uint8_t);
intptr_t __fastcall SetState(void*,void*,int,int,int);
}
#endif
