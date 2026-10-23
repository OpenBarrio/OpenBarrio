#pragma once
#include "ts3/layout.h"
namespace ts3::anim::m68 {
struct Link { Link* next; Link* previous; };
struct Resource { void** vtable; long refs; };
struct State { unsigned char opaque[0xb0]; uint32_t enabled; };
struct Node { void* vtable; Resource* retained; unsigned char opaque[16]; Link link; uint32_t reserved; State* state; };
struct Manager { Link link; uintptr_t opaque[2]; Node* cursor; uintptr_t reserved; void* begin; void* end; void* capacity; uintptr_t descriptor; uint32_t flags; };
struct Text { char16_t* begin; char16_t* end; char16_t* capacity; uintptr_t descriptor; };
struct NameOwner { unsigned char opaque[0x40]; uint64_t hash; Text* name; };
TS3_X86_OFFSET(Node,link,0x18); TS3_X86_OFFSET(Node,state,0x24);
TS3_X86_OFFSET(Manager,cursor,0x10); TS3_X86_OFFSET(Manager,flags,0x28);
TS3_X86_OFFSET(NameOwner,name,0x48);
// TS3W.exe 0x00571B90: Update linked animation instances, retaining resources across callbacks and substepping active nodes.
uintptr_t __fastcall Update(Manager*,void*,float);
// TS3W.exe 0x00572080: Copy a nonempty UTF16 missing-animation name and hash it; zero/one-byte spans are ignored.
void __fastcall SetName(NameOwner*,void*,Text*);
// TS3W.exe 0x00572130: Allocate a 0x40 animation factory resource and construct it, or return null.
void* __fastcall Create(void*,void*);
// TS3W.exe 0x00572160: Forward the four stack arguments to the resource loader, ignoring the incoming ECX.
uint8_t __stdcall Load(void*,void*,uint32_t,uint32_t);
// TS3W.exe 0x00572180: Return the animation factory resource class identifier.
uint32_t __fastcall Type(void*,void*);
// TS3W.exe 0x005721A0: Register clip, rig and track-mask factories in order, including null allocation results.
uintptr_t __cdecl RegisterFactories();
}
