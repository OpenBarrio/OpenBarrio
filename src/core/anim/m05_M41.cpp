#include "ts3/anim/m05_M41.h"
namespace ts3::anim::m41 {
namespace {
void* Table(uint32_t a) {
#if defined(_M_IX86)
    return reinterpret_cast<void*>(uintptr_t(a));
#else
    (void)a; ts3::NotRewritten("M41 game vtable");
#endif
}
void Tables(Transform* p) {
    p->base.vtable=Table(0x00fc1e80);
    p->base.secondary8=Table(0x00fc1e74);
    p->base.secondaryC=Table(0x00fc1e70);
    p->delegateTable=Table(0x00fc1e5c);
}
}
uint32_t Type(void* p) {
    // Verified twin ResourceManClassId, differing only in the immediate.
    (void)p; return 0x06b26363u;
}
Transform* Query(void* p,uint32_t key) {
    return key==0x06b26363u ? reinterpret_cast<Transform*>(static_cast<unsigned char*>(p)-offsetof(Controller,secondary8)) : nullptr;
}
Transform* DeleteSecondary(void* p,uint32_t flags) {
    // Verified twin ResourceBaseDestroySecondary: adjustment is 12, target 0056CC90.
    auto* base=reinterpret_cast<Transform*>(static_cast<unsigned char*>(p)-offsetof(Controller,secondaryC));
    return CallGameMethod<Transform*,0x0056cc90>(base,flags);
}
Transform* Construct(Transform* p,void* host,Controller* child) {
    b08::Construct(&p->base,host);
    Tables(p);
    p->translation={}; p->rotation={}; p->rotation.lane[3]=1.0f;
    p->child=child;
    if(child) child->refs=static_cast<int32_t>(static_cast<uint32_t>(child->refs)+1u);
    if(p->child) CallGameMethod<void,0x00566170>(&p->child->secondaryC,&p->delegateTable,-1);
    return p;
}
uint8_t Load(Transform* p,b08::StreamState* s) {
    uint32_t version=~0u;
    auto status=CallGameFunction<uint32_t,0x004dd700>(s->handle,&version,s->extra);
    CallGameMethod<void,0x007d77c0>(s,status);
    if(!s->good || version>1u) return 0;
    CallGameMethod<void,0x007d79f0>(s,&p->rotation);
    if(!s->good) return 0;
    CallGameMethod<void,0x007d7930>(s,&p->translation);
    if(!s->good) return 0;
    auto* child=CallGameFunction<Controller*,0x00565c80>(p->base.hostSlot,s);
    CallGameMethod<void,0x00cf3c70>(&p->child,child);
    return s->good;
}
uintptr_t Destroy(Transform* p) {
    Tables(p);
    if(p->child) CallGameMethod<void,0x00566590>(&p->child->secondaryC,&p->delegateTable);
    auto* child=p->child; // callback may change the retained pointer
    if(child) {
        child->refs=static_cast<int32_t>(static_cast<uint32_t>(child->refs)-1u);
        if(!child->refs) {
            child->refs=1;
            CallGameVirtual<void*,0>(child,1u);
        }
    }
    return CallGameMethod<uintptr_t,0x00565b40>(&p->base);
}
}
