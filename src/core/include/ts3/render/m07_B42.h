#pragma once
#include "ts3/render/m07_abi.h"
namespace ts3::render::m07::b42 {
struct Scene {
    uint8_t prefix[0x210]; uint8_t member[0x28]; uint32_t bounds[4];
    uint8_t gap248[0x56]; uint8_t flags; uint8_t gap29f[0xd]; void* handle;
};
struct Geometry { uint8_t prefix[0x214]; void* format; };
struct Lot { uint8_t prefix[0x10b9]; uint8_t enabled; uint8_t gap[0x6e]; uint8_t map[0x14]; };
struct alignas(16) Tree {
    uint8_t* levels[12]; alignas(16) float extent[4],inverse[4],origin[4];
    int32_t depth; float top,bottom; uint32_t bytes,nodes;
};
// TS3W.exe 0x006F4820: Query scene interface, otherwise forward to the base query.
void* __fastcall Query(Scene*,void*,uint32_t);
// TS3W.exe 0x006F49C0: Insert the dereferenced key into the embedded collection.
uint32_t __fastcall Insert(Scene*,void*,uint32_t,const uint32_t*);
// TS3W.exe 0x006F49E0: Return whether the dereferenced key is present (full EAX boolean).
uint32_t __fastcall Contains(Scene*,void*,const uint32_t*);
// TS3W.exe 0x006F4A00: Forward removal of a dereferenced key.
uint32_t __fastcall Remove(Scene*,void*,const uint32_t*);
// TS3W.exe 0x006F4A20: Copy four bounds words in order; preserve output-pointer high EAX with AL=1.
uintptr_t __fastcall Bounds(Scene*,void*,uint32_t*);
// TS3W.exe 0x006F4B10: Return distance, optionally multiplying its rounded float by the global scale.
long double __fastcall Distance(Scene*,void*);
// TS3W.exe 0x006F4B40: Forward to the embedded member at +210 (verified twin with changed offset/target).
uint32_t __fastcall Member(Scene*,void*);
// TS3W.exe 0x006F4C30: Temporarily install a cloned submission handle and restore the previous one.
void __fastcall Submit(Scene*,void*,uint32_t,uint32_t);
// TS3W.exe 0x006923E0: Lazily create the two-element shadow vertex format; setup failure is ignored.
uint8_t __fastcall EnsureFormat(Geometry*,void*);
// TS3W.exe 0x00702850: Find or allocate a scene model array, register it and submit its parameters.
void* __fastcall Model(Lot*,void*,uint64_t,uint32_t,uint32_t,uint32_t,uint32_t,uint32_t,void*);
// TS3W.exe 0x00705260: Find or insert a tagged SpeedTree cluster set, returning its inline value.
void* __fastcall Cluster(void*,void*,uint32_t);
// TS3W.exe 0x007057B0: Rebuild quadtree levels and parent/child links, preserving wraparound counts.
uint8_t __fastcall BuildTree(Tree*,void*,const float*,int32_t);
TS3_X86_OFFSET(Scene,bounds,0x238); TS3_X86_OFFSET(Scene,flags,0x29e);
TS3_X86_OFFSET(Scene,handle,0x2ac); TS3_X86_OFFSET(Lot,map,0x1128);
TS3_X86_OFFSET(Tree,extent,0x30); TS3_X86_OFFSET(Tree,depth,0x60);
}
