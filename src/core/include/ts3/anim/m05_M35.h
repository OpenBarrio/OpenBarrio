#pragma once
#include "ts3/anim/m05_layouts.h"
namespace ts3::anim::m35 {
struct alignas(16) Rotation { float axis[4]; float unused, speed, period, tail; };
struct State {
    Controller base; float time;
    Rotation* begin; Rotation* end; Rotation* capacity; void* descriptor;
    Rotation* rates; Rotation* ratesEnd; Rotation* ratesCapacity; void* ratesDescriptor;
    void* extra; void* extraEnd; void* extraCapacity; void* extraDescriptor;
};
TS3_X86_OFFSET(State,time,0x28);
TS3_X86_OFFSET(State,begin,0x2c);
TS3_X86_OFFSET(State,rates,0x3c);
TS3_X86_OFFSET(State,extra,0x4c);
struct Event { State* sender; ResourcePrefix* resource; uint32_t a,b,c; uint8_t flags[3]; };
TS3_X86_SIZE(Rotation,0x20);
TS3_X86_SIZE(State,0x5c);
TS3_X86_SIZE(Event,0x18);
// TS3W.exe 0x0056AC60: Adjust the secondary interface back to the owner and perform deleting destruction.
State* SecondaryDelete(void*,uint32_t);
// TS3W.exe 0x0056AC70: Destroy the three owned ranges in reverse order and tail-call base destruction, preserving EAX.
uintptr_t Destroy(State*);
// TS3W.exe 0x0056ACF0: Destroy the base rotation range and optionally free the owner, returning the owner even after free.
State* BaseDelete(State*,uint32_t);
// TS3W.exe 0x0056AD40: Dispatch the initial-time event, advance positive time and release event/owner references.
uintptr_t Tick(State*,float);
// TS3W.exe 0x0056AE00: Destroy derived rotation ranges and optionally free the owner, returning the owner even after free.
State* Delete(State*,uint32_t);
// TS3W.exe 0x0056AE20: Tick and apply normalized incremental quaternion rotations with two rsqrt refinements; preserve partial AH.
uintptr_t Rotate(State*,float);
}
namespace ts3::anim {
struct m35_hooks {
    uint32_t SecondaryDelete(uint32_t f) { return uint32_t(reinterpret_cast<uintptr_t>(m35::SecondaryDelete(this,f))); }
    uint32_t Destroy() { return uint32_t(m35::Destroy(reinterpret_cast<m35::State*>(this))); }
    uint32_t BaseDelete(uint32_t f) { return uint32_t(reinterpret_cast<uintptr_t>(m35::BaseDelete(reinterpret_cast<m35::State*>(this),f))); }
    uint32_t Tick(float dt) { return uint32_t(m35::Tick(reinterpret_cast<m35::State*>(this),dt)); }
    uint32_t Delete(uint32_t f) { return uint32_t(reinterpret_cast<uintptr_t>(m35::Delete(reinterpret_cast<m35::State*>(this),f))); }
    uint32_t Rotate(float dt) { return uint32_t(m35::Rotate(reinterpret_cast<m35::State*>(this),dt)); }
};
}
