#pragma once
#include "ts3/anim/m05_B09.h"
namespace ts3::anim::m50 {
struct Object { Controller base; void* interface28; Controller* child; };
struct Forwarder { Controller base; Controller* child; };
TS3_X86_OFFSET(Object,child,0x2c);
TS3_X86_OFFSET(Forwarder,child,0x28);
// TS3W.exe 0x0056FC10: Adjust the secondary interface by -0xC and delete the owner.
Object* DeleteInterface(void* self,uint32_t flags);
// TS3W.exe 0x0056FC20: Construct a controller and retain its optional child.
Object* Construct(Object* self,void* host,Controller* child);
// TS3W.exe 0x0056FCB0: Load a versioned optional child, returning the reader's status byte.
uint8_t Load(Object* self,b09impl::Reader* reader);
// TS3W.exe 0x0056FD20: Release the child, destroy the base and optionally free, returning this.
Object* Delete(Object* self,uint32_t flags);
// TS3W.exe 0x0056FD80: Forward slot12 or return false; only AL is defined.
uint8_t Query(Forwarder* self);
// TS3W.exe 0x0056FDA0: Forward reset slot1 when a child exists; EAX is undefined.
void Reset(Forwarder* self);
#if defined(_M_IX86)
namespace hooks {
Object* __fastcall DeleteInterface(void*,void*,uint32_t);
Object* __fastcall Construct(Object*,void*,void*,Controller*);
uint8_t __fastcall Load(Object*,void*,b09impl::Reader*);
Object* __fastcall Delete(Object*,void*,uint32_t);
uint8_t __fastcall Query(Forwarder*,void*);
void __fastcall Reset(Forwarder*,void*);
}
#endif
}
