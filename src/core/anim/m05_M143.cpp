#include "ts3/anim/m05_M143.h"
namespace ts3::anim::m143 {
namespace {
struct Ref { void* vt; int32_t refs; unsigned char pad[4]; unsigned char delegates[4]; };
struct Request { void* source; float values[4]; uint8_t flag,other; uint16_t untouched; uint32_t last; };
struct Result { void* controller; uint8_t flag; };
uintptr_t Clear(Body* s) {
    auto* c=static_cast<Ref*>(s->active); if(!c) return 0;
    uintptr_t r=CallGameMethod<uintptr_t,0x00566590>(c->delegates,&s->interfaceTable);
    c=static_cast<Ref*>(s->active); s->active=nullptr;
    if(c) { r=static_cast<uint32_t>(--c->refs); if(!r) { c->refs=1; r=CallGameVirtual<uintptr_t,0>(c,1u); } }
    return r;
}
void Reset(Body* s) { CallGameMethod<void,0x0085A8C0>(s->reset,0,0); }
uintptr_t Cancel(Body* s) { return CallGameVirtual<uintptr_t,22>(s->host,s->request); }
uintptr_t Cleanup(Request& q,Result& r) {
    CallGameMethod<void,0x0055CFA0>(&r);
    return CallGameMethod<uintptr_t,0x008499B0>(&q);
}
uintptr_t Build(Body* s,void* builder,int state,bool checkBool=false,bool fallbackReset=true,bool construct=false) {
    Request q; q.source=s->source; for(float& f:q.values) f=0; q.flag=0; q.other=0; q.last=0;
    if(construct) CallGameMethod<void,0x008495D0>(&q,s->source);
    Result r; r.controller=nullptr; r.flag=0;
    uintptr_t ok=0;
    if(builder) ok=CallGameVirtual<uintptr_t,3>(builder,&q,&r);
    if(builder && (!checkBool || static_cast<uint8_t>(ok)) && r.controller) {
        CallGameMethod<void,0x00854A80>(s,r.controller,state);
        auto ret=Cleanup(q,r); s->state=state; return ret;
    }
    if(fallbackReset) Reset(s);
    State(s,10);
    if(fallbackReset) Cancel(s);
    return Cleanup(q,r);
}
}
uintptr_t State(Body* s,int32_t state) {
    for(;;) {
        uintptr_t ret=Clear(s);
        if(!s->source) { s->state=state; return ret; }
        ret=static_cast<uint32_t>(state-1);
        switch(state) {
        case 1:return Build(s,s->first,state,true);
        case 2:return Build(s,s->two,state,true);
        case 3:return Build(s,s->three,state,true);
        case 5:state=static_cast<uint8_t>(CallGameFunction<uintptr_t,0x0055DA30>(s->condition))?7:6;continue;
        case 4: {
            if(static_cast<uint8_t>(CallGameVirtual<uintptr_t,27>(s->host,s->request))) { state=8;continue; }
            Request q; q.source=s->source; for(float& f:q.values) f=0; q.other=0; q.last=0;
            Result r; r.controller=nullptr;r.flag=0;
            bool flip=!static_cast<uint8_t>(CallGameFunction<uintptr_t,0x0055DA30>(s->condition)) && s->flag84;
            q.flag=s->flag80;
            void* builder=(flip==(s->state==7))?s->same:s->change;
            if(builder) CallGameVirtual<void,3>(builder,&q,&r);
            if(r.controller) { CallGameMethod<void,0x00854A80>(s,r.controller,state); ret=Cleanup(q,r);s->state=state;return ret; }
            State(s,10);return Cleanup(q,r);
        }
        case 6:return Build(s,s->six,state);
        case 7:return Build(s,s->seven,state);
        case 8:return Build(s,s->state==6?s->fromSix:s->fromOther,state);
        case 10:ret=Cancel(s);break;
        default:break;
        }
        s->state=state;return ret;
    }
}
uintptr_t Update(void* iface,void* const* event,int32_t kind) {
    auto* s=reinterpret_cast<Body*>(static_cast<unsigned char*>(iface)-offsetof(Body,interfaceTable));
    int previous=s->state; ++s->refs;
    if(*event) {
        // The callback's controller is independent of the currently retained one.
        CallGameMethod<void,0x00566590>(static_cast<unsigned char*>(*event)+12,&s->interfaceTable);
        auto* c=static_cast<Ref*>(s->active);s->active=nullptr;
        if(c && !--c->refs) {c->refs=1;CallGameVirtual<void,0>(c,1u);}
    }
    if(s->source) CallGameVirtual<void,10>(s->source);
    uintptr_t ret=static_cast<uint32_t>(kind-1);
    switch(kind) {
    case 1:case 2:case 3:case 4:case 6:case 7:case 8:
        CallGameMethod<void,0x008540F0>(s);break;
    default:goto release;
    }
    switch(kind) {
    case 1:
        if(--s->remaining<=0) ret=State(s,4);
        else { ret=Clear(s); if(s->source) ret=State(s,static_cast<uint8_t>(CallGameFunction<uintptr_t,0x0055DA30>(s->condition))?7:6);else s->state=5; } break;
    case 2:case 3: --s->remaining; [[fallthrough]];
    case 4:ret=Clear(s);if(s->source) ret=Cancel(s);s->state=10;break;
    case 7:ret=State(s,--s->remaining>0?6:4);break;
    case 6:
        if(--s->remaining<=0) {ret=State(s,4);break;}
        Clear(s);ret=reinterpret_cast<uintptr_t>(s->source);
        if(s->source) ret=Build(s,s->seven,7,false,true,true);
        // Failure leaves State(10), unlike the no-source success path.
        else s->state=7;
        break;
    case 8:
        Clear(s);ret=reinterpret_cast<uintptr_t>(s->source);
        if(s->source) ret=Build(s,previous==6?s->six:s->seven,previous==6?6:7);
        else s->state=previous==6?6:7;
        break;
    }
release:
    ret=static_cast<uint32_t>(--s->refs);
    if(!ret) {s->refs=1;ret=CallGameVirtual<uintptr_t,0>(&s->secondary,1u);}
    return ret;
}
void* Destroy(void* s,uint32_t flags) {
    CallGameMethod<void,0x00854C70>(s);
    if(flags&1) CallGameFunction<void,0x004E3F70>(s);
    return s;
}
uint8_t Start(Body* s,void* arg) {
    CallGameMethod<void,0x00854060>(s->request,arg);
    void* manager=nullptr;
#if defined(_M_IX86)
    manager=*reinterpret_cast<void**>(0x011D9140);
#else
    AbortNotRewritten("stairs routing manager");
#endif
    auto query=[&]() {return static_cast<uint8_t>(CallGameMethod<uintptr_t,0x00844BE0>(manager,s->keys[0],s->keys[1],s->keys[2],s->keys[3],s->mode,&s->context));};
    if(!query()) {if(s->mode!=2 && s->mode!=3) return 0;s->mode=0;if(!query()) return 0;}
    alignas(16) float unused[4],delta[4],rotation[4];
    CallGameVirtual<void,7>(s->host,unused);
    for(int i=0;i<4;++i) delta[i]=s->finish[i]-s->start[i];
    delta[1]=0;
    CallGameFunction<void,0x00860460>(delta,rotation);
    CallGameVirtual<void,12>(s->host,s->start,rotation);
    ++s->refs;s->remaining=s->count;
    if(s->count<=1 && s->two) State(s,2);
    else if(s->count==2 && s->three) {s->remaining=1;State(s,3);}
    else {if(s->flag84)s->remaining-=2;else if(s->flag83)--s->remaining;State(s,1);}
    if(s->source) {void* x=CallGameVirtual<void*,1>(s->source,0xf4c95ed2u);if(x)CallGameVirtual<void,2>(x,1.0f);}
    if(!--s->refs) {s->refs=1;CallGameVirtual<void,0>(&s->secondary,1u);}
    return 1;
}
Brain* Construct(Brain* s,void* a,void* b) {
    CallGameMethod<void,0x0085A100>(s,a,b);
#if defined(_M_IX86)
    s->vtable=reinterpret_cast<void*>(0x010273D0);s->secondary=reinterpret_cast<void*>(0x010273CC);s->interfaceTable=reinterpret_cast<void*>(0x010273BC);
#else
    AbortNotRewritten("off-screen brain vtables");
#endif
    s->value=0;return s;
}
void* DestroySecondary(void* s,uint32_t flags) {
    return CallGameMethod<void*,0x00856340>(static_cast<unsigned char*>(s)-sizeof(void*),flags);
}
#if defined(_M_IX86)
namespace hooks {
uintptr_t __fastcall State(Body* s,void*,int32_t v){return m143::State(s,v);}
uintptr_t __fastcall Update(void* s,void*,void* const* e,int32_t v){return m143::Update(s,e,v);}
void* __fastcall Destroy(void* s,void*,uint32_t f){return m143::Destroy(s,f);}
uint8_t __fastcall Start(Body* s,void*,void* a){return m143::Start(s,a);}
Brain* __fastcall Construct(Brain* s,void*,void* a,void* b){return m143::Construct(s,a,b);}
void* __fastcall DestroySecondary(void* s,void*,uint32_t f){return m143::DestroySecondary(s,f);}
}
#endif
}
