#pragma once
#include "ts3/layout.h"
namespace ts3::resource::m03::g07 {
struct Key { uint32_t words[4]; };
struct Payload { Key key; uintptr_t value, spare; };
struct Node { Node* next; Node* previous; Payload payload; };
struct List { Node* next; Node* previous; const char* name; };
struct Owner {
    uint8_t prefix[0x18];
    uint32_t primary[2], fallback[2];
    void* fields[8];
    List tables;
};
TS3_X86_OFFSET(Owner,tables,0x48);
TS3_X86_SIZE(Node,32);
// TS3W.exe 0x0059BDE0: rebuild locale table list, primary keys before deferred fallback keys; AL-only return.
bool Reload(Owner* self);
// TS3W.exe 0x0059BFC0: update a resource under the locale mutex and return the full unlock result.
uintptr_t ResourceChanged(uintptr_t ignored,const Key* key,Owner* self);
#if defined(_M_IX86)
bool __fastcall HookReload(Owner*,void*);
#endif
}
