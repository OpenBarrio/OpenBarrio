#include "ts3/render/m07_B10.h"
#if defined(_M_IX86)
#include <intrin.h>
#endif
#include <cstring>
namespace ts3::render::b10 {
namespace {
template<class R,size_t Slot,class... A> R Com(void* p,A... a) {
    using Fn=R (__stdcall*)(void*,A...);
    return reinterpret_cast<Fn>((*static_cast<void***>(p))[Slot])(p,a...);
}
template<class T,uint32_t Address> T& Global() {
#if defined(_M_IX86)
    return *reinterpret_cast<T*>(Address);
#else
    throw std::runtime_error("B10 x86 global unavailable");
#endif
}
void Tables(Index* p,bool base=false) {
#if defined(_M_IX86)
    p->primary=reinterpret_cast<void*>(base?0x00FF39F8:0x00FE8318);
    if(!base) p->secondary=reinterpret_cast<void*>(0x00FE8328);
#else
    (void)p;(void)base;throw std::runtime_error("B10 x86 vtables unavailable");
#endif
}
// Telemetry queries are observable even if the logging manager is disabled.
void Account(void* receiver,void* const& gpu,bool create) {
    if(!Global<uint32_t,0x011DC18C>()) return;
    for(unsigned slot: {3u,2u}) {
        uint32_t bytes=slot==3?CallGameVirtual<uint32_t,3>(receiver):CallGameVirtual<uint32_t,2>(receiver);
        if(!bytes || !Global<uint32_t,0x011DC18C>()) continue;
        auto manager=CallGameFunction<void**,0x00931080>();
        if(!*manager || !*(static_cast<uint8_t*>(*manager)+0x56c)) continue;
        // The game keys are 32-bit GPU tokens, not native owning pointers.
        uint32_t key=uint32_t(reinterpret_cast<uintptr_t>(gpu))|0x80000000u;
        if(slot==3) ++key;
        if(create) CallGameMethod<void,0x009336F0>(*manager,0u,key,bytes,slot==3?uintptr_t(0x00FE8300):uintptr_t(0x00FE82E8),bytes);
        else CallGameMethod<void,0x00933A60>(*manager,0u,key,bytes);
    }
}
}
void* __fastcall LockIndex(Index* p,void*,uint32_t offset,uint32_t size,uint32_t flags) {
    if(!p->gpuBuffer) return nullptr;
    uint32_t mode=0;
    if(flags&1) mode=0x2000;
    if(flags&2) mode|=0x1000;
    if(flags&4) mode=0x10;
    // The original reuses the flags stack word as its output pointer.
    void* out=reinterpret_cast<void*>(uintptr_t(flags));
    if(Com<int32_t,11>(p->gpuBuffer,offset,size,&out,mode)<0) return nullptr;
    return out;
}
int32_t __fastcall UnlockIndex(Index* p,void*) { return Com<int32_t,12>(p->gpuBuffer); }
bool __fastcall CreateIndex(Index* p,void*) {
    auto host=CallGameFunction<m07::DeviceHost*,0x0060E2C0>();
    uint32_t pool=1,usage=0;
    if(p->flags&1) { pool=0;usage=0x208; }
    uint32_t format;
#if defined(_M_IX86)
    format=reinterpret_cast<const uint32_t*>(0x00FE82E0)[p->formatSelector];
#else
    throw std::runtime_error("B10 index format table unavailable");
#endif
    uint32_t bytes=p->count<<((p->formatSelector+1)&31);
    if(Com<int32_t,27>(host->device,bytes,usage,format,pool,&p->gpuBuffer,static_cast<void*>(nullptr))<0) return false;
    Account(&p->secondary,p->gpuBuffer,true);
    return true;
}
void __fastcall ReleaseIndex(Index* p,void*) {
    if(!p->gpuBuffer) return;
    Account(&p->secondary,p->gpuBuffer,false);
    CallGameMethod<void,0x0060F120>(Global<m07::DeviceHost*,0x011CECC8>(),p);
    Com<uint32_t,2>(p->gpuBuffer);
    p->gpuBuffer=nullptr;
}
bool __fastcall InitializeIndex(Index* p,void*,uint32_t count,uint32_t format,uint32_t flags) {
    p->count=count;p->formatSelector=format;p->flags=flags;
    if(!CreateIndex(p,nullptr)) return false;
    CallGameMethod<void,0x00613F10>(&p->secondary);
    return true;
}
void __fastcall DetachIndex(Index* p,void*) {
    CallGameMethod<void,0x00613F30>(&p->secondary);ReleaseIndex(p,nullptr);
}
bool __fastcall UploadIndex(Index* p,void*,const void* source,uint32_t size) {
    if(!p->gpuBuffer) return false;
    void* out=reinterpret_cast<void*>(uintptr_t(size));
    if(Com<int32_t,11>(p->gpuBuffer,0u,size,&out,0u)<0 || !out) return false;
    std::memcpy(out,source,size);Com<int32_t,12>(p->gpuBuffer);return true;
}
Index* __fastcall ConstructIndex(Index* p,void*) {
    Tables(p,true); // ref initialization is an exchange, not a release
#if defined(_M_IX86)
    _InterlockedExchange(reinterpret_cast<volatile long*>(&p->engineRefs),0);
#else
    p->engineRefs=0;
#endif
    CallGameMethod<void*,0x00613EE0>(&p->secondary);
    Tables(p);p->gpuBuffer=nullptr;p->count=0;return p;
}
void __fastcall DestroyIndex(Index* p,void*) {
    Tables(p);DetachIndex(p,nullptr);CallGameMethod<void,0x00613F00>(&p->secondary);Tables(p,true);
}
void __fastcall DetachSurface(SurfaceBuffer* p,void*) {
    CallGameMethod<void,0x00613F30>(&p->secondary);
    if(!p->gpu) return;
    Account(&p->secondary,p->gpu,false);Com<uint32_t,2>(p->gpu);p->gpu=nullptr;
}
bool __fastcall SurfaceAvailable(SurfaceBuffer* p,void*) {
    if(!p->gpu) return false;
    uint32_t desc[8];
    return Com<int32_t,12>(p->gpu,desc)>=0 && desc[4]!=0;
}
void __fastcall AdoptSurface(SurfaceBuffer* p,void*,void* gpu,uint32_t tag) {
    p->tag=tag;p->gpu=gpu;
    uint32_t desc[8];Com<int32_t,12>(gpu,desc); // HRESULT deliberately ignored
    p->width=desc[6];p->height=desc[7];p->flags=desc[3]==2?0x100:0;
    uint8_t usage=uint8_t(desc[2]);
    if(!(usage&3)) return;
    if(usage&1) p->flags|=1;else if(usage&2) p->flags|=0x11;
    CallGameMethod<void,0x00613F10>(&p->secondary);
}
void __fastcall ReleaseSurface(AdjustedSurface* p,void*) {
    if(!p->gpu) return;
    Account(p,p->gpu,false);Com<uint32_t,2>(p->gpu);p->gpu=nullptr;
}
bool __fastcall InitializeSurface(SurfaceBuffer* p,void*,uint32_t width,uint32_t height,uint32_t tag,uint32_t flags) {
    flags|=1;
    if(CallGameFunction<bool,0x006177D0>(tag)) flags|=0x10;
    p->width=width;p->height=height;p->tag=tag;p->flags=flags;
    if(!CallGameVirtual<bool,4>(&p->secondary)) return false;
    CallGameMethod<void,0x00613F10>(&p->secondary);return true;
}
}
