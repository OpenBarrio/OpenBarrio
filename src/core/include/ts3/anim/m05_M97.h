#pragma once
#include "ts3/layout.h"
namespace ts3::anim::m97 {
struct Node { Node* next; Node* previous; void* controller; };
struct Group { void* vtable; Node sentinel; uint8_t active; uint8_t secondary; uint8_t pad[2]; void* fallback; };
TS3_X86_OFFSET(Group, active, 0x10);
TS3_X86_OFFSET(Group, fallback, 0x14);
struct Scene { void* vtable; void* manager; void* primary; void* fallback; uint8_t opaque[5]; uint8_t secondary; };
TS3_X86_OFFSET(Scene, secondary, 0x15);
struct Context { Scene* group; };
// TS3W.exe 0x005827B0: detach each nonnull controller in list order; empty path has no defined EAX.
uintptr_t Detach(Group*, uintptr_t incomingEax=0);
// TS3W.exe 0x00582820: latch activation before detaching; already-active path has no defined EAX.
uintptr_t Activate(Group*, uintptr_t incomingEax=0);
// TS3W.exe 0x005828B0: return the first nonnull controller lookup result.
void* Find(Group*, uint32_t);
// TS3W.exe 0x00582930: process a parent/unparent event through the scene interfaces.
uintptr_t ParentEvent(Context*, void*, uintptr_t, uintptr_t);
// TS3W.exe 0x00583050: resolve object and surface parameters and apply the event payload.
uintptr_t SurfaceEvent(Group*, Context*, void*, uintptr_t, uintptr_t);
// TS3W.exe 0x00583120: process visibility events, retaining the strict threshold and NaN behavior.
void VisibilityEvent(Context*, void*, uintptr_t, uintptr_t);
}
#if defined(_M_IX86)
namespace ts3::anim::m97::hooks {
uintptr_t __fastcall Detach(Group*,void*);
uintptr_t __fastcall Activate(Group*,void*);
inline void* __fastcall Find(Group* g,void*,uint32_t k) {return m97::Find(g,k);}
inline uintptr_t __stdcall ParentEvent(Context* c,void* e,uintptr_t a,uintptr_t b) {return m97::ParentEvent(c,e,a,b);}
inline uintptr_t __fastcall SurfaceEvent(Group* g,void*,Context* c,void* e,uintptr_t a,uintptr_t b) {return m97::SurfaceEvent(g,c,e,a,b);}
inline void __stdcall VisibilityEvent(Context* c,void* e,uintptr_t a,uintptr_t b) {m97::VisibilityEvent(c,e,a,b);}
}
#endif
