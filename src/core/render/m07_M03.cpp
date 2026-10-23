#include "ts3/render/m07_M03.h"
#include <initializer_list>
namespace ts3::render::m07::m03 {
Dependencies dependencies;
namespace {
uint32_t Count(uint8_t* b,uint8_t* e,uint32_t stride) {
    // Original subtracts in 32 bits and uses signed division, then unsigned loop bounds.
    return uint32_t(int32_t(uint32_t(reinterpret_cast<uintptr_t>(e)-reinterpret_cast<uintptr_t>(b)))/int32_t(stride));
}
int32_t Tell(Context& c) {
    auto fn=reinterpret_cast<Thiscall<int32_t,void,uint32_t>>((*static_cast<void***>(c.stream))[9]);
    return CallThis(fn,c.stream,0u);
}
uint8_t Seek(Context& c,int32_t p) {
    auto fn=reinterpret_cast<Thiscall<uint8_t,void,int32_t,uint32_t>>((*static_cast<void***>(c.stream))[10]);
    return CallThis(fn,c.stream,p,0u);
}
void Word(Context& c,uint32_t value) {
    c.success &= Original(0x004DDBD0,dependencies.word)(c.stream,value,c.mode);
}
void Pair(Context& c,uint32_t low,uint32_t high) {
    c.success &= Original(0x004DDD20,dependencies.pair)(c.stream,low,high,c.mode);
}
Fixup* Allocate(uint32_t count) {
    if(!count) return nullptr;
    auto* p=static_cast<Fixup*>(Original(0x004E3F90,dependencies.allocate)(count*16u,"Renderer/ModelBuilder",0,0,
#if defined(_M_IX86)
        reinterpret_cast<const char*>(0x00F98980),
#else
        "ModelBuilder",
#endif
        231));
    // Deliberately not an allocation failure recovery path (EA continues walking).
    for(uint32_t i=0;i<count;++i) {
        auto* q=reinterpret_cast<Fixup*>(reinterpret_cast<uintptr_t>(p)+uintptr_t(i)*16);
        if(q) *q=Fixup{};
    }
    return p;
}
void Reserve(Context& c,Fixup& f) {
    f.first=Tell(c); Word(c,0);
    f.second=Tell(c); Word(c,0);
}
void Patch(Context& c,int32_t where,uint32_t size) {
    const int32_t end=Tell(c);
    if(end==-1 || !Seek(c,where)) { c.success=0; return; }
    Word(c,size);
    if(!Seek(c,end)) c.success=0;
}
void Patch(Context& c,const Fixup& f) { Patch(c,f.first,f.firstSize); Patch(c,f.second,f.secondSize); }
}
uint8_t __fastcall Build(Builder* self,void*,void* output,const Metadata* meta) {
    Context c;
    CallThis(Original(0x007D91B0,dependencies.construct),&c,output,1u);
    const uint32_t ns=Count(self->streams,self->streamsEnd,72);
    const uint32_t nc=Count(self->channels,self->channelsEnd,12);
    const uint32_t nv=Count(self->vertices,self->verticesEnd,24);
    const uint32_t total=CallThis(Original(0x006D0C30,dependencies.count),self);
    Word(c,1); Word(c,1); Word(c,0); Word(c,0); Word(c,total);
    CallThis(Original(0x006D08F0,dependencies.header),self,&c,0x01661233u,meta->version,meta);
    for(uint32_t i=0;i<nv;++i) { const uint32_t v=meta->version; Pair(c,i,0); Word(c,0x01D10F34); Word(c,v); }
    for(uint32_t tag : {0x01D0E6FBu,0x01D0E70Fu,0u}) {
        const uint32_t v=meta->version; Pair(c,meta->low,meta->high); Word(c,tag); Word(c,v);
    }
    for(uint32_t i=0;i<ns;++i) { const uint32_t v=meta->version; Pair(c,i,0); Word(c,0x01D0E723); Word(c,v); }
    for(uint32_t i=0;i<nc;++i) { const uint32_t v=meta->version; Pair(c,i,0); Word(c,0x01D0E75D); Word(c,v); }
    Fixup* streams=Allocate(ns);
    Fixup* channels=Allocate(nc);
    Fixup* vertices=Allocate(nv);
    Fixup base,positions,normals,extras;
    Reserve(c,base);
    for(uint32_t i=0;i<nv;++i) Reserve(c,vertices[i]);
    Reserve(c,positions); Reserve(c,normals);
    Reserve(c,extras);
    for(uint32_t i=0;i<ns;++i) Reserve(c,streams[i]);
    for(uint32_t i=0;i<nc;++i) Reserve(c,channels[i]);
    uint32_t offset=24u*total+20u;
    CallThis(Original(0x006D1A80,dependencies.base),self,&c,&offset,&base);
    for(uint32_t i=0;i<nv;++i) CallThis(Original(0x006D2250,dependencies.vertex),self,&c,i,&offset,vertices+i);
    CallThis(Original(0x006D0CA0,dependencies.positions),self,&c,&offset,&positions);
    CallThis(Original(0x006D0DA0,dependencies.normals),self,&c,&offset,&normals);
    CallThis(Original(0x006D0F20,dependencies.extras),self,&c,&offset,&extras);
    for(uint32_t i=0;i<ns;++i) CallThis(Original(0x006D12B0,dependencies.stream),self,&c,&offset,streams+i,i);
    for(uint32_t i=0;i<nc;++i) CallThis(Original(0x006D13E0,dependencies.channel),self,&c,&offset,channels+i,i);
    Patch(c,base);
    for(uint32_t i=0;i<nv;++i) Patch(c,vertices[i]);
    Patch(c,positions); Patch(c,normals); Patch(c,extras);
    for(uint32_t i=0;i<ns;++i) Patch(c,streams[i]);
    for(uint32_t i=0;i<nc;++i) Patch(c,channels[i]);
    auto release=Original(0x004E3F70,dependencies.free);
    if(vertices) release(vertices);
    if(channels) release(channels);
    if(streams) release(streams);
    CallThis(Original(0x007D7760,dependencies.destroy),&c);
    return 1;
}
}
