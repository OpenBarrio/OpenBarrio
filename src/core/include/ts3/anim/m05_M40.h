#pragma once
#include "ts3/anim/m05_layouts.h"
namespace ts3::anim::m40 {
struct alignas(16) TransformController {
    Controller base;
    void* secondary28;
    alignas(16) Float4Value translation;
    Float4Value rotation;
    Controller* child;
};
TS3_X86_OFFSET(TransformController,translation,0x30);
TS3_X86_OFFSET(TransformController,rotation,0x40);
TS3_X86_OFFSET(TransformController,child,0x50);
struct Cursor {
    void* vtable; uintptr_t unknown4; void* stream; uintptr_t limit; uint32_t mode; uint8_t good;
};
TS3_X86_OFFSET(Cursor,good,0x14);
// TS3W.exe 0x0056C890: Delegate time to child slot 10, otherwise read the global fallback.
float Time(TransformController*);
// TS3W.exe 0x0056C8B0: Test for a nonidentity transform with SSE max/ordered tolerance comparisons; translation W is ignored.
bool Nonidentity(const TransformController*);
// TS3W.exe 0x0056C940: Delegate the child's slot 12 boolean, or return false.
bool Active(TransformController*);
// TS3W.exe 0x0056C960: Invoke child slot 1 if present (void return).
void Notify(TransformController*);
// TS3W.exe 0x0056C970: Serialize rotation, translation, then child, stopping at each failed cursor latch.
uint8_t Save(TransformController*,Cursor*);
// TS3W.exe 0x0056C9E0: Construct the base and identity transform, leaving alignment padding untouched.
TransformController* Construct(TransformController*,void*);
#if defined(_M_IX86)
namespace hooks {
inline float __fastcall Time(TransformController* p,void*) {return m40::Time(p);}
inline bool __fastcall Nonidentity(TransformController* p,void*) {return m40::Nonidentity(p);}
inline bool __fastcall Active(TransformController* p,void*) {return m40::Active(p);}
inline void __fastcall Notify(TransformController* p,void*) {m40::Notify(p);}
inline uint8_t __fastcall Save(TransformController* p,void*,Cursor* c) {return m40::Save(p,c);}
inline TransformController* __fastcall Construct(TransformController* p,void*,void* host) {return m40::Construct(p,host);}
}
#endif
}
