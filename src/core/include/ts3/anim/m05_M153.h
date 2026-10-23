#pragma once
#include "ts3/layout.h"
#include <cstdint>
namespace ts3::anim::m153 {
struct Component {
    void* vtable; void* host; uint32_t opaque08[6];
    void* rig; void* state; void* look; uint32_t flags;
    uint32_t opaque30[2]; uint8_t dirty;
};
TS3_X86_OFFSET(Component, rig, 0x20);
TS3_X86_OFFSET(Component, flags, 0x2c);
TS3_X86_OFFSET(Component, dirty, 0x38);
// TS3W.exe 0x0093F740: Copies selected pose-state fields, preserving holes and returning this.
void* CopyPoseState(void* self, const void* source);
// TS3W.exe 0x0093F920: Builds and serializes the component evaluator graph, preserving EA flag and stream ordering.
bool SerializePose(Component* self, void* stream);
// TS3W.exe 0x009402C0: Activates and queries the host component unless bit 0x800 is set.
uintptr_t Activate(Component* self);
// TS3W.exe 0x00940310: Detaches the component and releases its optional pose state.
void Deactivate(Component* self);
}
namespace ts3::anim {
struct m153_hooks {
    uint32_t CopyPoseState(void* source) { return static_cast<uint32_t>(reinterpret_cast<uintptr_t>(m153::CopyPoseState(this,source))); }
    bool SerializePose(void* stream) { return m153::SerializePose(reinterpret_cast<m153::Component*>(this),stream); }
    uint32_t Activate() { return static_cast<uint32_t>(m153::Activate(reinterpret_cast<m153::Component*>(this))); }
    void Deactivate() { m153::Deactivate(reinterpret_cast<m153::Component*>(this)); }
};
}
