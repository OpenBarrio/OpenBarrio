#pragma once
#include "ts3/anim/m05_B03.h"
#include "ts3/anim/m05_B01.h"
#include "ts3/anim/m05_B23.h"
#include "ts3/anim/m05_layouts.h"
namespace ts3::anim {
struct M05Payload { alignas(16) unsigned char blocks[32]; };
struct M05Context {
    unsigned char opaque[0x40]; RigResource* rig; void* actor; void* resolver;
    void* deltas; float weight; uint8_t flags;
};
struct M05SamplePose {
    alignas(16) unsigned char opaque[0x10]; float root[8];
    RigResource* rig; SrtValue* srt; SrtValue* world; float* weights; void* auxiliary; void* workspace; float weight;
};
using M05ClipResource=b23::Resource;
TS3_X86_OFFSET(M05Context,flags,0x54);
TS3_X86_OFFSET(M05SamplePose,weight,0x48);
TS3_X86_OFFSET(M05ClipResource,view,0x4c);
using M05Node=b01::Node;
// TS3W.exe 0x0054E480: Copy two aligned blocks in order and return the input (including overlapping inputs).
const void* __fastcall PayloadE480(void*,void*,const void*);
// TS3W.exe 0x0054E4A0: Release second resource before first, reset base and optionally free; return this after free.
B03Evaluator* __fastcall DeleteE4A0(B03Evaluator*,void*,uint32_t);
// TS3W.exe 0x0054E4F0: Sample a clip and its root, or write an identity root; null resources leave the pose alone.
void __fastcall SampleE4F0(B03Evaluator*,void*,void*,void*);
// TS3W.exe 0x0054E630: Evaluate the clip, IK contributions, and optional bone weights.
void __fastcall EvaluateE630(B03Evaluator*,void*,void*,void*);
// TS3W.exe 0x0054EE20: Construct the base evaluator and install the derived table, leaving derived fields untouched.
M05Node* __fastcall ConstructEE20(M05Node*,void*,void*);
// TS3W.exe 0x0054EE40: Return the evaluator class ID.
uint32_t __fastcall TypeEE40(void*,void*);
}
