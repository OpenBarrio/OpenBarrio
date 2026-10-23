#pragma once
#include "ts3/layout.h"
namespace ts3::anim::m142 {
struct Body {
    void* vtable;
    void* secondary;
    unsigned char base[0x10];
    void* delegate;
    unsigned char state[0x104];
    unsigned char listener[0x24];
    void* controller;
    uint32_t flags;
    void* host;
};
TS3_X86_OFFSET(Body,delegate,0x18);
TS3_X86_OFFSET(Body,listener,0x120);
TS3_X86_OFFSET(Body,controller,0x144);
TS3_X86_OFFSET(Body,host,0x14c);
// TS3W.exe 0x00854A80: replace the time-scale controller, preserving notification and reference ordering.
uintptr_t SetController(Body* self, void* source, uint32_t event);
// TS3W.exe 0x00854BA0: construct the body and register its listener with the host.
Body* Construct(Body* self, void* baseArgument, void* host);
// TS3W.exe 0x00854C20: return the body interface identifier.
uint32_t TypeId(void* self);
// TS3W.exe 0x00854C30: return self for either of the two supported interface identifiers.
void* Query(void* self, uint32_t id);
// TS3W.exe 0x00854C60: adjust the secondary interface then forward the deleting destructor.
void* DeleteSecondary(void* self, uint32_t flags);
// TS3W.exe 0x00854C70: unregister, release controller, and destroy embedded states before the base.
uintptr_t Destruct(Body* self);
}
namespace ts3::anim::m142 {
#if defined(_M_IX86)
uintptr_t __fastcall HookSet(Body*,void*,void*,uint32_t);
Body* __fastcall HookConstruct(Body*,void*,void*,void*);
uint32_t __fastcall HookType(void*,void*);
void* __fastcall HookQuery(void*,void*,uint32_t);
void* __fastcall HookDelete(void*,void*,uint32_t);
uintptr_t __fastcall HookDestruct(Body*,void*);
#endif
}
