#include "ts3/anim/m05_M27.h"
#include <cstring>
#include <initializer_list>
namespace ts3::anim::m27detail {
namespace {
uint32_t Low(uint32_t r,uint8_t b) { return (r&0xffffff00u)|b; }
uint32_t Status(Archive* a,uint32_t r) { return CallGameMethod<uint32_t,0x007D77C0>(a,r); }
void Blend(Sequence* s,float a,float b=0) { CallGameMethod<void,0x0056BF10>(s->transition,s->first.controller,s->second.controller,a,b); }
void Clear(Sequence* s,Slot* p) { CallGameMethod<void,0x00566650>(p,s,1); }
void Shift(Sequence* s) {
    Clear(s,&s->first); CallGameMethod<void,0x00566840>(&s->first,&s->second);
    CallGameMethod<void,0x005661D0>(&s->second);
}
bool Active(Sequence* s) {
    return s->nodes.next!=&s->nodes && !(reinterpret_cast<Slot*>(s->nodes.next+1)->flags&4);
}
float InfiniteTime() {
#if defined(_M_IX86)
    return *reinterpret_cast<const float*>(0x0114F594);
#else
    AbortNotRewritten("sequence infinite-time global"); return 0;
#endif
}
}
uint32_t SetTime(Clip* c,float t) {
    uint32_t mode=c->mode;
    c->requested=t; c->current=t; c->previous=t;
    if(mode==1) c->cycles=t>c->period;
    else {
        c->cycles=0;
        if(t>c->period) do { ++c->cycles; c->current=c->current-c->period; } while(c->current>c->period);
        if(mode && c->cycles>mode) c->cycles=mode;
    }
    return mode;
}
uint32_t SetTimeWrapper(Clip* c,float t) {
    // The forwarding thunk loads/stores through x87: signaling NaNs are quieted.
    uint32_t bits; std::memcpy(&bits,&t,4);
    if((bits&0x7f800000u)==0x7f800000u && (bits&0x007fffffu)) {
        bits|=0x00400000u; std::memcpy(&t,&bits,4);
    }
    return SetTime(c,t);
}
Sequence* DeleteSequence(Sequence* s,uint32_t flags) {
    CallGameMethod<void,0x005675E0>(s);
    if(flags&1) CallGameFunction<void,0x004E3F70>(s);
    return s;
}
uint32_t WriteClip(Clip* c,Archive* a) {
    uint32_t r=Status(a,CallGameFunction<uint32_t,0x004DDBD0>(a->stream,3,a->context));
    if(!a->ok) return Low(r,0);
    r=Status(a,CallGameFunction<uint32_t,0x004DDBD0>(a->stream,c->extra!=nullptr,a->context));
    if(!a->ok) return Low(r,0);
    uint32_t keys[2][4]{};
    if(c->resource) std::memcpy(keys[0],c->resource->key,16);
    if(c->resource2) std::memcpy(keys[1],c->resource2->key,16);
    for(auto& key:keys) {
        r=Status(a,CallGameFunction<uint32_t,0x007D74E0>(a->stream,key,a->context));
        if(!a->ok) return Low(r,0);
    }
    void* temp=a;
    r=CallGameMethod<uint32_t,0x00577890>(&c->ik,a,&temp);
    if(!a->ok) return Low(r,0);
    if(c->extra) {
        r=CallGameMethod<uint32_t,0x00581CE0>(c->extra,a,&temp);
        if(!a->ok) return Low(r,0);
    }
    for(float t:{c->current,c->previous,c->requested}) {
        r=CallGameMethod<uint32_t,0x00565EA0>(a,t);
        if(!a->ok) return Low(r,0);
    }
    for(uint32_t n:{c->mode,c->cycles}) {
        r=CallGameMethod<uint32_t,0x00565730>(a,n);
        if(!a->ok) return Low(r,0);
    }
    r=CallGameMethod<uint32_t,0x005680A0>(a,c->index);
    if(!a->ok) return Low(r,0);
    for(unsigned i:{0u,1u,2u,5u}) {
        r=CallGameMethod<uint32_t,0x00565780>(a,uint32_t(c->flags[i]));
        if(!a->ok) return Low(r,0);
    }
    return Low(r,a->ok);
}
uint32_t ReadSequence(Sequence* s,Archive* a) {
    uint32_t r=CallGameMethod<uint32_t,0x005676B0>(s);
    if(!s->transition) return Low(r,0);
    uint32_t version=~0u;
    r=Status(a,CallGameFunction<uint32_t,0x004DD700>(a->stream,&version,a->context));
    if(!a->ok || version>1) return Low(r,0);
    r=Status(a,CallGameFunction<uint32_t,0x004DD700>(a->stream,&s->serial,a->context));
    if(!a->ok) return Low(r,0);
    float t=-1, w=-1;
    r=CallGameMethod<uint32_t,0x00565700>(a,&t);
    if(!a->ok) return Low(r,0);
    r=CallGameMethod<uint32_t,0x00565700>(a,&w);
    if(!a->ok) return Low(r,0);
    r=CallGameMethod<uint32_t,0x00566420>(s,&s->first,a);
    if(!a->ok) return Low(r,0);
    r=CallGameMethod<uint32_t,0x00566420>(s,&s->second,a);
    if(!a->ok) return Low(r,0);
    if(t!=-1 || w!=-1) Blend(s,t,w);
    uint32_t count=0;
    r=CallGameMethod<uint32_t,0x00565700>(a,&count);
    if(!a->ok) return Low(r,0);
    for(uint32_t i=0;i<count;++i) {
        CallGameMethod<void,0x00566A80>(&s->nodes);
        r=CallGameMethod<uint32_t,0x00566420>(s,s->nodes.previous+1,a);
        if(!a->ok) return Low(r,0);
    }
    return Low(r,a->ok);
}
uint32_t UpdateSequence(Sequence* s,float dt) {
    if(s) ++s->base.refs;
    uint8_t changed=0;
    if(!uint8_t(CallGameMethod<uint32_t,0x0056BC80>(s->transition))) {
        if(s->first.controller && (CallGameVirtual<double,7>(s->first.controller)<=0) && !(s->first.flags&8)) Clear(s,&s->first);
        if(s->second.controller && (CallGameVirtual<double,7>(s->second.controller)<=0)) {
            if(s->second.flags&8) {
                Shift(s);
                if(s->first.controller) {
                    Controller* old=s->first.controller; ++old->refs;
                    void* mem=CallGameFunction<void*,0x004E3F90>(44,"Animation/HoldPoseController",0,0,0,0);
                    Controller* hold=mem?CallGameMethod<Controller*,0x0056FE70>(mem,old->hostSlot,old):nullptr;
                    if(hold) ++hold->refs;
                    CallGameMethod<void,0x00CF3C70>(&s->first,hold);
                    CallGameVirtual<void,1>(old);
                    CallGameMethod<void,0x0055CFA0>(&hold);
                    CallGameMethod<void,0x0055CFA0>(&old);
                }
            } else Clear(s,&s->second);
        }
        if(s->second.controller) {
            float end=static_cast<float>(CallGameVirtual<double,7>(s->second.controller));
            float limit=static_cast<float>(CallGameVirtual<double,9>(s->second.controller));
            float a=-1,b=s->second.weight;
            Controller* retained=nullptr;
            if(Active(s)) {
                if(end==InfiniteTime()) {
                    auto* p=CallGameVirtual<Controller*,1>(&s->second.controller->secondary8,uint32_t(0xb489fc2c));
                    if(p) { ++p->refs; auto n=CallGameMethod<uint32_t,0x005681F0>(p); CallGameMethod<void,0x005681E0>(p,n+1); }
                    CallGameMethod<void,0x0055CFA0>(&p);
                }
                a=reinterpret_cast<Slot*>(s->nodes.next+1)->time;
                // Adjacent a,b,retained are an EA three-field temporary, not independent locals.
                struct Temporary { float a,b; Controller* retained; } temp{a,b,nullptr};
                CallGameMethod<void,0x005665F0>(&temp.retained,s->nodes.next+1);
                a=temp.a; b=temp.b; retained=temp.retained;
            } else if(!(s->second.flags&0x10)) b=0;
            if(b!=-1 && a==-1) a=b;
            if(a!=-1 && b==-1) b=a;
            if(a==-1 && b==-1) a=b=0;
            float chosen=b>a?a:b;
            float before=limit;
            if(!(limit>chosen)) chosen=limit;
            limit=chosen;
            // COMISS's unordered path takes the early-out too.
            if(!(limit<before) && !(limit!=limit || before!=before)) {
                if(Active(s)) {
                    Shift(s); CallGameMethod<void,0x00566FB0>(s,&changed); Blend(s,limit);
                } else if(end!=InfiniteTime()) {
                    if(!(before>0)) { Clear(s,&s->first); Clear(s,&s->second); }
                    else { Shift(s); CallGameMethod<void,0x00566FB0>(s,&changed); CallGameMethod<void,0x0056BEA0>(s->transition); Blend(s,limit); }
                }
            }
            if(retained && --retained->refs==0) { retained->refs=1; CallGameVirtual<void,0>(retained,1); }
        } else if(Active(s)) {
            bool hold=false;
            if(s->first.controller) {
                if(!(s->first.flags&8)) Clear(s,&s->first);
                else hold=CallGameVirtual<uint32_t,0>(&s->first.controller->secondary8)==0x07a11ba8;
            }
            Clear(s,&s->second); CallGameMethod<void,0x00566FB0>(s,&changed);
            float t=s->second.time; if(hold || t==-1) t=0;
            CallGameMethod<void,0x0056BEA0>(s->transition);
            if(t>0) Blend(s,t);
        }
    }
    if(uint8_t(CallGameMethod<uint32_t,0x0056BC80>(s->transition))) {
        CallGameVirtual<void,2>(s->transition,dt);
        if(uint8_t(CallGameMethod<uint32_t,0x0056BCA0>(s->transition))) {
            CallGameMethod<void,0x0056BEA0>(s->transition); Clear(s,&s->first);
        }
    } else if(s->second.controller && !changed) CallGameVirtual<void,2>(s->second.controller,dt);
    uint32_t r=--s->base.refs;
    if(!r) { s->base.refs=1; return CallGameVirtual<uint32_t,0>(s,1); }
    return r;
}
}
