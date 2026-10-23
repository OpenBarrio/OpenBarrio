#pragma once
#include "ts3/anim/m05_B01.h"
#include "ts3/anim/m05_B20.h"
#include <xmmintrin.h>

namespace ts3::anim::m21 {
using Node = b01::Node;
using Pose = b20::Pose;
struct Evaluator {
    Node node;
    unsigned char opaque24[12];
    uint32_t f_30, f_34, f_38, f_3C;
    uint8_t f_40, f_41, f_42, f_43;
};
TS3_X86_OFFSET(Evaluator, f_30, 0x30);
TS3_X86_OFFSET(Evaluator, f_40, 0x40);
struct alignas(16) Context {
    __m128 matrix[4];
    RigResource* rig;
    void* opaque44;
    void* scene;
    uint32_t opaque4c;
    float deltaTime;
    uint8_t flags;
    unsigned char opaque55[0x5b];
    __m128 smoothed;
    unsigned char opaqueC0[0x70];
    __m128 angle, combinedAngle;
    unsigned char opaque150[0x10];
    uint8_t dirty;
};
TS3_X86_OFFSET(Context, flags, 0x54);
TS3_X86_OFFSET(Context, smoothed, 0xb0);
TS3_X86_OFFSET(Context, angle, 0x130);
TS3_X86_OFFSET(Context, combinedAngle, 0x140);
TS3_X86_OFFSET(Context, dirty, 0x160);
// TS3W.exe 0x005625B0: Evaluate child and terrain-dependent local pose; rejected terrain paths preserve flags and return residual EAX.
uintptr_t Evaluate(Node*, Context*, Pose*);
#if defined(_M_IX86)
// TS3W.exe 0x005625B0: ABI adapter for the evaluator thiscall entry.
uintptr_t __fastcall HookEvaluate(Node*, void*, Context*, Pose*);
#endif
}
