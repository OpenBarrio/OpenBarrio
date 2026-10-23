#pragma once
#include "ts3/anim/m05_layouts.h"
#include <cstdint>
namespace ts3::anim::m13 {
// Only the pointer field consumed by the rejection prefix is established here.
struct RigView {
    unsigned char opaque[0x9c];
    void* bindWorld;
    unsigned char opaqueA0[0x1c];
    int32_t indices[11];
};
struct alignas(16) EvaluationView {
    unsigned char transform[0x40];
    RigView* rig;
    void* debugOwner;
    unsigned char tail[0x161-0x48];
};
TS3_X86_OFFSET(EvaluationView, rig, 0x40);
TS3_X86_OFFSET(RigView, indices, 0xbc);
// TS3W.exe 0x005561C0: Evaluate the rig constraint blend, preserving SSE lane order and missing-index interior-pointer returns.
int32_t* __stdcall Evaluate(EvaluationView*, void* pose, const void* target, float weight);
}
