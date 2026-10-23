#pragma once
#include "ts3/layout.h"
namespace ts3::anim::m107 {
struct Ref { void** vtable; long references; };
struct String { uint16_t *begin,*end,*capacity; void* descriptor; };
struct Style { Ref* resources[20]; uint32_t values[5]; uint8_t flags[6],padding[2]; String text; };
struct Entity { void** vtable; void** secondary; uint32_t references; struct Link { Link* next; Link* previous; } links; void* owner; };
struct Definition { uint8_t opaque[0x50]; uint32_t flags; uint32_t unknown54; void* source; uint8_t unknown5c[12]; float weight; uint8_t unknown6c[12]; void* selector; };
struct Request { uint8_t opaque[8]; void* actor; };
struct Context { uint8_t opaque[0xd4]; uint8_t changed; };
TS3_X86_OFFSET(Style,text,0x6c);
TS3_X86_OFFSET(Entity,owner,0x14);
TS3_X86_OFFSET(Definition,selector,0x78);
// TS3W.exe 0x00754060: Destroy string storage then release all twenty resources in reverse order, preserving residual EAX.
uintptr_t ReleaseStyle(Style*);
// TS3W.exe 0x00757690: Copy-construct style with atomic resource retains and a deep string copy.
Style* CopyStyle(Style*,const Style*);
// TS3W.exe 0x007F6D90: Create and route a null controller according to selector and context state; only low AL predicates count.
uintptr_t NullController(Definition*,Request*,Context*,uintptr_t,uintptr_t);
// TS3W.exe 0x00839750: Reset base tables and owner without initializing links or reference count; EAX is untouched.
void ResetEntity(Entity*);
// TS3W.exe 0x008397A0: Adjust secondary this to the parent, destroy and optionally free it, returning its address.
Entity* DestroySecondary(void*,uint32_t);
// TS3W.exe 0x008397E0: Initialize entity tables, references, circular sentinel and borrowed owner.
Entity* ConstructEntity(Entity*,void*);
}

#if defined(_M_IX86)
namespace ts3::anim::m107 {
// TS3W.exe 0x00754060: x86 ABI adapter.
uintptr_t __fastcall HookReleaseStyle(Style* s, void*);
// TS3W.exe 0x00757690: x86 ABI adapter.
Style* __fastcall HookCopyStyle(Style* s, void*, const Style* o);
// TS3W.exe 0x007F6D90: x86 ABI adapter.
uintptr_t __fastcall HookNullController(Definition* s, void*, Request* r, Context* c, uintptr_t a, uintptr_t b);
// TS3W.exe 0x00839750: x86 ABI adapter.
void __fastcall HookResetEntity(Entity* s, void*);
// TS3W.exe 0x008397A0: x86 ABI adapter.
Entity* __fastcall HookDestroySecondary(void* s, void*, uint32_t f);
// TS3W.exe 0x008397E0: x86 ABI adapter.
Entity* __fastcall HookConstructEntity(Entity* s, void*, void* o);
}
#endif
