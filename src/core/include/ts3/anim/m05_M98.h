#pragma once
#include "ts3/layout.h"
namespace ts3::anim::m98_impl {
struct Event { uint32_t unknown; void* resource; uint32_t type; uint32_t data[14]; };
struct State { void* controller; void* manager; void* unknown; void* name; uint8_t opaque[5]; uint8_t remap; };
struct Context { State* state; };
struct Link { Link* next; Link* previous; };
struct Node { Link link; void* controller; uint32_t key; };
struct Groups { void* vtable; Link list; const char* descriptor; void* unknown; void* host; };
TS3_X86_OFFSET(Event,type,8); TS3_X86_OFFSET(State,remap,0x15);
TS3_X86_OFFSET(Groups,host,0x14); TS3_X86_OFFSET(Node,key,0xc);
TS3_X86_OFFSET(Groups,list,4); TS3_X86_OFFSET(Groups,descriptor,0xc);
TS3_X86_OFFSET(Node,controller,8); TS3_X86_OFFSET(Event,data,0xc);
TS3_X86_OFFSET(State,manager,4); TS3_X86_OFFSET(State,name,0xc);
// TS3W.exe 0x005834D0: create an effect sound and destroy temporary vectors in reverse order.
uintptr_t CreateEffect(Event* event);
// TS3W.exe 0x005835C0: handle effect start/stop events, retaining the scene entity during callbacks.
uintptr_t HandleEffect(Context* context, Event* event, float start, float end);
// TS3W.exe 0x005839E0: find or append a sequence-controller slot; returns the interior retained-pointer address.
void** FindSequence(Groups* self, uint32_t key);
// TS3W.exe 0x00583AB0: dispatch events using controller time bounds; resource name lookup is deliberately repeated.
uintptr_t Dispatch(void* receiver, Context* context, Event* event);
// TS3W.exe 0x00583B68: interior dispatch block of 00583AB0, forwards the rounded time bounds to 00582930.
uintptr_t DispatchTrack(void* receiver, Context* context, Event* event, float start, float end);
// TS3W.exe 0x00583B86: interior dispatch block of 00583AB0, forwards the rounded time bounds to 00583050.
uintptr_t DispatchStop(void* receiver, Context* context, Event* event, float start, float end);
}

namespace ts3::anim {
struct m98 {
 // TS3W.exe 0x005835C0: stdcall registration facade for effect handling.
 static uintptr_t HandleEffect(void* context,void* event,float start,float end) {
  return m98_impl::HandleEffect(static_cast<m98_impl::Context*>(context),static_cast<m98_impl::Event*>(event),start,end);
 }
 // TS3W.exe 0x005839E0: thiscall registration facade for sequence slots.
 void* FindSequence(uint32_t key) { return m98_impl::FindSequence(reinterpret_cast<m98_impl::Groups*>(this),key); }
 // TS3W.exe 0x00583AB0: thiscall registration facade for event dispatch.
 uintptr_t Dispatch(void* context,void* event) { return m98_impl::Dispatch(this,static_cast<m98_impl::Context*>(context),static_cast<m98_impl::Event*>(event)); }
};
}
