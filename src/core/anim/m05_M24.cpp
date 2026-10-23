#include "ts3/anim/m05_M24.h"
#include <initializer_list>
#include <cstring>
namespace ts3::anim::m24 {
static void* Address(uint32_t a) {
#if defined(_M_IX86)
    return reinterpret_cast<void*>(uintptr_t(a));
#else
    (void)a; return nullptr;
#endif
}
static void Release(Controller* p) {
    if(p && --p->refs==0) { p->refs=1; CallGameVirtual<void,0>(p,1u); }
}
static void Assign(Controller*& to,Controller* p) {
    if(to==p) return;
    if(p) ++p->refs;
    auto* old=to; to=p; Release(old);
}
static void Reset(Entry* e) {
    auto* old=e->controller;
    e->handle=0; e->flags=0; e->time=0; e->weight=0; e->controller=nullptr;
    Release(old);
}
uintptr_t Forward(Sequence* p,void* arg) {
#if defined(_M_IX86)
    *static_cast<uint32_t*>(Address(0x011cd7d8))=p->mode;
#else
    AbortNotRewritten("sequence mode global");
#endif
    Controller* selected;
    if(CallGameMethod<bool,0x0056bc80>(p->transition)) selected=p->transition;
    else if(p->second.controller) selected=p->second.controller;
    else selected=p->first.controller;
    return selected?CallGameVirtual<uintptr_t,3>(selected,arg):0;
}
// The original's FLD/FSTP argument staging quiets signaling NaNs, including their sign/payload.
static float StagedFloat(float value) {
    uint32_t bits; std::memcpy(&bits,&value,4);
    if((bits&0x7f800000u)==0x7f800000u && (bits&0x007fffffu)) bits|=0x00400000u;
    std::memcpy(&value,&bits,4); return value;
}
uint8_t Save(Sequence*,Entry* e,Stream* s) {
    CallGameFunction<void,0x005657a0>(e->controller,s);
    if(!s->good) return false;
    for(uint32_t value:{e->handle,e->flags}) {
        auto result=CallGameFunction<uint32_t,0x004ddbd0>(s->context,value,s->options);
        CallGameMethod<void,0x007d77c0>(s,result);
        if(!s->good) return false;
    }
    auto result=CallGameFunction<uint32_t,0x004ddff0>(s->context,StagedFloat(e->time),s->options);
    CallGameMethod<void,0x007d77c0>(s,result);
    if(!s->good) return false;
    CallGameMethod<void,0x00565ea0>(s,StagedFloat(e->weight));
    return s->good;
}
void Register(Sequence* p,void* delegate) {
    for(auto* c:{p->first.controller,p->second.controller})
        if(c) CallGameMethod<bool,0x00566170>(&c->secondaryC,delegate,-1);
    for(auto* node=p->nodes.next;node!=&p->nodes;node=node->next) {
        auto* c=reinterpret_cast<Node*>(node)->controller;
        if(!c) continue;
        auto* r=CallGameFunction<Registration*,0x004e3f90>(sizeof(Registration),Address(0x00fc1a08),0,0,0,0);
        if(!r) continue;
        r->delegate=delegate; r->mask=-1;
        auto* sentinel=&c->delegates;
        r->links.previous=sentinel->previous; r->links.next=sentinel;
        sentinel->previous=&r->links; r->links.previous->next=&r->links;
    }
}
uint8_t Load(Sequence* p,Entry* e,Stream* s) {
    auto* c=CallGameFunction<Controller*,0x00565c80>(p->base.hostSlot,s);
    Assign(e->controller,c);
    for(auto* value:{&e->handle,&e->flags,reinterpret_cast<uint32_t*>(&e->time),reinterpret_cast<uint32_t*>(&e->weight)}) {
        auto result=CallGameFunction<uint32_t,0x004dd700>(s->context,value,s->options);
        CallGameMethod<void,0x007d77c0>(s,result);
        if(!s->good) return false;
    }
    if(e->handle && (e->flags&4)) CallGameFunction<void,0x00577120>(&p->secondary,e->handle);
    return s->good;
}
void Clear(Entry* e,Sequence* parent,bool notify) {
    if(!(e->flags&2)) CallGameFunction<void,0x00576d00>(e->handle);
    CallGameFunction<void,0x00576d40>(parent?&parent->secondary:nullptr,e->handle);
    auto* c=e->controller;
    if(c) ++c->refs;
    Reset(e);
    if(notify && c) CallGameVirtual<void,1>(c);
    Release(c);
}
Controller** Extract(Sequence* p,Controller** out) {
    *out=nullptr; // Deliberately does not release an existing output value.
    if(CallGameMethod<bool,0x0056bc80>(p->transition)) {
        Assign(*out,p->transition);
        auto* memory=CallGameFunction<Controller*,0x004e3f90>(0x38,Address(0x00fc1a28),0,0,0,0);
        auto* replacement=memory?CallGameMethod<Controller*,0x0056bda0>(memory,p->base.hostSlot):nullptr;
        Assign(p->transition,replacement);
    } else {
        auto** source=p->first.controller?&p->first.controller:&p->second.controller;
        if(*source) CallGameMethod<Controller**,0x005665f0>(out,source);
    }
    Reset(&p->first); Reset(&p->second);
    return out;
}
#if defined(_M_IX86)
uintptr_t __fastcall HookForward(Sequence* p,void*,void* a) { return Forward(p,a); }
uint8_t __fastcall HookSave(Sequence* p,void*,Entry* e,Stream* s) { return Save(p,e,s); }
void __fastcall HookRegister(Sequence* p,void*,void* d) { Register(p,d); }
uint8_t __fastcall HookLoad(Sequence* p,void*,Entry* e,Stream* s) { return Load(p,e,s); }
void __fastcall HookClear(Entry* p,void*,Sequence* s,bool n) { Clear(p,s,n); }
Controller** __fastcall HookExtract(Sequence* p,void*,Controller** o) { return Extract(p,o); }
#endif
}
