#include "ts3/anim/m05_M94.h"
#include <intrin.h>
namespace ts3::anim::m94 {
void SetVector(LookAtTask* self,const Float4Value* value) {
    if (!(self->enableBits & 1)) static_cast<LookAtTarget*>(self->target)->vector=*value;
}
LookAtTask* __stdcall Create(void* source,int32_t mode,uint32_t argument) {
    if (mode<0) return nullptr;
    auto p=CallGameFunction<LookAtTask*,0x004e3f90>(uint32_t(0x40),uintptr_t(0x00fcb6d0),0,0,0,0);
    if(p) p=CallGameMethod<LookAtTask*,0x00580480>(p,source,mode,argument);
    ++p->refs; // EA deliberately dereferences null after allocation failure.
    return p;
}
LookAtTask* Destroy(LookAtTask* self,uint32_t flags) {
#if defined(_M_IX86)
    self->vtable=reinterpret_cast<void*>(0x00fcb6cc);
#endif
    CallGameFunction<void,0x004e3f70>(self->target);
    self->target=nullptr;
#if defined(_M_IX86)
    self->vtable=reinterpret_cast<void*>(0x0107396c);
#endif
    if(flags&1) CallGameFunction<void,0x004e3f70>(self);
    return self;
}
State* Initialize(State* self) {
    self->previous=self; self->next=self; self->value=0.0f; self->descriptor=nullptr;
    return self;
}
void* Resolve(LookAtTarget* self) {
    if (!(self->identifierLow|self->identifierHigh)) return nullptr;
    auto manager=CallGameFunction<void*,0x00938b50>();
    if(!manager) return nullptr;
    auto resource=CallGameMethod<ResourcePrefix*,0x00939560>(manager,self->identifierLow,self->identifierHigh);
    if(!resource) return nullptr;
    _InterlockedExchangeAdd(reinterpret_cast<volatile long*>(&resource->atomicRefs),1);
    uint32_t key=0;
#if defined(_M_IX86)
    key=*reinterpret_cast<const uint32_t*>(0x011e9114);
#endif
    auto component=CallGameMethod<void*,0x00935770>(resource,key);
    void* result=nullptr;
    if(component) {
        auto id=CallGameVirtual<uint32_t,1>(self);
        auto object=CallGameMethod<void*,0x00951f00>(component,id);
        if(object) result=CallGameVirtual<void*,3>(object,uint32_t(1));
    }
    if(_InterlockedExchangeAdd(reinterpret_cast<volatile long*>(&resource->atomicRefs),-1)==1) {
        _InterlockedExchange(reinterpret_cast<volatile long*>(&resource->atomicRefs),1);
        CallGameVirtual<void,0>(resource,uint32_t(1));
    }
    return result;
}
void Populate(LookAtTask* self,Output* out) {
    if(!self->target) return;
    auto target=static_cast<LookAtTarget*>(self->target);
    if(CallGameVirtual<void*,0>(target)) {
        auto source=CallGameVirtual<const uint32_t*,0>(target);
        out->low=source[2]; out->high=source[3];
    }
    if(Resolve(target)) {
        auto object=Resolve(target);
        if(static_cast<uint8_t>(CallGameVirtual<uint32_t,4>(object))) {
            auto source=CallGameVirtual<void*,0>(target);
            out->index=CallGameMethod<uint32_t,0x0080d990>(object,source);
        }
    }
}
}
