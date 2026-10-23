#include "ts3/render/m07_B16.h"
#include "ts3/ui/clu_6e8410_UI.h"
#include <cstring>
namespace ts3::render::m07::b16 {
namespace {
void* Address(uint32_t a) {
#if defined(_M_IX86)
return reinterpret_cast<void*>(uintptr_t(a));
#else
(void)a; throw std::runtime_error("M07 image datum unavailable on native host");
#endif
}
void* Heap() { return *static_cast<void**>(Address(0x011CB864)); }
void FreeCode(void* p) { CallGameMethod<void,0x004E5920>(Heap(),p); }
void Release(void* p) { auto f=reinterpret_cast<ComMethod<uint32_t,void>>((*static_cast<void***>(p))[2]); f(p); }
void Copy(float& d,const float& s) {
#if defined(_M_IX86)
const float* a=&s; float* b=&d;
__asm { mov eax,a }
__asm { fld dword ptr [eax] }
__asm { mov eax,b }
__asm { fstp dword ptr [eax] }
#else
uint32_t v; std::memcpy(&v,&s,4); if((v&0x7f800000)==0x7f800000 && (v&0x7fffff)) v|=0x400000; std::memcpy(&d,&v,4);
#endif
}
uint8_t Create(Shader* s,const void* input,unsigned slot) {
    if(!input) input=s->code;
    void* device=CallGameMethod<void*,0x0060F740>(s);
    auto fn=reinterpret_cast<ComMethod<int32_t,void,const void*,void**>>((*static_cast<void***>(device))[slot]);
    int32_t hr=fn(device,input,&s->handle);
    uint8_t ok=hr==0 && s->handle!=nullptr;
    if(s->code) { FreeCode(s->code); s->code=nullptr; s->bytes=0; }
    return ok;
}
Shader* Destroy(Shader* s,uint32_t flags,uint32_t table) {
    s->vtable=Address(table);
    if(table!=0x01015818 && s->handle) Release(s->handle);
    s->vtable=Address(0x01015818);
    if(s->code) { FreeCode(s->code); s->code=nullptr; }
    if(flags&1) CallGameFunction<void,0x004E3F70>(s);
    return s;
}
uint32_t Clone(Shader* s,Shader** out,uint32_t tag,uint32_t table) {
    auto* n=CallGameFunction<Shader*,0x004E3F90>(uint32_t(0x14),Address(tag),0,0,0,0);
    if(n) {
        CallGameMethod<Shader*,0x007998B0>(n,s);
        n->vtable=Address(table); n->handle=s->handle;
        if(n->handle) { auto f=reinterpret_cast<ComMethod<uint32_t,void>>((*static_cast<void***>(n->handle))[1]); f(n->handle); }
    }
    *out=n; return n!=nullptr;
}
}
int32_t __fastcall Records(Owner* s,void*,int32_t n,void** out,uint32_t* stride) {
    int32_t result=s->count<n?s->count:n; *out=reinterpret_cast<uint8_t*>(s)+0x64; *stride=0x60; return result;
}
uint32_t __fastcall Forward(Owner* s,void*,uint32_t i,uint32_t n,void** out,uint32_t* stride) {
    CallGameVirtual<int32_t,0>(s->entries[i],n,out,stride); return 1;
}
uint8_t __fastcall Resolve(void*,void*,uint32_t a,uint32_t b,uint32_t c,void** out) {
    struct Key { uint32_t a,b,type,c; } key{a,b,0x00b2d882,c};
    void* manager=CallGameFunction<void*,0x004AFD20>(); void* resource=reinterpret_cast<void*>(uintptr_t(c));
    if(CallGameVirtual<uint8_t,3>(manager,&key,&resource,0,0,0,0)) {
        void* p=resource?CallGameVirtual<void*,3>(resource,uint32_t(0xb3d7a7)):nullptr;
        *out=p?static_cast<uint8_t*>(p)+0x28:nullptr; return 1;
    }
    key.type=0x2f7d0004;
    void* image=CallGameFunction<void*,0x007E88D0>(&key,0);
    if(image) {
        CallGameVirtual<void,0>(image);
        void* holder=*reinterpret_cast<void**>(static_cast<uint8_t*>(image)+sizeof(void*));
        if(holder) {
            auto field=[&](){return *reinterpret_cast<void**>(static_cast<uint8_t*>(holder)+sizeof(void*));};
            if(!field()) CallGameVirtual<void,4>(holder);
            *out=field(); if(*out) CallGameVirtual<void,1>(*out);
        }
        CallGameVirtual<void,1>(image);
    }
    return 0;
}
void __fastcall Bounds(Owner* s,void*,int32_t i,const float* v,float r) {
    if(i<0 || i>=s->end-s->begin) return;
    Texture* p=s->begin+i;
    for(int j=3;j<6;++j) Copy(p->bounds[j],v[j]);
    for(int j=0;j<3;++j) Copy(p->bounds[j],v[j]);
    for(int j=0;j<3;++j) p->bounds[j]-=r;
    for(int j=3;j<6;++j) p->bounds[j]+=r;
    if(p->resource && !static_cast<ts3::ui::UiEntry*>(p->resource)->HasBinding()) {
        float scale=*reinterpret_cast<float*>(static_cast<uint8_t*>(p->resource)+0x7c);
        float expanded=scale*r;
        if(expanded>r) { float d=expanded-r; for(int j=0;j<3;++j) p->bounds[j]-=d; for(int j=3;j<6;++j) p->bounds[j]+=d; }
    }
}
uint32_t __fastcall Submit(Owner* s,void*,uint32_t n) {
    if(!n) return 0;
    void* manager=CallGameMethod<void*,0x006879A0>(s);
    CallGameVirtual<void,18>(s->context);
    auto* p=s->active+s->index;
    auto* key=reinterpret_cast<uint32_t*>(reinterpret_cast<uint8_t*>(p)+0x20);
    if(key[0]||key[1]) {
        void* store=CallGameFunction<void*,0x006FDE10>();
        void* item=CallGameMethod<void*,0x006FE3B0>(store,key[0],key[1]);
        if(!item) return 0;
        CallGameVirtual<void,29>(item);
        void* data=CallGameMethod<void*,0x006FB3A0>(item,0);
        if(!data || !(static_cast<uint8_t*>(data)[0x224]&0x80)) return 0;
        *reinterpret_cast<void**>(static_cast<uint8_t*>(s->state)+0x30)=data;
    }
    auto* record=reinterpret_cast<uint8_t*>(s)+0x64;
    for(uint32_t j=0;j<n;++j,record+=0x60) {
        CallGameMethod<void,0x006FD4D0>(s->context,record+0x40);
        if(manager) { float f; Copy(f,*reinterpret_cast<float*>(record+0x50)); CallGameMethod<void,0x00687DC0>(manager,s->context,f); }
        CallGameMethod<void,0x0071BF00>(reinterpret_cast<uint8_t*>(s)-0x9c,s->context,record);
    }
    *reinterpret_cast<void**>(static_cast<uint8_t*>(s->state)+0x30)=nullptr;
    return 0;
}
Storage* __fastcall InitStorage(Storage* s,void*) { s->begin=s->end=s->capacity=nullptr; s->tag=static_cast<const char*>(Address(0x010157F0)); s->begin2=s->end2=s->capacity2=nullptr; s->tag2=static_cast<const char*>(Address(0x010157D0)); return s; }
uint8_t __fastcall CreateVertex(Shader* s,void*,const void* p) { return Create(s,p,91); }
uint8_t __fastcall CreatePixel(Shader* s,void*,const void* p) { return Create(s,p,106); }
Shader* __fastcall DestroyBase(Shader* s,void*,uint32_t f) { return Destroy(s,f,0x01015818); }
Shader* __fastcall DestroyVertex(Shader* s,void*,uint32_t f) { return Destroy(s,f,0x01015830); }
Shader* __fastcall DestroyPixel(Shader* s,void*,uint32_t f) { return Destroy(s,f,0x01015848); }
uint32_t __fastcall CloneVertex(Shader* s,void*,Shader** p) { return Clone(s,p,0x00FFC50C,0x01015830); }
uint32_t __fastcall ClonePixel(Shader* s,void*,Shader** p) { return Clone(s,p,0x00FFC4F8,0x01015848); }
}
