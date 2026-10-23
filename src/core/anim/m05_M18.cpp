#include "ts3/anim/m05_M18.h"
#if defined(_MSC_VER)
#include <intrin.h>
#endif
namespace ts3::anim::m18 {
namespace {
void* Table(uint32_t a) {
#if defined(_M_IX86)
    return reinterpret_cast<void*>(a);
#else
    (void)a; return nullptr;
#endif
}
void Release(ResourcePrefix* r) {
    if(!r) return;
#if defined(_MSC_VER)
    auto old=_InterlockedExchangeAdd(reinterpret_cast<volatile long*>(&r->atomicRefs),-1);
    if(static_cast<uint32_t>(old)-1u) return;
    _InterlockedExchange(reinterpret_cast<volatile long*>(&r->atomicRefs),1);
#else
    if(--r->atomicRefs) return;
    r->atomicRefs=1;
#endif
    CallGameVirtual<void,0>(r,1u);
}
// Keep the unrounded x87 sum for the reverse fallback decision.
bool NegativeSum(ResourcePrefix* c,uint32_t lo,uint32_t hi,float delta) {
#if defined(_M_IX86)
    using Fn=double(__thiscall*)(ResourcePrefix*,uint32_t,uint32_t);
    auto fn=reinterpret_cast<Fn>((*reinterpret_cast<void***>(c))[41]);
    unsigned char negative;
    __asm {
        push hi
        push lo
        mov ecx,c
        call fn
        fadd delta
        fldz
        fcomip st(0),st(1)
        fstp st(0)
        seta negative
    }
    return negative!=0;
#else
    return CallGameVirtual<double,41>(c,lo,hi)+delta<0;
#endif
}
float Sum(ResourcePrefix* c,uint32_t lo,uint32_t hi,float delta) {
#if defined(_M_IX86)
    using Fn=double(__thiscall*)(ResourcePrefix*,uint32_t,uint32_t);
    auto fn=reinterpret_cast<Fn>((*reinterpret_cast<void***>(c))[41]);
    float sum;
    __asm {
        push hi
        push lo
        mov ecx,c
        call fn
        fadd delta
        fstp sum
    }
    return sum;
#else
    return static_cast<float>(CallGameVirtual<double,41>(c,lo,hi)+delta);
#endif
}
}
void Destroy(Sampler* s) {
    s->vtable=Table(0x00fc1618);
    Release(s->track);
    Release(s->controller);
    s->vtable=Table(0x00fc1080);
}
Sampler* Delete(Sampler* s,uint32_t flags) {
    // Same deleting-wrapper pattern as verified GlyphMetricsMap::DeleteAndFree.
    Destroy(s);
    if(flags&1u) CallGameFunction<void,0x004e3f70>(s);
    return s;
}
bool Sample(Sampler* s,float time,float delta,uint8_t enabled,Float4Value* output,Float4Value* auxiliary) {
    alignas(16) Float4Value temp;
    if(s->track && enabled && time>=0.f) {
        if(!CallGameMethod<uint8_t,0x0085af40>(s->track)) {
            *output=*CallGameMethod<Float4Value*,0x0085cc70>(s->track,&temp,time,auxiliary);
            return true;
        }
    }
    if(s->track && time<0.f) {
        auto h=&reinterpret_cast<TrackLinks*>(s->track)->links;
        unsigned count=0;
        for(auto p=h->next;p!=h;p=p->next) ++count;
        if(count>=2) {
            uint8_t fallback=0;
            // EA uses SUBSS (-0)-time, not a sign-bit flip.
            float reverse=-0.f-time;
            *output=*CallGameMethod<Float4Value*,0x0085d0a0>(s->track,&temp,reverse,0u,&fallback,auxiliary);
            if(!fallback) return true;
            if(s->controller && NegativeSum(s->controller,s->keyLow,s->keyHigh,delta)) return true;
        }
    }
    if(!s->controller) return false;
    float sum=Sum(s->controller,s->keyLow,s->keyHigh,delta);
    if(!s->controller || !(sum>=0.f)) return false;
    *output=*CallGameFunction<Float4Value*,0x0055e090>(&temp,s->controller,s->keyLow,s->keyHigh,delta,0u,auxiliary,s->track);
    return true;
}
bool SampleSimple(Sampler* s,float time,float delta,uint8_t enabled,Float4Value* output) {
    alignas(16) Float4Value auxiliary;
    return Sample(s,time,delta,enabled,output,&auxiliary);
}
#if defined(_M_IX86)
namespace hooks {
void __fastcall Destroy(Sampler* s,void*){m18::Destroy(s);}
Sampler* __fastcall Delete(Sampler* s,void*,uint32_t f){return m18::Delete(s,f);}
bool __fastcall Sample(Sampler* s,void*,float t,float d,uint8_t e,Float4Value* o,Float4Value* a){return m18::Sample(s,t,d,e,o,a);}
bool __fastcall SampleSimple(Sampler* s,void*,float t,float d,uint8_t e,Float4Value* o){return m18::SampleSimple(s,t,d,e,o);}
}
#endif
}
