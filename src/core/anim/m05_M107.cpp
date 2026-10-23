#include "ts3/anim/m05_M107.h"
#include "ts3/sacs/clu_582090_SACS.h"
#include <intrin.h>
#include <cstring>
namespace ts3::anim::m107 {
uintptr_t ReleaseStyle(Style* self) {
    uintptr_t result=reinterpret_cast<uintptr_t>(self->text.begin);
    const auto distance=static_cast<int32_t>((reinterpret_cast<uintptr_t>(self->text.capacity)-result)&~uintptr_t(1));
    if(distance>2 && self->text.begin) result=CallGameFunction<uintptr_t,0x004e3f70>(self->text.begin);
    for(int i=19;i>=0;--i) {
        Ref* ref=self->resources[i];
        if(ref) {
            result=reinterpret_cast<uintptr_t>(&ref->references);
            if(_InterlockedDecrement(&ref->references)==0) {
                _InterlockedExchange(&ref->references,1);
                result=CallGameVirtual<uintptr_t,0>(ref,uint32_t(1));
            }
        }
    }
    return result;
}
Style* CopyStyle(Style* self,const Style* other) {
    for(unsigned i=0;i<20;++i) {
        self->resources[i]=other->resources[i];
        if(self->resources[i]) _InterlockedExchangeAdd(&self->resources[i]->references,1);
    }
    for(unsigned i=0;i<5;++i) self->values[i]=other->values[i];
    for(unsigned i=0;i<6;++i) self->flags[i]=other->flags[i];
    CallGameMethod<void,0x00408eb0>(&self->text,&other->text);
    return self;
}
void ResetEntity(Entity* self) {
#if defined(_M_IX86)
    self->vtable=reinterpret_cast<void**>(0x01022de4);
    self->secondary=reinterpret_cast<void**>(0x0107396c);
#else
    self->vtable=nullptr; self->secondary=nullptr;
#endif
    self->owner=nullptr;
}
Entity* DestroySecondary(void* secondary,uint32_t flags) {
    // Twin 00409770: same subobject adjustment; target differs to 00839810.
    auto* self=reinterpret_cast<Entity*>(static_cast<unsigned char*>(secondary)-offsetof(Entity,secondary));
    ResetEntity(self);
    if(flags&1) CallGameFunction<void,0x004e3f70>(self);
    return self;
}
Entity* ConstructEntity(Entity* self,void* owner) {
#if defined(_M_IX86)
    self->vtable=reinterpret_cast<void**>(0x01022de4);
    self->secondary=reinterpret_cast<void**>(0x01022de0);
#else
    self->vtable=nullptr; self->secondary=nullptr;
#endif
    self->references=0;
    self->links.next=self->links.previous=&self->links;
    self->owner=owner;
    return self;
}
uintptr_t NullController(Definition* self,Request* request,Context* context,uintptr_t,uintptr_t) {
    uintptr_t result=reinterpret_cast<uintptr_t>(request);
    if(!request || !request->actor) return result;
    void* actor=request->actor;
    void* selector=self->selector;
    bool changed=context->changed!=0,transition=false;
    result=CallGameMethod<uintptr_t,0x007f3950>(context,request,selector);
    if(static_cast<uint8_t>(result)) {
        transition=(self->flags&0x60)!=0x60;
        if(selector) {
            result=CallGameMethod<uintptr_t,0x007f3850>(context,selector);
            actor=reinterpret_cast<void*>(result);
            // The original re-loads request into EAX only when the latch was clear.
            if(!changed) { result=reinterpret_cast<uintptr_t>(request); changed=actor!=request->actor; }
        }
    } else if(selector && selector!=*reinterpret_cast<void**>(static_cast<unsigned char*>(actor)+32)) return result;
    if(!actor) return result;
    void* host=CallGameMethod<void*,0x00581560>(actor,uint32_t(0));
    result=CallGameMethod<uintptr_t,0x00800220>(actor);
    bool inactive=!static_cast<uint8_t>(result) && (self->flags&1);
    void* source=self->source;
    if(!(self->flags&4)) { result=CallGameMethod<uintptr_t,0x007f37f0>(context); source=reinterpret_cast<void*>(result); }
    if(!host) return result;
    void* global=nullptr;
#if defined(_M_IX86)
    global=*reinterpret_cast<void**>(0x011dc484);
#endif
    auto* manager=CallGameMethod<unsigned char*,0x00935770>(host,global);
    void* memory=CallGameFunction<void*,0x004e3f90>(uint32_t(40),"Animation/NullController",uint32_t(0),uint32_t(0),uint32_t(0),uint32_t(0));
    Ref* controller=memory ? CallGameMethod<Ref*,0x00570f40>(memory,manager?manager+8:nullptr):nullptr;
    if(controller) ++controller->references;
    uint32_t mode=inactive?0:2;
    if(transition) {
        void* target=CallGameMethod<void*,0x007f36f0>(context);
        if(target) {
            if(changed) CallGameMethod<void,0x00582270>(target,controller,source,uint32_t(0),self->weight,-1.0f,uint32_t(0),uint32_t(0));
            else {
                ts3::sacs::UpdateEntry(target,nullptr,controller,static_cast<uint32_t>(reinterpret_cast<uintptr_t>(source)),mode,self->weight,-1.0f,0,0);
                context->changed=1;
            }
        }
    } else {
        void* target=CallGameMethod<void*,0x007f3740>(context);
        if(target) CallGameMethod<void,0x00582270>(target,controller,source,mode,self->weight,-1.0f,uint32_t(0),uint32_t(0));
        else CallGameVirtual<void,5>(manager+8,source,controller,mode,self->weight,-1.0f,uint32_t(0),uint32_t(0),uint32_t(0),uint32_t(0));
    }
    return CallGameMethod<uintptr_t,0x0055cfa0>(&controller);
}
}

#if defined(_M_IX86)
namespace ts3::anim::m107 {
uintptr_t __fastcall HookReleaseStyle(Style* s, void*) { return ReleaseStyle(s); }
Style* __fastcall HookCopyStyle(Style* s, void*, const Style* o) { return CopyStyle(s,o); }
uintptr_t __fastcall HookNullController(Definition* s, void*, Request* r, Context* c, uintptr_t a, uintptr_t b) { return NullController(s,r,c,a,b); }
void __fastcall HookResetEntity(Entity* s, void*) { return ResetEntity(s); }
Entity* __fastcall HookDestroySecondary(void* s, void*, uint32_t f) { return DestroySecondary(s,f); }
Entity* __fastcall HookConstructEntity(Entity* s, void*, void* o) { return ConstructEntity(s,o); }
}
#endif
