#pragma once
#include "ts3/anim/m05_B08.h"
#include "ts3/anim/m05_B21.h"
namespace ts3::anim::m39 {
struct alignas(16) BoneEntry { uint32_t key; unsigned char opaque[12]; Float4Value value; float time; unsigned char tail[12]; BoneEntry* next; };
struct BoneMap { void* opaque; BoneEntry** buckets; uint32_t bucketCount,size; float maxLoad,growth; uint32_t hash; void* allocator; };
struct BoneController { Controller base; BoneMap map; };
struct ForwardController { Controller base; unsigned char opaque[0x28]; Controller* child; };
struct InsertResult { BoneEntry* node; BoneEntry** bucket; uint8_t inserted; };
TS3_X86_OFFSET(BoneEntry,next,0x30);
TS3_X86_OFFSET(BoneController,map,0x28);
TS3_X86_OFFSET(ForwardController,child,0x50);
// TS3W.exe 0x0056C650: Construct a bone-map controller, leaving opaque map bytes untouched.
BoneController* Construct(BoneController*,void*);
// TS3W.exe 0x0056C6B0: Destroy map and base, optionally free storage, and return this after free.
BoneController* Destroy(BoneController*,uint32_t);
// TS3W.exe 0x0056C720: Set bone time and identity quaternion; return node, null rig, or minus one index in EAX.
intptr_t SetTime(BoneController*,uint32_t,float);
// TS3W.exe 0x0056C830: Forward virtual slot 7 or load the shared fallback float.
float First(ForwardController*);
// TS3W.exe 0x0056C850: Forward virtual slot 8 or load the shared fallback float.
float Second(ForwardController*);
// TS3W.exe 0x0056C870: Forward virtual slot 9 or load the shared fallback float.
float Third(ForwardController*);
#if defined(_M_IX86)
inline BoneController* __fastcall HookConstruct(BoneController* p,void*,void* h) { return Construct(p,h); }
inline BoneController* __fastcall HookDestroy(BoneController* p,void*,uint32_t f) { return Destroy(p,f); }
inline intptr_t __fastcall HookSetTime(BoneController* p,void*,uint32_t k,float t) { return SetTime(p,k,t); }
inline float __fastcall HookFirst(ForwardController* p,void*) { return First(p); }
inline float __fastcall HookSecond(ForwardController* p,void*) { return Second(p); }
inline float __fastcall HookThird(ForwardController* p,void*) { return Third(p); }
#endif
}
