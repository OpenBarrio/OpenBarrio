#pragma once
#include "ts3/anim/m05_B01.h"
#include "ts3/anim/m05_B03.h"
namespace ts3::anim::m04 {
using Node = b01::Node;
struct ClipEvaluator {
    Node base; uint32_t opaque24;
    B03Binding bindings[5][11];
    B03Key primary,secondary;
    B03Resource* primaryResource;
    B03Resource* secondaryResource;
    float time,weight; uint8_t enabled,flag;
};
TS3_X86_OFFSET(ClipEvaluator,bindings,0x28);
TS3_X86_OFFSET(ClipEvaluator,secondaryResource,0x3bc);
struct Weights { float* begin; float* end; float* capacity; void* descriptor; };
struct alignas(16) Pose {
    void** vtable; int32_t refs; unsigned char opaque08[8];
    Float4Value translation,rotation;
    RigResource* rig; SrtValue* srt; void* opaque38; float* mask;
    void* opaque40; void* workspace; void* opaque48; Weights* weights;
    uint16_t flags,boneCount;
};
struct Context { unsigned char opaque00[0x40]; RigResource* rig; unsigned char opaque44[0x10]; uint8_t flags; };
TS3_X86_OFFSET(Pose,mask,0x3c);
TS3_X86_OFFSET(Pose,weights,0x4c);
TS3_X86_OFFSET(Context,flags,0x54);
// TS3W.exe 0x0054D000: Reset the weighted evaluator and optionally free, returning this even after free.
Node* DestroyWeighted(Node*,uint32_t);
// TS3W.exe 0x0054D030: Reset the binary evaluator and optionally free, returning this even after free.
Node* DestroyBinary(Node*,uint32_t);
// TS3W.exe 0x0054D060: Evaluate two weighted children, masks, selected SRTs and vertex weights; other child counts pass through or do nothing.
uintptr_t EvaluateWeighted(Node*,Context*,Pose*);
// TS3W.exe 0x0054D7D0: Evaluate two children and blend SRTs using the last child's weight, releasing temporaries in reverse order.
uintptr_t EvaluateBinary(Node*,Context*,Pose*);
// TS3W.exe 0x0054DDB0: Initialize the clip evaluator, preserving sibling links, padding and opaque bytes.
ClipEvaluator* ConstructClip(ClipEvaluator*,void*);
// TS3W.exe 0x0054DEF0: Retain a nonnull replacement before releasing the old resource and copy its key; null is a no-op returning true in AL.
bool SetResource(ClipEvaluator*,B03Resource*);
}
#if defined(_M_IX86)
namespace ts3::anim::m04::hooks {
Node* __fastcall DestroyWeighted(Node*,void*,uint32_t);
Node* __fastcall DestroyBinary(Node*,void*,uint32_t);
uintptr_t __fastcall EvaluateWeighted(Node*,void*,Context*,Pose*);
uintptr_t __fastcall EvaluateBinary(Node*,void*,Context*,Pose*);
ClipEvaluator* __fastcall ConstructClip(ClipEvaluator*,void*,void*);
bool __fastcall SetResource(ClipEvaluator*,void*,B03Resource*);
}
#endif
