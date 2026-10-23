#pragma once
#include "ts3/anim/m05_B01.h"
namespace ts3::anim::m14 {
struct Evaluator {
    b01::Node base;
    unsigned char opaque24[12];
    Float4Value offset;
    uint32_t key[2];
    int32_t targetIndex;
};
struct BoundTarget {
    void** vtable;
    unsigned char opaque04[0x4c];
    Float4Value minimum,maximum;
};
TS3_X86_OFFSET(BoundTarget,minimum,0x50);
struct alignas(16) Context {
    Matrix44Value matrix;
    RigResource* rig;
    void* renderer;
    void* registry;
    uint32_t opaque4c;
    float delta;
    unsigned char opaque54[12];
    Float4Value target;
    float weight;
    uint32_t opaque74;
    float yaw,pitch,roll;
    unsigned char opaque84[0x5c];
    Float4Value evaluatedTarget;
    float evaluatedWeight;
    float attenuation,evaluatedYaw,evaluatedPitch,evaluatedRoll;
    unsigned char opaque104[0x5c];
    uint8_t dirty;
};
TS3_X86_OFFSET(Evaluator,key,0x40);
TS3_X86_OFFSET(Context,delta,0x50);
TS3_X86_OFFSET(Context,target,0x60);
TS3_X86_OFFSET(Context,evaluatedTarget,0xe0);
TS3_X86_OFFSET(Context,evaluatedWeight,0xf0);
TS3_X86_OFFSET(Context,dirty,0x160);
// TS3W.exe 0x00558FD0: evaluate bound/unbound targets, solve complete limb constraints, and bridge neck evaluation. Sign-bit clamps and first-child-only dispatch are kept.
uintptr_t EvaluateUnbound(Evaluator*,Context*,void* pose);
}

#if defined(_M_IX86)
namespace ts3::anim::m14::hooks {
uintptr_t __fastcall Evaluate(Evaluator*,void*,Context*,void*);
}
#endif
