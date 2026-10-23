#include "ts3/anim/m05_M53.h"
#include <xmmintrin.h>
#include <intrin.h>
namespace ts3::anim::m53 {
static void* Address(uint32_t a) {
#if defined(_M_IX86)
    return reinterpret_cast<void*>(uintptr_t(a));
#else
    (void)a; return nullptr;
#endif
}
static void Tables(Motion* p) {
    p->f_00=Address(0x00fc21e0); p->f_08=Address(0x00fc21d4);
    p->f_0C=Address(0x00fc21d0); p->f_28=Address(0x00fc21bc);
}
static Ref* Retained(Motion* p) { return static_cast<Ref*>(p->f_2C); }
static void* Interface(Ref* p) { return reinterpret_cast<unsigned char*>(p)+2*sizeof(void*)+sizeof(int32_t); }
static uintptr_t Release(Ref* p) {
    uint32_t n=uint32_t(p->refs)-1u; p->refs=int32_t(n);
    if(n) return n;
    p->refs=1; return CallGameVirtual<uintptr_t,0>(p,1u);
}
Motion* Construct(Motion* p,void* host,void* retained,const float* from,const float* to) {
    CallGameMethod<void,0x00565a80>(p,host);
    Tables(p); p->f_2C=retained;
    if(retained) ++Retained(p)->refs;
    float duration=CallGameVirtual<float,8>(retained);
    __m128 delta=_mm_sub_ps(_mm_load_ps(to),_mm_load_ps(from));
    __m128 sq=_mm_mul_ps(delta,delta);
    __m128 sum=_mm_add_ps(_mm_add_ps(_mm_shuffle_ps(sq,sq,2),_mm_shuffle_ps(sq,sq,1)),sq);
    p->f_40=_mm_cvtss_f32(_mm_sqrt_ps(sum));
    _mm_store_ps(p->f_30,_mm_div_ps(delta,_mm_set1_ps(duration)));
    CallGameMethod<void,0x00566170>(retained?Interface(Retained(p)):nullptr,&p->f_28,-1);
    return p;
}
uintptr_t Destruct(Motion* p) {
    Tables(p);
    if(p->f_2C) CallGameMethod<void,0x00566590>(Interface(Retained(p)),&p->f_28);
    if(p->f_2C) Release(Retained(p));
    return CallGameMethod<uintptr_t,0x00565b40>(p);
}
static Motion* Owner(void* sub) { return reinterpret_cast<Motion*>(static_cast<unsigned char*>(sub)-offsetof(Motion,f_28)); }
template<unsigned Target> static uintptr_t Dispatch(Motion* p,const Event* event) {
    ++p->f_04;
    Event copy; // Padding is deliberately not initialized, as in EA's stack record.
    copy.source=event->source; copy.resource=event->resource;
    if(copy.resource) _InterlockedIncrement(reinterpret_cast<volatile long*>(&copy.resource->refs));
    copy.arg0=event->arg0; copy.arg1=event->arg1;
    copy.flags[0]=event->flags[0]; copy.flags[1]=event->flags[1]; copy.flags[2]=event->flags[2];
    copy.controller=p->f_1C; copy.source=p;
    CallGameMethod<void,Target>(p,&copy);
    if(copy.resource && _InterlockedExchangeAdd(reinterpret_cast<volatile long*>(&copy.resource->refs),-1)==1) {
        _InterlockedExchange(reinterpret_cast<volatile long*>(&copy.resource->refs),1);
        CallGameVirtual<void,0>(copy.resource,1u);
    }
    return Release(reinterpret_cast<Ref*>(p));
}
uintptr_t MotionEvent(void* sub,const Event* event,uint32_t) {
    Motion* p=Owner(sub);
    if(p->f_1C) {
        void* target=CallGameVirtual<void*,1>(p->f_1C,0x549b548bu);
        if(target && p->f_40>0.0f) {
            CallGameVirtual<void,10>(target,0.0f); CallGameVirtual<void,14>(target,0.0f);
            CallGameVirtual<void,12>(target,1.0f); CallGameVirtual<void,16>(target,1.0f);
            alignas(16) float rotation[4]={0,0,0,1};
            CallGameVirtual<void,4>(target,rotation);
            __m128 q=_mm_xor_ps(_mm_load_ps(rotation),_mm_set_ps(0.0f,-0.0f,-0.0f,-0.0f));
            __m128 v=_mm_load_ps(p->f_30);
            __m128 q12=_mm_shuffle_ps(q,q,0x12), q09=_mm_shuffle_ps(q,q,9);
            __m128 cross=_mm_sub_ps(_mm_mul_ps(_mm_shuffle_ps(v,v,0x12),q09),_mm_mul_ps(_mm_shuffle_ps(v,v,9),q12));
            __m128 t=_mm_add_ps(cross,_mm_mul_ps(_mm_shuffle_ps(q,q,255),v));
            __m128 rotated=_mm_add_ps(_mm_mul_ps(_mm_sub_ps(_mm_mul_ps(_mm_shuffle_ps(t,t,0x12),q09),_mm_mul_ps(_mm_shuffle_ps(t,t,9),q12)),_mm_set1_ps(2)),v);
            __m128 sq=_mm_mul_ps(rotated,rotated);
            __m128 sum=_mm_add_ps(_mm_add_ps(_mm_shuffle_ps(sq,sq,2),_mm_shuffle_ps(sq,sq,1)),sq);
            sum=_mm_shuffle_ps(sum,sum,0);
            __m128 r=_mm_rsqrt_ps(sum),one=_mm_set1_ps(1),half=_mm_set1_ps(0.5f);
            r=_mm_add_ps(_mm_mul_ps(_mm_sub_ps(one,_mm_mul_ps(_mm_mul_ps(r,r),sum)),_mm_mul_ps(half,r)),r);
            r=_mm_add_ps(_mm_mul_ps(_mm_sub_ps(one,_mm_mul_ps(_mm_mul_ps(r,r),sum)),_mm_mul_ps(half,r)),r);
            alignas(16) float velocity[4];
            _mm_store_ps(velocity,_mm_mul_ps(_mm_mul_ps(r,rotated),_mm_set1_ps(p->f_40)));
            CallGameVirtual<void,6>(target,velocity);
        }
    }
    return Dispatch<0x00565a30>(p,event);
}
uintptr_t ForwardEvent(void* sub,const Event* event,uint32_t) { return Dispatch<0x00568620>(Owner(sub),event); }
void* __stdcall FactoryA(void* arg,void* storage) {
    if(!storage) return nullptr;
    return CallGameMethod<void*,0x0054c7f0>(storage,arg);
}
void* __stdcall FactoryB(void* arg,void* storage) {
    if(!storage) return nullptr;
    return CallGameMethod<void*,0x0054cce0>(storage,arg);
}
#if defined(_M_IX86)
Motion* __fastcall HookConstruct(Motion* p,void*,void* a,void* b,const float* c,const float* d) { return Construct(p,a,b,c,d); }
uintptr_t __fastcall HookDestruct(Motion* p,void*) { return Destruct(p); }
uintptr_t __fastcall HookMotionEvent(void* p,void*,const Event* e,uint32_t a) { return MotionEvent(p,e,a); }
uintptr_t __fastcall HookForwardEvent(void* p,void*,const Event* e,uint32_t a) { return ForwardEvent(p,e,a); }
#endif
}
