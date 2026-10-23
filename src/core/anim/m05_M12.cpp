#include "ts3/anim/m05_M12.h"
#include "ts3/layout.h"
#include <cmath>
#include <cstring>
namespace ts3::anim::m12 {
namespace {
using V=__m128;
V S(float x) { return _mm_set1_ps(x); }
float F(V x) { return _mm_cvtss_f32(x); }
float From(uint32_t u) { float f; std::memcpy(&f,&u,4); return f; }
uint32_t Bits(float f) { uint32_t u; std::memcpy(&u,&f,4); return u; }
V Dot3(V a,V b) { V p=_mm_mul_ps(a,b); V q=_mm_add_ps(_mm_add_ps(_mm_shuffle_ps(p,p,2),_mm_shuffle_ps(p,p,1)),p); return _mm_shuffle_ps(q,q,0); }
V Dot4(V a,V b) { V p=_mm_mul_ps(a,b); V q=_mm_add_ps(_mm_shuffle_ps(p,p,14),p); q=_mm_add_ps(_mm_shuffle_ps(q,q,1),q); return _mm_shuffle_ps(q,q,0); }
V Refine(V r,V n) { return _mm_add_ps(_mm_mul_ps(_mm_sub_ps(S(1),_mm_mul_ps(_mm_mul_ps(r,r),n)),_mm_mul_ps(S(.5f),r)),r); }
V Norm(V v,bool four,bool twice) { V n=four?Dot4(v,v):Dot3(v,v); V r=Refine(_mm_rsqrt_ps(n),n); if(twice) r=Refine(r,n); return _mm_mul_ps(r,v); }
V Cross(V a,V b) { return _mm_sub_ps(_mm_mul_ps(_mm_shuffle_ps(a,a,9),_mm_shuffle_ps(b,b,18)),_mm_mul_ps(_mm_shuffle_ps(a,a,18),_mm_shuffle_ps(b,b,9))); }
float Sin(float x) {
#if defined(_M_IX86)
    float r; __asm { fld x
 fsin
 fstp r
 } return r;
#else
    return static_cast<float>(std::sin(static_cast<double>(x)));
#endif
}
float Cos(float x) {
#if defined(_M_IX86)
    float r; __asm { fld x
 fcos
 fstp r
 } return r;
#else
    return static_cast<float>(std::cos(static_cast<double>(x)));
#endif
}
__declspec(noinline) float Acos(float x,uintptr_t* eaxOut=nullptr) {
#if defined(_M_IX86)
    float r; uintptr_t saved; uintptr_t fn=0x00f7edde; __asm { fld x
 call fn
 mov saved,eax
 fstp r
 }
    if(eaxOut) *eaxOut=saved;
    return r;
#else
    if(eaxOut) *eaxOut=0;
    return static_cast<float>(std::acos(static_cast<double>(x)));
#endif
}
V Blend(V q,V rotation,float weight,uintptr_t& result) {
    V dot=Dot4(q,rotation),zero=_mm_setzero_ps(),w=S(weight);
    if(F(dot)<0) { rotation=_mm_sub_ps(zero,rotation); dot=_mm_sub_ps(zero,dot); }
    if(F(dot)>Cos(From(0x3db2b8c3))) {
        V mask=_mm_cmplt_ps(zero,Dot4(q,rotation));
        V a=_mm_add_ps(_mm_mul_ps(_mm_sub_ps(q,rotation),w),rotation);
        V b=_mm_sub_ps(rotation,_mm_mul_ps(_mm_add_ps(q,rotation),w));
        return Norm(_mm_or_ps(_mm_and_ps(a,mask),_mm_andnot_ps(mask,b)),true,true);
    }
    V angle=S(Acos(F(dot),&result));
    V a=_mm_mul_ps(_mm_sub_ps(S(1),w),angle), b=_mm_mul_ps(angle,w);
    V left=_mm_mul_ps(_mm_div_ps(S(Sin(F(a))),S(Sin(F(angle)))),rotation);
    V right=_mm_mul_ps(_mm_div_ps(S(Sin(F(b))),S(Sin(F(angle)))),q);
    return _mm_add_ps(left,right);
}
}
__m128* Constraint(__m128* out,const __m128* target,const __m128* frame,const __m128* pose) {
#if defined(_M_IX86)
    auto flags=reinterpret_cast<uint32_t*>(0x011cd420);
    auto storage=reinterpret_cast<float*>(0x011cd41c);
    if(!(*flags&1)) { *flags|=1; *storage=From(1057360530); }
    const float limit=*storage;
#else
    const float limit=From(1057360530);
#endif
    alignas(16) V inverse[3];
    CallGameFunction<void,0x0054eef0>(frame,inverse);
    V direction=Norm(_mm_sub_ps(*target,*frame),false,true);
    V unit=_mm_set_ps(0,0,0,1),axis;
    CallGameFunction<void,0x00555110>(&unit,frame,&axis);
    V normal=Norm(Cross(axis,direction),false,true);
    V a=Norm(axis,false,false), b=Norm(direction,false,false);
    V dot=_mm_min_ps(_mm_max_ps(Dot3(b,a),S(-1)),S(1));
    V angle=S(Acos(F(dot)));
    if(F(Dot3(_mm_sub_ps(_mm_mul_ps(_mm_shuffle_ps(b,b,18),_mm_shuffle_ps(a,a,9)),_mm_mul_ps(_mm_shuffle_ps(b,b,9),_mm_shuffle_ps(a,a,18))),normal))<0)
        angle=_mm_sub_ps(S(From(0x40c90fdb)),angle);
    // EA's clamp tests the SIGN BIT of the subtraction, not an ordered comparison.

    float low=From(0x80000000)-limit;
    uint32_t mask=0u-(Bits(low-F(angle))>>31);
    float clamped=From((mask&Bits(F(angle)))|(~mask&Bits(low)));
    mask=0u-(Bits(limit-clamped)>>31);
    clamped=From((mask&Bits(limit))|(~mask&Bits(clamped)));
    V localAxis;
    CallGameFunction<void,0x00555110>(&normal,inverse,&localAxis);
    float half=F(_mm_div_ps(S(clamped),S(2)));
    V s=S(Sin(half));
    V x=_mm_mul_ps(_mm_shuffle_ps(localAxis,localAxis,0),s);
    V y=_mm_mul_ps(_mm_shuffle_ps(localAxis,localAxis,85),s);
    V z=_mm_mul_ps(_mm_shuffle_ps(localAxis,localAxis,170),s);
    V q=_mm_shuffle_ps(_mm_shuffle_ps(x,y,16),_mm_shuffle_ps(z,S(Cos(half)),50),136);
    V r=pose[1],qw=_mm_shuffle_ps(q,q,255),rw=_mm_shuffle_ps(r,r,255);
    V xyz=_mm_add_ps(_mm_add_ps(_mm_sub_ps(_mm_mul_ps(_mm_shuffle_ps(r,r,18),_mm_shuffle_ps(q,q,9)),_mm_mul_ps(_mm_shuffle_ps(r,r,9),_mm_shuffle_ps(q,q,18))),_mm_mul_ps(qw,r)),_mm_mul_ps(rw,q));
    V w=_mm_sub_ps(_mm_mul_ps(rw,qw),Dot3(q,r));
    *out=_mm_shuffle_ps(xyz,_mm_shuffle_ps(xyz,w,10),132);
    return out;
}
uintptr_t BlendFirst(const __m128* target,const __m128* frame,__m128* pose,float weight) {
    alignas(16) V q[2];
    uintptr_t result=CallGameFunction<uintptr_t,0x00555360>(q,target,frame,pose);
    pose[1]=Blend(q[0],pose[1],weight,result);
    return result;
}
uintptr_t BlendSecond(const __m128* target,const __m128* frame,__m128* pose,float weight) {
    alignas(16) V q[2];
    uintptr_t result=reinterpret_cast<uintptr_t>(Constraint(q,target,frame,pose));
    pose[1]=Blend(q[0],pose[1],weight,result);
    return result;
}
}
