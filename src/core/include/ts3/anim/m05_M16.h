#pragma once
#include "ts3/anim/m05_B01.h"
#include "ts3/anim/m05_B20.h"
namespace ts3::anim::m16 {
using Pose=b20::Pose;
struct Context { unsigned char opaque[0x40]; RigResource* rig; };
struct Buffer { void** vtable; int32_t refs; unsigned char opaque[8]; SrtValue srt[300]; void* vertexWeights; };
struct Evaluator : b01::Node { Buffer* buffer; };
TS3_X86_OFFSET(Context,rig,0x40);
TS3_X86_OFFSET(Buffer,srt,0x10);
TS3_X86_OFFSET(Buffer,vertexWeights,0x3850);
TS3_X86_OFFSET(Evaluator,buffer,0x24);
// TS3W.exe 0x0055C0D0: Evaluate the five-neck look-at chain, preserving sign-bit clamps and global smoothing.
uintptr_t EvaluateChain(b01::Node*,Context*,Pose*);
// TS3W.exe 0x0055C8E0: Evaluate head and eyes, falling back to the five-neck chain (Neck0 tests nonzero, not nonnegative).
uintptr_t Evaluate(b01::Node*,Context*,Pose*);
// TS3W.exe 0x0055CF10: Copy the local buffer into a nonnull destination pose and forward vertex weights.
void Copy(Buffer*,Pose*);
// TS3W.exe 0x0055CF50: Reset the destination before copying the evaluator's retained local buffer.
void EvaluateBuffer(Evaluator*,Context*,Pose*);
// TS3W.exe 0x0055CFD0: Construct a buffer evaluator, leaving sibling links untouched.
Evaluator* Construct(Evaluator*,void*);
// TS3W.exe 0x0055D070: Release the buffer nonatomically and optionally free the evaluator, returning its address.
Evaluator* Destroy(Evaluator*,uint32_t);
#if defined(_M_IX86)
namespace hooks {
uintptr_t __fastcall EvaluateChain(b01::Node*,void*,Context*,Pose*);
uintptr_t __fastcall Evaluate(b01::Node*,void*,Context*,Pose*);
void __fastcall Copy(Buffer*,void*,Pose*);
void __fastcall EvaluateBuffer(Evaluator*,void*,Context*,Pose*);
Evaluator* __fastcall Construct(Evaluator*,void*,void*);
Evaluator* __fastcall Destroy(Evaluator*,void*,uint32_t);
}
#endif
}
