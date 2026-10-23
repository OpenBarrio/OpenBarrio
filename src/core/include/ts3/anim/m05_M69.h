#pragma once
#include "ts3/anim/m05_layouts.h"
namespace ts3::anim::m69 {
struct Name { char* begin; char* end; char* capacity; const char* descriptor; };
struct Manager {
    Manager* next; Manager* previous;
    void* listNext; void* listPrevious; void* owner;
    uint32_t count; void* removeBegin; void* removeEnd; void* removeCapacity;
    const char* descriptor; uint32_t mode;
    unsigned char flags[13]; unsigned char opaque39[7];
    uint64_t missingKey; Name* missingName; unsigned char opaque4c[4];
};
struct Render { RenderState state; uint32_t flags; };
TS3_X86_OFFSET(Manager,flags,0x2c);
TS3_X86_OFFSET(Manager,missingKey,0x40);
TS3_X86_OFFSET(Manager,missingName,0x48);
TS3_X86_OFFSET(Render,flags,0x34);
// TS3W.exe 0x00572380: Lazily resolve the missing-animation name, with formatted fallback.
Name* MissingName(Manager*);
// TS3W.exe 0x00572490: Initialize the manager and register its debug controls, preserving opaque bytes.
Manager* Construct(Manager*);
// TS3W.exe 0x00572AD0: Register factories once and allocate the manager; only AL is a boolean.
uintptr_t Initialize();
// TS3W.exe 0x00573320: Set render dirty bit without touching the remaining flags.
void MarkDirty(Render*);
// TS3W.exe 0x00573330: Update matrices and pack weights, clamping negative weights in the source.
void Update(Render*,RigPose*);
// TS3W.exe 0x005734E0: Fetch a world matrix; AL indicates validity and upper EAX is preserved.
uintptr_t GetMatrix(Render*,int32_t,Matrix44Value*);
#if defined(_M_IX86)
inline Name* __fastcall HookMissingName(Manager* p,void*) { return MissingName(p); }
inline Manager* __fastcall HookConstruct(Manager* p,void*) { return Construct(p); }
inline void __fastcall HookMarkDirty(Render* p,void*) { MarkDirty(p); }
inline void __fastcall HookUpdate(Render* p,void*,RigPose* pose) { Update(p,pose); }
inline uintptr_t __fastcall HookGetMatrix(Render* p,void*,int32_t i,Matrix44Value* m) { return GetMatrix(p,i,m); }
#endif
}
