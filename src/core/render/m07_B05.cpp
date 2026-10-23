#include "ts3/render/m07_B05.h"
namespace ts3::render {
using Host=b05;
TS3_X86_OFFSET(Host,state152,0x3a4);
namespace {
template<unsigned Slot,class... A> uint32_t Device(Host* h,A... a) {
    using Fn=m07::Stdcall<uint32_t,void*,A...>;
    auto table=*static_cast<void***>(h->device);
    return reinterpret_cast<Fn>(table[Slot/4])(h->device,a...);
}
constexpr uint32_t primitives[]={1,2,3,4,6,5};
}
uint32_t b05::GetLight(Host* h,uint32_t i,void* out) { return Device<0xdc>(h,i,out); }
uint32_t b05::LightEnable(Host* h,uint32_t i,uint32_t on) { return Device<0xe0>(h,i,on); }
Host* b05::SetState152(Host* h,uint32_t v) { return CallGameMethod<Host*,0x0060E860>(h,uint32_t(152),v); }
uint32_t b05::GetState152(Host* h) { return h->state152; }
uint32_t b05::GetViewport(Host* h) { return Device<0xc0>(h,static_cast<uint8_t*>(h->privateData)+4); }
uintptr_t b05::SetDeclaration(Host* h,m07::IndexBuffer* r) {
    void* p=r?r->gpuBuffer:nullptr;
    if(h->declaration==p) return reinterpret_cast<uintptr_t>(p);
    h->declaration=p;
    return Device<0x15c>(h,p);
}
uintptr_t b05::RemoveDeclaration(Host* h,m07::IndexBuffer* r) {
    if(!r || h->declaration!=r->gpuBuffer) return reinterpret_cast<uintptr_t>(r);
    auto result=Device<0x15c>(h,static_cast<void*>(nullptr));
    h->declaration=nullptr;
    return result;
}
uintptr_t b05::SetStream(Host* h,uint32_t i,m07::IndexBuffer* r,uint32_t offset,uint32_t stride) {
    if((offset|stride)&3) return offset|stride;
    auto& s=h->streams[i];
    if(r) {
        if(s.buffer==r->gpuBuffer && s.offset==offset && s.stride==stride) return reinterpret_cast<uintptr_t>(&s);
        s.buffer=r->gpuBuffer; s.offset=offset; s.stride=stride;
        return Device<0x190>(h,i,s.buffer,offset,stride);
    }
    if(!s.buffer) return reinterpret_cast<uintptr_t>(&s);
    s.buffer=nullptr;
    return Device<0x190>(h,i,static_cast<void*>(nullptr),uint32_t(0),uint32_t(0));
}
uintptr_t b05::RemoveStreams(Host* h,m07::IndexBuffer* r) {
    uintptr_t result=reinterpret_cast<uintptr_t>(r);
    for(uint32_t i=0;i<2;++i) {
        if(h->streams[i].buffer==r->gpuBuffer) {
            h->streams[i].buffer=nullptr;
            result=Device<0x190>(h,i,static_cast<void*>(nullptr),uint32_t(0),uint32_t(0));
        }
    }
    return result;
}
uintptr_t b05::SetIndices(Host* h,m07::IndexBuffer* r) {
    void* p=r?r->gpuBuffer:nullptr;
    if(h->indices==p) return reinterpret_cast<uintptr_t>(p);
    h->indices=p;
    return Device<0x1a0>(h,p);
}
uintptr_t b05::RemoveIndices(Host* h,m07::IndexBuffer* r) {
    if(h->indices!=r->gpuBuffer) return reinterpret_cast<uintptr_t>(h->indices);
    h->indices=nullptr;
    return Device<0x1a0>(h,static_cast<void*>(nullptr));
}
uint32_t b05::Draw(Host* h,uint32_t type,uint32_t start,uint32_t count) {
    ++h->draws;
    return Device<0x144>(h,primitives[type],start,count);
}
uint32_t b05::DrawIndexed(Host* h,uint32_t type,int32_t base,uint32_t min,uint32_t vertices,uint32_t start,uint32_t count) {
    ++h->draws;
    return Device<0x148>(h,primitives[type],base,min,vertices,start,count);
}
uint8_t b05::Flag(Host* h) { return h->flag; }
}
