#pragma once
#include "ts3/anim/m05_B01.h"
namespace ts3::anim::m20 {
struct Evaluator {
    b01::Node base;
    uint32_t preserved24;
    uint32_t values[6]; // Serialized scalar words at +28..+3c, not pointers.
    uint8_t flags[4];
};
TS3_X86_OFFSET(Evaluator,values,0x28);
TS3_X86_OFFSET(Evaluator,flags,0x40);
TS3_X86_SIZE(Evaluator,0x44);
// TS3W.exe 0x00561B70: Initialize the evaluator, preserving sibling links and the +24 gap.
Evaluator* Construct(Evaluator*,void* host);
// TS3W.exe 0x005621A0: Reset the base vtable and optionally free on bit zero, returning this even after free.
Evaluator* Destroy(Evaluator*,uint32_t flags);
}
#if defined(_M_IX86)
namespace ts3::anim::m20::hooks {
Evaluator* __fastcall Construct(Evaluator*,void*,void*);
Evaluator* __fastcall Destroy(Evaluator*,void*,uint32_t);
}
#endif
