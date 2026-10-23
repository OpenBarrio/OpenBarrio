#include "ts3/anim/m05_M38.h"
#include "ts3/anim/m05_M23.h"
#include "ts3/resourcesystem/clu_4a0a80_ResourceSystem.h"
namespace ts3::anim::m38 {
namespace {
void* Table(uintptr_t address) {
#if defined(_M_IX86)
    return reinterpret_cast<void*>(address);
#else
    (void)address; return nullptr;
#endif
}
void Release(Controller* p) {
    if(p) {
        // Wrap just like ADD EAX,-1, including INT_MIN.
        p->refs=static_cast<int32_t>(static_cast<uint32_t>(p->refs)-1u);
        if(!p->refs) { p->refs=1; CallGameVirtual<void,0>(p,1u); }
    }
}
Blend* Owner(void* p) { return reinterpret_cast<Blend*>(static_cast<unsigned char*>(p)-offsetof(Controller,secondaryC)); }
}
Blend* DeleteSecondary(void* p,uint32_t flags) { return Delete(Owner(p),flags); }
void Destroy(Blend* p) {
    p->base.vtable=Table(0x00fc1d98); p->base.secondary8=Table(0x00fc1d8c); p->base.secondaryC=Table(0x00fc1d88);
    Release(p->second); Release(p->first);
    m23::DestroyController(&p->base);
}
b01::Node* Evaluate(Blend* p,void* context) {
    float secondWeight=p->weight;
    float firstWeight=1.0f-secondWeight;
    b01::Node* first=nullptr; b01::Node* second=nullptr;
    if(p->first && firstWeight>0.0f) first=CallGameVirtual<b01::Node*,3>(p->first,context);
    if(p->second && secondWeight>0.0f) second=CallGameVirtual<b01::Node*,3>(p->second,context);
    // FLD/FSTP quiets signaling NaNs; a selected child always has positive weight.
    if(first) b01::SetWeight(first,firstWeight);
    if(second) b01::SetWeight(second,secondWeight);
    if(!first) return second;
    if(!second) return first;
    auto* factory=static_cast<resourcesystem::ResourceObjectPool*>(context)->Create(0xd3dfc020u);
    auto* result=CallGameVirtual<b01::Node*,1>(factory,0xd3dfc020u);
    CallGameVirtual<void,3>(result,first);
    CallGameVirtual<void,3>(result,second);
    return result;
}
uint8_t Load(Blend* p,b09impl::Reader* s) {
    uint32_t version=~0u;
    auto status=CallGameFunction<uintptr_t,0x004dd700>(s->source,&version,s->context);
    CallGameMethod<uintptr_t,0x007d77c0>(s,status);
    if(!s->good || version>1) return 0;
    auto* first=b09impl::ReadController(p->base.hostSlot,s);
    CallGameMethod<void,0x00cf3c70>(&p->first,first);
    auto* second=b09impl::ReadController(p->base.hostSlot,s);
    CallGameMethod<void,0x00cf3c70>(&p->second,second);
    status=CallGameFunction<uintptr_t,0x004dd700>(s->source,&p->value30,s->context);
    CallGameMethod<uintptr_t,0x007d77c0>(s,status);
    if(!s->good) return 0;
    status=CallGameFunction<uintptr_t,0x004dd700>(s->source,&p->weight,s->context);
    CallGameMethod<uintptr_t,0x007d77c0>(s,status);
    return s->good;
}
Blend* Delete(Blend* p,uint32_t flags) {
    Destroy(p);
    if(flags&1) CallGameFunction<void,0x004e3f70>(p);
    return p;
}
void* DeleteOtherSecondary(void* p,uint32_t flags) { return CallGameMethod<void*,0x0056c6b0>(Owner(p),flags); }
}
#if defined(_M_IX86)
namespace ts3::anim::m38::hooks {
Blend* __fastcall DeleteSecondary(void* p,void*,uint32_t f) {return m38::DeleteSecondary(p,f);}
void __fastcall Destroy(Blend* p,void*) {m38::Destroy(p);}
b01::Node* __fastcall Evaluate(Blend* p,void*,void* c) {return m38::Evaluate(p,c);}
uint8_t __fastcall Load(Blend* p,void*,b09impl::Reader* s) {return m38::Load(p,s);}
Blend* __fastcall Delete(Blend* p,void*,uint32_t f) {return m38::Delete(p,f);}
void* __fastcall DeleteOtherSecondary(void* p,void*,uint32_t f) {return m38::DeleteOtherSecondary(p,f);}
}
#endif
