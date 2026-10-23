#pragma once
#include "ts3/anim/m05_B01.h"
#include "ts3/anim/m05_B05.h"
namespace ts3::anim::m15 {
struct alignas(16) Entry { unsigned char value[16]; int32_t index; unsigned char padding[12]; };
struct Node { b01::Node base; Entry *begin,*end,*capacity; void* descriptor; };
struct Context { unsigned char opaque[0x44]; unsigned char* state; unsigned char opaque48[12]; uint8_t flags; };
struct Pose {
    unsigned char opaque[0x10]; Float4Value translation,rotation;
    void* rig; SrtValue* srt; void* world; float* weights;
    void* unknown40; void* workspace; void* unknown48; void* vertexWeights;
    uint16_t flags,boneCount;
};
TS3_X86_OFFSET(Node,begin,0x24);
TS3_X86_OFFSET(Context,flags,0x54);
TS3_X86_OFFSET(Pose,weights,0x3c);
TS3_X86_OFFSET(Pose,boneCount,0x52);
using Writer=b05_impl::Writer;
// TS3W.exe 0x0055BA70: evaluate first child or bind pose then overwrite indexed rotations and weights.
uintptr_t Evaluate(Node*,Context*,Pose*);
// TS3W.exe 0x0055BB50: serialize stride32 records, preserving signed cursor and low-byte refill semantics.
uintptr_t Save(Node*,Writer*);
// TS3W.exe 0x0055BC90: construct orient evaluator and its empty descriptor-backed range.
Node* ConstructOrient(Node*,void*);
// TS3W.exe 0x0055BD00: clear range, free records, reset base and optionally free this; return this.
Node* DeleteOrient(Node*,uint32_t);
// TS3W.exe 0x0055BFB0: construct evaluator base and install derived table.
Node* ConstructPlain(Node*,void*);
// TS3W.exe 0x0055C010: reset derived/base tables and optionally free this; return freed address.
Node* DeletePlain(Node*,uint32_t);
}
#if defined(_M_IX86)
namespace ts3::anim::m15::hooks {
uintptr_t __fastcall Evaluate(Node*,void*,Context*,Pose*);
uintptr_t __fastcall Save(Node*,void*,Writer*);
Node* __fastcall ConstructOrient(Node*,void*,void*);
Node* __fastcall DeleteOrient(Node*,void*,uint32_t);
Node* __fastcall ConstructPlain(Node*,void*,void*);
Node* __fastcall DeletePlain(Node*,void*,uint32_t);
}
#endif
