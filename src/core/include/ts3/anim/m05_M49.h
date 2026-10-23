#pragma once
#include "ts3/anim/m05_layouts.h"
namespace ts3::anim::m49 {
struct State { Controller base; void* callback; Controller* target; uint32_t key; float weight; };
struct Cursor { void* vtable; uint32_t value4; void* stream; uint32_t valueC; void* context; uint8_t good; };
struct Event { Controller* owner; ResourcePrefix* resource; void* host; uint32_t a,b; uint8_t x,y,z; };
// TS3W.exe 0x0056F910: Read a target and its callback state, rejecting versions above one.
bool Load(State*, Cursor*);
// TS3W.exe 0x0056F9D0: Detach callback, release target, and destroy controller base.
void Destroy(State*);
// TS3W.exe 0x0056FA30: Dispatch a retained event through the containing controller.
void Dispatch(void*, const Event*, uint32_t);
// TS3W.exe 0x0056FAF0: Destroy and optionally free, returning the former object address.
State* Delete(State*, uint32_t);
// TS3W.exe 0x0056FB50: Write version one and the target reference.
bool Save(State*, Cursor*);
// TS3W.exe 0x0056FBA0: Construct a controller with an empty retained target.
State* Construct(State*, void*);
}
#if defined(_M_IX86)
namespace ts3::anim::m49 {
bool __fastcall HookLoad(State*,void*,Cursor*);
void __fastcall HookDestroy(State*,void*);
void __fastcall HookDispatch(void*,void*,const Event*,uint32_t);
State* __fastcall HookDelete(State*,void*,uint32_t);
bool __fastcall HookSave(State*,void*,Cursor*);
State* __fastcall HookConstruct(State*,void*,void*);
}
#endif
