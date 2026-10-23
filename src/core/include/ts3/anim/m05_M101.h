#pragma once
#include "ts3/anim/m05_B21.h"
namespace ts3::anim::m101 {
struct Clip { uint32_t words[10]; };
struct Rig { unsigned char prefix[0x28]; b21::SkeletonView skeleton; };
// TS3W.exe 0x00584230: Map 20-byte channel hashes to skeleton indices; only AL is set on return.
uintptr_t Map(Clip*,Rig*,int32_t*,void*);
// TS3W.exe 0x005843F0: Dispatch version-one clips to the original 12-byte mapper, otherwise Map.
uintptr_t Bind(Clip*,Rig*,int32_t*,void*);
// TS3W.exe 0x00584430: Lower-bound unsigned 16-bit keys in six-byte records.
const unsigned char* Lower6(const unsigned char*,const unsigned char*,const uint32_t*);
// TS3W.exe 0x00584640: Lower-bound unsigned 16-bit keys in ten-byte records.
const unsigned char* Lower10(const unsigned char*,const unsigned char*,const uint32_t*);
// TS3W.exe 0x005847A0: Lower-bound unsigned 16-bit keys in twelve-byte records.
const unsigned char* Lower12(const unsigned char*,const unsigned char*,const uint32_t*);
// TS3W.exe 0x00584AE0: Lower-bound unsigned 16-bit keys in eight-byte records.
const unsigned char* Lower8(const unsigned char*,const unsigned char*,const uint32_t*);
#if defined(_M_IX86)
inline uintptr_t __fastcall HookBind(Clip* c,void*,Rig* r,int32_t* o,void* x) { return Bind(c,r,o,x); }
#endif
}
