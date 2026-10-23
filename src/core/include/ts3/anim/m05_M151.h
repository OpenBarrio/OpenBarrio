#pragma once
#include "ts3/layout.h"
namespace ts3::anim::m151 {
struct alignas(16) Pose { float values[44]; };
struct alignas(16) State { Pose pose; void* manager; uint32_t reserved; float weight, rate; };
struct Identity { unsigned char opaque[8]; uint32_t low, high; };
struct Component { void* vtable; Identity* identity; unsigned char opaque[28]; State* state; uint32_t unknown, flags; };
struct Renderer { void* vtable; void* service; };
TS3_X86_OFFSET(State, manager, 0xb0);
TS3_X86_OFFSET(Component, state, 0x24);
TS3_X86_OFFSET(Component, flags, 0x2c);
// TS3W.exe 0x0093E170: Forward a controller lookup to the optional track manager.
uintptr_t Lookup(Component* self, uint32_t key);
// TS3W.exe 0x0093E190: Query the manager unless bit15 is set; preserve the shifted high EAX bits.
uint32_t Query(Component* self);
// TS3W.exe 0x0093E340: Refresh the identity, resolve its interface, then invoke slot zero.
uintptr_t Refresh(Component* self);
// TS3W.exe 0x0093E3B0: Initialize the pose vectors and scalar defaults, returning this.
Pose* InitPose(Pose* self);
// TS3W.exe 0x0093E550: Initialize pose state and allocate its optional track manager.
State* InitState(State* self, void* host);
// TS3W.exe 0x0093E990: Install the renderer interface and acquire the renderer service.
Renderer* InitRenderer(Renderer* self);
}

#if defined(_M_IX86)
namespace ts3::anim::m151 {
inline uintptr_t __fastcall HookLookup(Component* self, void*, uint32_t key) { return Lookup(self, key); }
inline uint32_t __fastcall HookQuery(Component* self, void*) { return Query(self); }
inline uintptr_t __fastcall HookRefresh(Component* self, void*) { return Refresh(self); }
inline Pose* __fastcall HookInitPose(Pose* self, void*) { return InitPose(self); }
inline State* __fastcall HookInitState(State* self, void*, void* host) { return InitState(self, host); }
inline Renderer* __fastcall HookInitRenderer(Renderer* self, void*) { return InitRenderer(self); }
}
#endif
