#pragma once
#include "ts3/layout.h"
namespace ts3::anim::m06 {
struct Node {
    void** vtable; Node* next; Node* previous; Node* first; Node* last; Node* parent;
    void* owner; float weight, oldWeight; Node* left; Node* right;
};
struct Pose { void** vtable; int32_t refs; unsigned char opaque[0x34]; float* weights; };
struct Context { unsigned char opaque[0x40]; void* rig; };
TS3_X86_OFFSET(Node,left,0x24);
TS3_X86_OFFSET(Pose,weights,0x3c);
TS3_X86_OFFSET(Context,rig,0x40);
// TS3W.exe 0x0054EE50: Query the base or derived evaluator interface.
void* Query(void*,uint32_t);
// TS3W.exe 0x0054EE80: Serialize left then right child, retaining the second helper's EAX.
uintptr_t Save(Node*,void*);
// TS3W.exe 0x0054EEB0: Load left then right child through the owner registry.
uintptr_t Load(Node*,void*);
// TS3W.exe 0x0054F010: Destroy the evaluator and optionally free it, returning this.
void* Destroy(Node*,uint32_t);
// TS3W.exe 0x0054F040: Compose right with inverse left, copy weights with x87 NaN quieting, release right before left and retain release EAX.
uintptr_t Evaluate(Node*,Context*,Pose*);
// TS3W.exe 0x0054F260: Append a debug segment with the same color at each end (ECX plus ret 12).
void* Segment(void*,const void*,const void*,uint32_t);
}
#if defined(_M_IX86)
namespace ts3::anim::m06 {
inline void* __fastcall HookQuery(void* s,void*,uint32_t id) { return Query(s,id); }
inline uintptr_t __fastcall HookSave(Node* s,void*,void* stream) { return Save(s,stream); }
inline uintptr_t __fastcall HookLoad(Node* s,void*,void* stream) { return Load(s,stream); }
inline void* __fastcall HookDestroy(Node* s,void*,uint32_t flags) { return Destroy(s,flags); }
inline uintptr_t __fastcall HookEvaluate(Node* s,void*,Context* c,Pose* p) { return Evaluate(s,c,p); }
inline void* __fastcall HookSegment(void* s,void*,const void* a,const void* b,uint32_t c) { return Segment(s,a,b,c); }
}
#endif
