#include "ts3/math/m04_B03.h"
#include <xmmintrin.h>
#include <intrin.h>
#include <cstring>
#include <cmath>
#pragma float_control(precise, on, push)
namespace ts3::math::b03 {
namespace {
__m128 Load(const void* p, size_t offset=0) { __m128 v; std::memcpy(&v, static_cast<const char*>(p)+offset,16); return v; }
void Store(void* p, __m128 v, size_t offset=0) { std::memcpy(static_cast<char*>(p)+offset,&v,16); _ReadWriteBarrier(); }
float Get(const void* p, unsigned i) { float v; std::memcpy(&v,static_cast<const char*>(p)+4*i,4); return v; }
void Put(void* p, unsigned i, float v) { std::memcpy(static_cast<char*>(p)+4*i,&v,4); }
uint32_t GetU(const void* p,unsigned i) { uint32_t v; std::memcpy(&v,static_cast<const char*>(p)+4*i,4); return v; }
void PutU(void* p,unsigned i,uint32_t v) { std::memcpy(static_cast<char*>(p)+4*i,&v,4); }
float Add(float a,float b) { return _mm_cvtss_f32(_mm_add_ss(_mm_set_ss(a),_mm_set_ss(b))); }
float Sub(float a,float b) { return _mm_cvtss_f32(_mm_sub_ss(_mm_set_ss(a),_mm_set_ss(b))); }
float Mul(float a,float b) { return _mm_cvtss_f32(_mm_mul_ss(_mm_set_ss(a),_mm_set_ss(b))); }
// MSVC may commute intrinsic ADDPS/MULPS even in precise mode. The game
// operand order selects between two different NaNs (including SSE indefinite).
__declspec(noinline) __m128 AddPS(__m128 a,__m128 b) {
#if defined(_M_IX86)
    __m128 r;
    __asm { movaps xmm0, a }
    __asm { addps xmm0, b }
    __asm { movaps r, xmm0 }
    return r;
#else
    return _mm_add_ps(a,b);
#endif
}
__declspec(noinline) __m128 MulPS(__m128 a,__m128 b) {
#if defined(_M_IX86)
    __m128 r;
    __asm { movaps xmm0, a }
    __asm { mulps xmm0, b }
    __asm { movaps r, xmm0 }
    return r;
#else
    return _mm_mul_ps(a,b);
#endif
}
__m128 Norm(__m128 v) {
    v=_mm_mul_ps(v,v);
    __m128 sum=_mm_add_ps(_mm_shuffle_ps(v,v,2),_mm_shuffle_ps(v,v,1));
    sum=_mm_add_ps(sum,v);
    return _mm_sqrt_ps(_mm_shuffle_ps(sum,sum,0));
}
void Advance(void* p, unsigned n) {
    uint32_t old=GetU(p,n), diff=(old+1)^old;
    PutU(p,n,old+1);
    float step=0.5f;
    do {
        float v=Get(p,0);
        Put(p,0,(old&1)?Sub(v,step):Add(v,step));
        diff>>=1; old>>=1; step=Mul(step,0.5f);
    } while(diff);
    uint32_t counter=GetU(p,n+1)+1,mask=3,inc=1;
    PutU(p,n+1,counter); step=1.0f/3.0f;
    while((counter&mask)==mask) {
        counter+=inc; PutU(p,n+1,counter);
        Put(p,1,Sub(Get(p,1),Mul(step,2.0f)));
        mask<<=2; inc<<=2; step=Mul(step,1.0f/3.0f);
    }
    Put(p,1,Add(step,Get(p,1)));
}
}
bool Heading(void*,float x,float y,float* out) {
    const float epsilon=0.01f,halfpi=1.5707963705062866f;
    float result; unsigned char valid;
#if defined(_M_IX86)
    // Keep the extended x87 sum and angle until the original explicit float spill.
    __asm {
        fld x
        fld st(0)
        fabs
        fld y
        fld st(0)
        fabs
        faddp st(2), st(0)
        fld epsilon
        fxch st(2)
        fcomip st(0), st(2)
        fstp st(1)
        jb small
        fchs
        fxch st(1)
        fpatan
        fadd halfpi
        fst result
        fldz
        fcomip st(0), st(1)
        fstp st(0)
        seta valid
        jmp angleDone
    small:
        fstp st(1)
        fstp st(0)
        mov valid, 2
        mov result, 0
    angleDone:
    }
    if(valid==2) { *out=0.0f; return false; }
    if(valid) result=Add(result,6.2831854820251465f);
#else
    if(!(std::fabs(static_cast<double>(x))+std::fabs(static_cast<double>(y))>=epsilon)) { *out=0.0f; return false; }
    double angle=std::atan2(-static_cast<double>(y),static_cast<double>(x))+halfpi;
    result=static_cast<float>(angle);
    if(angle<0) result=Add(result,6.2831854820251465f);
#endif
    *out=result; return true;
}
bool Heading4(void* self,uint32_t,float x,float y,float* out) { return Heading(self,x,y,out); }
uint32_t Sequence2(void* p) { Advance(p,2); return GetU(p,2); }
uint32_t Sequence3(void* p) {
    Advance(p,3);
    uint32_t count=GetU(p,5)+1,mask=7,inc=3,target=5;
    PutU(p,5,count); float step=0.2f;
    while((count&mask)==target) {
        count+=inc; PutU(p,5,count);
        Put(p,2,Sub(Get(p,2),Mul(step,4.0f)));
        mask<<=3; inc<<=3; target<<=3; step=Mul(step,0.2f);
    }
    Put(p,2,Add(Get(p,2),step)); return GetU(p,3);
}
const void* Set(void* dst,const void* lo,const void* hi) { Store(dst,Load(lo)); Store(dst,Load(hi),16); return lo; }
void* Center(const void* box,void* out) { Store(out,_mm_mul_ps(_mm_add_ps(Load(box,16),Load(box)),_mm_set1_ps(0.5f))); return out; }
void Order(void* box) { __m128 a=Load(box),b=Load(box,16); Store(box,_mm_min_ps(a,b)); Store(box,_mm_max_ps(a,b),16); }
const void* Include(void* box,const void* other) { Store(box,_mm_min_ps(Load(box),Load(other))); Store(box,_mm_max_ps(Load(box,16),Load(other,16)),16); return other; }
bool Intersect(void* out,const void* a,const void* b) {
    __m128 loA=Load(a),loB=Load(b);
    if(_mm_movemask_ps(_mm_or_ps(_mm_cmplt_ps(Load(a,16),loB),_mm_cmplt_ps(Load(b,16),loA)))&7) return false;
    Store(out,_mm_max_ps(loA,loB)); Store(out,_mm_min_ps(Load(a,16),Load(b,16)),16); return true;
}
const void* IncludePoint(void* box,const void* point) {
    __m128 lo=Load(box),hi=Load(box,16);
    Store(box,_mm_min_ps(Load(point),lo)); Store(box,_mm_max_ps(Load(point),hi),16); return point;
}
void Empty(void* box) { Store(box,_mm_set1_ps(16777216.0f)); Store(box,_mm_set1_ps(-16777216.0f),16); }
void* Sphere(const void* box,void* center,float* radius) {
    Store(center,_mm_mul_ps(_mm_add_ps(Load(box),Load(box,16)),_mm_set1_ps(0.5f)));
    __m128 d=_mm_mul_ps(_mm_sub_ps(Load(box,16),Load(box)),_mm_set1_ps(0.5f));
    *radius=_mm_cvtss_f32(Norm(_mm_max_ps(d,_mm_setzero_ps()))); return center;
}
const void* CenterExtents(void* out,const void* box) {
    Store(out,_mm_mul_ps(_mm_sub_ps(Load(box,16),Load(box)),_mm_set1_ps(0.5f)),16);
    float radius=_mm_cvtss_f32(Norm(Load(out,16)));
    __m128 center=_mm_mul_ps(_mm_add_ps(Load(box,16),Load(box)),_mm_set1_ps(0.5f));
    alignas(16) float v[4]; _mm_store_ps(v,center); v[3]=radius; Store(out,_mm_load_ps(v)); return box;
}
const void* Transform(void* out,const void* box,const void* matrix) {
    __m128 lo=Load(box),hi=Load(box,16),half=_mm_set1_ps(0.5f);
    __m128 c=MulPS(AddPS(lo,hi),half),e=MulPS(_mm_sub_ps(hi,lo),half);
    __m128 x=Load(matrix),y=Load(matrix,16),z=Load(matrix,32),t=Load(matrix,48);
    __m128 center=AddPS(MulPS(_mm_shuffle_ps(c,c,0),x),MulPS(_mm_shuffle_ps(c,c,0x55),y));
    center=AddPS(center,MulPS(_mm_shuffle_ps(c,c,0xaa),z)); center=AddPS(center,t);
    __m128 neg=_mm_set1_ps(-1.0f);
    __m128 extent=AddPS(MulPS(_mm_max_ps(MulPS(neg,x),x),_mm_shuffle_ps(e,e,0)),MulPS(_mm_max_ps(MulPS(y,neg),y),_mm_shuffle_ps(e,e,0x55)));
    extent=AddPS(extent,MulPS(_mm_max_ps(MulPS(z,neg),z),_mm_shuffle_ps(e,e,0xaa)));
    Store(out,_mm_sub_ps(center,extent)); Store(out,AddPS(extent,center),16); return matrix;
}
}
#pragma float_control(pop)
