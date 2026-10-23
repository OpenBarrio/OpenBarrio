#include "ts3/anim/m05_M44.h"
#include "ts3/layout.h"
namespace ts3::anim::m44 {
namespace {
void* Address(uintptr_t a) {
#if defined(_M_IX86)
    return reinterpret_cast<void*>(a);
#else
    (void)a; AbortNotRewritten("m44 game vtables"); return nullptr;
#endif
}
void Tables(RandomController* s) {
    s->base.vtable=Address(0xfc1fd8);
    s->base.secondary8=Address(0xfc1fd0);
    s->base.secondaryC=Address(0xfc1fcc);
    s->interface28=Address(0xfc1fb8);
}
void Release(ResourcePrefix* p) {
    if(p && --p->atomicRefs==0) {
        p->atomicRefs=1;
        CallGameVirtual<void,0>(p,1u);
    }
}
void Free(void* p) { if(p) CallGameFunction<void,0x4e3f70>(p); }
}
RandomController* Construct(RandomController* s,void* host) {
    CallGameMethod<void,0x565a80>(&s->base,host);
    Tables(s);
    s->sequence=s->current=nullptr; s->listener=nullptr;
    s->clips={nullptr,nullptr,nullptr,Address(0xfc1f90)};
    s->keys={nullptr,nullptr,nullptr,Address(0xfc1f6c)};
    s->repeats={nullptr,nullptr,nullptr,Address(0xfc1f44)};
    s->weights={nullptr,nullptr,nullptr,Address(0xfc1f1c)};
    s->time=0; s->delay=0; s->pending=0; s->busy=0;
    auto p=CallGameFunction<void*,0x4e3f90>(0x70u,Address(0xfc1ef4),0,0,0,0);
    auto next=p?CallGameMethod<ResourcePrefix*,0x567480>(p,host,-1):nullptr;
    if(next!=s->sequence) {
        if(next) ++next->atomicRefs;
        auto old=s->sequence; s->sequence=next; Release(old);
    }
    return s;
}
uint32_t TypeId() { return 0x06c12d61; }
void* Query(void* self,uint32_t id) {
    if(id==0x06c12d61 || id==0x7489d72b)
        return static_cast<unsigned char*>(self)-offsetof(Controller,secondary8);
    return nullptr;
}
void* DeleteInterface(void* self,uint32_t flags) {
    auto s=static_cast<unsigned char*>(self)-offsetof(Controller,secondaryC);
    return CallGameMethod<void*,0x56d780>(s,flags);
}
uintptr_t Destruct(RandomController* s) {
    Tables(s);
    CallGameMethod<void,0x56d180>(s,0);
    Free(s->weights.begin); Free(s->repeats.begin); Free(s->keys.begin);
    CallGameMethod<void,0x5af640>(&s->clips,s->clips.begin,s->clips.end);
    Free(s->clips.begin);
    if(s->listener) CallGameVirtual<void,1>(s->listener);
    Release(s->current); Release(s->sequence);
    return CallGameMethod<uintptr_t,0x565b40>(&s->base);
}
uintptr_t Notify(void* self,uintptr_t,uintptr_t) {
    auto s=reinterpret_cast<RandomController*>(static_cast<unsigned char*>(self)-offsetof(RandomController,interface28));
    s->pending=static_cast<int32_t>(static_cast<uint32_t>(s->pending)-1u);
    return CallGameMethod<uintptr_t,0x56d470>(s);
}
#if defined(_M_IX86)
RandomController* __fastcall HookConstruct(RandomController* s,void*,void* h) { return Construct(s,h); }
uint32_t __fastcall HookTypeId(void*,void*) { return TypeId(); }
void* __fastcall HookQuery(void* s,void*,uint32_t id) { return Query(s,id); }
void* __fastcall HookDelete(void* s,void*,uint32_t f) { return DeleteInterface(s,f); }
uintptr_t __fastcall HookDestruct(RandomController* s,void*) { return Destruct(s); }
uintptr_t __fastcall HookNotify(void* s,void*,uintptr_t a,uintptr_t b) { return Notify(s,a,b); }
#endif
}
