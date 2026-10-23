#pragma once
#include "ts3/layout.h"
namespace ts3::anim::m67 {
struct Link { Link* next; Link* previous; };
struct Node { unsigned char opaque[0x18]; Link link; };
struct Key { uint32_t low, high; };
struct Resource { void** vtable; uint32_t refs; Key key; };
struct Manager {
    Link active, pending;
    Node* current;
    Resource* resource;
    Key* begin;
    Key* end;
    unsigned char opaque[0x20];
    Key key;
};
TS3_X86_OFFSET(Node,link,0x18);
TS3_X86_OFFSET(Resource,key,8);
TS3_X86_OFFSET(Manager,current,0x10);
TS3_X86_OFFSET(Manager,resource,0x14);
TS3_X86_OFFSET(Manager,begin,0x18);
TS3_X86_OFFSET(Manager,key,0x40);
// TS3W.exe 0x005718C0: prepend a node to the active sentinel, returning its embedded link.
Link* InsertActive(Manager*,Node*);
// TS3W.exe 0x005718E0: prepend a node to the pending sentinel, returning its embedded link.
Link* InsertPending(Manager*,Node*);
// TS3W.exe 0x00571900: unlink a node and advance the current cursor; sentinel conversion is not special-cased.
Node* Remove(Manager*,Node*);
// TS3W.exe 0x005719A0: query the animation interface and invoke its reset method; null leaves EAX unspecified.
void ResetObject(void*);
// TS3W.exe 0x005719D0: reuse or replace the cached key-matched resource, releasing before loading.
Resource* GetResource(Manager*);
// TS3W.exe 0x00571A50: consume queued identifiers backwards, reset objects and remove them from the world.
uintptr_t Drain(Manager*);
}
#if defined(_M_IX86)
namespace ts3::anim::m67::hooks {
Link* __fastcall InsertActive(Manager*,void*,Node*);
Link* __fastcall InsertPending(Manager*,void*,Node*);
Node* __fastcall Remove(Manager*,void*,Node*);
void __stdcall ResetObject(void*);
Resource* __fastcall GetResource(Manager*,void*);
uintptr_t __fastcall Drain(Manager*,void*);
}
#endif
