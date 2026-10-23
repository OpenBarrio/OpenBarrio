#include "ts3/anim/m05_M50.h"
#include "ts3/anim/m05_B08.h"
#include "ts3/anim/m05_M23.h"
#include "ts3/scratch/clu_4e3f70_Scratch.h"
namespace ts3::anim::m50 {
namespace {
void* Table(uint32_t ea) {
#if defined(_M_IX86)
    return reinterpret_cast<void*>(ea);
#else
    (void)ea; NotRewritten("M50 original controller tables");
#endif
}
void Tables(Object* s) {
    s->base.vtable=Table(0xFC2128); s->base.secondary8=Table(0xFC211C);
    s->base.secondaryC=Table(0xFC2118); s->interface28=Table(0xFC2104);
}
}
Object* DeleteInterface(void* self,uint32_t flags) {
    return Delete(reinterpret_cast<Object*>(static_cast<unsigned char*>(self)-offsetof(Controller,secondaryC)),flags);
}
Object* Construct(Object* self,void* host,Controller* child) {
    b08::Construct(&self->base,host); Tables(self); self->child=child;
    if(child) child->refs=static_cast<int32_t>(static_cast<uint32_t>(child->refs)+1u);
    return self;
}
uint8_t Load(Object* self,b09impl::Reader* reader) {
    uint32_t version=~0u;
    auto status=CallGameFunction<uintptr_t,0x004DD700>(reader->source,&version,reader->context);
    CallGameMethod<void,0x007D77C0>(reader,status);
    if(!reader->good || version>1) return 0;
    auto child=b09impl::ReadController(self->base.hostSlot,reader);
    CallGameMethod<void,0x00CF3C70>(&self->child,child);
    if(!self->child) return 0;
    return reader->good;
}
Object* Delete(Object* self,uint32_t flags) {
    Tables(self);
    if(auto child=self->child) {
        child->refs=static_cast<int32_t>(static_cast<uint32_t>(child->refs)-1u);
        if(!child->refs) { child->refs=1; CallGameVirtual<void,0>(child,1u); }
    }
    m23::DestroyController(&self->base);
    if(flags&1) ts3::scratch::Free4E3F70(self);
    return self;
}
uint8_t Query(Forwarder* self) {
    return self->child?CallGameVirtual<uint8_t,12>(self->child):0;
}
void Reset(Forwarder* self) {
    if(self->child) CallGameVirtual<void,1>(self->child);
}
#if defined(_M_IX86)
namespace hooks {
Object* __fastcall DeleteInterface(void* p,void*,uint32_t f){return m50::DeleteInterface(p,f);}
Object* __fastcall Construct(Object* p,void*,void* h,Controller* c){return m50::Construct(p,h,c);}
uint8_t __fastcall Load(Object* p,void*,b09impl::Reader* r){return m50::Load(p,r);}
Object* __fastcall Delete(Object* p,void*,uint32_t f){return m50::Delete(p,f);}
uint8_t __fastcall Query(Forwarder* p,void*){return m50::Query(p);}
void __fastcall Reset(Forwarder* p,void*){m50::Reset(p);}
}
#endif
}
