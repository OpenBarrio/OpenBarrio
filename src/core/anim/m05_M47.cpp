#include "ts3/anim/m05_M47.h"
#include <atomic>
#include <xmmintrin.h>
namespace ts3::anim::m47 {
namespace {
void* Table(uintptr_t a) {
#if defined(_M_IX86)
    return reinterpret_cast<void*>(a);
#else
    (void)a; return nullptr;
#endif
}
void Add(Ref* r,bool atomic) {
    if(r) { if(atomic) { std::atomic_ref<int32_t>(r->refs).fetch_add(1); } else ++r->refs; }
}
void Release(Ref* r,bool atomic) {
    if(!r) return;
    int32_t old=atomic?std::atomic_ref<int32_t>(r->refs).fetch_sub(1):r->refs--;
    if(old==1) { if(atomic) std::atomic_ref<int32_t>(r->refs).exchange(1); else r->refs=1;
        CallGameVirtual<uintptr_t,0>(r,1u); }
}
void Tables(Scratch* s) {
    s->base.vtable=Table(0xFC2068); s->base.secondary8=Table(0xFC2060);
    s->base.secondaryC=Table(0xFC205C); s->listener=Table(0xFC1E5C);
}
unsigned Count(Ref* curve) {
    // Opaque external curve layout: list sentinel +14 (x86); not a reconstructed class.
    auto sentinel=reinterpret_cast<void**>(reinterpret_cast<unsigned char*>(curve)+sizeof(void*)*5);
    unsigned count=0;
    for(void* p=*sentinel;p!=sentinel;p=*static_cast<void**>(p)) ++count;
    return count;
}
float Constant(uintptr_t address) {
#if defined(_M_IX86)
    return *reinterpret_cast<const float*>(address);
#else
    (void)address; AbortNotRewritten("M47 game float constant"); return 0;
#endif
}
}
Scratch* Construct(Scratch* s,void* host,Ref* source,Ref* curve,Ref* controller,uint32_t a,uint32_t b,float weight,uint8_t x,uint8_t y,uint8_t z) {
    CallGameMethod<void,0x00565A80>(&s->base,host);
    Tables(s);
    s->source=source; Add(source,true);
    s->controller=controller; Add(controller,false);
    s->curve=curve; Add(curve,true);
    s->value3c=a; s->value40=b; s->weight=weight;
    s->flag44=x; s->flag45=y; s->flag46=z;
    if(controller) CallGameMethod<void,0x00566170>(reinterpret_cast<unsigned char*>(controller)+3*sizeof(void*),&s->listener,-1);
    return s;
}
uintptr_t Destroy(Scratch* s) {
    Tables(s);
    if(s->controller) CallGameMethod<void,0x00566590>(reinterpret_cast<unsigned char*>(s->controller)+3*sizeof(void*),&s->listener);
    Release(s->curve,true);
    Release(s->controller,false);
    Release(s->source,true);
    return CallGameMethod<uintptr_t,0x00565B40>(&s->base);
}
Scratch* Delete(Scratch* s,uint32_t flags) {
    Destroy(s);
    if(flags&1) CallGameFunction<void,0x004E3F70>(s);
    return s;
}
uintptr_t Relay(void* listener,const Event* event,uint32_t unused) {
    (void)unused;
    auto s=reinterpret_cast<Scratch*>(reinterpret_cast<unsigned char*>(listener)-offsetof(Scratch,listener));
    if(s) ++s->base.refs;
    Event copy; copy.sender=event->sender; copy.resource=event->resource;
    Add(copy.resource,true);
    copy.host=s->base.hostSlot; copy.a=event->a; copy.b=event->b;
    copy.x=event->x; copy.y=event->y; copy.z=event->z; copy.sender=s;
    uintptr_t result=CallGameMethod<uintptr_t,0x00565A30>(s,&copy);
    Release(copy.resource,true);
    if(s) {
        result=static_cast<uint32_t>(--s->base.refs);
        if(!s->base.refs) { s->base.refs=1; result=CallGameVirtual<uintptr_t,0>(s,1u); }
    }
    return result;
}
Float4Value* Sample(Scratch* s,Float4Value* out,uint32_t a,uint32_t b,float t) {
    (void)a; (void)b;
    auto forward=[&]() { CallGameMethod<void,0x0085D890>(s->curve,out,t); return out; };
    auto reverse=[&]() { CallGameMethod<void,0x0085F570>(s->curve,out,Constant(0xF988F0)-t); return out; };
    if(t>=Constant(0xF98964) && !CallGameMethod<uint8_t,0x0085AF40>(s->curve)) return forward();
    if(t<0.f) {
        if(Count(s->curve)>0 && !CallGameMethod<uint8_t,0x0085AF40>(s->curve)) return reverse();
        if(Count(s->curve)>0) return reverse();
    }
    if(!CallGameMethod<uint8_t,0x0085AF40>(s->curve)) return forward();
    if(Count(s->curve)>0) return reverse();
    *out={}; return out;
}
float Crossing(Scratch* s,uint32_t a,uint32_t b,float t,float step) {
    bool curveOnly=s->curve && !CallGameMethod<uint8_t,0x0085AF40>(s->curve) && Count(s->curve)>=3;
    bool mixed=false;
    if(!curveOnly) {
        if(s->curve && !s->source) return 0.f;
        mixed=s->curve && Count(s->curve)>=3;
        if(!mixed && !s->source) return 0.f;
    }
    float base=t-step*Constant(0xF9A5AC);
    float adjusted=base;
    if((curveOnly||mixed) && s->flag45) adjusted-=*reinterpret_cast<float*>(reinterpret_cast<unsigned char*>(s->curve)+0xA0);
    alignas(16) Float4Value values[4];
    float times[4]={adjusted-step,adjusted,adjusted+step,step*Constant(0xF98A58)+adjusted};
    float rawTimes[4]={base-step,base,base+step,step*Constant(0xF98A58)+base};
    for(unsigned i=0;i<4;++i) {
        if(curveOnly || (mixed && times[i]<0.f)) Sample(s,&values[i],a,b,times[i]);
        else CallGameMethod<Float4Value*,0x0056E1E0>(s,&values[i],a,b,rawTimes[i]);
    }
    float phase=(t-base)/step;
    __m128 v0=_mm_load_ps(reinterpret_cast<const float*>(&values[0]));
    __m128 v1=_mm_load_ps(reinterpret_cast<const float*>(&values[1]));
    __m128 v2=_mm_load_ps(reinterpret_cast<const float*>(&values[2]));
    __m128 v3=_mm_load_ps(reinterpret_cast<const float*>(&values[3]));
    auto small=[&](__m128 d) {
        __m128 neg=_mm_mul_ps(_mm_set1_ps(Constant(0xFAA210)),d);
        return (_mm_movemask_ps(_mm_cmple_ps(_mm_max_ps(neg,d),_mm_set1_ps(Constant(0xFC2044))))&7)==7;
    };
    if(small(_mm_sub_ps(v0,v1)) || small(_mm_sub_ps(v2,v3))) return 0.f;
    float result=0.f;
    CallGameFunction<void,0x007D2460>(phase,values,0u,0u,&result);
    return result;
}
}

#if defined(_M_IX86)
namespace ts3::anim::m47_hooks {
using namespace m47;
Scratch* __fastcall Construct(Scratch* s,void*,void* host,Ref* source,Ref* curve,Ref* controller,uint32_t a,uint32_t b,float weight,uint8_t x,uint8_t y,uint8_t z) { return m47::Construct(s,host,source,curve,controller,a,b,weight,x,y,z); }
uintptr_t __fastcall Destroy(Scratch* s,void*) { return m47::Destroy(s); }
uintptr_t __fastcall Relay(void* s,void*,const Event* e,uint32_t unused) { return m47::Relay(s,e,unused); }
Scratch* __fastcall Delete(Scratch* s,void*,uint32_t flags) { return m47::Delete(s,flags); }
Float4Value* __fastcall Sample(Scratch* s,void*,Float4Value* out,uint32_t a,uint32_t b,float t) { return m47::Sample(s,out,a,b,t); }
float __fastcall Crossing(Scratch* s,void*,uint32_t a,uint32_t b,float t,float step) { return m47::Crossing(s,a,b,t,step); }
}
#endif
