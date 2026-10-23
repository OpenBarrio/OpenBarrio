#include "ts3/anim/m05_B20.h"
#include <cstring>
#include <limits>
#include <intrin.h>
namespace ts3::anim::b20 {
static void* Address(uint32_t a) {
#if defined(_M_IX86)
    return reinterpret_cast<void*>(uintptr_t(a));
#else
    (void)a; return nullptr;
#endif
}
uintptr_t Release(ResourcePrefix** slot) {
    auto* p=*slot; *slot=nullptr;
    if(!p) return 0;
    if(_InterlockedExchangeAdd(reinterpret_cast<volatile long*>(&p->atomicRefs),-1)==1) {
        _InterlockedExchange(reinterpret_cast<volatile long*>(&p->atomicRefs),1);
        return CallGameVirtual<uintptr_t,0>(p,1u);
    }
    return reinterpret_cast<uintptr_t>(p);
}
void SetResource(Coherent* p,void* arg) {
    Release(&p->resource);
    if(arg) CallGameMethod<void,0x00b96150>(&p->resource,arg);
    p->flags|=1;
}
SrtValue* Local(Pose* p) {
    if(!p->local) p->local=p->localStorage;
    p->flags &= 0xfffe;
    return p->local;
}
uint32_t* Fill(Pose* p,float value) {
    uint32_t bits; std::memcpy(&bits,&value,4);
    for(auto& word:p->weightStorage) word=bits;
    p->weights=reinterpret_cast<float*>(p->weightStorage);
    return p->weightStorage;
}
void ClearFill(Pose* p) { p->weights=nullptr; }
float* Weights(Pose* p) { p->auxiliary=p->weightsStorage; return p->weightsStorage; }
void ClearWeights(Pose* p) { p->auxiliary=nullptr; }
void* Auxiliary(Pose* p) { p->workspace=p->auxiliaryStorage; return p->workspace; }
void ClearAuxiliary(Pose* p) { p->workspace=nullptr; }
bool InitializePool() {
#if defined(_M_IX86)
    auto* initialized=static_cast<uint8_t*>(Address(0x011cd8b0));
    if(!*initialized) {
        CallGameMethod<void,0x004e4350>(Address(0x011cd8b4),0x19b30,16,4,0,-1,Address(0x00574340),Address(0x004e8590),0);
        *initialized=1;
    }
#else
    AbortNotRewritten("pose pool global");
#endif
    return true;
}
void* Reset(Pose* p) {
    Local(p);
    if(!p->world) { p->world=p->worldStorage; p->flags&=0xfffe; }
    std::memcpy(p->local,p->rig->localBegin,size_t(p->count)*48);
    void* result=std::memcpy(p->world,p->rig->worldBegin,size_t(p->count)*48);
    p->flags|=1;
    const float q[4]={0,0,0,1};
    std::memcpy(&p->identity,q,16); std::memset(&p->zero,0,16);
    return result;
}
float* ZeroWeights(Pose* p) {
    Weights(p);
    for(uint32_t i=0;i<p->count;++i) p->weightsStorage[i]=0;
    return p->weightsStorage;
}
static void* Allocate(uint32_t n) { return CallGameFunction<void*,0x004e3f90>(n,Address(0x00fc2d34),0,0,0,0); }
static void Free(void* p) { CallGameFunction<void,0x004e3f70>(p); }
BoneDeltas* Construct(BoneDeltas* p,uint32_t count) {
    p->vtable=Address(0x0107396c);
    _InterlockedExchange(reinterpret_cast<volatile long*>(&p->atomicRefs),0);
    p->vtable=Address(0x00fc2d30); p->count=count; p->srt=nullptr; p->flag=1;
    if(count) {
        uint64_t n=uint64_t(count)*48;
        p->srt=static_cast<SrtValue*>(Allocate(n>UINT32_MAX?UINT32_MAX:uint32_t(n)));
        std::memset(p->srt,0,uint32_t(count*48));
    }
    return p;
}
BoneDeltas* Destroy(BoneDeltas* p,uint32_t flags) {
    p->vtable=Address(0x00fc2d30);
    auto* storage=p->srt; p->count=0; p->srt=nullptr;
    if(storage) Free(storage);
    p->vtable=Address(0x0107396c);
    if(flags&1) Free(p);
    return p;
}
BoneDeltas* Clone(BoneDeltas* p) {
    auto* copy=static_cast<BoneDeltas*>(Allocate(sizeof(BoneDeltas)));
    if(copy) Construct(copy,p->count);
    if(p->count && p->srt && copy->srt) std::memcpy(copy->srt,p->srt,uint32_t(p->count*48));
    return copy;
}
}

#if defined(_M_IX86)
namespace ts3::anim::b20 {
uintptr_t __fastcall HookRelease(ResourcePrefix** p, void*) { return Release(p); }
void __fastcall HookSetResource(Coherent* p, void*, void* a) { SetResource(p,a); }
SrtValue* __fastcall HookLocal(Pose* p, void*) { return Local(p); }
uint32_t* __fastcall HookFill(Pose* p, void*, float a) { return Fill(p,a); }
void __fastcall HookClearFill(Pose* p, void*) { ClearFill(p); }
float* __fastcall HookWeights(Pose* p, void*) { return Weights(p); }
void __fastcall HookClearWeights(Pose* p, void*) { ClearWeights(p); }
void* __fastcall HookAuxiliary(Pose* p, void*) { return Auxiliary(p); }
void __fastcall HookClearAuxiliary(Pose* p, void*) { ClearAuxiliary(p); }
void* __fastcall HookReset(Pose* p, void*) { return Reset(p); }
float* __fastcall HookZeroWeights(Pose* p, void*) { return ZeroWeights(p); }
BoneDeltas* __fastcall HookConstruct(BoneDeltas* p, void*, uint32_t a) { return Construct(p,a); }
BoneDeltas* __fastcall HookDestroy(BoneDeltas* p, void*, uint32_t a) { return Destroy(p,a); }
BoneDeltas* __fastcall HookClone(BoneDeltas* p, void*) { return Clone(p); }
}
#endif
