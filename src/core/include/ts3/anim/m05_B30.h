#pragma once
#include "ts3/layout.h"
namespace ts3::anim::b30 {
struct State { void** vtable; uint32_t opaque[2]; void* payload; };
struct Link { Link* next; Link* previous; };
struct Entity { void** vtable; void** secondary; uint32_t refs; Link links; void* payload; };
TS3_X86_OFFSET(State,payload,0xc);
TS3_X86_OFFSET(Entity,secondary,4);
TS3_X86_OFFSET(Entity,refs,8);
TS3_X86_OFFSET(Entity,links,0xc);
TS3_X86_OFFSET(Entity,payload,0x14);
// TS3W.exe 0x008396B0: Restore the base state vtable without initializing other bytes.
void InitState(State*);
// TS3W.exe 0x008396C0: Return the state type identifier.
uint32_t StateType(State*);
// TS3W.exe 0x008396D0: Return this only for the exact state type identifier.
State* QueryState(State*,uint32_t);
// TS3W.exe 0x008396F0: Initialize the state vtable and borrowed payload, returning this.
State* ConstructState(State*,void*);
// TS3W.exe 0x00839710: Restore the state vtable and optionally free, returning the original pointer.
State* DeleteState(State*,uint32_t);
// TS3W.exe 0x00839730: Tail-forward the float result from virtual slot five.
float ForwardTime(Entity*);
// TS3W.exe 0x00839740: Return the original constant 5.0f (not the clone's minus one).
float DefaultTime(Entity*);
// TS3W.exe 0x00839770: Return the controller type identifier.
uint32_t ControllerType(Entity*);
// TS3W.exe 0x00839780: Return this only for the exact controller type identifier.
Entity* QueryController(Entity*,uint32_t);
// TS3W.exe 0x008397B0: Release the wrapped entity secondary interface, restoring refs to one before deletion; preserve EAX.
uintptr_t Release(Entity**);
// TS3W.exe 0x00839970: Reset entity tables and list, returning the embedded sentinel (not this).
Link* ResetEntity(Entity*);
// TS3W.exe 0x008399A0: Return the entity type identifier.
uint32_t EntityType(Entity*);
// TS3W.exe 0x008399B0: Return this only for the exact entity type identifier.
Entity* QueryEntity(Entity*,uint32_t);
// TS3W.exe 0x008399D0: Adjust the secondary-interface pointer to its owner and invoke deletion.
Entity* DeleteSecondary(void*,uint32_t);
// TS3W.exe 0x00839A70: Reset entity state and optionally free, returning this even after free.
Entity* DeleteEntity(Entity*,uint32_t);
}

#if defined(_M_IX86)
namespace ts3::anim::b30::hooks {
inline void __fastcall InitState(State* self, void*) { b30::InitState(self); }
inline uint32_t __fastcall StateType(State* self, void*) { return b30::StateType(self); }
inline State* __fastcall QueryState(State* self, void*, uint32_t key) { return b30::QueryState(self,key); }
inline State* __fastcall ConstructState(State* self, void*, void* payload) { return b30::ConstructState(self,payload); }
inline State* __fastcall DeleteState(State* self, void*, uint32_t flags) { return b30::DeleteState(self,flags); }
inline float __fastcall ForwardTime(Entity* self, void*) { return b30::ForwardTime(self); }
inline float __fastcall DefaultTime(Entity* self, void*) { return b30::DefaultTime(self); }
inline uint32_t __fastcall ControllerType(Entity* self, void*) { return b30::ControllerType(self); }
inline Entity* __fastcall QueryController(Entity* self, void*, uint32_t key) { return b30::QueryController(self,key); }
inline Link* __fastcall ResetEntity(Entity* self, void*) { return b30::ResetEntity(self); }
inline uint32_t __fastcall EntityType(Entity* self, void*) { return b30::EntityType(self); }
inline Entity* __fastcall QueryEntity(Entity* self, void*, uint32_t key) { return b30::QueryEntity(self,key); }
inline Entity* __fastcall DeleteEntity(Entity* self, void*, uint32_t flags) { return b30::DeleteEntity(self,flags); }
}
#endif
