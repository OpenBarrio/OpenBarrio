#pragma once
#include "ts3/anim/m05_B01.h"
namespace ts3::anim::m02 {
using b01::Node;
using b01::Cursor;
struct Evaluator { Node base; Node* source; };
TS3_X86_OFFSET(Evaluator,source,0x24);
// Evaluation's context is not the RigPose layout: its retained rig is at +40.
struct Context { unsigned char opaque[0x40]; RigResource* rig; };
struct Pose {
    void** vtable; int32_t refs; unsigned char opaque08[0x2c];
    SrtValue* srt; void* auxiliary; float* weights;
};
TS3_X86_OFFSET(Pose,srt,0x34);
TS3_X86_OFFSET(Pose,weights,0x3c);
// TS3W.exe 0x0054C870: Serialize the referenced evaluator, returning success in AL.
bool SaveSource(Evaluator*,Cursor*);
// TS3W.exe 0x0054C890: Deserialize a referenced evaluator using this node's host.
bool LoadSource(Evaluator*,Cursor*);
// TS3W.exe 0x0054C9F0: Reset derived/base tables and optionally free; return this even after free.
Evaluator* DeleteBlend(Evaluator*,uint32_t);
// TS3W.exe 0x0054CA50: Evaluate first child and blend a temporary source pose with two rsqrt refinements.
uintptr_t Evaluate(Evaluator*,Context*,Pose*);
// TS3W.exe 0x0054CCE0: Construct the additive evaluator, preserving sibling links and trailing storage.
Evaluator* ConstructAdd(Evaluator*,void*);
// TS3W.exe 0x0054CD40: Reset additive/base tables and optionally free; return this even after free.
Evaluator* DeleteAdd(Evaluator*,uint32_t);
}
#if defined(_M_IX86)
namespace ts3::anim::m02::hooks {
bool __fastcall SaveSource(Evaluator*,void*,Cursor*);
bool __fastcall LoadSource(Evaluator*,void*,Cursor*);
Evaluator* __fastcall DeleteBlend(Evaluator*,void*,uint32_t);
uintptr_t __fastcall Evaluate(Evaluator*,void*,Context*,Pose*);
Evaluator* __fastcall ConstructAdd(Evaluator*,void*,void*);
Evaluator* __fastcall DeleteAdd(Evaluator*,void*,uint32_t);
}
#endif
