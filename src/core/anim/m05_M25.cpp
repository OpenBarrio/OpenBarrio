#include "ts3/anim/m05_M25.h"
namespace ts3::anim::m25 {
namespace {
void Retain(Controller* c) { if(c) ++c->refs; }
void Release(Controller* c) { if(c && --c->refs==0) { c->refs=1; CallGameVirtual<void,0>(c,1u); } }
void Remove(Record* r,Sequence* s,uint32_t flag) { CallGameMethod<void,0x00566650>(r,s,flag); }
void Assign(Record* to,const Record* from) { CallGameMethod<void,0x00566840>(to,from); }
}
void Promote(Sequence* s) {
    if(!s->next.child || !s->current.child) return;
    auto* iface=&s->current.child->secondary8;
    if(CallGameVirtual<uint32_t,0>(iface)!=0x07a11ba8) return;
    Record copy;
    CallGameMethod<void,0x00566210>(&copy,&s->current);
    CallGameMethod<void,0x005661d0>(&s->current);
    Remove(&copy,s,1);
    Release(copy.child);
}
uint8_t Save(Sequence* s,Reader* r) {
    if(!s->transition) return false;
    auto mark=[&](bool ok) { CallGameMethod<void,0x007d77c0>(r,ok); return r->good!=0; };
    if(!mark(CallGameFunction<bool,0x004ddbd0>(r->stream,1u,r->endian))) return false;
    if(!mark(CallGameFunction<bool,0x004ddff0>(r->stream,s->rate,r->endian))) return false;
    float time=CallGameMethod<float,0x0056bcc0>(s->transition);
    if(!mark(CallGameFunction<bool,0x004ddff0>(r->stream,time,r->endian))) return false;
    CallGameMethod<void,0x00565ea0>(r,CallGameMethod<float,0x0056f640>(s->transition));
    if(!r->good) return false;
    CallGameMethod<void,0x005660c0>(s,&s->current,r); if(!r->good) return false;
    CallGameMethod<void,0x005660c0>(s,&s->next,r); if(!r->good) return false;
    auto count=CallGameMethod<uint32_t,0x00566630>(&s->nodes);
    CallGameMethod<void,0x00565730>(r,count); if(!r->good) return false;
    for(auto* n=s->nodes.next;n!=&s->nodes;n=n->next) {
        CallGameMethod<void,0x005660c0>(s,&reinterpret_cast<Node*>(n)->value,r);
        if(!r->good) return false;
    }
    return r->good;
}
void Start(Sequence* s,const Record* r) {
    if(r->duration>0.0f && (s->next.child || s->current.child || (r->flags&0x10))) {
        Controller* blend=nullptr;
        CallGameMethod<void,0x00566710>(s,&blend);
        Assign(&s->next,r);
        Record temp{blend,nullptr,0,0.0f,-1.0f}; Retain(blend);
        Assign(&s->current,&temp);
        CallGameMethod<void,0x0056bf10>(s->transition,s->current.child,s->next.child,r->duration,0.0f);
        Release(temp.child); Release(blend);
    } else {
        CallGameMethod<void,0x0056bea0>(s->transition);
        Remove(&s->current,s,1); Remove(&s->next,s,1);
        Assign(&s->next,r);
        if(s->next.child) CallGameVirtual<void,2>(s->next.child,0.0f);
    }
}
bool Enqueue(Sequence* s,Controller* child,float duration,float weight,bool one,void* key,uint32_t mode,bool eight) {
    Retain(child);
    Record r{child,key,s->flags,duration,weight};
    if(one) r.flags|=1;
    if(eight) r.flags|=8;
    if(mode==1 || s->current.child || s->next.child || s->nodes.next!=&s->nodes) {
        if(mode==1 && key) { r.flags|=6; CallGameFunction<void,0x00577120>(&s->secondary,key); }
#if defined(_M_IX86)
        constexpr uintptr_t source=0x00f98980;
#else
        constexpr uintptr_t source=0;
#endif
        auto* n=CallGameFunction<Node*,0x004e3f90>(static_cast<uint32_t>(sizeof(Node)),s->descriptor,0u,0u,source,0xe7u);
        n->value=r; Retain(child);
        n->links.next=&s->nodes; n->links.previous=s->nodes.previous;
        s->nodes.previous->next=&n->links; s->nodes.previous=&n->links;
        Release(child); return false;
    }
    Start(s,&r);
    CallGameFunction<void,0x00576cb0>(key);
    if(child) CallGameVirtual<void,2>(child,0.0f);
    CallGameMethod<void,0x0055cfa0>(&r);
    return true;
}
void Flush(Sequence* s,uint32_t flag) {
    if(s->nodes.next==&s->nodes) return;
    struct List { ListLink links; void* descriptor; } temp;
    temp.links.next=temp.links.previous=&temp.links;
#if defined(_M_IX86)
    temp.descriptor=reinterpret_cast<void*>(0x00fc1a48);
#else
    temp.descriptor=nullptr;
#endif
    CallGameMethod<void,0x00566ae0>(&temp,&temp.links,s->nodes.next,&s->nodes,flag);
    CallGameMethod<void,0x0081f320>(&s->nodes);
    s->nodes.next=s->nodes.previous=&s->nodes;
    for(auto* n=temp.links.next;n!=&temp.links;n=n->next) Remove(&reinterpret_cast<Node*>(n)->value,s,flag);
    CallGameMethod<void,0x0081f320>(&temp);
}
bool Pop(Sequence* s,uint8_t* notified) {
    *notified=0;
    Remove(&s->next,s,1);
    if(s->nodes.next==&s->nodes || (reinterpret_cast<Node*>(s->nodes.next)->value.flags&4)) return false;
    auto* n=reinterpret_cast<Node*>(s->nodes.next);
    Assign(&s->next,&n->value);
    n->links.previous->next=n->links.next; n->links.next->previous=n->links.previous;
    Release(n->value.child); CallGameFunction<void,0x004e3f70>(n);
    Promote(s);
    Controller* child=s->next.child; Retain(child);
    void* key=s->next.key; uint32_t flags=s->next.flags;
    if(child) CallGameVirtual<void,2>(child,0.0f);
    if(!(flags&2)) *notified=CallGameFunction<uint8_t,0x00576cb0>(key);
    Release(child); return true;
}
}
