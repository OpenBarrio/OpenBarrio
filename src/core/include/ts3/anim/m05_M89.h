#pragma once
#include "ts3/layout.h"
#include "ts3/math/float_values.h"
#include "ts3/anim/m05_B21.h"
namespace ts3::anim::m89 {
struct alignas(16) Element { unsigned char data[0x2c]; int32_t index; float weight; unsigned char tail[12]; };
struct alignas(16) Packet {
    Element *begin,*end,*capacity; uint32_t descriptor;
    alignas(16) Element storage[11];
    float vector0[4],vector1[4]; float total,weight; int32_t index; uint32_t untouched;
};
struct alignas(16) Workspace {
    Packet *begin,*end,*capacity; uint32_t descriptor;
    alignas(16) unsigned char storage[0x3000];
    float weight; unsigned char tail[12];
};
struct alignas(16) Pose {
    unsigned char prefix[0x10]; float translation[4],rotation[4];
    unsigned char opaque30[0x14]; Workspace* workspace; float weight;
    unsigned char opaque4c[0x7554];
    alignas(16) Workspace inlineWorkspace[6]; unsigned char gap[0x4b0];
    float saved[8];
};
struct Event {
    void* vtable; uint32_t argument,type,zero; float time,interval,duration;
    uint32_t mode,reserved0,reserved1; unsigned char flag;
};
TS3_X86_SIZE(Packet,0x300); TS3_X86_SIZE(Workspace,0x3020);
TS3_X86_OFFSET(Packet,weight,0x2f4); TS3_X86_OFFSET(Packet,index,0x2f8);
TS3_X86_OFFSET(Pose,workspace,0x44); TS3_X86_OFFSET(Pose,inlineWorkspace,0x75a0);
TS3_X86_OFFSET(Pose,saved,0x19b10); TS3_X86_OFFSET(Event,flag,0x28);
// TS3W.exe 0x0057E500: reset pose accumulators and six inline workspace ranges, returning the final cursor.
uintptr_t Reset(Pose*);
// TS3W.exe 0x0057E560: accumulate weighted pose and packets, preserving multiplication order and -1 packet omission.
uintptr_t Accumulate(const float*,const float*,float,Pose*,Pose*);
// TS3W.exe 0x0057E730: accumulate pose and packet weights without per-element masks.
uintptr_t AccumulateSimple(float,Pose*,Pose*);
// TS3W.exe 0x0057F420: construct type C1E40002 event, leaving trailing bytes untouched.
Event* ConstructEvent2(Event*,uint32_t);
// TS3W.exe 0x0057F470: construct type C1E40003 event, clearing the single flag byte only.
Event* ConstructEvent3(Event*,uint32_t);
struct alignas(16) IkState {
    unsigned char opaque00[0x40];
    ts3::math::Float4Value pole,target,orientation;
    int32_t first,middle,last,mode,count;
    float angle,threshold,relaxation;
    Pose* pose;
    ts3::math::SrtValue *local,*world;
    b21::SkeletonView* skeleton;
};
TS3_X86_OFFSET(IkState,mode,0x7c); TS3_X86_OFFSET(IkState,pose,0x90);
TS3_X86_OFFSET(IkState,skeleton,0x9c);
// TS3W.exe 0x0057D670: solve the clip's IK chain and end orientation, preserving SSE order and caller-stack residue in modes 0/2.
uintptr_t Solve(IkState*);
// Explicit native form of Solve: residue supplies the otherwise uninitialized original stack quaternion.
extern "C" uintptr_t M89SolveWithResidue(IkState*,const ts3::math::Float4Value* residue);
namespace hooks {
#if defined(_M_IX86)
Event* __fastcall ConstructEvent2(Event*,void*,uint32_t);
Event* __fastcall ConstructEvent3(Event*,void*,uint32_t);
#endif
}
}
