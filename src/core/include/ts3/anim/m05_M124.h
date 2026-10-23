#pragma once
#include "ts3/layout.h"
namespace ts3::anim::m124 {
struct Style {
    void* table;
    uint32_t opaque[2];
    const char* name;
};
TS3_X86_OFFSET(Style,name,0x0c);
// TS3W.exe 0x008467C0: register two embedded interfaces, then return the refresh callback's full EAX.
uint32_t Register(void* self);
// TS3W.exe 0x00846820: construct the named base and install the style table.
Style* Construct(Style* self,const char* name);
// TS3W.exe 0x00846840: return the style class identifier.
uint32_t ClassId(Style* self);
// TS3W.exe 0x00846850: return this for either supported interface ID, otherwise null.
void* Query(Style* self,uint32_t id);
// TS3W.exe 0x00846880: restore the style table and tail-call the base destructor, preserving incoming EAX.
#if defined(_M_IX86)
uint32_t __fastcall Destroy(Style* self,void* unused=nullptr);
#else
void Destroy(Style* self);
#endif
// TS3W.exe 0x00846890: destroy, optionally free on flag bit zero, and return this even after free.
Style* Delete(Style* self,uint32_t flags);

#if defined(_M_IX86)
inline uint32_t __fastcall HookRegister(void* self, void*) { return Register(self); }
inline Style* __fastcall HookConstruct(Style* self, void*, const char* name) { return Construct(self,name); }
inline uint32_t __fastcall HookClassId(Style* self, void*) { return ClassId(self); }
inline void* __fastcall HookQuery(Style* self, void*, uint32_t id) { return Query(self,id); }
inline Style* __fastcall HookDelete(Style* self, void*, uint32_t flags) { return Delete(self,flags); }
#endif
}
