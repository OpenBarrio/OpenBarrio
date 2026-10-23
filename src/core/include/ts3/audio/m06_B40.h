#pragma once
#include "ts3/layout.h"
namespace ts3::audio::b40 {
struct Children {
    void** begin; void** end; void** capacity; const char* label;
    void** storage; void* inlineItems[10];
};
struct Scene {
    void** vtable; void** secondary; void** interfaceTable; uint32_t refs;
    Children children;
    uint8_t dirty; uint8_t padding[3];
    uint32_t keyLow,keyHigh,tick;
    void* callback;
    float value; uint8_t registered; uint8_t padding2[3]; void* owner;
};
TS3_X86_OFFSET(Scene,children,0x10);
TS3_X86_OFFSET(Scene,dirty,0x4c);
TS3_X86_OFFSET(Scene,callback,0x5c);
TS3_X86_OFFSET(Scene,owner,0x68);
// TS3W.exe 0x008A64B0: Read the dirty byte (only AL is defined).
uint8_t Dirty(const Scene*);
// TS3W.exe 0x008A64C0: Store and return the low argument byte.
uint8_t SetDirty(Scene*,uint32_t);
// TS3W.exe 0x008A64D0: Store and return the callback pointer.
void* SetCallback(Scene*,void*);
// TS3W.exe 0x008A64E0: Register unconditionally on enable, unregister only when registered; no defined no-op EAX.
void Enable(Scene*,uint32_t);
// TS3W.exe 0x008A6530: Store the two-word aggregation key, returning its low word.
uint32_t SetKey(Scene*,uint32_t,uint32_t);
// TS3W.exe 0x008A6550: Return the borrowed owner pointer.
void* Owner(const Scene*);
// TS3W.exe 0x008A6560: Query three supported interfaces, including a null-preserving secondary adjustment.
void* Query(Scene*,uint32_t);
// TS3W.exe 0x008A6590: Pop a 0x70-byte pooled allocation, refilling as necessary, or use the fallback allocator.
void* Allocate(uint32_t);
// TS3W.exe 0x008A6600: Return the signed pointer-vector count from the secondary view.
int32_t Count(const void*);
// TS3W.exe 0x008A6610: Aggregate child values with per-step float stores and ordered min/max comparisons.
float Aggregate(Scene*);
// TS3W.exe 0x008A6850: Increment the non-atomic reference count with wrapping arithmetic.
uint32_t AddRef(Scene*);
// TS3W.exe 0x008A6860: Disable, clear matching-owner children and release all children; EAX is begin with AL=1.
uintptr_t Clear(Scene*);
// TS3W.exe 0x008A68E0: Update once per nonzero tick, propagate dirty flags and recompute the cached value.
void Update(Scene*,float,uint32_t);
// TS3W.exe 0x008A6960: Release vector entries and free non-inline storage without resetting the vector.
uintptr_t DestroyChildren(Children*);
// TS3W.exe 0x008A69A0: Adjust secondary this back to primary and call the original deleting destructor.
void* DeleteSecondary(void*,uint32_t);
// ABI adapters for replacement hooks; short getters cannot host a five-byte jump.
#if defined(_M_IX86)
inline uint8_t __fastcall HookSetDirty(Scene* s,void*,uint32_t v) { return SetDirty(s,v); }
inline void* __fastcall HookSetCallback(Scene* s,void*,void* p) { return SetCallback(s,p); }
inline void __fastcall HookEnable(Scene* s,void*,uint32_t v) { Enable(s,v); }
inline uint32_t __fastcall HookSetKey(Scene* s,void*,uint32_t lo,uint32_t hi) { return SetKey(s,lo,hi); }
inline void* __fastcall HookQuery(Scene* s,void*,uint32_t id) { return Query(s,id); }
inline int32_t __fastcall HookCount(void* s,void*) { return Count(s); }
inline float __fastcall HookAggregate(Scene* s,void*) { return Aggregate(s); }
inline uint32_t __fastcall HookAddRef(Scene* s,void*) { return AddRef(s); }
inline uintptr_t __fastcall HookClear(Scene* s,void*) { return Clear(s); }
inline void __fastcall HookUpdate(Scene* s,void*,float dt,uint32_t tick) { Update(s,dt,tick); }
inline uintptr_t __fastcall HookDestroyChildren(Children* s,void*) { return DestroyChildren(s); }
inline void* __fastcall HookDeleteSecondary(void* s,void*,uint32_t flags) { return DeleteSecondary(s,flags); }
#endif
}
