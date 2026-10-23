#include "ts3/render/m07_B06.h"
#include <cstring>
namespace ts3::render::b06 {
namespace {
uint32_t Word(const void* p,size_t o) { uint32_t v;std::memcpy(&v,static_cast<const uint8_t*>(p)+o,4);return v; }
template<class R,size_t Slot,class... A> R Com(void* self,A... a) {
    using Fn=R (__stdcall*)(void*,A...);
    return reinterpret_cast<Fn>((*static_cast<void***>(self))[Slot])(self,a...);
}
template<uint32_t Address> uint32_t Table(uint32_t i) {
#if defined(_M_IX86)
    return reinterpret_cast<const uint32_t*>(Address)[i];
#else
    (void)i;throw std::runtime_error("B06 x86 table unavailable");
#endif
}
}
int32_t __fastcall Draw(Host* h,void*,uint32_t type,int32_t base,uint32_t min,uint32_t vertices,uint32_t start,uint32_t count,uint32_t instances) {
    const auto primitive=Table<0x00FE7784>(type);
    Com<int32_t,102>(h->device,0u,instances|0x40000000u);
    Com<int32_t,102>(h->device,1u,0x80000001u);
    Com<int32_t,82>(h->device,primitive,base,min,vertices,start,count);
    Com<int32_t,102>(h->device,0u,1u);
    auto result=Com<int32_t,102>(h->device,1u,1u);
    ++h->draws;
    return result;
}
bool __fastcall CreateImmediate(Host* h,void*) {
    if(Com<int32_t,28>(h->device,4u,4u,22u,0u,0u,1u,&h->texture,static_cast<void*>(nullptr))<0) return false;
    auto allocate=[](uint32_t bytes,uint32_t name) { return CallGameFunction<void*,0x004E3F90>(bytes,name,0u,0u,0u,0u); };
    void* p=allocate(0x20,0x00FE7AD4);
    h->vertex=p?CallGameMethod<void*,0x0061A380>(p):nullptr;
    if(!h->vertex) return false;
    CallGameVirtual<uint32_t,1>(h->vertex);
    if(!CallGameMethod<bool,0x0061A1C0>(h->vertex,h->vertexCount,1u,uintptr_t(0x00FE7ACC))) return false;
    p=allocate(0x24,0x00FE7AB4);
    h->index=p?CallGameMethod<void*,0x00616EE0>(p):nullptr;
    if(!h->index) return false;
    CallGameVirtual<uint32_t,1>(h->index);
    return CallGameMethod<bool,0x00616DF0>(h->index,h->indexCount,0u,1u);
}
bool __fastcall AcquireSurfaces(Host* h,void*) {
    CallGameVirtual<void,4>(h->color);
    CallGameVirtual<void,4>(h->depth);
    void* surface;
    if(Com<int32_t,38>(h->device,0u,&surface)<0) return false;
    CallGameVirtual<void,5>(h->color,surface,61u);
    if(Com<int32_t,40>(h->device,&surface)<0) return false;
    CallGameVirtual<void,5>(h->depth,surface,141u);
    return true;
}
uint32_t __fastcall ReadStates(Host* h,void*) {
    uint32_t value;
    for(uint32_t i=0;i<102;++i) {
        auto key=Table<0x00FE77D8>(i);
        Com<int32_t,58>(h->device,key,&value);
        h->Word(0x144+key*4)=value;
        h->Word(0x470+key*4)=value;
    }
    for(uint32_t o=0x7b8;o<=0x7ec;o+=4) h->Word(o)=0;
    for(uint32_t stage=0;stage<16;++stage) {
        for(uint32_t i=0;i<13;++i) {
            auto key=Table<0x00FE77A4>(i);
            Com<int32_t,68>(h->device,stage,key,&value);
            h->Word(0x80c+stage*0x74+key*4)=value;
            h->Word(0x840+stage*0x74+key*4)=value;
        }
        h->Word(0x878+stage*0x74)=0;
        h->Word(0x87c+stage*0x74)=0;
    }
    return 16;
}
void* __fastcall Mode(Host* h,void*) { return static_cast<uint8_t*>(h->privateData)+0x1c; }
bool __fastcall CheckFormat(Host* h,void*,uint32_t format,uint32_t usage,uint32_t kind) {
    return Com<int32_t,10>(h->d3d9,Word(h->privateData,0x20),Word(h->privateData,0x1c),Word(h->privateData,0x1bc),usage,kind,format)>=0;
}
void __fastcall CaptureGamma(Host* h,void*) {
    if(!h->flag24 && h->flag27 && Word(h->privateData,0x1a4)==0)
        Com<void,21>(h->device,0u,0u,&h->Word(0xf58));
}
void** __fastcall GammaRamps(Host* h,void*,void** a,void** b,void** c) {
    *a=&h->Word(0xf58);*b=&h->Word(0x1158);*c=&h->Word(0x1358);return c;
}
bool __fastcall WideIndices(Host* h,void*) { return Word(h->privateData,0xd4)>0xffff; }
void* __cdecl Singleton() {
#if defined(_M_IX86)
    return *reinterpret_cast<void**>(0x011CECCC);
#else
    throw std::runtime_error("B06 singleton unavailable");
#endif
}
void __fastcall State168(Host* h,void*,uint32_t value) { CallGameMethod<void,0x0060E860>(h,168u,value); }
void __fastcall Cull(Host* h,void*,uint32_t value) { CallGameMethod<void,0x0060E860>(h,22u,Table<0x00FE7970>(value)); }
void __fastcall ZEnable(Host* h,void*,uint32_t value) { CallGameMethod<void,0x0060E860>(h,7u,uint32_t(uint8_t(value)!=0)); }
}
