#pragma once
#include "ts3/layout.h"
namespace ts3::anim::m150impl {
struct Progression {
    void* vtable;
    unsigned char untouched[8];
    float lower[2],upper[2];
    uint32_t mode;
    uint8_t flags[2];
};
struct Host { unsigned char opaque[0xb0]; void* scene; };
struct Component { unsigned char opaque[0x24]; Host* host; };
TS3_X86_OFFSET(Progression,lower,0xc);
TS3_X86_OFFSET(Progression,flags,0x20);
TS3_X86_OFFSET(Component,host,0x24);
TS3_X86_OFFSET(Host,scene,0xb0);
// TS3W.exe 0x0085A900: Initialize progression bounds, leaving the opaque bytes untouched.
Progression* Construct(Progression* self);
// TS3W.exe 0x0093DF70: Search the attached scene for a controller identifier.
uintptr_t Find(Component* self,uint32_t key);
// TS3W.exe 0x0093DF90: Search a scene track's controller for a nested identifier.
uintptr_t FindNested(Component* self,uint32_t track,uint32_t key);
// TS3W.exe 0x0093DFD0: Query a track, defaulting to one without a scene; keep full forwarded EAX.
uintptr_t Query(Component* self,uint32_t track);
// TS3W.exe 0x0093DFF0: Forward the track command, returning zero without a scene.
uintptr_t Command(Component* self,uint32_t track,uint32_t value);
// TS3W.exe 0x0093E100: Return a track controller's virtual floating result, or zero.
double Value(Component* self,uint32_t track);
}

namespace ts3::anim {
// ABI-only member adapters used by the generated thiscall replacement wrappers.
struct m150 {
    m150impl::Progression* Construct() { return m150impl::Construct(reinterpret_cast<m150impl::Progression*>(this)); }
    uintptr_t Find(uint32_t key) { return m150impl::Find(Self(),key); }
    uintptr_t FindNested(uint32_t track,uint32_t key) { return m150impl::FindNested(Self(),track,key); }
    uintptr_t Query(uint32_t track) { return m150impl::Query(Self(),track); }
    uintptr_t Command(uint32_t track,uint32_t value) { return m150impl::Command(Self(),track,value); }
    double Value(uint32_t track) { return m150impl::Value(Self(),track); }
private:
    m150impl::Component* Self() { return reinterpret_cast<m150impl::Component*>(this); }
};
}
