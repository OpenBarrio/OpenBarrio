#include "ts3/math/m04_B11.h"
#include "ts3/math/m04_pose.h"
#include <emmintrin.h>
#include <cstring>
#include <cstdint>
#include <intrin.h>
#pragma float_control(precise, on, push)
#pragma fenv_access(on)
namespace ts3::math::b11 {
namespace {
// Out-of-line operations retain legacy two-operand NaN precedence.
__declspec(noinline) __m128 Mul(__m128 a,__m128 b) { return _mm_mul_ps(a,b); }
__declspec(noinline) __m128 Add(__m128 a,__m128 b) { return _mm_add_ps(a,b); }
__m128 XyzMask() {
#if defined(_M_IX86)
    // Runtime state, not an assumed constant: mapped EXE fixtures initialize it.
    return _mm_load_ps(reinterpret_cast<const float*>(0x011AF670));
#else
    return _mm_castsi128_ps(_mm_set_epi32(0,-1,-1,-1));
#endif
}
}
__m128 *ComposePose(__m128 *a1, __m128 *a2, __m128 *a3)
{
  __m128 v4; // xmm2
  __m128 v5; // xmm0
  __m128 v6; // xmm4
  __m128 v7; // xmm3
  __m128 v8; // xmm1
  __m128 v9; // xmm5
  __m128 v10; // xmm0
  __m128 v11; // xmm1
  __m128 v12; // xmm2
  __m128 v13; // xmm2
  __m128 v14; // xmm1
  __m128 v15; // xmm3
  __m128 v16; // xmm4
  __m128 v17; // xmm0

  a3[2] = Mul(a1[2], a2[2]);
  v4 = a2[1];
  v5 = a1[1];
  v6 = _mm_shuffle_ps(v4, v4, 255);
  v7 = _mm_shuffle_ps(v5, v5, 255);
  v8 = _mm_sub_ps(
         Mul(_mm_shuffle_ps(v5, v5, 18), _mm_shuffle_ps(v4, v4, 9)),
         Mul(_mm_shuffle_ps(v5, v5, 9), _mm_shuffle_ps(v4, v4, 18)));
  v9 = Mul(v5, v6);
  v10 = Mul(v5, v4);
  v11 = Add(Add(v8, v9), Mul(v4, v7));
  v12 = Add(Add(_mm_shuffle_ps(v10, v10, 2), _mm_shuffle_ps(v10, v10, 1)), v10);
  a3[1] = _mm_shuffle_ps(v11, _mm_shuffle_ps(v11, _mm_sub_ps(Mul(v7, v6), _mm_shuffle_ps(v12, v12, 0)), 10), 132);
  v13 = a2[1];
  v14 = Mul(*a1, a2[2]);
  v15 = _mm_shuffle_ps(v13, v13, 9);
  v16 = _mm_shuffle_ps(v13, v13, 18);
  v17 = Add(
          _mm_sub_ps(Mul(_mm_shuffle_ps(v14, v14, 18), v15), Mul(_mm_shuffle_ps(v14, v14, 9), v16)),
          Mul(_mm_shuffle_ps(v13, v13, 255), v14));
  *a3 = Add(
          Add(
            Mul(
              _mm_sub_ps(Mul(_mm_shuffle_ps(v17, v17, 18), v15), Mul(_mm_shuffle_ps(v17, v17, 9), v16)),
              _mm_set1_ps(2.0f)),
            v14),
          *a2);
  return a2;
}


__m128 *InvertPose(__m128 *a1, __m128 *a2)
{
  __m128 v2; // xmm2
  __m128 v4; // xmm0
  __m128 v5; // xmm1
  __m128 v6; // xmm0
  __m128 v7; // xmm3
  __m128 v8; // xmm4
  __m128 v9; // xmm0
  __m128 v10; // xmm1
  __m128 v11; // xmm3
  __m128 v12; // xmm4
  __m128 v13; // xmm5
  __m128 v14; // xmm1
  __m128 v15; // xmm6
  __m128 v16; // xmm3
  __m128 v17; // xmm1
  __m128 v18; // xmm6
  __m128 v19; // xmm4
  __m128 v20; // xmm2
  __m128 v21; // [esp+10h] [ebp-10h]

  v2 = a1[1];
  v4 = Mul(v2, v2);
  v5 = Add(_mm_shuffle_ps(v4, v4, 14), v4);
  v6 = Add(_mm_shuffle_ps(v5, v5, 1), v5);
  v7 = _mm_shuffle_ps(v6, v6, 0);
  v8 = _mm_rcp_ps(v7);
  v9 = _mm_set1_ps(2.0f);
  v10 = Mul(_mm_sub_ps(v9, Mul(v8, v7)), v8);
  v21 = _mm_castsi128_ps(_mm_set_epi32(0, INT32_MIN, INT32_MIN, INT32_MIN));
  a2[1] = Mul(Mul(_mm_sub_ps(v9, Mul(v10, v7)), v10), _mm_xor_ps(v21, v2));
  v11 = a2[1];
  v12 = _mm_shuffle_ps(v11, v11, 18);
  v13 = _mm_shuffle_ps(v11, v11, 9);
  v14 = Add(
          _mm_sub_ps(Mul(_mm_shuffle_ps(*a1, *a1, 18), v13), Mul(_mm_shuffle_ps(*a1, *a1, 9), v12)),
          Mul(_mm_shuffle_ps(v11, v11, 255), *a1));
  v15 = Mul(_mm_shuffle_ps(v14, v14, 18), v13);
  v16 = _mm_shuffle_ps(v14, v14, 9);
  v17 = a1[2];
  v18 = Add(Mul(_mm_sub_ps(v15, Mul(v16, v12)), v9), *a1);
  v19 = _mm_rcp_ps(v17);
  v20 = Mul(_mm_sub_ps(v9, Mul(v17, v19)), v19);
  a2[2] = Mul(_mm_sub_ps(v9, Mul(v17, v20)), v20);
  *a2 = _mm_sub_ps(_mm_setzero_ps(), v18);
  return a2;
}


float *AccumulatePose(__m128 *a1, float *a2, __m128 *a3, float a4)
{
  __m128 v4; // xmm0
  __m128 v5; // xmm1
  __m128 v6; // xmm0
  __m128 v7; // xmm0
  __m128 v8; // xmm0
  __m128 v9; // xmm3
  __m128 v10; // xmm2
  float v12; // [esp+0h] [ebp-20h]

  v4 = Mul(a3[1], a1[1]);
  v5 = Add(_mm_shuffle_ps(v4, v4, 14), v4);
  v6 = Add(_mm_shuffle_ps(v5, v5, 1), v5);
  v12 = _mm_shuffle_ps(v6, v6, 0).m128_f32[0];
  v7 = _mm_setzero_ps();
  v7.m128_f32[0] = a4;
  v8 = _mm_shuffle_ps(v7, v7, 0);
  v9 = Mul(v8, a3[1]);
  v10 = a1[1];
  if (!(v12 >= 0.0f))
    a1[1] = _mm_sub_ps(v10, v9);
  else
    a1[1] = Add(v9, v10);
  *a1 = Add(Mul(*a3, v8), *a1);
  a1[2] = Add(Mul(a3[2], v8), a1[2]);
  *a2 = *a2 + a4;
  return a2;
}


__m128 *MultiplyMatrix(__m128 *a1, __m128 *a2, __m128 *a3)
{
  __m128 v3; // xmm5
  __m128 v4; // xmm7
  __m128 v5; // xmm3
  __m128 v6; // xmm0
  __m128 v7; // xmm1
  __m128 v8; // xmm2
  __m128 v9; // xmm4
  __m128 v10; // xmm0
  __m128 v11; // xmm2
  __m128 v12; // xmm3
  __m128 v13; // xmm4
  __m128 v14; // xmm0
  __m128 v15; // xmm3
  __m128 v16; // xmm4
  __m128 v17; // xmm5
  __m128 v18; // xmm0

  v3 = a2[1];
  v4 = a2[2];
  v5 = a2[3];
  v6 = a1[1];
  v7 = Add(
         Add(
           Add(Mul(_mm_shuffle_ps(*a1, *a1, 0), *a2), Mul(_mm_shuffle_ps(*a1, *a1, 85), v3)),
           Mul(_mm_shuffle_ps(*a1, *a1, 170), v4)),
         Mul(_mm_shuffle_ps(*a1, *a1, 255), v5));
  v8 = Add(
         Add(Mul(_mm_shuffle_ps(v6, v6, 0), *a2), Mul(_mm_shuffle_ps(v6, v6, 85), v3)),
         Mul(_mm_shuffle_ps(v6, v6, 170), v4));
  v9 = _mm_shuffle_ps(v6, v6, 255);
  v10 = a1[2];
  v11 = Add(v8, Mul(v9, v5));
  v12 = Add(
          Add(Mul(_mm_shuffle_ps(v10, v10, 0), *a2), Mul(_mm_shuffle_ps(v10, v10, 85), v3)),
          Mul(_mm_shuffle_ps(v10, v10, 170), v4));
  v13 = Mul(_mm_shuffle_ps(v10, v10, 255), a2[3]);
  v14 = a1[3];
  v15 = Add(v12, v13);
  v16 = Add(
          Add(Mul(_mm_shuffle_ps(v14, v14, 0), *a2), Mul(_mm_shuffle_ps(v14, v14, 85), v3)),
          Mul(_mm_shuffle_ps(v14, v14, 170), v4));
  v17 = _mm_shuffle_ps(v14, v14, 255);
  v18 = a2[3];
  *a3 = v7;
  a3[1] = v11;
  a3[2] = v15;
  a3[3] = Add(v16, Mul(v17, v18));
  return a3;
}


__m128 *SrtMatrix(__m128 *self, __m128 *a2)
{
  __m128 v3; // xmm0
  __m128 v4; // xmm1
  __m128 v5; // xmm2
  __m128 v6; // xmm3
  __m128 v7; // xmm0
  __m128 v8; // xmm2
  __m128 v9; // xmm5
  __m128 v10; // xmm1
  __m128 v11; // xmm3
  __m128 v12; // xmm4
  __m128 v13; // xmm0
  __m128 v14; // xmm4
  __m128 v15; // xmm1
  __m128 v16; // xmm4
  __m128 v17; // xmm0

  v3 = Mul(_mm_set1_ps(1.41421353816986083984375f), *(self + 1));
  v4 = Add(
         Mul(_mm_sub_ps(_mm_setzero_ps(), v3), v3),
         _mm_set1_ps(0.5f));
  v5 = Mul(_mm_shuffle_ps(v3, v3, 9), v3);
  v6 = Mul(_mm_shuffle_ps(v3, v3, 255), _mm_shuffle_ps(v3, v3, 18));
  v7 = Add(v6, v5);
  v8 = _mm_sub_ps(v5, v6);
  v9 = Add(_mm_shuffle_ps(v4, v4, 9), v4);
  v10 = *(self + 2);
  v11 = Mul(_mm_shuffle_ps(_mm_shuffle_ps(v9, v7, 1), v8, 40), _mm_shuffle_ps(v10, v10, 0));
  v12 = _mm_shuffle_ps(_mm_shuffle_ps(v8, v9, 32), v7, 24);
  v13 = Mul(_mm_shuffle_ps(_mm_shuffle_ps(v7, v8, 18), v9, 8), _mm_shuffle_ps(v10, v10, 170));
  v14 = Mul(v12, _mm_shuffle_ps(v10, v10, 85));
  v15 = _mm_shuffle_ps(
          *self,
          _mm_shuffle_ps(*self, _mm_set1_ps(1.0f), 250),
          132);
  v16 = _mm_and_ps(v14, XyzMask());
  v17 = _mm_and_ps(v13, XyzMask());
  *a2 = _mm_and_ps(v11, XyzMask());
  a2[1] = v16;
  a2[2] = v17;
  a2[3] = v15;
  return a2;
}


__m128 *RigidMatrix(__m128 *self, __m128 *a2)
{
  __m128 v3; // xmm0
  __m128 v4; // xmm1
  __m128 v5; // xmm2
  __m128 v6; // xmm3
  __m128 v7; // xmm0
  __m128 v8; // xmm2
  __m128 v9; // xmm3
  __m128 v10; // xmm1
  __m128 v11; // xmm4
  __m128 v12; // xmm0
  __m128 v13; // xmm1
  __m128 v14; // xmm2
  __m128 v15; // xmm3

  v3 = Mul(_mm_set1_ps(1.41421353816986083984375f), *(self + 1));
  v4 = Add(
         Mul(_mm_sub_ps(_mm_setzero_ps(), v3), v3),
         _mm_set1_ps(0.5f));
  v5 = Mul(_mm_shuffle_ps(v3, v3, 9), v3);
  v6 = Mul(_mm_shuffle_ps(v3, v3, 255), _mm_shuffle_ps(v3, v3, 18));
  v7 = Add(v6, v5);
  v8 = _mm_sub_ps(v5, v6);
  v9 = Add(_mm_shuffle_ps(v4, v4, 9), v4);
  v10 = _mm_shuffle_ps(v9, v7, 1);
  v11 = _mm_shuffle_ps(_mm_shuffle_ps(v8, v9, 32), v7, 24);
  v12 = _mm_shuffle_ps(_mm_shuffle_ps(v7, v8, 18), v9, 8);
  v13 = _mm_shuffle_ps(v10, v8, 40);
  v14 = *(self + 2);
  v15 = XyzMask();
  *a2 = _mm_and_ps(v13, XyzMask());
  a2[1] = _mm_and_ps(v11, v15);
  a2[2] = _mm_and_ps(v12, v15);
  a2[3] = _mm_shuffle_ps(
            v14,
            _mm_shuffle_ps(v14, _mm_set1_ps(1.0f), 250),
            132);
  return a2;
}


__m128 *StoreRigidMatrix(__m128 *self, __m128 *a2)
{
  __m128 v3; // xmm0
  __m128 v4; // xmm1
  __m128 v5; // xmm2
  __m128 v6; // xmm3
  __m128 v7; // xmm0
  __m128 v8; // xmm2
  __m128 v9; // xmm3
  __m128 v10; // xmm1
  __m128 v11; // xmm4
  __m128 v12; // xmm0
  __m128 v13; // xmm1
  __m128 v14; // xmm2
  __m128 v15; // xmm4
  __m128 v16; // xmm0

  v3 = Mul(_mm_set1_ps(1.41421353816986083984375f), *a2);
  v4 = Add(
         Mul(_mm_sub_ps(_mm_setzero_ps(), v3), v3),
         _mm_set1_ps(0.5f));
  v5 = Mul(_mm_shuffle_ps(v3, v3, 9), v3);
  v6 = Mul(_mm_shuffle_ps(v3, v3, 255), _mm_shuffle_ps(v3, v3, 18));
  v7 = Add(v6, v5);
  v8 = _mm_sub_ps(v5, v6);
  v9 = Add(_mm_shuffle_ps(v4, v4, 9), v4);
  v10 = _mm_shuffle_ps(v9, v7, 1);
  v11 = _mm_shuffle_ps(_mm_shuffle_ps(v8, v9, 32), v7, 24);
  v12 = _mm_shuffle_ps(_mm_shuffle_ps(v7, v8, 18), v9, 8);
  v13 = _mm_shuffle_ps(v10, v8, 40);
  v14 = _mm_shuffle_ps(
          a2[1],
          _mm_shuffle_ps(a2[1], _mm_set1_ps(1.0f), 250),
          132);
  v15 = _mm_and_ps(v11, XyzMask());
  v16 = _mm_and_ps(v12, XyzMask());
  *(self + 27) = _mm_and_ps(v13, XyzMask());
  *(self + 28) = v15;
  *(self + 29) = v16;
  *(self + 30) = v14;
  return a2;
}


// Verified dependency retains the exact overlap-store and NaN-lane behavior.
__m128* TransformPoint(__m128* vector,__m128* pose,__m128* out) {
    ts3::math::TransformPoint(reinterpret_cast<Float4Value*>(vector), reinterpret_cast<SrtValue*>(pose),reinterpret_cast<Float4Value*>(out));
    return pose;
}
__m128* RotateVector(__m128* vector,__m128* pose,__m128* out) {
    ts3::math::RotateVector(reinterpret_cast<Float4Value*>(vector), reinterpret_cast<SrtValue*>(pose),reinterpret_cast<Float4Value*>(out));
    return pose;
}
__m128* RotatedExtent(__m128* extent,__m128* quaternion,__m128* out) {
    const __m128 e=*extent, q=*quaternion;
    const __m128 q09=_mm_shuffle_ps(q,q,9),q12=_mm_shuffle_ps(q,q,18),qw=_mm_shuffle_ps(q,q,255);
    const auto rotate=[&](__m128 v) {
        __m128 t=Add(_mm_sub_ps(Mul(_mm_shuffle_ps(v,v,18),q09),Mul(_mm_shuffle_ps(v,v,9),q12)),Mul(qw,v));
        return Add(Mul(_mm_sub_ps(Mul(_mm_shuffle_ps(t,t,18),q09),Mul(_mm_shuffle_ps(t,t,9),q12)),_mm_set1_ps(2.0f)),v);
    };
    const __m128 x=rotate(_mm_set_ps(e.m128_f32[0],0,0,e.m128_f32[0]));
    const __m128 y=rotate(_mm_set_ps(0,0,e.m128_f32[1],0));
    const __m128 z=rotate(_mm_set_ps(0,e.m128_f32[2],0,0));
    const __m128 minus=_mm_set1_ps(-1.0f);
    *out=_mm_max_ps(_mm_max_ps(Mul(minus,z),z),_mm_max_ps(_mm_max_ps(Mul(minus,x),x),_mm_max_ps(Mul(minus,y),y)));
    return extent;
}
__m128* SphereProjection(float u,float v,int flip,__m128* out) {
    const float x=(u-0.5f)*2.0f,y=(v-0.5f)*2.0f;
    const float xx=x*x,yy=y*y,rr=xx+yy;
    const float reciprocal=1.0f/(rr+1.0f);
    const float x2=x*2.0f,y2=y*2.0f;
    const __m128 value=flip ? _mm_set_ps(-0.0f-x2,(1.0f-xx)-yy,-0.0f-y2,-0.0f-x2) : _mm_set_ps(x2,rr-1.0f,y2,x2);
    *out=value;
    _ReadWriteBarrier();
    *out=Mul(_mm_set1_ps(reciprocal),*out);
    return out;
}
__m128* CubeProjection(int face,float u,float v,__m128* out) {
    const float x=(u-0.5f)*2.0f,y=(v-0.5f)*2.0f;
    __m128 direction;
    // Invalid face selectors read uninitialized stack in EA; no supported value contract.
    switch(face) {
    case 0: direction=_mm_set_ps(1,-0.0f-x,-0.0f-y,1); break;
    case 1: direction=_mm_set_ps(-1,x,-0.0f-y,-1); break;
    case 2: direction=_mm_set_ps(x,y,1,x); break;
    case 3: direction=_mm_set_ps(x,-0.0f-y,-1,x); break;
    case 4: direction=_mm_set_ps(x,1,-0.0f-y,x); break;
    case 5: direction=_mm_set_ps(-0.0f-x,-1,-0.0f-y,-0.0f-x); break;
    default: __assume(0);
    }
    __m128 sq=Mul(direction,direction);
    __m128 sum=Add(Add(_mm_shuffle_ps(sq,sq,2),_mm_shuffle_ps(sq,sq,1)),sq);
    sum=_mm_shuffle_ps(sum,sum,0);
    __m128 estimate=_mm_rsqrt_ps(sum);
    const __m128 one=_mm_set1_ps(1.0f),half=_mm_set1_ps(0.5f);
    estimate=Add(Mul(_mm_sub_ps(one,Mul(Mul(estimate,estimate),sum)),Mul(half,estimate)),estimate);
    estimate=Add(Mul(_mm_sub_ps(one,Mul(Mul(estimate,estimate),sum)),Mul(half,estimate)),estimate);
    *out=Mul(estimate,direction);
    return out;
}
#if defined(_M_IX86)
// 577240 consumes EDX after this call, despite its normally volatile ABI.
__declspec(naked) __m128* __fastcall SrtMatrixEntry(__m128*,void*,__m128*) {
    __asm {
        push ecx
        push edx
        push dword ptr [esp+12]
        push ecx
        call SrtMatrix
        add esp,8
        pop edx
        pop ecx
        ret 4
    }
}
#else
__m128* __fastcall SrtMatrixEntry(__m128* self,void*,__m128* out) { return SrtMatrix(self,out); }
#endif
__m128* __fastcall RigidMatrixEntry(__m128* self,void*,__m128* out) { return RigidMatrix(self,out); }
__m128* __fastcall StoreRigidMatrixEntry(__m128* self,void*,__m128* source) { return StoreRigidMatrix(self,source); }

}
#pragma float_control(pop)
