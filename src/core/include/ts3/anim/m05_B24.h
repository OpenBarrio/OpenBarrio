#pragma once
#include "ts3/anim/m05_layouts.h"
namespace ts3::anim::b24 {
template<class T> struct Vector { T* begin; T* end; T* capacity; void* descriptor; };
struct Chain { Vector<int32_t> joints; int32_t indices[14]; uint32_t flags; };
TS3_X86_SIZE(Chain,0x4c);
// TS3W.exe 0x0057B320: Query the rig resource interfaces.
void* __fastcall Query(RigResource*,void*,uint32_t);
// TS3W.exe 0x0057B360: Test the first seven signed rig indices.
uint32_t __fastcall HasIndices(RigResource*,void*);
// TS3W.exe 0x0057B4A0: Lazily resolve the vertex channel count, defaulting to four.
uint32_t __fastcall ChannelCount(RigResource*,void*);
// TS3W.exe 0x0057B4F0: Resolve a vertex channel hash with the four-channel fallback.
int32_t __fastcall ChannelIndex(RigResource*,void*,uint32_t);
// TS3W.exe 0x0057B730: Copy aligned SRT records forward and return the destination end.
SrtValue* __cdecl CopySrt(const SrtValue*,const SrtValue*,SrtValue*);
// TS3W.exe 0x0057B7B0: Initialize an IK chain's collection and fourteen invalid indices.
Chain* __fastcall InitChain(Chain*,void*);
// TS3W.exe 0x0057B830: Destroy chain storage, preserving dangling vector fields.
void __fastcall DestroyChains(Vector<Chain>*,void*);
// TS3W.exe 0x0057B960: Return the rig resource type identifier.
uint32_t __fastcall Type(RigResource*,void*);
// TS3W.exe 0x0057B9E0: Insert copies of an SRT into a vector, preserving aliasing.
void __fastcall InsertSrt(Vector<SrtValue>*,void*,SrtValue*,uint32_t,const SrtValue*);
// TS3W.exe 0x0057BCE0: Resize SRT storage; growth deliberately copies an uninitialized SRT.
void __fastcall ResizeSrt(Vector<SrtValue>*,void*,uint32_t);
// TS3W.exe 0x0057BD90: Assign chains forward and return the destination end.
Chain* __cdecl CopyChains(const Chain*,const Chain*,Chain*);
// TS3W.exe 0x0057BEA0: Erase a chain range and return its old beginning.
Chain* __fastcall EraseChains(Vector<Chain>*,void*,Chain*,Chain*);
// TS3W.exe 0x0057BF10: Insert chain copies with deep collection copy and geometric growth.
void __fastcall InsertChains(Vector<Chain>*,void*,Chain*,uint32_t,const Chain*);
// TS3W.exe 0x0057C0F0: Resize the chain vector with default invalid-index chains.
void __fastcall ResizeChains(Vector<Chain>*,void*,uint32_t);
// TS3W.exe 0x0057C760: Return the float at object offset 0x38.
float __fastcall Weight(const void*,void*);
}
