#pragma once
#include "ts3/anim/m05_layouts.h"
namespace ts3::anim::m71 {
struct WeightedRecord { unsigned char opaque[0x30]; float weight; unsigned char tail[12]; };
struct WeightSum { WeightedRecord* begin; WeightedRecord* end; unsigned char opaque[0x2f0-2*sizeof(void*)]; float total; };
struct FloatRange { float* begin; float* end; };
// RigPose's opaque +38 region contains a pointer. Widen it explicitly on native builds.
struct alignas(16) Pose {
    unsigned char opaque00[0x10]; Float4Value translation,rotation;
    RigResource* rig; SrtValue* local; SrtValue* world; void* aux3c; void* aux40;
    void* workspace; void* aux48; FloatRange* weights; uint16_t flags,boneCount;
    unsigned char gap[12]; SrtValue inlineLocal[300];
    SrtValue inlineWorld[300];
    unsigned char gap75a0[0x75a0-0x38a0-300*48]; unsigned char inlineWorkspace[16];
};
TS3_X86_OFFSET(Pose,local,0x34); TS3_X86_OFFSET(Pose,world,0x38);
TS3_X86_OFFSET(Pose,flags,0x50); TS3_X86_OFFSET(Pose,inlineLocal,0x60);
TS3_X86_OFFSET(Pose,inlineWorld,0x38a0); TS3_X86_OFFSET(Pose,inlineWorkspace,0x75a0);
// TS3W.exe 0x00574860: Sum +30 weights in record order and return the end pointer.
WeightedRecord* SumWeights(WeightSum* self);
// TS3W.exe 0x005748A0: Pop the locked global free list, retrying refill while AL is nonzero.
void* AcquirePose();
// TS3W.exe 0x00574900: Replace retained rig, reset workspace/count, then release the argument even on alias.
uintptr_t AssignRig(Pose* self, RigResource* rig);
// TS3W.exe 0x00574960: Clear the optional weight range to positive zero.
void ClearWeights(Pose* self);
// TS3W.exe 0x00574990: Evaluate world pose on demand and set flag bit zero.
void EnsureWorld(Pose* self);
// TS3W.exe 0x005749B0: Install inline world storage if absent and return the evaluated world array.
SrtValue* GetWorld(Pose* self);
}
namespace ts3::anim::m71::hooks {
inline WeightedRecord* __fastcall SumWeights(WeightSum* p,void*) { return m71::SumWeights(p); }
inline uintptr_t __fastcall AssignRig(Pose* p,void*,RigResource* r) { return m71::AssignRig(p,r); }
inline void __fastcall ClearWeights(Pose* p,void*) { m71::ClearWeights(p); }
inline void __fastcall EnsureWorld(Pose* p,void*) { m71::EnsureWorld(p); }
inline SrtValue* __fastcall GetWorld(Pose* p,void*) { return m71::GetWorld(p); }
}
