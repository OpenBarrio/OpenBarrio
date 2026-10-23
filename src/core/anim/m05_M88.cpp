#include "ts3/anim/m05_M88.h"
#include <xmmintrin.h>
#include <cmath>
#pragma float_control(precise,on,push)
namespace ts3::anim::m88 {
namespace {
__m128 Splat(__m128 v) { return _mm_shuffle_ps(v,v,0); }
__m128 Dot3(__m128 v) { return Splat(_mm_add_ps(_mm_add_ps(_mm_shuffle_ps(v,v,2),_mm_shuffle_ps(v,v,1)),v)); }
__m128 Refine(__m128 n) {
    __m128 r=_mm_rsqrt_ps(n), one=_mm_set1_ps(1), half=_mm_set1_ps(.5f);
    __m128 t=_mm_sub_ps(one,_mm_mul_ps(_mm_mul_ps(r,r),n));
    __m128 s=_mm_add_ps(_mm_mul_ps(t,_mm_mul_ps(half,r)),r);
    return _mm_add_ps(_mm_mul_ps(_mm_sub_ps(one,_mm_mul_ps(_mm_mul_ps(s,s),n)),_mm_mul_ps(half,s)),s);
}
__declspec(noinline) float Trig(float f,int op,uintptr_t* eax) {
#if defined(_M_IX86)
    float result;
    if(op==0) {
        uintptr_t reg; const uintptr_t target=0x00f7edde;
        __asm { fld f
            call target
            mov reg,eax
            fstp result }
        *eax=reg;
    } else if(op==1) {
        __asm { fld f
            fsin
            fstp result }
    } else {
        __asm { fld f
            fcos
            fstp result }
    }
    return result;
#else
    (void)eax;
    return float(op==0?std::acos(double(f)):op==1?std::sin(double(f)):std::cos(double(f)));
#endif
}
}
uintptr_t Finalize(b20::Pose* p) {
    auto* bind=p->rig->localBegin;
    auto* dst=b20::Local(p);
    float* weights=static_cast<float*>(p->auxiliary);
    uintptr_t result=reinterpret_cast<uintptr_t>(dst);
    if(p->count) result=reinterpret_cast<uintptr_t>(bind)-reinterpret_cast<uintptr_t>(dst);
    for(unsigned i=0;i<p->count;++i) {
        auto* d=reinterpret_cast<float*>(dst+i);
        if(weights[i]>0.005f) {
            __m128 inv=_mm_set1_ps(1.0f/weights[i]);
            _mm_store_ps(d,_mm_mul_ps(inv,_mm_load_ps(d)));
            _mm_store_ps(d+8,_mm_mul_ps(_mm_load_ps(d+8),inv));
            __m128 q=_mm_mul_ps(inv,_mm_load_ps(d+4));
            __m128 n=_mm_mul_ps(q,q);
            __m128 t=_mm_add_ps(_mm_shuffle_ps(n,n,0x0e),n);
            n=Splat(_mm_add_ps(_mm_shuffle_ps(t,t,1),t));
            _mm_store_ps(d+4,_mm_mul_ps(Refine(n),q));
        } else {
            auto* s=reinterpret_cast<const float*>(bind+i);
            _mm_store_ps(d,_mm_load_ps(s));
            _mm_store_ps(d+4,_mm_load_ps(s+4));
            _mm_store_ps(d+8,_mm_load_ps(s+8));
        }
    }
    b20::ClearWeights(p);
    return result;
}
uintptr_t Rotation(const Float4Value* a,const Float4Value* b,Float4Value* out) {
    __m128 av=_mm_load_ps(reinterpret_cast<const float*>(a)),bv=_mm_load_ps(reinterpret_cast<const float*>(b));
    __m128 n=_mm_sqrt_ps(_mm_mul_ps(Dot3(_mm_mul_ps(av,av)),Dot3(_mm_mul_ps(bv,bv))));
    __m128 r=_mm_rcp_ps(n);
    r=_mm_mul_ps(_mm_sub_ps(_mm_set1_ps(2),_mm_mul_ps(r,n)),r);
    uintptr_t result=reinterpret_cast<uintptr_t>(b);
    auto identity=[&]() { _mm_store_ps(reinterpret_cast<float*>(out),_mm_set_ps(1,0,0,0)); };
    if(!(_mm_cvtss_f32(_mm_max_ps(_mm_mul_ps(_mm_set1_ps(-1),n),n))>1.0e-6f)) { identity(); return result; }
    __m128 dot=_mm_mul_ps(Dot3(_mm_mul_ps(av,bv)),r);
    float angle=Trig(_mm_cvtss_f32(_mm_min_ps(_mm_set1_ps(1),_mm_max_ps(_mm_set1_ps(-1),dot))),0,&result);
    __m128 axis=_mm_sub_ps(_mm_mul_ps(_mm_shuffle_ps(av,av,9),_mm_shuffle_ps(bv,bv,0x12)),_mm_mul_ps(_mm_shuffle_ps(av,av,0x12),_mm_shuffle_ps(bv,bv,9)));
    bool negative=!(_mm_cvtss_f32(dot)>=0); // COMISS/JB also selects this on unordered.
    float length=_mm_cvtss_f32(Dot3(_mm_mul_ps(axis,axis)));
    if(!(1.0e-12f<length)) {
        if(!negative) { identity(); return result; }
        // EA chooses the largest absolute component, with ties falling toward Z.
        alignas(16) float v[4]; _mm_store_ps(v,av);
        __m128 abs=_mm_max_ps(_mm_mul_ps(_mm_set1_ps(-1),av),av);
        alignas(16) float m[4]; _mm_store_ps(m,abs);
        unsigned k=m[0]>m[1] ? (m[0]>m[2]?0:2) : (m[1]>m[2]?1:2);
        // The max above is abs; its operand order is significant for NaNs and signed zero.
        alignas(16) float cross[4]; _mm_store_ps(cross,axis);
        float neg;
        if(k==0) { neg=0.0f-v[1]; axis=_mm_set_ps(cross[3],0,v[0],neg); }
        else if(k==1) { neg=0.0f-v[2]; axis=_mm_set_ps(cross[3],v[1],neg,0); }
        else { neg=0.0f-v[0]; axis=_mm_set_ps(cross[3],neg,0,v[2]); }
        angle=3.1415927410125732f;
    }
    float half=_mm_cvtss_f32(_mm_div_ps(_mm_set1_ps(angle),_mm_set1_ps(2)));
    float sine=Trig(half,1,&result),cosine=Trig(half,2,&result);
    __m128 norm=_mm_mul_ps(Refine(Dot3(_mm_mul_ps(axis,axis))),axis);
    alignas(16) float v[4]; _mm_store_ps(v,_mm_mul_ps(norm,_mm_set1_ps(sine)));
    v[3]=cosine;
    _mm_store_ps(reinterpret_cast<float*>(out),_mm_load_ps(v));
    return negative?reinterpret_cast<uintptr_t>(out):result;
}
}
#pragma float_control(pop)
