#pragma once
#include "ts3/anim/m05_B08.h"
#include "ts3/anim/m05_B21.h"
namespace ts3::anim::m37 {
struct alignas(16) BoneEntry {
    uint32_t key;
    unsigned char opaque04[12];
    Float4Value start, target, current;
    float duration, elapsed, stamp;
    unsigned char opaque4c[4];
    BoneEntry* next;
};
struct BoneMap {
    void* opaque00;
    BoneEntry** buckets;
    uint32_t bucketCount, size;
    float maxLoad, growth;
    uint32_t hash;
    void* allocator;
};
struct Transform { Controller base; BoneMap map; };
struct Time { Controller base; Controller* first; Controller* second; float scale, offset; };
struct InsertResult { BoneEntry* node; BoneEntry** bucket; uint8_t inserted; };
TS3_X86_OFFSET(BoneEntry,start,0x10);
TS3_X86_OFFSET(BoneEntry,target,0x20);
TS3_X86_OFFSET(BoneEntry,current,0x30);
TS3_X86_OFFSET(BoneEntry,duration,0x40);
TS3_X86_OFFSET(BoneEntry,next,0x50);
TS3_X86_OFFSET(Transform,map,0x28);
TS3_X86_OFFSET(Time,scale,0x30);
// TS3W.exe 0x0056B850: Adjust secondary-interface this by 12 and tail-call deleting destruction.
Transform* DestroySecondary(void*,uint32_t);
// TS3W.exe 0x0056B950: Construct a transform controller and its empty inline bone map.
Transform* ConstructTransform(Transform*,void*);
// TS3W.exe 0x0056B9B0: Destroy bone chains, optional bucket array, and base, returning this after free.
Transform* DestroyTransform(Transform*,uint32_t);
// TS3W.exe 0x0056BA20: Set a bone target and reset interpolation; NaN duration does not snap current.
void SetTarget(Transform*,uint32_t,const Float4Value*,float);
// TS3W.exe 0x0056BD10: Save time-controller children and floats, short-circuiting on stream status (AL only).
uint8_t SaveTime(Time*,b08::StreamState*);
// TS3W.exe 0x0056BDA0: Construct a time controller with null children and two minus-one floats.
Time* ConstructTime(Time*,void*);
#if defined(_M_IX86)
inline Transform* __fastcall HookDestroySecondary(void* p,void*,uint32_t f) { return DestroySecondary(p,f); }
inline Transform* __fastcall HookConstructTransform(Transform* p,void*,void* h) { return ConstructTransform(p,h); }
inline Transform* __fastcall HookDestroyTransform(Transform* p,void*,uint32_t f) { return DestroyTransform(p,f); }
inline void __fastcall HookSetTarget(Transform* p,void*,uint32_t k,const Float4Value* v,float t) { SetTarget(p,k,v,t); }
inline uint8_t __fastcall HookSaveTime(Time* p,void*,b08::StreamState* s) { return SaveTime(p,s); }
inline Time* __fastcall HookConstructTime(Time* p,void*,void* h) { return ConstructTime(p,h); }
#endif
}
