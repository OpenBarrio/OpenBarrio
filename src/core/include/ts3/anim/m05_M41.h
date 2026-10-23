#pragma once
#include "ts3/anim/m05_B08.h"
namespace ts3::anim::m41 {
struct alignas(16) Transform {
    Controller base;
    void* delegateTable;
    alignas(16) Float4Value translation;
    Float4Value rotation;
    Controller* child;
};
TS3_X86_OFFSET(Transform,delegateTable,0x28);
TS3_X86_OFFSET(Transform,translation,0x30);
TS3_X86_OFFSET(Transform,rotation,0x40);
TS3_X86_OFFSET(Transform,child,0x50);
// TS3W.exe 0x0056CA60: Return the transform interface type identifier.
uint32_t Type(void*);
// TS3W.exe 0x0056CA70: Query the secondary interface, returning the enclosing controller on an exact match.
Transform* Query(void*,uint32_t);
// TS3W.exe 0x0056CA90: Adjust the secondary this pointer and invoke the original deleting destructor.
Transform* DeleteSecondary(void*,uint32_t);
// TS3W.exe 0x0056CAA0: Construct a transform, retain its child, and subscribe the embedded delegate.
Transform* Construct(Transform*,void*,Controller*);
// TS3W.exe 0x0056CBA0: Load version, rotation, translation and child in order, returning stream status in AL.
uint8_t Load(Transform*,b08::StreamState*);
// TS3W.exe 0x0056CC30: Unsubscribe then release the child and destroy the base, preserving base EAX.
uintptr_t Destroy(Transform*);
#if defined(_M_IX86)
inline uint32_t __fastcall HookType(void* p,void*) { return Type(p); }
inline Transform* __fastcall HookQuery(void* p,void*,uint32_t k) { return Query(p,k); }
inline Transform* __fastcall HookDeleteSecondary(void* p,void*,uint32_t f) { return DeleteSecondary(p,f); }
inline Transform* __fastcall HookConstruct(Transform* p,void*,void* h,Controller* c) { return Construct(p,h,c); }
inline uint8_t __fastcall HookLoad(Transform* p,void*,b08::StreamState* s) { return Load(p,s); }
inline uintptr_t __fastcall HookDestroy(Transform* p,void*) { return Destroy(p); }
#endif
}
