#include "ts3/anim/m05_M35.h"
#include "ts3/scratch/clu_4e3f70_Scratch.h"
#include <xmmintrin.h>
#include <cmath>
#include <intrin.h>
namespace ts3::anim::m35 {
namespace {
uintptr_t Free(void* p) { return reinterpret_cast<uintptr_t>(scratch::Free4E3F70(p)); }
void Tables(State* s,bool derived) {
#if defined(_M_IX86)
    s->base.vtable=reinterpret_cast<void*>(uintptr_t(derived?0xfc1cb0:0xfc1c00));
    s->base.secondary8=reinterpret_cast<void*>(uintptr_t(derived?0xfc1ca8:0xfc1bf8));
    s->base.secondaryC=reinterpret_cast<void*>(uintptr_t(derived?0xfc1ca4:0xfc1bf4));
#else
    (void)s; (void)derived;
    AbortNotRewritten("m05_M35 original controller vtables");
#endif
}
uintptr_t BaseDestroy(State* s,uintptr_t seed) {
#if defined(_M_IX86)
    // Empty delegate destruction leaves EAX untouched: pass the exact incoming value.
    uintptr_t result;
    __asm {
        mov ecx,s
        mov eax,seed
        mov edx,0x00565b40
        call edx
        mov result,eax
    }
    return result;
#else
    (void)seed; return CallGameMethod<uintptr_t,0x00565b40>(s);
#endif
}
uintptr_t DestroyRange(State* s) {
    Tables(s,false); auto p=s->begin; s->end=p;
    uintptr_t result=reinterpret_cast<uintptr_t>(p);
    if(p) result=Free(p);
    return BaseDestroy(s,result);
}
}
uintptr_t Destroy(State* s) {
    Tables(s,true);
    s->end=s->begin; s->ratesEnd=s->rates; s->extraEnd=s->extra;
    if(s->extra) Free(s->extra);
    if(s->rates) Free(s->rates);
    return DestroyRange(s);
}
State* BaseDelete(State* s,uint32_t flags) {
    DestroyRange(s);
    if(flags&1) Free(s);
    return s;
}
State* Delete(State* s,uint32_t flags) {
    Destroy(s);
    if(flags&1) Free(s);
    return s;
}
State* SecondaryDelete(void* secondary,uint32_t flags) {
    return Delete(reinterpret_cast<State*>(static_cast<unsigned char*>(secondary)-offsetof(State,base)-offsetof(Controller,secondaryC)),flags);
}
uintptr_t Tick(State* s,float dt) {
    s->base.refs=static_cast<int32_t>(uint32_t(s->base.refs)+1);
    Event event{s,nullptr,0,0,0,{0,0,0}};
    if(s->time<=0.0f) CallGameMethod<void,0x00568620>(s,&event);
    if(dt>0.0f) s->time=s->time+dt;
    if(event.resource) {
        auto p=event.resource;
        if(_InterlockedDecrement(reinterpret_cast<volatile long*>(&p->atomicRefs))==0) {
            _InterlockedExchange(reinterpret_cast<volatile long*>(&p->atomicRefs),1);
            CallGameVirtual<uintptr_t,0>(p,1u);
        }
    }
    auto refs=uint32_t(s->base.refs)-1u; s->base.refs=static_cast<int32_t>(refs);
    if(refs) return refs;
    s->base.refs=1;
    return CallGameVirtual<uintptr_t,0>(s,1u);
}
uintptr_t Rotate(State* s,float dt) {
    auto result=Tick(s,dt);
    auto p=s->rates; auto end=s->ratesEnd; auto out=s->begin;
    const __m128 one=_mm_set1_ps(1.0f),half=_mm_set1_ps(0.5f);
    for(;p!=end;++p,++out) {
        float period=p->period;
        unsigned ah=std::isnan(period)?0x47:period==0?0x42:period<0?0x03:0x02;
        result=(result&~uintptr_t(0xff00))|(uintptr_t(ah)<<8);
        // Ordered zero alone bypasses the divide (unordered takes the arithmetic path).
        float angle=0;
        if(period!=0) angle=(p->speed*dt)/period;
        angle=angle/2.0f;
        float sn,cosine;
#if defined(_M_IX86)
        __asm {
            fld angle
            fsin
            fstp sn
            fld angle
            fcos
            fstp cosine
        }
#else
        sn=std::sin(angle); cosine=std::cos(angle);
#endif
        auto axis=_mm_load_ps(p->axis);
        auto x=_mm_mul_ps(_mm_shuffle_ps(axis,axis,0),_mm_set1_ps(sn));
        auto y=_mm_mul_ps(_mm_shuffle_ps(axis,axis,0x55),_mm_set1_ps(sn));
        auto z=_mm_mul_ps(_mm_shuffle_ps(axis,axis,0xaa),_mm_set1_ps(sn));
        auto w=_mm_set1_ps(cosine);
        auto q=_mm_shuffle_ps(_mm_shuffle_ps(x,y,0x10),_mm_shuffle_ps(z,w,0x32),0x88);
        auto a=_mm_load_ps(out->axis);
        auto cross=_mm_sub_ps(_mm_mul_ps(_mm_shuffle_ps(q,q,0x12),_mm_shuffle_ps(a,a,9)),_mm_mul_ps(_mm_shuffle_ps(q,q,9),_mm_shuffle_ps(a,a,0x12)));
        auto aw=_mm_shuffle_ps(a,a,0xff),qw=_mm_shuffle_ps(q,q,0xff);
        auto v=_mm_add_ps(_mm_add_ps(cross,_mm_mul_ps(aw,q)),_mm_mul_ps(a,qw));
        auto prod=_mm_mul_ps(a,q);
        auto dot=_mm_add_ps(_mm_add_ps(_mm_shuffle_ps(prod,prod,2),_mm_shuffle_ps(prod,prod,1)),prod);
        auto scalar=_mm_sub_ps(_mm_mul_ps(qw,aw),_mm_shuffle_ps(dot,dot,0));
        auto tmp=_mm_shuffle_ps(v,scalar,0x0a);
        auto quat=_mm_shuffle_ps(v,tmp,0x84);
        auto sq=_mm_mul_ps(quat,quat);
        auto sum=_mm_add_ps(_mm_shuffle_ps(sq,sq,0x0e),sq);
        auto len=_mm_add_ps(_mm_shuffle_ps(sum,sum,1),sum); len=_mm_shuffle_ps(len,len,0);
        auto r=_mm_rsqrt_ps(len);
        auto err=_mm_sub_ps(one,_mm_mul_ps(_mm_mul_ps(r,r),len));
        r=_mm_add_ps(_mm_mul_ps(err,_mm_mul_ps(half,r)),r);
        err=_mm_sub_ps(one,_mm_mul_ps(_mm_mul_ps(r,r),len));
        r=_mm_add_ps(_mm_mul_ps(err,_mm_mul_ps(half,r)),r);
        _mm_store_ps(out->axis,_mm_mul_ps(r,quat));
    }
    return result;
}
}
