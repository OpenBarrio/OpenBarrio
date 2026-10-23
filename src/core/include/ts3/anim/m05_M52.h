#pragma once
#include "ts3/anim/m05_layouts.h"
namespace ts3::anim {
struct M52Controller { Controller base; void* observer; Controller* child; };
TS3_X86_OFFSET(M52Controller,observer,0x28);
TS3_X86_OFFSET(M52Controller,child,0x2c);
struct M52Stream { void* table; void* opaque4; void* source; uint32_t opaqueC,value; uint8_t ok; };
// TS3W.exe 0x0056FF60: Read version and replace the retained child, accepting versions zero and one.
uint8_t M52Load(M52Controller*, M52Stream*);
// TS3W.exe 0x0056FFD0: Release child, destroy base and optionally free, returning this even after free.
M52Controller* M52Destroy(M52Controller*,uint32_t);
// TS3W.exe 0x00570030: Forward child time (slot five), without a null guard.
float M52Time(M52Controller*);
// TS3W.exe 0x00570040: Forward child start (slot seven), without a null guard.
float M52Start(M52Controller*);
// TS3W.exe 0x00570050: Forward child end (slot eight), without a null guard.
float M52End(M52Controller*);
// TS3W.exe 0x00570060: Return child slot eleven's low byte normalized to zero or one.
bool M52Active(M52Controller*);
}
#if defined(_M_IX86)
namespace ts3::anim::m52hooks {
uint8_t __fastcall Load(M52Controller*,void*,M52Stream*);
M52Controller* __fastcall Destroy(M52Controller*,void*,uint32_t);
float __fastcall Time(M52Controller*,void*);
float __fastcall Start(M52Controller*,void*);
float __fastcall End(M52Controller*,void*);
bool __fastcall Active(M52Controller*,void*);
}
#endif
