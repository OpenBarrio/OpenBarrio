#pragma once
#include "ts3/anim/m05_B20.h"
namespace ts3::anim::m87 {
struct FloatRange { float *begin,*end,*capacity; void* descriptor; };
struct Mask {
    ResourcePrefix base;
    FloatRange values;
    float weight;
    // TS3W.exe 0x0057C830: Destroy the mask payload and base, optionally freeing this; returns this after free.
    Mask* Destroy(uint32_t flags);
    // TS3W.exe 0x0057C8D0: Load mask versions 0x200..0x2ff, defaulting version 0x200 weight to one.
    bool Load(void* stream,uint32_t unused,bool swap);
};
TS3_X86_OFFSET(Mask,values,0x28);
TS3_X86_OFFSET(Mask,weight,0x38);
// Pointer-bearing views of the queried controller variants; unobserved fields remain opaque.
struct Unary28 { Controller base; Controller* child; };
struct Unary2C { Controller base; void* extra; Controller* child; };
struct Unary30 { Controller base; void* extra[2]; Controller* child; };
struct Unary50 { Controller base; unsigned char opaque28[0x28]; Controller* child; };
struct Binary { Controller base; void* extra; Controller *left,*right; float weight; };
struct Weighted { Controller base; void* extra; Controller* child; float weight; };
struct Clip { Controller base; unsigned char opaque28[0x5a8]; void* resource; };
TS3_X86_OFFSET(Unary28,child,0x28);
TS3_X86_OFFSET(Unary2C,child,0x2c);
TS3_X86_OFFSET(Unary30,child,0x30);
TS3_X86_OFFSET(Unary50,child,0x50);
TS3_X86_OFFSET(Binary,left,0x2c);
TS3_X86_OFFSET(Binary,right,0x30);
TS3_X86_OFFSET(Binary,weight,0x34);
TS3_X86_OFFSET(Weighted,weight,0x30);
TS3_X86_OFFSET(Clip,resource,0x5d0);
// TS3W.exe 0x0057C9A0: Append a recursive controller description, querying every interface before selecting a branch.
uintptr_t Describe(Controller*,void* string);
// TS3W.exe 0x0057CCD0: Zero local SRTs, bone weights and optional vertex weights.
uintptr_t Clear(b20::Pose*);
// TS3W.exe 0x0057CD20: Add nonzero source vertex weights; unordered values are included, signed zero skipped.
void AddVertex(b20::Pose* source,b20::Pose* destination,float weight);
struct alignas(16) Root { unsigned char opaque00[16]; Float4Value translation,rotation; unsigned char opaque30[24]; float weight; };
TS3_X86_OFFSET(Root,weight,0x48);
// TS3W.exe 0x0057CD80: Accumulate translation and sign-adjusted quaternion without normalization, including NaN branch behavior.
void AddRoot(Root*,const Float4Value* translationAndRotation,float weight);
}
