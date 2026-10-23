#include "ts3/math/m04_B04.h"
#include "ts3/math/m04_corners.h"
#include <xmmintrin.h>
#include <emmintrin.h>
#include <cstring>
#include <cmath>
namespace ts3::math {
namespace {
uint32_t Bits(float f) { uint32_t u; std::memcpy(&u,&f,4); return u; }
float FromBits(uint32_t u) { float f; std::memcpy(&f,&u,4); return f; }
__m128 OrderedAdd(__m128 a,__m128 b) {
    // MSVC may commute ADDPS even under precise FP. EA's destination operand
    // wins when both operands are NaNs; enforce that payload priority.
    __m128 r=_mm_add_ps(a,b);
    __m128 mask=_mm_cmpunord_ps(a,a);
    __m128 quiet=_mm_or_ps(a,_mm_castsi128_ps(_mm_set1_epi32(0x00400000)));
    return _mm_or_ps(_mm_and_ps(mask,quiet),_mm_andnot_ps(mask,r));
}
float X87Copy(float f) { // FLD/FSTP quiets signalling NaNs, without DAZ.
    uint32_t u=Bits(f);
    if ((u & 0x7f800000u)==0x7f800000u && (u & 0x7fffffu)) u|=0x400000u;
    return FromBits(u);
}
}
int RectHalfOpen(const float* r,float x,float y) noexcept {
    return x>=r[0] && y>=r[1] && r[2]>x && r[3]>y;
}
int RectPoint(const float* r,const float* p) noexcept { return RectHalfOpen(r,p[0],p[1]); }
int RectClosed(const float* r,float x,float y) noexcept {
    return x>=r[0] && y>=r[1] && r[2]>=x && r[3]>=y;
}
int RectContains(const float* r,const float* p) noexcept {
    return p[0]>=r[0] && r[2]>=p[2] && p[1]>=r[1] && r[3]>=p[3];
}
uintptr_t RectIntersect(float* out,const float* a,const float* b) noexcept {
    uintptr_t residue=reinterpret_cast<uintptr_t>(a)&~uintptr_t(255);
    float x=a[0],bx=b[0];
    if (bx>=a[2] || x>=b[2] || b[1]>=a[3] || a[1]>=b[3]) {
        out[1]=0; out[2]=0; out[0]=0; out[3]=0;
        return residue;
    }
    out[0]=x>bx?x:bx;
    out[1]=a[1]>b[1]?a[1]:b[1];
    out[2]=b[2]>a[2]?a[2]:b[2];
    out[3]=b[3]>a[3]?a[3]:b[3];
    return residue|1;
}
uintptr_t RectUnion(float* out,const float* a,const float* b,uintptr_t incomingEax) noexcept {
    auto flags=[](float x,float y)->uintptr_t {
        if (std::isnan(x)||std::isnan(y)) return 0x4700;
        return x==y?0x4200:x<y?0x0300:0x0200;
    };
    float bx=b[0];
    if (b[2]==bx || b[3]==b[1]) {
        for(unsigned i=0;i<4;++i) out[i]=X87Copy(a[i]);
        return reinterpret_cast<uintptr_t>(a);
    }
    uintptr_t f=flags(a[2],a[0]);
    bool empty=a[2]==a[0];
    if (!empty) { f=flags(a[3],a[1]); empty=a[3]==a[1]; }
    uintptr_t result=(incomingEax&~uintptr_t(0xff00))|f;
    if(empty) {
        out[0]=bx;
        for(unsigned i=1;i<4;++i) out[i]=X87Copy(b[i]);
    } else {
        out[0]=bx>a[0]?a[0]:bx;
        out[1]=b[1]>a[1]?a[1]:b[1];
        out[2]=a[2]>b[2]?a[2]:b[2];
        out[3]=a[3]>b[3]?a[3]:b[3];
    }
    return result;
}
Float4Value* LotCorners(const UnionBox* box,Float4Value* out) noexcept {
    auto* s=reinterpret_cast<const unsigned char*>(box);
    auto* d=reinterpret_cast<unsigned char*>(out);
    for(unsigned i=0;i<8;++i) {
        // Two SHUFPS: final W is the X lane of the Z source, not W or one.
        uint32_t v[4];
        std::memcpy(v,s+((i&4)?16:0),4);
        std::memcpy(v+1,s+((i&2)?20:4),4);
        std::memcpy(v+2,s+((i&1)?24:8),4);
        std::memcpy(v+3,s+((i&1)?16:0),4);
        std::memcpy(d+16*i,v,16);
    }
    return out;
}
const Float4Value* LotTransformCorners(const UnionBox* box,const Float4Value* matrix,Float4Value* out) noexcept {
    alignas(16) Float4Value corners[8];
    LotCorners(box,corners);
    for(unsigned i=0;i<8;++i) {
        __m128 c=_mm_load_ps(reinterpret_cast<const float*>(corners+i));
        // Matrix rows reload each iteration (matrix/output may overlap).
        auto* m=reinterpret_cast<const float*>(matrix);
        __m128 r=_mm_add_ps(_mm_mul_ps(_mm_shuffle_ps(c,c,0),_mm_load_ps(m)),_mm_load_ps(m+12));
        r=_mm_add_ps(r,_mm_mul_ps(_mm_shuffle_ps(c,c,85),_mm_load_ps(m+4)));
        r=_mm_add_ps(r,_mm_mul_ps(_mm_shuffle_ps(c,c,170),_mm_load_ps(m+8)));
        _mm_store_ps(reinterpret_cast<float*>(out+i),r);
    }
    return matrix;
}
namespace {
void TransformBounds(UnionBox* out,const UnionBox* box,const Float4Value* matrix,bool project) noexcept {
    alignas(16) Float4Value corners[8];
    BoxCorners(box,corners);
    auto* m=reinterpret_cast<const float*>(matrix);
    const __m128 m0=_mm_load_ps(m),m1=_mm_load_ps(m+4),m2=_mm_load_ps(m+8),m3=_mm_load_ps(m+12);
    __m128 lo=_mm_set1_ps(16777216.0f),hi=_mm_set1_ps(-16777216.0f);
    for(unsigned i=0;i<8;++i) {
        __m128 c=_mm_load_ps(reinterpret_cast<const float*>(corners+i));
        __m128 r=_mm_add_ps(_mm_mul_ps(_mm_shuffle_ps(c,c,0),m0),_mm_mul_ps(_mm_shuffle_ps(c,c,85),m1));
        r=_mm_add_ps(r,_mm_mul_ps(_mm_shuffle_ps(c,c,170),m2));
        r=_mm_add_ps(r,_mm_mul_ps(_mm_shuffle_ps(c,c,255),m3));
        if(project) {
            __m128 w=_mm_shuffle_ps(r,r,255),rcp=_mm_rcp_ps(w),two=_mm_set1_ps(2.0f);
            rcp=_mm_mul_ps(_mm_sub_ps(two,_mm_mul_ps(rcp,w)),rcp);
            rcp=_mm_mul_ps(_mm_sub_ps(two,_mm_mul_ps(rcp,w)),rcp);
            if(!(_mm_cvtss_f32(w)>0.0f)) continue;
            r=_mm_mul_ps(rcp,r);
        }
        lo=_mm_min_ps(r,lo); hi=_mm_max_ps(r,hi);
    }
    _mm_store_ps(reinterpret_cast<float*>(out),lo);
    _mm_store_ps(reinterpret_cast<float*>(out)+4,hi);
}
}
void ProjectBounds(UnionBox* out,const UnionBox* box,const Float4Value* m) noexcept { TransformBounds(out,box,m,true); }
void AffineBounds(UnionBox* out,const UnionBox* box,const Float4Value* m) noexcept { TransformBounds(out,box,m,false); }
const UnionBox* BoundsSphere(Float4Value* out,const UnionBox* box) noexcept {
    auto* s=reinterpret_cast<const float*>(box);
    __m128 a=_mm_load_ps(s),b=_mm_load_ps(s+4),d=_mm_sub_ps(b,a);
    __m128 q=_mm_mul_ps(d,d);
    __m128 sum=_mm_add_ps(_mm_add_ps(_mm_shuffle_ps(q,q,2),_mm_shuffle_ps(q,q,1)),q);
    __m128 c=OrderedAdd(a,b),r=_mm_sqrt_ps(_mm_shuffle_ps(sum,sum,0));
    __m128 t=_mm_shuffle_ps(c,r,250);
    _mm_store_ps(reinterpret_cast<float*>(out),_mm_mul_ps(_mm_shuffle_ps(c,t,132),_mm_set1_ps(0.5f)));
    return box;
}
const float* ScalarInclude(float* box,const float* p) noexcept {
    float low=box[0],high=box[3];
    if(low>high) {
        for(unsigned i=0;i<6;++i) box[i]=X87Copy(p[i%3]);
    } else {
        for(unsigned i=0;i<3;++i) {
            float v=p[i];
            if(box[i]>v) box[i]=v;
            else if(v>box[i+3]) box[i+3]=v;
        }
    }
    return p;
}
const float* TriangleWeirdBounds(float* out,const float* a,const float* b,const float* c) noexcept {
    auto pick=[](float x,float y)->float { // choose by subtraction sign bit, not ordered comparison.
        __m128 diff=_mm_sub_ss(_mm_set_ss(y),_mm_set_ss(x));
        return (Bits(_mm_cvtss_f32(diff))>>31)?x:y;
    };
    auto minpick=[](float x,float y)->float {
        __m128 diff=_mm_sub_ss(_mm_set_ss(y),_mm_set_ss(x));
        return (Bits(_mm_cvtss_f32(diff))>>31)?y:x;
    };
    float z=minpick(pick(a[2],b[2]),c[2]);
    float y=minpick(pick(a[1],b[1]),c[1]);
    float x=minpick(pick(a[0],b[0]),c[0]);
    out[1]=y; out[0]=x; out[2]=z;
    return out;
}
const void* ScalarTransform(float* box,const void* transform) noexcept
{
    const auto* t=static_cast<const float*>(transform);

  const void* result; // eax
  float v3; // xmm0_4
  float v4; // xmm7_4
  float v5; // xmm6_4
  float v6; // xmm1_4
  float v7; // xmm4_4
  float v8; // xmm5_4
  float v9; // xmm0_4
  float v10; // xmm6_4
  float v11; // xmm3_4
  float v12; // xmm7_4
  float v13; // xmm1_4
  float v14; // xmm2_4
  float v15; // xmm3_4
  float v16; // xmm5_4
  float v17; // xmm7_4
  float v18; // xmm0_4
  float v19; // xmm4_4
  float v20; // xmm5_4
  float v21; // xmm3_4
  float v22; // xmm2_4
  float v23; // xmm1_4
  float v24; // xmm0_4
  float v25; // xmm2_4
  float v26; // xmm3_4
  float v27; // xmm1_4
  float v28; // xmm0_4
  float v29; // xmm1_4
  float v30; // xmm0_4
  float v31; // xmm1_4
  float v32; // [esp+0h] [ebp-3Ch]
  float v33; // [esp+8h] [ebp-34h]
  float v34; // [esp+18h] [ebp-24h]
  float v35; // [esp+1Ch] [ebp-20h]
  float v36; // [esp+20h] [ebp-1Ch]
  float v37; // [esp+24h] [ebp-18h]
  float v38; // [esp+28h] [ebp-14h]
  float v39; // [esp+2Ch] [ebp-10h]
  float v40; // [esp+30h] [ebp-Ch]
  float v41; // [esp+34h] [ebp-8h]
  float v42; // [esp+38h] [ebp-4h]

  result = transform;
  v3 = *box;
  if ( (static_cast<const unsigned char*>(transform)[0] & 2) != 0 )
  {
    v4 = *(box + 2);
    v34 = std::fabs(t[5]);
    v5 = *(box + 1);
    v35 = std::fabs(t[6]);
    v6 = *(box + 3) - v3;
    v36 = std::fabs(t[7]);
    v7 = *(box + 3) + v3;
    v37 = std::fabs(t[8]);
    v8 = *(box + 4) + v5;
    v38 = std::fabs(t[9]);
    v33 = *(box + 5) + v4;
    v9 = t[4] * 0.5f;
    v39 = std::fabs(t[10]);
    v10 = (float)(*(box + 4) - v5) * v9;
    v11 = (float)(*(box + 5) - v4) * v9;
    v40 = std::fabs(t[11]);
    v12 = v6 * v9;
    v41 = std::fabs(t[12]);
    v42 = std::fabs(t[13]);
    v13 = (float)((float)(v40 * v11) + (float)(v37 * v10)) + (float)(v34 * (float)(v6 * v9));
    v14 = (float)((float)(v41 * v11) + (float)(v38 * v10)) + (float)(v35 * v12);
    v15 = (float)((float)(v42 * v11) + (float)(v39 * v10)) + (float)(v36 * v12);
    v32 = v7 * v9;
    v16 = v8 * v9;
    v17 = v33 * v9;
    v18 = (float)((float)((float)(t[11] * (float)(v33 * v9)) + (float)(t[8] * v16))
                + (float)(t[5] * (float)(v7 * v9)))
        + t[1];
    v19 = t[2]
        + (float)((float)((float)(t[12] * v17) + (float)(t[9] * v16))
                + (float)(t[6] * v32));
    v20 = t[3]
        + (float)((float)((float)(t[13] * v17) + (float)(t[10] * v16))
                + (float)(t[7] * v32));
    *box = v18 - v13;
    *(box + 1) = v19 - v14;
    *(box + 2) = v20 - v15;
    *(box + 3) = v13 + v18;
    *(box + 4) = v14 + v19;
    *(box + 5) = v15 + v20;
  }
  else
  {
    v21 = t[4];
    v22 = *(box + 2);
    v23 = *(box + 1) * v21;
    *(box + 1) = v23;
    v24 = v3 * v21;
    *box = v24;
    v25 = v22 * v21;
    *(box + 2) = v25;
    v26 = t[2] + v23;
    v27 = t[3];
    *box = v24 + t[1];
    *(box + 1) = v26;
    *(box + 2) = v27 + v25;
    v28 = t[4];
    *(box + 3) = *(box + 3) * v28;
    v29 = v28 * *(box + 4);
    *(box + 5) = v28 * *(box + 5);
    *(box + 4) = v29;
    v30 = t[2] + v29;
    v31 = t[3] + *(box + 5);
    *(box + 3) = *(box + 3) + t[1];
    *(box + 4) = v30;
    *(box + 5) = v31;
  }
  return result;
}


}
#if defined(_M_IX86)
namespace ts3::math {
// These naked entries have no generated counter proxy: such a proxy would
// change incoming EAX or the caller-frame-relative scratch pointer residue.
__declspec(naked) uintptr_t __fastcall RectUnionEntry(float*,void*,const float*,const float*) noexcept {
    __asm {
        push ebp
        mov ebp,esp
        push ecx
        push eax
        push dword ptr [ebp+12]
        push dword ptr [ebp+8]
        push ecx
        call RectUnion
        add esp,16
        mov ecx,[ebp-4]
        mov edx,[ebp+12]
        mov esp,ebp
        pop ebp
        ret 8
    }
}
__declspec(naked) uintptr_t __fastcall ProjectBoundsEntry(UnionBox*,void*,const UnionBox*,const Float4Value*) noexcept {
    __asm {
        push ebp
        mov ebp,esp
        push ecx
        push dword ptr [ebp+12]
        push dword ptr [ebp+8]
        push ecx
        call ProjectBounds
        add esp,12
        mov edx,[ebp-4]
        xor ecx,ecx
        mov eax,ebp
        and eax,0xfffffff0
        mov esp,ebp
        pop ebp
        ret 8
    }
}
__declspec(naked) uintptr_t __fastcall AffineBoundsEntry(UnionBox*,void*,const UnionBox*,const Float4Value*) noexcept {
    __asm {
        push ebp
        mov ebp,esp
        push ecx
        push dword ptr [ebp+12]
        push dword ptr [ebp+8]
        push ecx
        call AffineBounds
        add esp,12
        mov edx,[ebp-4]
        xor ecx,ecx
        mov eax,ebp
        and eax,0xfffffff0
        mov esp,ebp
        pop ebp
        ret 8
    }
}
int __fastcall RectHalfOpenEntry(const float* a,void*,float x,float y) noexcept { return RectHalfOpen(a,x,y); }
int __fastcall RectPointEntry(const float* a,void*,const float* b) noexcept { return RectPoint(a,b); }
int __fastcall RectClosedEntry(const float* a,void*,float x,float y) noexcept { return RectClosed(a,x,y); }
int __fastcall RectContainsEntry(const float* a,void*,const float* b) noexcept { return RectContains(a,b); }
uintptr_t __fastcall RectIntersectEntry(float* o,void*,const float* a,const float* b) noexcept { return RectIntersect(o,a,b); }
Float4Value* __fastcall LotCornersEntry(const UnionBox* a,void*,Float4Value* b) noexcept { return LotCorners(a,b); }
const Float4Value* __fastcall LotTransformCornersEntry(const UnionBox* a,void*,const Float4Value* b,Float4Value* c) noexcept { return LotTransformCorners(a,b,c); }
const UnionBox* __fastcall BoundsSphereEntry(Float4Value* a,void*,const UnionBox* b) noexcept { return BoundsSphere(a,b); }
const float* __fastcall ScalarIncludeEntry(float* a,void*,const float* b) noexcept { return ScalarInclude(a,b); }
const void* __fastcall ScalarTransformEntry(float* a,void*,const void* b) noexcept { return ScalarTransform(a,b); }
}
#endif
