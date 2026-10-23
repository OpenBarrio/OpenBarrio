#pragma once
#include "ts3/layout.h"
namespace ts3::anim::b15 {
struct Child { void** vtable; uint32_t refs; };
struct Blend { void** vtable; uint32_t refs; void* interfaces[2]; unsigned char opaque[0x18]; Child* first; Child* second; float duration,phase; };
struct Forward { unsigned char opaque[0x50]; Child* child; };
struct Reader { void** vtable; uint32_t unused; void* stream; uint32_t unknown,endian; uint8_t good; };
struct Node { uint32_t key; unsigned char payload[0x2c]; Node* next; };
struct Table { void* opaque; Node** buckets; uint32_t capacity,count; unsigned char policy[12]; uint32_t allocator; };
struct InsertResult { Node* node; Node** bucket; uint8_t inserted; };
TS3_X86_OFFSET(Blend,first,0x28); TS3_X86_OFFSET(Blend,phase,0x34);
TS3_X86_OFFSET(Forward,child,0x50); TS3_X86_OFFSET(Reader,good,0x14);
TS3_X86_OFFSET(Node,next,0x30); TS3_X86_OFFSET(Table,allocator,0x1c);
// TS3W.exe 0x0056BCD0: Return the maximum-float time sentinel.
float Sentinel();
// TS3W.exe 0x0056BCE0: Query second child before first, otherwise return maximum float.
float Time(Blend*);
// TS3W.exe 0x0056BDF0: Return the blend interface identifier.
uint32_t BlendType();
// TS3W.exe 0x0056BE00: Return the enclosing blend for either supported interface id.
void* BlendQuery(void*,uint32_t);
// TS3W.exe 0x0056BFA0: Tick children then advance phase using EA's sign-bit clamps.
uintptr_t Tick(Blend*,float);
// TS3W.exe 0x0056C1A0: Stop and release children in first/second order, clearing before release.
void Clear(Blend*);
// TS3W.exe 0x0056C300: Read a boolean word (unsigned <= 1) with the reader's sticky status.
bool ReadPositive(void*,Reader*);
// TS3W.exe 0x0056C350: Free every hash chain and clear bucket heads.
void __stdcall FreeChains(Node**,uint32_t);
// TS3W.exe 0x0056C520: Return the transform interface identifier.
uint32_t TransformType();
// TS3W.exe 0x0056C530: Return the enclosing transform for either supported interface id.
void* TransformQuery(void*,uint32_t);
// TS3W.exe 0x0056C560: Find or insert a key, delegating allocation and rehash policy to EA.
InsertResult* Insert(Table*,InsertResult*,const uint32_t*,void*);
// TS3W.exe 0x0056C7B0: Forward the tick to the optional child.
void ForwardTick(Forward*,float);
// TS3W.exe 0x0056C7D0: Forward the set-time operation to the optional child.
void ForwardSet(Forward*,float);
// TS3W.exe 0x0056C7F0: Return child time or positive zero.
float ForwardTime(Forward*);
// TS3W.exe 0x0056C810: Return child duration or positive zero.
float ForwardDuration(Forward*);
#if defined(_M_IX86)
inline float __fastcall HookTime(Blend* s,void*) { return Time(s); }
inline void* __fastcall HookBlendQuery(void* s,void*,uint32_t id) { return BlendQuery(s,id); }
inline uintptr_t __fastcall HookTick(Blend* s,void*,float t) { return Tick(s,t); }
inline void __fastcall HookClear(Blend* s,void*) { return Clear(s); }
inline bool __fastcall HookReadPositive(void* s,void*,Reader* r) { return ReadPositive(s,r); }
inline void* __fastcall HookTransformQuery(void* s,void*,uint32_t id) { return TransformQuery(s,id); }
inline InsertResult* __fastcall HookInsert(Table* s,void*,InsertResult* r,const uint32_t* k,void* v) { return Insert(s,r,k,v); }
inline void __fastcall HookForwardTick(Forward* s,void*,float t) { return ForwardTick(s,t); }
inline void __fastcall HookForwardSet(Forward* s,void*,float t) { return ForwardSet(s,t); }
inline float __fastcall HookForwardTime(Forward* s,void*) { return ForwardTime(s); }
inline float __fastcall HookForwardDuration(Forward* s,void*) { return ForwardDuration(s); }
#endif
}
