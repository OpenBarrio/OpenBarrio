#pragma once
#include "ts3/anim/m05_B20.h"
namespace ts3::anim::m70 {
struct alignas(16) Scratch {
    void* vtable;
    unsigned char gap04[12];
    void* field10;
    unsigned char gap14[4];
    void* field18;
    unsigned char gap1c[4];
    RigResource* rig;
    ResourcePrefix* resource;
    void* palette;
    void* field2c;
    void* field30;
    uint32_t flags;
    unsigned char gap38[8];
    Float4Value values;
};
TS3_X86_OFFSET(Scratch,rig,0x20);
TS3_X86_OFFSET(Scratch,values,0x40);
// TS3W.exe 0x005737B0: Compute four scratch deformation weights, preserving SSE normalization and x87 angle wrapping.
uint32_t Evaluate(Scratch*,b20::Pose*);
// TS3W.exe 0x00574020: Consume a rig reference, invalidate cached storage, and release mismatched retained data.
void SetRig(Scratch*,RigResource*);
// TS3W.exe 0x00574100: Initialize only the observed Scratch fields and return this.
Scratch* Construct(Scratch*);
// TS3W.exe 0x00574460: Generate hierarchical matrices using joint index58, not the parent field.
void Matrices(b20::Pose*,Matrix44Value*);
// TS3W.exe 0x00574500: Update a signed range of world SRTs and clear the valid bit even for an empty range.
int32_t WorldRange(b20::Pose*,int32_t,int32_t);
// TS3W.exe 0x00574690: Copy 0x34 bytes per 0x40 record backwards, leaving the tail untouched; return destination begin.
void* CopyBackward(const void*,const void*,void*);
#if defined(_M_IX86)
namespace hooks {
uint32_t __fastcall Evaluate(Scratch*,void*,b20::Pose*);
void __fastcall SetRig(Scratch*,void*,RigResource*);
Scratch* __fastcall Construct(Scratch*,void*);
void __fastcall Matrices(b20::Pose*,void*,Matrix44Value*);
int32_t __fastcall WorldRange(b20::Pose*,void*,int32_t,int32_t);
}
#endif
}
