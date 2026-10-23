#pragma once
#include "ts3/anim/m05_layouts.h"
namespace ts3::anim::m72 {
struct Pose {
    void** vtable;
    unsigned char opaque[16-sizeof(void*)];
    Float4Value translation,rotation;
    RigResource* rig;
    SrtValue* srt;
    void* auxiliary38;
    void* auxiliary3c;
    void* auxiliary40;
    void* workspace;
    void* auxiliary48;
    void* weights;
    uint16_t flags,boneCount;
};
TS3_X86_OFFSET(Pose,weights,0x4c);
TS3_X86_OFFSET(Pose,flags,0x50);
struct Weights { float* begin; float* end; float* capacity; void* descriptor; };
struct alignas(16) Record { unsigned char bytes[64]; };
struct Records { Record* begin; Record* end; Record* capacity; };
// TS3W.exe 0x005749F0: Insert a 64-byte record; the broken null-destination growth path is retained.
void Insert(Records*,Record*,const Record*);
// TS3W.exe 0x00574BE0: Destroy pose storage and release the rig after freeing its weights.
void Destroy(Pose*);
// TS3W.exe 0x00574C40: Reset pose transforms, six workspace cursors, flags and cached arrays.
void Reset(Pose*);
// TS3W.exe 0x00574CD0: Lazily allocate the padded vertex-weight vector.
void EnsureWeights(Pose*);
// TS3W.exe 0x00574D90: Interpolate vertex weights in subtract/multiply/add order.
void Blend(Pose*,const Weights*,const Weights*,float);
// TS3W.exe 0x00574E20: Resolve a channel hash and assign an in-range vertex weight.
void SetWeight(Pose*,uint32_t,float);
}

#if defined(_M_IX86)
namespace ts3::anim::m72::hooks {
inline void __fastcall Insert(Records* s,void*,Record* p,const Record* v) { m72::Insert(s,p,v); }
inline void __fastcall Destroy(Pose* s,void*) { m72::Destroy(s); }
inline void __fastcall Reset(Pose* s,void*) { m72::Reset(s); }
inline void __fastcall EnsureWeights(Pose* s,void*) { m72::EnsureWeights(s); }
inline void __fastcall Blend(Pose* s,void*,const Weights* a,const Weights* b,float t) { m72::Blend(s,a,b,t); }
inline void __fastcall SetWeight(Pose* s,void*,uint32_t h,float v) { m72::SetWeight(s,h,v); }
}
#endif
