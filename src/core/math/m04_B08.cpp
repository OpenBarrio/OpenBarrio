#include "ts3/math/m04_B08.h"
#include <cmath>
#include <cstring>
#include <emmintrin.h>
namespace ts3::math::b08 {
namespace {
__m128 Raw(uint64_t x) { return _mm_castsi128_ps(_mm_set_epi64x(0,x)); }
float Sin(float x) {
#if defined(_M_IX86)
 float r; __asm { fld x } __asm { fsin } __asm { fstp r } return r;
#else
 return std::sin(x);
#endif
}
float Cos(float x) {
#if defined(_M_IX86)
 float r; __asm { fld x } __asm { fcos } __asm { fstp r } return r;
#else
 return std::cos(x);
#endif
}
}
int Project(int axis, const float* src, const float* origin, float* out) {
int r=0;
 for(int i=0;i<4;++i) {
 r=0;
 if(axis) { out[2*i]=src[3*i]-origin[0]; r=1; }
 if(axis!=1) { out[2*i+r]=src[3*i+1]-origin[1]; ++r; }
 if(axis!=2) { r+=2*i; out[r]=src[3*i+2]-origin[2]; }
 }
 return r;
}

__m128* __stdcall Wrap(__m128* a1, __m128* a2) {
const float pi=3.141592741012573242f;
 while(a2->m128_f32[1]-a1->m128_f32[1]>pi) a2->m128_f32[1]=a2->m128_f32[1]-6.283185482025146484f;
 while(a1->m128_f32[1]-a2->m128_f32[1]>pi) a2->m128_f32[1]=6.283185482025146484f+a2->m128_f32[1];
 return a2;
}

__m128* Transform(__m128* a1, __m128* a2, __m128* a3) {

  __m128 v4; // [esp+0h] [ebp-10h]

  v4 = *a2;
  v4.m128_i32[3] = 0;
  *a1 = _mm_add_ps(
          _mm_add_ps(
            _mm_add_ps(_mm_mul_ps(_mm_shuffle_ps(v4, v4, 85), a3[1]), _mm_mul_ps(_mm_shuffle_ps(v4, v4, 0), *a3)),
            _mm_mul_ps(_mm_shuffle_ps(v4, v4, 170), a3[2])),
          _mm_mul_ps(_mm_shuffle_ps(v4, v4, 255), a3[3]));
  return a1;

}

__m128* Angles(__m128* out, float a, float b) {
float sa=Sin(a), ca=Cos(a), sb=Sin(b), cb=Cos(b);
 float x=sb*ca; *out=_mm_setr_ps(x,sa,-0.0f-cb*ca,x); return out;
}

__m128* Cross(__m128* result, __m128* a2, __m128* a3, __m128* a4) {

  __m128 v4; // xmm0
  __m128 v5; // xmm1
  __m128 v6; // xmm4
  __m128 v7; // xmm5
  __m128 v8; // xmm3
  __m128 v9; // xmm6
  __m128 v10; // xmm1
  __m128 v11; // xmm7
  __m128 v12; // xmm2
  __m128 v13; // xmm3

  v4 = _mm_sub_ps(*a3, *a2);
  v5 = _mm_sub_ps(*a4, *a2);
  v6 = _mm_shuffle_ps(v4, v4, 85);
  v7 = _mm_shuffle_ps(v5, v5, 0);
  v8 = _mm_shuffle_ps(v4, v4, 0);
  v9 = _mm_shuffle_ps(v5, v5, 85);
  v10 = _mm_shuffle_ps(v5, v5, 170);
  v11 = _mm_mul_ps(v10, v8);
  v12 = _mm_sub_ps(_mm_mul_ps(v9, v8), _mm_mul_ps(v7, v6));
  v13 = _mm_shuffle_ps(v4, v4, 170);
  *result = _mm_shuffle_ps(
              _mm_shuffle_ps(
                _mm_sub_ps(_mm_mul_ps(v10, v6), _mm_mul_ps(v13, v9)),
                _mm_sub_ps(_mm_mul_ps(v13, v7), v11),
                16),
              v12,
              40);
  return result;

}

__m128* Difference(__m128* result, __m128* a2, __m128* a3) {

  *result = _mm_shuffle_ps(
              _mm_shuffle_ps(
                _mm_sub_ps(_mm_shuffle_ps(*a3, *a3, 0), _mm_shuffle_ps(*a2, *a2, 0)),
                _mm_sub_ps(_mm_shuffle_ps(*a3, *a3, 85), _mm_shuffle_ps(*a2, *a2, 85)),
                16),
              _mm_sub_ps(_mm_shuffle_ps(*a3, *a3, 170), _mm_shuffle_ps(*a2, *a2, 170)),
              40);
  return result;

}

uint16_t* Quantize(const float* origin, float c, float s, float x, float y, float sx, float sy, uint16_t* u, uint16_t* v) {
float dx=x-origin[0], dy=y-origin[1];
 float rx=(-0.0f-c)*dy+s*dx;
 float ry=s*dy+c*dx;
 float qx=(0.5f+rx*sx)*4096.0f;
 float qy=(0.5f+ry*sy)*4096.0f;
 *u=static_cast<uint16_t>(_mm_cvtt_ss2si(_mm_set_ss(qx)));
 *v=static_cast<uint16_t>(_mm_cvtt_ss2si(_mm_set_ss(qy)));
 return v;
}

__m128* Heading(float angle, __m128* out) {
float c=Cos(angle), s=Sin(angle); float n=0.0f-s;
 *out=_mm_setr_ps(c,0.0f,n,n); return out;
}

uint32_t Snap(const float* src, float step, float* out) {
float inv=1.0f/step;
 float x=src[0]*inv, y=src[1]*inv; int ix,iy;
#if defined(_M_IX86)
 __asm { fld x } __asm { fistp ix } __asm { fld y } __asm { fistp iy }
#else
 ix=_mm_cvtss_si32(_mm_set_ss(x)); iy=_mm_cvtss_si32(_mm_set_ss(y));
#endif
 out[0]=static_cast<float>(ix)*step; out[1]=static_cast<float>(iy)*step;
 uint32_t z; std::memcpy(&z,src+2,4); std::memcpy(out+2,&z,4); return z;
}

__m128* Spherical(__m128* out, float a, float b) {
alignas(16) float v[4]; v[3]=0;
#if defined(_M_IX86)
 float x,y,z;
 __asm {
 fld b
 fld st(0)
 fcos
 fxch st(1)
 fsin
 fstp z
 fld a
 fld st(0)
 fsin
 fmul st(0), st(2)
 fstp y
 fcos
 fmulp st(1), st(0)
 fstp x
 }
 v[0]=x; v[1]=y; v[2]=z;
#else
 v[0]=static_cast<float>(std::cos(double(b))*std::cos(double(a)));
 v[1]=static_cast<float>(std::cos(double(b))*std::sin(double(a))); v[2]=Sin(b);
#endif
 *out=_mm_load_ps(v); return out;
}

__m128* Frame(__m128* a1, __m128* a2, __m128* a3, __m128* a4, float offset) {

  __m128 v4; // xmm2
  __m128 v5; // xmm3
  __m128 v6; // xmm4
  __m128 v7; // xmm1
  __m128 v8; // xmm7
  __m128 v9; // xmm6
  __m128 v10; // xmm3
  __m128 v11; // xmm4
  __m128 v12; // xmm3
  __m128 v13; // xmm6
  __m128 v14; // xmm3
  __m128 v15; // xmm7
  __m128 v16; // xmm5
  __m128 v17; // xmm4
  __m128 v18; // xmm3
  __m128 v19; // xmm1
  __m128 v20; // xmm4
  __m128 v21; // xmm1
  __m128 v22; // xmm1
  __m128 v23; // xmm4
  __m128 v24; // xmm1
  __m128 v25; // xmm4
  __m128 v26; // xmm1
  __m128 v27; // xmm5
  __m128 v28; // xmm1
  __m128 v29; // xmm0
  __m128 v31; // [esp+0h] [ebp-80h]
  __m128 v32; // [esp+10h] [ebp-70h]
  __m128 v33; // [esp+20h] [ebp-60h]
  __m128 v34; // [esp+30h] [ebp-50h]

  v4 = _mm_sub_ps(_mm_setzero_ps(), *a1);
  v5 = _mm_shuffle_ps(v4, v4, 18);
  v6 = _mm_shuffle_ps(v4, v4, 9);
  v7 = _mm_sub_ps(
         _mm_mul_ps(_mm_shuffle_ps(Raw(0x3F80000000000000ULL), Raw(0x3F80000000000000ULL), 9), v5),
         _mm_mul_ps(_mm_shuffle_ps(Raw(0x3F80000000000000ULL), Raw(0x3F80000000000000ULL), 18), v6));
  v8 = _mm_mul_ps(_mm_shuffle_ps(v7, v7, 9), v5);
  v9 = _mm_mul_ps(_mm_shuffle_ps(v7, v7, 18), v6);
  v10 = _mm_mul_ps(v7, v7);
  v11 = _mm_add_ps(_mm_shuffle_ps(v10, v10, 14), v10);
  v12 = _mm_add_ps(_mm_shuffle_ps(v11, v11, 1), v11);
  v13 = _mm_sub_ps(v9, v8);
  v14 = _mm_shuffle_ps(v12, v12, 0);
  v15 = _mm_rsqrt_ps(v14);
  v11.m128_f32[0] = 1.0;
  v31 = _mm_shuffle_ps(v11, v11, 0);
  v16 = _mm_shuffle_ps(Raw(0x3F000000ULL), Raw(0x3F000000ULL), 0);
  v17 = _mm_add_ps(_mm_mul_ps(_mm_sub_ps(v31, _mm_mul_ps(_mm_mul_ps(v15, v15), v14)), _mm_mul_ps(v16, v15)), v15);
  v18 = _mm_mul_ps(
          _mm_add_ps(_mm_mul_ps(_mm_sub_ps(v31, _mm_mul_ps(_mm_mul_ps(v17, v17), v14)), _mm_mul_ps(v16, v17)), v17),
          v7);
  v19 = _mm_mul_ps(v13, v13);
  v20 = _mm_add_ps(_mm_shuffle_ps(v19, v19, 14), v19);
  v21 = _mm_add_ps(_mm_shuffle_ps(v20, v20, 1), v20);
  v22 = _mm_shuffle_ps(v21, v21, 0);
  v23 = _mm_rsqrt_ps(v22);
  v33 = v22;
  v24 = _mm_add_ps(_mm_mul_ps(_mm_sub_ps(v31, _mm_mul_ps(_mm_mul_ps(v23, v23), v22)), _mm_mul_ps(v16, v23)), v23);
  v25 = _mm_mul_ps(
          _mm_add_ps(_mm_mul_ps(_mm_sub_ps(v31, _mm_mul_ps(_mm_mul_ps(v24, v24), v33)), _mm_mul_ps(v16, v24)), v24),
          v13);
  v32 = _mm_add_ps(
          _mm_mul_ps(_mm_set1_ps(offset), *a1),
          *a2);
  v32.m128_i32[3] = 1065353216;
  v26 = _mm_shuffle_ps(_mm_shuffle_ps(*a3, *a3, 16), *a3, 40);
  v34 = _mm_shuffle_ps(_mm_shuffle_ps(_mm_setzero_ps(), v26, 80), _mm_setzero_ps(), 232);
  v27 = _mm_shuffle_ps(_mm_shuffle_ps(_mm_setzero_ps(), v26, 5), _mm_setzero_ps(), 226);
  v28 = _mm_shuffle_ps(_mm_setzero_ps(), _mm_shuffle_ps(_mm_setzero_ps(), v26, 175), 36);
  v29 = _mm_shuffle_ps(_mm_setzero_ps(), _mm_shuffle_ps(_mm_setzero_ps(), v31, 250), 132);
  *a4 = _mm_add_ps(
          _mm_add_ps(
            _mm_add_ps(_mm_mul_ps(_mm_shuffle_ps(v27, v27, 85), v25), _mm_mul_ps(_mm_shuffle_ps(v27, v27, 0), v18)),
            _mm_mul_ps(_mm_shuffle_ps(v27, v27, 170), v4)),
          _mm_mul_ps(_mm_shuffle_ps(v27, v27, 255), v32));
  a4[1] = _mm_add_ps(
            _mm_add_ps(
              _mm_add_ps(_mm_mul_ps(_mm_shuffle_ps(v34, v34, 0), v18), _mm_mul_ps(_mm_shuffle_ps(v34, v34, 85), v25)),
              _mm_mul_ps(_mm_shuffle_ps(v34, v34, 170), v4)),
            _mm_mul_ps(_mm_shuffle_ps(v34, v34, 255), v32));
  a4[2] = _mm_add_ps(
            _mm_add_ps(
              _mm_add_ps(_mm_mul_ps(_mm_shuffle_ps(v28, v28, 0), v18), _mm_mul_ps(_mm_shuffle_ps(v28, v28, 85), v25)),
              _mm_mul_ps(_mm_shuffle_ps(v28, v28, 170), v4)),
            _mm_mul_ps(_mm_shuffle_ps(v28, v28, 255), v32));
  a4[3] = _mm_add_ps(
            _mm_add_ps(
              _mm_add_ps(_mm_mul_ps(_mm_shuffle_ps(v29, v29, 0), v18), _mm_mul_ps(_mm_shuffle_ps(v29, v29, 85), v25)),
              _mm_mul_ps(_mm_shuffle_ps(v29, v29, 170), v4)),
            _mm_mul_ps(_mm_shuffle_ps(v29, v29, 255), v32));
  return a4;

}

double __stdcall Distance(__m128* a, __m128* b) {
__m128 d=_mm_sub_ps(*a,*b);
 return (std::fabs(double(d.m128_f32[1]))+std::fabs(double(d.m128_f32[0])))+std::fabs(double(d.m128_f32[2]));
}

__m128* Normal(const signed char* a1, __m128* a2) {

  __m128 v3; // xmm1
  __m128 v4; // xmm5
  __m128 v5; // xmm0
  __m128 v6; // xmm2
  __m128 v7; // xmm4
  __m128 v8; // xmm0
  __m128 v9; // xmm3
  __m128 v10; // xmm1
  __m128 v11; // [esp+0h] [ebp-10h]

  v11.m128_f32[1] = (float)a1[4] - (float)a1[1];
  v11.m128_f32[3] = (float)a1[3] - (float)*a1;
  v11.m128_f32[2] = v11.m128_f32[3];
  v11.m128_f32[0] = v11.m128_f32[3];
  v3 = _mm_shuffle_ps(_mm_sub_ps(_mm_setzero_ps(), _mm_shuffle_ps(v11, v11, 85)), _mm_shuffle_ps(v11, v11, 0), 16);
  v4 = _mm_shuffle_ps(v3, v3, 8);
  v5 = _mm_mul_ps(v4, v4);
  v6 = _mm_add_ps(_mm_shuffle_ps(v5, v5, 85), _mm_shuffle_ps(v5, v5, 0));
  v7 = _mm_rsqrt_ps(v6);
  v8 = _mm_shuffle_ps(Raw(0x3F800000ULL), Raw(0x3F800000ULL), 0);
  v9 = _mm_shuffle_ps(Raw(0x3F000000ULL), Raw(0x3F000000ULL), 0);
  v10 = _mm_add_ps(_mm_mul_ps(_mm_sub_ps(v8, _mm_mul_ps(_mm_mul_ps(v7, v7), v6)), _mm_mul_ps(v9, v7)), v7);
  *a2 = _mm_mul_ps(
          _mm_add_ps(_mm_mul_ps(_mm_sub_ps(v8, _mm_mul_ps(_mm_mul_ps(v10, v10), v6)), _mm_mul_ps(v9, v10)), v10),
          v4);
  return a2;

}


__m128* __stdcall FrameEntry(__m128* a, __m128* b, __m128* c, __m128* out) {
#if defined(_M_IX86)
 return Frame(a,b,c,out,*reinterpret_cast<const float*>(0x01183F48));
#else
 return Frame(a,b,c,out,0.0f);
#endif
}
#if defined(_M_IX86)
__declspec(naked) void CrossEntry() {
 __asm { push [esp+8] }
 __asm { push [esp+8] }
 __asm { push ecx }
 __asm { push eax }
 __asm { call Cross }
 __asm { add esp,16 }
 __asm { ret }
}
__declspec(naked) void DifferenceEntry() {
 __asm { push [esp+4] }
 __asm { push ecx }
 __asm { push eax }
 __asm { call Difference }
 __asm { add esp,12 }
 __asm { ret }
}
__declspec(naked) void SphericalEntry() {
 __asm { push [esp+8] }
 __asm { push [esp+8] }
 __asm { push eax }
 __asm { call Spherical }
 __asm { add esp,12 }
 __asm { ret }
}
#endif
}
