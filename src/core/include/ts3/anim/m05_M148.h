#pragma once
#include "ts3/layout.h"
namespace ts3::anim::m148 {
// TS3W.exe 0x00858510: Construct the turn controller and register its three embedded components.
void* Construct(void* self, void* host, float speed);
// TS3W.exe 0x008585B0: Destroy the turn controller, optionally free it, and return the dangling this pointer.
void* Delete(void* self, uint32_t flags);
// TS3W.exe 0x00858970: Advance the turn controller and notify its host and slaves (void; EAX is incidental).
// Keeps low-byte predicates, two rsqrt refinements, and Y-delta mutation across slave iterations.
void Update(void* self, float dt);
// TS3W.exe 0x00859890: Detach vehicle components, free non-inline ranges, and destroy the base (void).
void DestructVehicle(void* self);
// TS3W.exe 0x00859950: Adjust the secondary interface by -4 and invoke the vehicle deleting destructor.
void* DeleteSecondary(void* self, uint32_t flags);
// TS3W.exe 0x00859970: Adjust the query interface by -0x18 before forwarding its type query.
void* QuerySecondary(void* self, uint32_t type);
namespace hooks {
#if defined(_M_IX86)
void* __fastcall Construct(void*,void*,void*,float);
void* __fastcall Delete(void*,void*,uint32_t);
void __fastcall Update(void*,void*,float);
void __fastcall DestructVehicle(void*,void*);
void* __fastcall DeleteSecondary(void*,void*,uint32_t);
void* __fastcall QuerySecondary(void*,void*,uint32_t);
#endif
}
}
