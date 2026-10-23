#pragma once
#include "ts3/anim/m05_layouts.h"
namespace ts3::anim::m27detail {
// Typed views of previously opaque controller fields; pointers widen on x64.
struct Slot { Controller* controller; uint32_t value; uint32_t flags; float time; float weight; };
struct Sequence {
    Controller base; void* secondary; Controller* transition; ListLink nodes; void* descriptor;
    Slot first, second; uint32_t serial, mode, flags;
};
TS3_X86_OFFSET(Sequence,first,0x3c);
TS3_X86_OFFSET(Sequence,second,0x50);
TS3_X86_OFFSET(Sequence,serial,0x64);
TS3_X86_SIZE(Sequence,0x70);
struct ResourceIdentity { void* vtable; int32_t refs; uint32_t key[4]; };
TS3_X86_OFFSET(ResourceIdentity,key,8);
struct Clip {
    Controller base; void* begin; void* end; void* capacity; uint32_t opaque34;
    unsigned char storage[0x4b8]; ClipIkInfo ik;
    ResourceIdentity* resource; ResourceIdentity* resource2; void* extra;
    float current, previous, requested, period; uint32_t mode, cycles; int32_t index;
    uint8_t flags[7]; uint8_t tail;
};
TS3_X86_OFFSET(Clip,current,0x5dc);
TS3_X86_OFFSET(Clip,resource,0x5d0);
TS3_X86_OFFSET(Clip,cycles,0x5f0);
TS3_X86_SIZE(Clip,0x600);
struct Archive { void* vtable; uint32_t value4; void* stream; uint32_t valueC; uint32_t context; uint8_t ok; };
// TS3W.exe 0x00567A50: advance the sequence controller and its transition.
uint32_t UpdateSequence(Sequence*, float);
// TS3W.exe 0x00567EF0: read sequence state, preserving the partial AL return.
uint32_t ReadSequence(Sequence*, Archive*);
// TS3W.exe 0x00568080: destroy a sequence and optionally free it; returns the freed pointer.
Sequence* DeleteSequence(Sequence*, uint32_t);
// TS3W.exe 0x00568210: set clip time using repeated subtraction and a capped cycle count.
uint32_t SetTime(Clip*, float);
// TS3W.exe 0x005682E0: forward to the clip time setter (no addition), quieting signaling NaNs through x87.
uint32_t SetTimeWrapper(Clip*, float);
// TS3W.exe 0x005683D0: serialize clip state, stopping at the first archive error.
uint32_t WriteClip(Clip*, Archive*);
}

namespace ts3::anim {
// Thiscall entry adapters; state remains in the typed native views above.
struct m27 {
    uint32_t UpdateSequence(float t) { return m27detail::UpdateSequence(reinterpret_cast<m27detail::Sequence*>(this),t); }
    uint32_t ReadSequence(void* a) { return m27detail::ReadSequence(reinterpret_cast<m27detail::Sequence*>(this),static_cast<m27detail::Archive*>(a)); }
    uint32_t SetTime(float t) { return m27detail::SetTime(reinterpret_cast<m27detail::Clip*>(this),t); }
    uint32_t SetTimeWrapper(float t) { return m27detail::SetTimeWrapper(reinterpret_cast<m27detail::Clip*>(this),t); }
    uint32_t WriteClip(void* a) { return m27detail::WriteClip(reinterpret_cast<m27detail::Clip*>(this),static_cast<m27detail::Archive*>(a)); }
};
}
