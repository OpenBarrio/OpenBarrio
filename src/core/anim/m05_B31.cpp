#include "ts3/anim/m05_B31.h"
#include <xmmintrin.h>
#if defined(_M_IX86)
#include <intrin.h>
#endif
namespace ts3::anim::b31 {
namespace {
uintptr_t Finish(Dispatcher* s) {
    uint32_t count=--s->refs;
    if(count) return count;
    s->refs=1;
    return CallGameVirtual<uintptr_t,0>(&s->refVtable,1u);
}
template<int Slot> uintptr_t Dispatch(Dispatcher* s, bool self) {
    ++s->refs;
    for(Link* p=s->listeners.next;p!=&s->listeners;p=p->next) {
        auto* observer=reinterpret_cast<Observer*>(reinterpret_cast<char*>(p)-offsetof(Observer,link));
        CallGameVirtual<uintptr_t,Slot>(observer);
    }
    if(self) CallGameVirtual<uintptr_t,6>(s);
    return Finish(s);
}
const void* Address(uintptr_t p) {
#if defined(_M_IX86)
    return reinterpret_cast<const void*>(p);
#else
    (void)p; static const uintptr_t nativeTable[4]={}; return nativeTable;
#endif
}
void Release(void* p) {
    if(!p) return;
    auto* r=reinterpret_cast<Dispatcher*>(p);
    if(--r->refs==0) {
        r->refs=1;
        CallGameVirtual<void,0>(&r->refVtable,1u);
    }
}
}
uintptr_t __fastcall NotifyAndSelf(Dispatcher* s,void*) { return Dispatch<4>(s,true); }
Link* __fastcall Attach(Dispatcher* s,void*,Observer* o) {
    Link* p=&o->link;
    p->previous=s->listeners.previous;
    p->next=&s->listeners;
    s->listeners.previous=p;
    p->previous->next=p;
    return p;
}
Link* __fastcall Detach(Dispatcher*,void*,Observer* o) {
    Link* next=o->link.next;
    o->link.previous->next=next;
    next->previous=o->link.previous;
    return next;
}
uintptr_t __fastcall Notify(Dispatcher* s,void*) { return Dispatch<4>(s,false); }
uintptr_t __fastcall NotifyOther(Dispatcher* s,void*) { return Dispatch<5>(s,false); }
void* __fastcall ActiveEntity(void* s,void*) {
    auto* view=static_cast<EntityView*>(s);
    if(!view->active) return nullptr;
    return view->entity;
}
uintptr_t __fastcall ForwardFirst(Proxy* s,void*,void* arg) {
    if(s->target) {
        void* i=CallGameVirtual<void*,1>(s->target,0xf4da7c8du);
        if(i) return CallGameVirtual<uintptr_t,2>(i,arg);
    }
    return CallGameVirtual<uintptr_t,7>(s,arg);
}
uintptr_t __fastcall ForwardSecond(Proxy* s,void*,void* arg) {
    if(s->target) {
        void* i=CallGameVirtual<void*,1>(s->target,0xf4da7c8du);
        if(i) return CallGameVirtual<uintptr_t,3>(i,arg);
    }
    return CallGameVirtual<uintptr_t,9>(s,arg);
}
float __fastcall Weight(Proxy* s,void*) { return s->target?CallGameVirtual<float,4>(s->target):1.0f; }
void __fastcall Vector(Proxy* s,void*,void* out) {
    if(s->target) CallGameVirtual<void,2>(s->target,out);
    else {
#if defined(_M_IX86)
        // MSVC folds a zero _mm_store_ps into unaligned scalar stores. The
        // original MOVAPS alignment fault is part of this entry's contract.
        __asm {
            mov eax, out
            xorps xmm0, xmm0
            movaps [eax], xmm0
        }
#else
        _mm_store_ps(static_cast<float*>(out),_mm_setzero_ps());
#endif
    }
}
SlaveState* __fastcall Construct(SlaveState* s,void*) {
    s->vtable=const_cast<void**>(static_cast<void* const*>(Address(0x01022e78)));
    s->opaque=0; s->resource=nullptr; s->value=0;
    s->begin=s->end=s->capacity=nullptr;
    s->descriptor=Address(0x01022e50); s->flag=0;
    s->first=s->second=nullptr; s->tail=0; s->enabled=0;
    return s;
}
uint32_t __fastcall Type(void*,void*) { return 0xf49c8e0bu; }
void* __fastcall Query(void* s,void*,uint32_t id) { return id==0xf49c8e0bu?s:nullptr; }
void __fastcall Destruct(SlaveState* s,void*) {
    s->vtable=const_cast<void**>(static_cast<void* const*>(Address(0x01022e78)));
    s->end=s->begin;
    Release(s->second); Release(s->first);
    if(s->begin) CallGameFunction<void,0x004e3f70>(s->begin);
    if(s->resource) {
        auto* refs=reinterpret_cast<volatile long*>(static_cast<char*>(s->resource)+sizeof(void*));
#if defined(_M_IX86)
        long old=_InterlockedExchangeAdd(refs,-1);
        if(old==1) { _InterlockedExchange(refs,1); CallGameVirtual<void,0>(s->resource,1u); }
#else
        (void)refs; NotRewritten("slave resource release");
#endif
    }
}
SlaveState* __fastcall Delete(SlaveState* s,void*,uint32_t flags) {
    Destruct(s,nullptr);
    if(flags&1) CallGameFunction<void,0x004e3f70>(s);
    return s;
}
}
