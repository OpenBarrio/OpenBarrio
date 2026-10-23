#include "ts3/anim/m05_M49.h"
#include "ts3/layout.h"
#include <cstddef>
#include <atomic>
namespace ts3::anim::m49 {
static void Release(Controller* p) {
    if (p && --p->refs == 0) { p->refs=1; CallGameVirtual<void,0>(p,uint32_t(1)); }
}
bool Load(State* s, Cursor* c) {
    uint32_t version=~0u;
    auto r=CallGameFunction<uintptr_t,0x004DD700>(c->stream,&version,c->context);
    CallGameMethod<void,0x007D77C0>(c,r);
    if (!c->good || version>1) return false;
    auto* target=CallGameFunction<Controller*,0x00565C80>(s->base.hostSlot,c);
    CallGameMethod<void,0x00CF3C70>(&s->target,target);
    if (!s->target) return false;
    r=CallGameFunction<uintptr_t,0x004DD700>(c->stream,&s->key,c->context);
    CallGameMethod<void,0x007D77C0>(c,r);
    if (!c->good) return false;
    CallGameMethod<void,0x00565700>(c,&s->weight);
    if (!c->good) return false;
    if(s->target) CallGameMethod<void,0x00566170>(&s->target->secondaryC,&s->callback,int32_t(-1));
    return c->good;
}
void Destroy(State* s) {
#if defined(_M_IX86)
    s->base.vtable=reinterpret_cast<void*>(0x00FC20C0);
    s->base.secondary8=reinterpret_cast<void*>(0x00FC20B8);
    s->base.secondaryC=reinterpret_cast<void*>(0x00FC20B4);
    s->callback=reinterpret_cast<void*>(0x00FC1E5C);
#endif
    if(s->target) CallGameMethod<void,0x00566590>(&s->target->secondaryC,&s->callback);
    Release(s->target);
    CallGameMethod<void,0x00565B40>(s);
}
void Dispatch(void* callback, const Event* in, uint32_t) {
    auto* s=reinterpret_cast<State*>(static_cast<unsigned char*>(callback)-offsetof(State,callback));
    if(s) ++s->base.refs;
    Event e=*in;
    if(e.resource) std::atomic_ref<int32_t>(e.resource->atomicRefs).fetch_add(1);
    e.owner=&s->base; e.host=s->base.hostSlot;
    CallGameMethod<void,0x00568710>(s,&e);
    if(e.resource && std::atomic_ref<int32_t>(e.resource->atomicRefs).fetch_sub(1)==1) {
        std::atomic_ref<int32_t>(e.resource->atomicRefs).exchange(1);
        CallGameVirtual<void,0>(e.resource,uint32_t(1));
    }
    Release(&s->base);
}
State* Delete(State* s,uint32_t flags) {
    Destroy(s);
    if(flags&1) CallGameFunction<void,0x004E3F70>(s);
    return s;
}
bool Save(State* s, Cursor* c) {
    auto r=CallGameFunction<uintptr_t,0x004DDBD0>(c->stream,uint32_t(1),c->context);
    CallGameMethod<void,0x007D77C0>(c,r);
    if(!c->good) return false;
    CallGameFunction<void,0x005657A0>(s->target,c);
    return c->good;
}
State* Construct(State* s,void* host) {
    CallGameMethod<void,0x00565A80>(s,host);
#if defined(_M_IX86)
    s->base.vtable=reinterpret_cast<void*>(0x00FC2128);
    s->base.secondary8=reinterpret_cast<void*>(0x00FC211C);
    s->base.secondaryC=reinterpret_cast<void*>(0x00FC2118);
    s->callback=reinterpret_cast<void*>(0x00FC2104);
#endif
    s->target=nullptr;
    return s;
}
}
#if defined(_M_IX86)
namespace ts3::anim::m49 {
bool __fastcall HookLoad(State* s,void*,Cursor* c){return Load(s,c);}
void __fastcall HookDestroy(State* s,void*){Destroy(s);}
void __fastcall HookDispatch(void* s,void*,const Event* e,uint32_t v){Dispatch(s,e,v);}
State* __fastcall HookDelete(State* s,void*,uint32_t f){return Delete(s,f);}
bool __fastcall HookSave(State* s,void*,Cursor* c){return Save(s,c);}
State* __fastcall HookConstruct(State* s,void*,void* h){return Construct(s,h);}
}
#endif
