#include "ts3/render/m07_B11.h"
#include <atomic>
namespace ts3::render {
namespace {
void* Table(uint32_t address) {
#if defined(_M_IX86)
    return reinterpret_cast<void*>(uintptr_t(address));
#else
    (void)address;
    throw std::runtime_error("M07 B11 game vtable unavailable on native host");
#endif
}
void* Device() {
#if defined(_M_IX86)
    return *reinterpret_cast<void**>(0x011CECCC);
#else
    throw std::runtime_error("M07 B11 device global unavailable on native host");
#endif
}
uint32_t Format(uint32_t index) {
#if defined(_M_IX86)
    return reinterpret_cast<uint32_t*>(0x011CECE0)[index];
#else
    (void)index;
    throw std::runtime_error("M07 B11 format global unavailable on native host");
#endif
}
template<class R,class... A> R Com(void* object,size_t slot,A... args) {
    using Fn=m07::ComMethod<R,void,A...>;
    return reinterpret_cast<Fn>((*static_cast<void***>(object))[slot])(object,args...);
}
void Tables(B11Buffer* p,uint32_t primary,uint32_t secondary) {
    p->vtable=Table(primary); p->iface.vtable=Table(secondary);
}
void BaseEnd(B11Buffer* p) {
    ts3::CallGameMethod<void,0x00613F00>(&p->iface);
    p->vtable=Table(0x00FF39F8);
}
B11Buffer* Finish(B11Buffer* p,uint32_t flags) {
    if(flags&1) ts3::CallGameFunction<void,0x004E3F70>(p);
    return p;
}
uint32_t Bytes(B11Interface* p) {
    uint32_t bits=ts3::CallGameFunction<uint32_t,0x00618470>(p->format);
    return (bits*p->height*p->width)>>3;
}
}
void B11Cleanup(B11Buffer* p) { ts3::CallGameMethod<void,0x00617960>(p); }
void B11Adopt(B11Buffer* p,void* surface,uint32_t tag) {
    ts3::CallGameMethod<void,0x00617A70>(p,surface,tag);
}
bool B11Create(B11Interface* p) {
    const uint32_t format=Format(p->format);
    if(!format) return false;
    if(p->handle) return true;
    int32_t hr;
    if(p->flags&0x10)
        hr=Com<int32_t>(Device(),0x74/4,p->width,p->height,format,0u,0u,1u,&p->handle,static_cast<void*>(nullptr));
    else
        hr=Com<int32_t>(Device(),0x70/4,p->width,p->height,format,0u,0u,(p->flags>>5)&1u,&p->handle,static_cast<void*>(nullptr));
    return hr>=0;
}
void B11Destroy(B11Buffer* p) {
    Tables(p,0xFE9468,0xFE9450); B11Cleanup(p); BaseEnd(p);
}
bool B11Copy(B11Buffer* dst,const B11Rect* dr,B11Buffer* src,const B11Rect* sr,uint8_t linear) {
    if(!dst->iface.handle || !src->iface.handle) return false;
    uint32_t dw=dst->iface.width,dh=dst->iface.height,sw=src->iface.width,sh=src->iface.height;
    B11Rect d{},s{};
    if(dr) {
        if(dr->left<0 || dr->top<0 || dr->right>static_cast<int32_t>(dw) || dr->bottom>static_cast<int32_t>(dh)) return false;
        d=*dr; dr=&d; dw=uint32_t(d.right)-uint32_t(d.left); dh=uint32_t(d.bottom)-uint32_t(d.top);
    }
    if(sr) {
        if(sr->left<0 || sr->top<0 || sr->right>static_cast<int32_t>(sw) || sr->bottom>static_cast<int32_t>(sh)) return false;
        s=*sr; sr=&s; sw=uint32_t(s.right)-uint32_t(s.left); sh=uint32_t(s.bottom)-uint32_t(s.top);
    }
    const bool stretch=dw!=sw || dh!=sh;
    if(!(dst->iface.flags&1) && ((src->iface.flags&1) || ((src->iface.flags|dst->iface.flags)&0x100) || stretch)) return false;
    void* device=Device(); if(!device) return false;
    uint32_t filter=stretch?(linear?2u:1u):0u;
    return Com<int32_t>(device,0x88/4,src->iface.handle,sr,dst->iface.handle,dr,filter)>=0;
}
B11Buffer* B11DeleteAdjusted(B11Interface* p,uint32_t flags) {
    return B11DeleteDerived(reinterpret_cast<B11Buffer*>(reinterpret_cast<char*>(p)-offsetof(B11Buffer,iface)),flags);
}
uint32_t B11SystemBytes(B11Interface* p) { return (p->flags&0x102)?Bytes(p):0; }
uint32_t B11VideoBytes(B11Interface* p) { return (p->flags&0x100)?0:Bytes(p); }
B11Buffer* B11Construct(B11Buffer* p) {
    p->vtable=Table(0xFF39F8);
    reinterpret_cast<std::atomic<uint32_t>*>(&p->refs)->exchange(0);
    ts3::CallGameMethod<void,0x00613EE0>(&p->iface);
    Tables(p,0xFE9468,0xFE9450); p->iface.handle=nullptr; return p;
}
B11Buffer* B11Delete(B11Buffer* p,uint32_t flags) { B11Destroy(p); return Finish(p,flags); }
B11Buffer* B11ConstructDerived(B11Buffer* p) {
    p->vtable=Table(0xFF39F8);
    reinterpret_cast<std::atomic<uint32_t>*>(&p->refs)->exchange(0);
    ts3::CallGameMethod<void,0x00613EE0>(&p->iface);
    Tables(p,0xFE9498,0xFE9480); p->iface.handle=nullptr; return p;
}
B11Buffer* B11DeleteDerived(B11Buffer* p,uint32_t flags) {
    Tables(p,0xFE9498,0xFE9480); B11Cleanup(p);
    B11Destroy(p); return Finish(p,flags);
}
void B11FormatDestroy(m07::VertexFormat* p) {
    p->primary=Table(0xFE9580); p->secondary=Table(0xFE9568);
    ts3::CallGameMethod<void,0x00619E10>(p);
    ts3::CallGameMethod<void,0x00613F00>(&p->secondary);
    p->primary=Table(0xFF39F8);
}
void* B11FormatDeleteAdjusted(void* p,uint32_t flags) {
    auto base=reinterpret_cast<char*>(p)-offsetof(m07::VertexFormat,secondary);
    return ts3::CallGameMethod<void*,0x0061A340>(base,flags);
}
}
