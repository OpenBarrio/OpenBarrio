#pragma once
#include "ts3/anim/m05_layouts.h"
namespace ts3::anim::m18 {
struct Sampler {
    void* vtable;
    unsigned char opaque04[0x20];
    ResourcePrefix* controller;
    ResourcePrefix* track;
    uint32_t opaque2c;
    uint32_t keyLow,keyHigh;
};
TS3_X86_OFFSET(Sampler,controller,0x24);
TS3_X86_OFFSET(Sampler,track,0x28);
TS3_X86_OFFSET(Sampler,keyLow,0x30);
struct TrackLinks { void* vtable; int32_t refs; unsigned char opaque08[12]; ListLink links; };
TS3_X86_OFFSET(TrackLinks,links,0x14);
// TS3W.exe 0x0055DF80: Release track then controller, resetting last refs to one before deletion.
void Destroy(Sampler*);
// TS3W.exe 0x0055E320: Destroy and optionally free, returning this even after free.
Sampler* Delete(Sampler*,uint32_t);
// TS3W.exe 0x0055E340: Sample forward/reverse track, falling back to controller time plus delta.
bool Sample(Sampler*,float time,float delta,uint8_t enabled,Float4Value* output,Float4Value* auxiliary);
// TS3W.exe 0x0055E4B0: Sample with an uninitialized disposable auxiliary vector.
bool SampleSimple(Sampler*,float time,float delta,uint8_t enabled,Float4Value* output);
#if defined(_M_IX86)
namespace hooks {
void __fastcall Destroy(Sampler*,void*);
Sampler* __fastcall Delete(Sampler*,void*,uint32_t);
bool __fastcall Sample(Sampler*,void*,float,float,uint8_t,Float4Value*,Float4Value*);
bool __fastcall SampleSimple(Sampler*,void*,float,float,uint8_t,Float4Value*);
}
#endif
}
