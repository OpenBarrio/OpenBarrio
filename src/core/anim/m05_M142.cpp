#include "ts3/anim/m05_M142.h"
namespace ts3::anim::m142 {
namespace {
struct Ref { void* vtable; uint32_t count; };
uint32_t& Count(void* p) { return static_cast<Ref*>(p)->count; }
void* Delegates(void* p) { return static_cast<unsigned char*>(p)+sizeof(void*)+8; }
uintptr_t Release(void* p) {
    uint32_t count = --Count(p);
    if (count) return count;
    Count(p)=1;
    return CallGameVirtual<uintptr_t,0>(p,1u);
}
void Tables(Body* self) {
#if defined(_M_IX86)
    self->vtable=reinterpret_cast<void*>(0x01027390);
    self->secondary=reinterpret_cast<void*>(0x0102738c);
    self->delegate=reinterpret_cast<void*>(0x01027378);
#else
    (void)self; NotRewritten("M142 original body vtables");
#endif
}
void Detach(Body* self) {
    if (!self->controller) return;
    CallGameMethod<void,0x00566590>(Delegates(self->controller),&self->delegate);
    void* old=self->controller;
    self->controller=nullptr;
    if (old) Release(old);
}
}
uintptr_t SetController(Body* self, void* source, uint32_t event) {
    Detach(self);
    void* memory=CallGameFunction<void*,0x004e3f90>(0x38u,
#if defined(_M_IX86)
        reinterpret_cast<void*>(0x00ffed50),
#else
        static_cast<void*>(nullptr),
#endif
        0u,0u,0u,0u);
    void* next=nullptr;
    if (memory) {
        float scale;
#if defined(_M_IX86)
        scale=*reinterpret_cast<const float*>(0x01020a7c);
#else
        NotRewritten("M142 time-scale constant");
#endif
        // The source controller field is a pointer, not a 32-bit scalar.
        void* sourceController=*reinterpret_cast<void**>(static_cast<unsigned char*>(source)+sizeof(void*)+0x18);
        next=CallGameMethod<void*,0x0056f830>(memory,sourceController,source,scale);
        if (next) ++Count(next);
    }
    CallGameMethod<void,0x00566170>(next?Delegates(next):nullptr,&self->delegate,event);
    uintptr_t result=CallGameVirtual<uintptr_t,5>(self->host,10000u,next,2u,0.0f,0.0f,0u,0u,0u,0u);
    void* old=self->controller;
    if (next!=old) {
        if (next) ++Count(next);
        self->controller=next;
        if (old) result=Release(old);
    }
    if (next) result=Release(next);
    return result;
}
Body* Construct(Body* self, void* baseArgument, void* host) {
    CallGameMethod<void,0x008397e0>(self,baseArgument);
    Tables(self);
    CallGameMethod<void,0x007513f0>(self->state);
    CallGameMethod<void,0x0085a900>(self->listener);
    self->controller=nullptr; self->host=host; self->flags=0;
    CallGameMethod<void,0x008399e0>(self,self->listener);
    CallGameVirtual<void,11>(self->host,1u);
    return self;
}
uint32_t TypeId(void*) { return 0x06fafaf6u; }
void* Query(void* self, uint32_t id) {
    if (id==0x06fafaf6u) return self;
    if (id==0x54cfb94fu) return self;
    return nullptr;
}
void* DeleteSecondary(void* self, uint32_t flags) {
    return CallGameMethod<void*,0x008555c0>(static_cast<unsigned char*>(self)-sizeof(void*),flags);
}
uintptr_t Destruct(Body* self) {
    Tables(self);
    CallGameVirtual<void,11>(self->host,0u);
    Detach(self);
    CallGameMethod<void,0x00839a10>(self,self->listener);
    if (self->controller) Release(self->controller);
    CallGameMethod<void,0x0085a7d0>(self->listener);
    CallGameMethod<void,0x00754060>(self->state);
    return CallGameMethod<uintptr_t,0x00839750>(self);
}
}
#if defined(_M_IX86)
namespace ts3::anim::m142 {
uintptr_t __fastcall HookSet(Body* p,void*,void* source,uint32_t event) { return SetController(p,source,event); }
Body* __fastcall HookConstruct(Body* p,void*,void* base,void* host) { return Construct(p,base,host); }
uint32_t __fastcall HookType(void* p,void*) { return TypeId(p); }
void* __fastcall HookQuery(void* p,void*,uint32_t id) { return Query(p,id); }
void* __fastcall HookDelete(void* p,void*,uint32_t flags) { return DeleteSecondary(p,flags); }
uintptr_t __fastcall HookDestruct(Body* p,void*) { return Destruct(p); }
}
#endif
