#pragma once
#include <cstddef>
#include <cstdint>
namespace ts3::anim::m154 {
struct alignas(16) Pose {
    float vectors[10][4];
    float weights[4];
    void* manager;
    uint32_t idleTicks;
    float motionScale;
};
struct Component {
    void* vtable;
    void* host;
    void* secondaryVtable;
    unsigned char opaque0c[0x14];
    void* context;
    Pose* pose;
    void* followup;
    uint32_t flags;
};
#if defined(_M_IX86)
static_assert(offsetof(Component,pose)==0x24);
static_assert(offsetof(Component,followup)==0x28);
static_assert(offsetof(Component,flags)==0x2c);
static_assert(offsetof(Pose,manager)==0xb0);
#endif
// TS3W.exe 0x00940430: perform the idle/reset subset; NaN and signed-zero time skip evaluation as in EA.
// Returns false WITHOUT changing state when active evaluation is needed. Not a replacement hook.
// On success, result is the original EAX (the current pose pointer, possibly null).
bool TryUpdateIdle(Component* self, float delta, Pose*& result);
// TS3W.exe 0x00940430: update tracks/root motion and consume transient inputs; keeps the +20 comparison, AL predicates, two rsqrt refinements and x87 rounding.
Pose* Update(Component* self,float delta);
#if defined(_M_IX86)
// TS3W.exe 0x00940430: x86 thiscall-compatible adapter for Update.
Pose* __fastcall HookUpdate(Component*,void*,float);
#endif
}
