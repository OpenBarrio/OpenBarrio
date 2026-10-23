#pragma once
#include "ts3/anim/m05_B01.h"
namespace ts3::anim::m22 {
struct Evaluator {
    b01::Node base; uint32_t opaque24;
    uint32_t keyLow,keyHigh; float amount; uint32_t selector;
    uint32_t extraLow,extraHigh;
};
struct Context {
    unsigned char opaque00[0x40]; void* rig;
    unsigned char opaque44[0x10]; uint8_t flags;
    unsigned char opaque55[0x7b]; float phase;
    unsigned char opaqueD4[0x7c]; float savedPhase;
    unsigned char opaque154[12]; uint8_t updated;
};
TS3_X86_OFFSET(Evaluator,keyLow,0x28);
TS3_X86_OFFSET(Evaluator,amount,0x30);
TS3_X86_OFFSET(Context,phase,0xd0);
TS3_X86_OFFSET(Context,updated,0x160);
// TS3W.exe 0x005650C0: Initialize the oscillating scale evaluator, preserving opaque bytes.
Evaluator* ConstructScale(Evaluator*,void*);
// TS3W.exe 0x005651F0: Reset base vtable and free iff bit zero is set, returning this after free.
Evaluator* DestroyScale(Evaluator*,uint32_t);
// TS3W.exe 0x00565220: Evaluate first child then apply oscillating scale to a strictly positive joint index.
uintptr_t EvaluateScale(Evaluator*,Context*,void*);
// TS3W.exe 0x005653C0: Initialize the paired-key evaluator without touching sibling links.
Evaluator* ConstructPair(Evaluator*,void*);
// TS3W.exe 0x00565400: Query paired-key or base interface identity.
void* QueryPair(void*,uint32_t);
// TS3W.exe 0x00565430: Serialize four fields in EA order, preserving negative cursor and EAX behavior.
uintptr_t SavePair(Evaluator*,b01::Cursor*);
}
#if defined(_M_IX86)
namespace ts3::anim::m22::hooks {
Evaluator* __fastcall ConstructScale(Evaluator*,void*,void*);
Evaluator* __fastcall DestroyScale(Evaluator*,void*,uint32_t);
uintptr_t __fastcall EvaluateScale(Evaluator*,void*,Context*,void*);
Evaluator* __fastcall ConstructPair(Evaluator*,void*,void*);
void* __fastcall QueryPair(void*,void*,uint32_t);
uintptr_t __fastcall SavePair(Evaluator*,void*,b01::Cursor*);
}
#endif
