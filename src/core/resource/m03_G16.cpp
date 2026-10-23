#include "ts3/resource/m03_G16.h"
#include <intrin.h>
#include <cstring>
namespace ts3::resource::m03::g16 {
namespace {
#if defined(_M_IX86)
uintptr_t Free(void* p) { return CallGameFunction<uintptr_t,0x004E3F70>(p); }
uintptr_t Lock(Queue* q) { return CallGameMethod<uintptr_t,0x004E16F0>(&q->mutex,reinterpret_cast<const uint32_t*>(0x00FFD4B8)); }
uintptr_t Unlock(Queue* q) { return CallGameMethod<uintptr_t,0x004E17B0>(&q->mutex); }
void Swap(Link* a,Link* b) {
    auto an=a->next; auto ap=a->prev;
    a->next=b->next; a->prev=b->prev; b->next=an; b->prev=ap;
    if(a->next==b) a->next=a->prev=a;
    else { a->prev->next=a; a->next->prev=a; }
    if(b->next==a) b->next=b->prev=b;
    else { b->prev->next=b; b->next->prev=b; }
}
#endif
}
uintptr_t __fastcall Destroy(Queue* q,void*) {
#if defined(_M_IX86)
    uintptr_t result=0;
    for(auto n=q->active.next;n!=&q->active;) { auto p=n; n=n->next; result=Free(p); }
    result=CallGameMethod<uintptr_t,0x004E1670>(&q->mutex);
    for(auto n=q->pending.next;n!=&q->pending;) { auto p=n; n=n->next; result=Free(p); }
    return result;
#else
    (void)q; NotRewritten("m03_G16 Destroy");
#endif
}
uint8_t __fastcall Run(Queue* q,void*,int32_t count,uint32_t timeout) {
#if defined(_M_IX86)
    Lock(q); Swap(&q->active,&q->pending); Unlock(q);
    alignas(8) uint8_t timer[32]{};
    CallGameMethod<void,0x004F35B0>(timer,4u,0u);
    CallGameMethod<void,0x004F34F0>(timer,timeout,1u);
    uint32_t removed=0; uint8_t ran=0; bool expired=false;
    for(int32_t i=0;i<count && q->active.next!=&q->active && !expired;++i) {
        auto* n=reinterpret_cast<Node*>(q->active.next);
        uintptr_t again;
        do {
            again=0; ran=1;
            if(n->callback) n->callback(n->context);
            else { again=n->step(n->context,n->flag); n->step=reinterpret_cast<decltype(n->step)>(again); }
            if(again==0) { n->link.prev->next=n->link.next; n->link.next->prev=n->link.prev; ++removed; Free(n); }
            if(timeout!=0xffffffffu) {
                int64_t now=0,deadline=0;
                using Clock=int (__stdcall*)(int64_t*);
                (*reinterpret_cast<Clock*>(0x00F953B8))(&now);
                std::memcpy(&deadline,timer+24,8);
                expired=static_cast<int64_t>(static_cast<uint64_t>(deadline)-static_cast<uint64_t>(now))<0;
            }
        } while(again!=0 && !expired);
    }
    // x86 interlocked xadd; the field is concurrently updated by producers.
    _InterlockedExchangeAdd(reinterpret_cast<volatile long*>(&q->count),-static_cast<long>(removed));
    if(q->active.next!=&q->active) {
        Lock(q);
        auto* first=q->active.next; auto* tail=q->active.prev; auto* end=&q->pending;
        tail->next=end; first->prev=end->prev; end->prev->next=first; end->prev=tail;
        q->active.next=q->active.prev=&q->active;
        Unlock(q);
    }
    return ran;
#else
    (void)q; (void)count; (void)timeout; NotRewritten("m03_G16 Run");
#endif
}
uint8_t __cdecl Parse(const char16_t* section,const char16_t* value,Config* c) {
#if defined(_M_IX86)
    using Compare=int (__cdecl*)(const char16_t*,const char16_t*,uint32_t);
    if((*reinterpret_cast<Compare*>(0x00F95494))(section,c->section,c->length)==0) {
        struct Text { const char16_t* begin; const char16_t* end; const char16_t* capacity; const char* name; } text;
        text.begin=text.end=reinterpret_cast<char16_t*>(0x011C7E54); text.capacity=reinterpret_cast<char16_t*>(0x011C7E56); text.name=reinterpret_cast<char*>(0x00FFD530);
        auto* end=value; while(*end) ++end;
        CallGameMethod<void,0x00408E20>(&text,value,end);
        uint32_t range[9]{};
        CallGameMethod<void,0x004AF170>(range);
        if(!CallGameFunction<uint8_t,0x004AF2E0>(&text,range)) c->valid=0;
        else CallGameMethod<void,0x0072AC20>(static_cast<uint8_t*>(c->ranges)+4,range);
        if(reinterpret_cast<uintptr_t>(text.capacity)-reinterpret_cast<uintptr_t>(text.begin)>2 && text.begin) Free(const_cast<char16_t*>(text.begin));
    }
    return 1;
#else
    (void)section; (void)value; (void)c; NotRewritten("m03_G16 Parse");
#endif
}
}
