#include "ts3/anim/m05_M28.h"
#include <atomic>
namespace ts3::anim::m28 {
namespace {
template<int Slot> void Dispatch(Controller* s,void* event) {
    ++s->value18;
    auto* sentinel=&s->delegates;
    auto* p=sentinel->next;
    while(p!=sentinel) {
        auto* node=reinterpret_cast<b08::DelegateNode*>(p);
        void* target=node->target;
        p=p->next; // callbacks may tombstone or unlink the current node
        if(target) CallGameVirtual<void,Slot>(target,event,node->payload);
    }
    b08::Leave(reinterpret_cast<b08::DelegateList*>(&s->secondaryC));
}
void* Table(uintptr_t address) {
#if defined(_M_IX86)
    return reinterpret_cast<void*>(address);
#else
    (void)address; return nullptr;
#endif
}
}
void Dispatch0(Controller* s,void* e) { Dispatch<0>(s,e); }
void Dispatch1(Controller* s,void* e) { Dispatch<1>(s,e); }
void Dispatch2(Controller* s,void* e) { Dispatch<2>(s,e); }
void Dispatch4(Controller* s,void* e) { Dispatch<4>(s,e); }
void Destroy(Clip* s) {
    s->base.vtable=Table(0x00fc1b48);
    s->base.secondary8=Table(0x00fc1b40);
    s->base.secondaryC=Table(0x00fc1b3c);
    if(s->retained) {
        std::atomic_ref<int32_t> refs(s->retained->atomicRefs);
        if(refs.fetch_sub(1)==1) {
            refs.exchange(1);
            CallGameVirtual<void,0>(s->retained,1u);
        }
    }
    if(s->resource1) CallGameVirtual<void,1>(s->resource1);
    if(s->resource0) CallGameVirtual<void,1>(s->resource0);
    CallGameMethod<void,0x00578a00>(&s->ik);
    CallGameMethod<void,0x00565b40>(&s->base);
}
Clip* DeleteInterface(void* s,uint32_t flags) {
    // Same adjusted-this idiom as ResourceBaseDestroySecondary; different offset/callee.
    auto* owner=reinterpret_cast<Clip*>(static_cast<unsigned char*>(s)-offsetof(Controller,secondaryC));
    return CallGameMethod<Clip*,0x005688e0>(owner,flags);
}
}
