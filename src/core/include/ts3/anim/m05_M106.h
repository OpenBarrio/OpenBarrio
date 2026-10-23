#pragma once
#include "ts3/anim/m05_B20.h"
namespace ts3::anim::m106 {
// Serialized clip fields are relative offsets, not native pointers.
struct Clip { uint32_t prefix[4]; float interval; uint32_t reserved,count,reserved1; int32_t channels,values; };
struct Style { uint32_t zero[20]; unsigned char untouched[28]; const char* begin; const char* end; const char* capacity; const char* descriptor; };
struct RenderOwner { unsigned char prefix[0x210]; unsigned char binding[0x88]; void* state; unsigned char gap[0x20]; int32_t transformBone; };
TS3_X86_OFFSET(Style,begin,0x6c);
TS3_X86_OFFSET(RenderOwner,state,0x298);
TS3_X86_OFFSET(RenderOwner,transformBone,0x2bc);
// TS3W.exe 0x005877B0: Sample root motion between clamped times, retaining SSE interpolation and mirroring signs.
uint8_t RootMotion(const Clip*,b20::Pose*,float,float,uint8_t);
// TS3W.exe 0x005886E0: Dispatch absolute sampling on the clip format word at +8.
uint8_t Sample(const Clip*,b20::Pose*,float,uint32_t,uint8_t);
// TS3W.exe 0x00588730: Dispatch root motion sampling on the clip format word at +8.
uint8_t Delta(const Clip*,b20::Pose*,float,float,uint8_t);
// TS3W.exe 0x006F5CA0: Prepare render state, bind the rig, and cache transformBone; null keeps the old index.
uintptr_t Bind(RenderOwner*,void*);
// TS3W.exe 0x007513F0: Initialize style zeros and empty sound string, preserving the opaque middle region.
Style* Construct(Style*);
#if defined(_M_IX86)
uint8_t __fastcall HookSample(const Clip*,void*,b20::Pose*,float,uint32_t,uint8_t);
uint8_t __fastcall HookDelta(const Clip*,void*,b20::Pose*,float,float,uint8_t);
uintptr_t __fastcall HookBind(RenderOwner*,void*,void*);
Style* __fastcall HookConstruct(Style*,void*);
#endif
}
