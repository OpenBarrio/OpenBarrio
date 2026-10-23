#pragma once
#include "ts3/anim/m05_layouts.h"
namespace ts3::anim::m03 {
struct Node {
    void* table;
    ListLink siblings;
    ListLink children;
    Node* parent;
    void* argument;
    float weight,previousWeight;
};
TS3_X86_OFFSET(Node,children,0x0c);
// TS3W.exe 0x0054CD70: Evaluate first child and add SRT deltas, preserving SSE refinement and incidental EAX.
uintptr_t AddDeltas(Node*,RigPose*,RigPose*);
// TS3W.exe 0x0054CE90: Construct the reset evaluator with base constructor and derived table.
Node* ConstructReset(Node*,void*);
// TS3W.exe 0x0054CEF0: Reset the output pose then initialize its weight buffer to positive zero.
uintptr_t Reset(Node*,RigPose*,RigPose*);
// TS3W.exe 0x0054CF10: Destroy reset evaluator, freeing iff flags bit zero is set, returning this even after free.
Node* DeleteReset(Node*,uint32_t);
// TS3W.exe 0x0054CF40: Construct the first specialized evaluator.
Node* ConstructFirst(Node*,void*);
// TS3W.exe 0x0054CFA0: Construct the second specialized evaluator.
Node* ConstructSecond(Node*,void*);
}
namespace ts3::anim::m03 {
inline uintptr_t __fastcall HookAddDeltas(Node* p,void*,RigPose* a,RigPose* b) { return AddDeltas(p,a,b); }
inline Node* __fastcall HookConstructReset(Node* p,void*,void* a) { return ConstructReset(p,a); }
inline uintptr_t __fastcall HookReset(Node* p,void*,RigPose* a,RigPose* b) { return Reset(p,a,b); }
inline Node* __fastcall HookDeleteReset(Node* p,void*,uint32_t a) { return DeleteReset(p,a); }
inline Node* __fastcall HookConstructFirst(Node* p,void*,void* a) { return ConstructFirst(p,a); }
inline Node* __fastcall HookConstructSecond(Node* p,void*,void* a) { return ConstructSecond(p,a); }
}
