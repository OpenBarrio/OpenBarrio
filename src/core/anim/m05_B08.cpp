#include "ts3/anim/m05_B08.h"
#include "ts3/scratch/clu_4e3f70_Scratch.h"
#if defined(_M_IX86)
#include <intrin.h>
#endif
namespace ts3::anim::b08 {
using ts3::CallGameFunction; using ts3::CallGameMethod; using ts3::CallGameVirtual;
template<unsigned A> static void* Table() {
#if defined(_M_IX86)
    return reinterpret_cast<void*>(A);
#else
    ts3::NotRewritten("B08 game table");
#endif
}
static int32_t Result(StreamState* s, uint32_t r) { return CallGameMethod<int32_t,0x7D77C0>(s,r); }
int32_t Write(StreamState* s,uint32_t v) { return Result(s,CallGameFunction<uint32_t,0x4DDA60>(s->handle,v)); }
static void Word(StreamState* s,uint32_t v) { Result(s,CallGameFunction<uint32_t,0x4DDBD0>(s->handle,v,s->extra)); }
uint8_t SaveOptional(Controller* c,StreamState* s) {
    Word(s,1); if(!s->good) return 0;
    if(c) { Word(s,CallGameVirtual<uint32_t,0>(&c->secondary8)); if(!s->good) return 0; CallGameVirtual<void,15>(c,s); }
    else Word(s,0);
    return s->good;
}
uint8_t Save(Controller* c,StreamState* s) {
    if(!s->good) return 0;
    Word(s,1); if(!s->good) return 0;
    Word(s,c->uniqueId); if(!s->good) return 0;
    // EA overwrites only the low byte of the argument's stack slot.
    Write(s,(static_cast<uint32_t>(reinterpret_cast<uintptr_t>(s)) & 0xffffff00u) | c->notificationLatch);
    if(!s->good) return 0;
    return CallGameVirtual<uint8_t,13>(c,s);
}
uint8_t Load(Controller* c,StreamState* s) {
    if(!s->good) return 0;
    uint32_t version=~0u;
    Result(s,CallGameFunction<uint32_t,0x4DD700>(s->handle,&version,s->extra));
    if(!s->good || version>1) return 0;
    Result(s,CallGameFunction<uint32_t,0x4DD700>(s->handle,&c->uniqueId,s->extra));
    if(!s->good) return 0;
    CallGameMethod<void,0x565760>(s,&c->notificationLatch);
    if(!s->good) return 0;
    return CallGameVirtual<uint8_t,14>(c,s);
}
Controller** Retain(Controller** p,Controller* c) { *p=c; if(c) c->refs=static_cast<int32_t>(static_cast<uint32_t>(c->refs)+1); return p; }
static void Unlink(ListLink* n) { n->previous->next=n->next; n->next->previous=n->previous; ts3::scratch::Free4E3F70(n); }
void Leave(DelegateList* l) {
    if(--l->depth) return;
    for(auto* n=l->links.next;n!=&l->links;) { auto* next=n->next; if(!reinterpret_cast<DelegateNode*>(n)->target) Unlink(n); n=next; }
}
void Clear(DelegateList* l) {
    l->vtable=Table<0xFC1988>();
    for(auto* n=l->links.next;n!=&l->links;) { auto* next=n->next; Unlink(n); n=next; }
    l->links.next=l->links.previous=&l->links;
}
DelegateList* DeleteList(DelegateList* l,uint32_t flags) { Clear(l); if(flags&1) ts3::scratch::Free4E3F70(l); return l; }
static DelegateList* Delegates(Controller* c) { return reinterpret_cast<DelegateList*>(&c->secondaryC); }
void Notify(Controller* c,void* event) {
    if(c->notificationLatch) return;
    c->notificationLatch=1;
    auto* l=Delegates(c); ++l->depth;
    for(auto* n=l->links.next;n!=&l->links;) { auto* d=reinterpret_cast<DelegateNode*>(n); n=n->next; if(d->target) CallGameVirtual<void,3>(d->target,event,d->payload); }
    Leave(l);
}
Controller* Construct(Controller* c,void* host) {
    c->refs=0; c->secondary8=Table<0xFC1990>(); c->secondaryC=Table<0xFC198C>();
    c->delegates.next=c->delegates.previous=&c->delegates; c->value18=0; c->hostSlot=host; c->notificationLatch=0; c->vtable=Table<0xFC1998>();
#if defined(_M_IX86)
    c->uniqueId=(*reinterpret_cast<uint32_t*>(0x114F590))++;
#else
    ts3::NotRewritten("B08 unique ID global");
#endif
    return c;
}
uint32_t Type(void*) { return 0x7489D72B; }
Controller* Query(void* s,uint32_t id) { return id==0x7489D72B ? reinterpret_cast<Controller*>(reinterpret_cast<char*>(s)-offsetof(Controller,secondary8)) : nullptr; }
Controller* DeleteInterface(void* s,uint32_t flags) { return Delete(reinterpret_cast<Controller*>(reinterpret_cast<char*>(s)-offsetof(Controller,secondaryC)),flags); }
void ReleaseNotify(Controller* c) {
    if(c) c->refs=static_cast<int32_t>(static_cast<uint32_t>(c->refs)+1u);
    struct Event { Controller* owner; Controller* retained; void* host; uint32_t a,b; uint8_t x,y,z; } event;
    event.owner=c; event.retained=nullptr; event.host=c->hostSlot; event.a=event.b=0; event.x=event.y=event.z=0;
    Notify(c,&event);
    if(event.retained) {
        int32_t old;
#if defined(_M_IX86)
        old=static_cast<int32_t>(_InterlockedExchangeAdd(reinterpret_cast<volatile long*>(&event.retained->refs),-1));
#else
        old=event.retained->refs--;
#endif
        if(old==1) {
#if defined(_M_IX86)
            _InterlockedExchange(reinterpret_cast<volatile long*>(&event.retained->refs),1);
#else
            event.retained->refs=1;
#endif
            CallGameVirtual<void,0>(event.retained,1u);
        }
    }
    c->refs=static_cast<int32_t>(static_cast<uint32_t>(c->refs)-1u);
    if(c->refs==0) { c->refs=1; CallGameVirtual<void,0>(c,1u); }
}
Controller* Delete(Controller* c,uint32_t flags) {
    c->vtable=Table<0xFC1998>(); c->secondary8=Table<0xFC1990>(); c->secondaryC=Table<0xFC198C>();
    Clear(Delegates(c)); c->vtable=Table<0x107396C>();
    if(flags&1) ts3::scratch::Free4E3F70(c);
    return c;
}
}
