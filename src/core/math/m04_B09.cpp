#include "ts3/math/m04_B09.h"
#include <cmath>
#include <cstring>
namespace ts3::math::b09 {

static float Sin(float x) {
#if defined(_M_IX86)
    float r;
    __asm {
 fld x
 fsin
 fstp r
 }
    return r;
#else
    return static_cast<float>(std::sin(x));
#endif
}
static float Cos(float x) {
#if defined(_M_IX86)
    float r;
    __asm {
 fld x
 fcos
 fstp r
 }
    return r;
#else
    return static_cast<float>(std::cos(x));
#endif
}

// Bit construction is a MOVD/MOVQ value, not a numeric float conversion.
static __m128 B(uint64_t x) { __m128 r{}; std::memcpy(&r,&x,8); return r; }
static uint32_t U(float f) { uint32_t x; std::memcpy(&x,&f,4); return x; }
__m128 *Kernel511BB0(ts3::ui::MatrixAccumulator *matrix, __m128 *a2)
{
  auto self = reinterpret_cast<__m128*>(matrix);
  __m128 v3;

  if ( !*((uint32_t *)self + 16) )
    *((uint32_t *)self + 16) = 1;
  v3 = _mm_shuffle_ps(*a2, _mm_shuffle_ps(*a2, _mm_shuffle_ps(B(0x3F800000u), B(0x3F800000u), 0), 250), 132);
  *self = _mm_mul_ps(*self, v3);
  *(self + 1) = _mm_mul_ps(*(self + 1), v3);
  *(self + 2) = _mm_mul_ps(*(self + 2), v3);
  *(self + 3) = _mm_mul_ps(*(self + 3), v3);
  return a2;
}


__m128 *Kernel511D60(ts3::ui::MatrixAccumulator *matrix, __m128 *a2, __m128 *a3)
{
  auto self = reinterpret_cast<__m128*>(matrix);
  *a2 = _mm_add_ps(
          _mm_add_ps(
            _mm_add_ps(_mm_mul_ps(_mm_shuffle_ps(*a3, *a3, 0), *self), *(self + 3)),
            _mm_mul_ps(_mm_shuffle_ps(*a3, *a3, 85), *(self + 1))),
          _mm_mul_ps(_mm_shuffle_ps(*a3, *a3, 170), *(self + 2)));
  return a2;
}


int Kernel5127D0(ts3::ui::MatrixAccumulator *matrix, unsigned int a2)
{
  auto self = reinterpret_cast<__m128*>(matrix);
  int result;
  __m128 v3;
  __m128 v4;
  __m128 v5;
  __m128 v6;
  __m128 v7;
  __m128 v8;
  __m128 v9;
  __m128 v10;
  unsigned int v11;
  unsigned int v12;
  float v13;

  result = 2;
  if ( *((int *)self + 16) < 2 )
    *((uint32_t *)self + 16) = 2;
  v13 = _mm_shuffle_ps(B(a2), B(a2), 0).m128_f32[0];
  v3 = _mm_shuffle_ps(B(0x3F800000u), B(0x3F800000u), 0);
  *(float *)&v11 = Sin(v13);
  v4 = _mm_shuffle_ps(B(v11), B(v11), 0);
  *(float *)&v12 = Cos(v13);
  v5 = *(self + 2);
  v6 = _mm_shuffle_ps(B(v12), B(v12), 0);
  v7 = _mm_shuffle_ps(_mm_shuffle_ps(v3, v6, 16), _mm_shuffle_ps(v6, v3, 50), 136);
  v8 = *(self + 3);
  v9 = _mm_shuffle_ps(
         _mm_shuffle_ps(B(0), v4, 16),
         _mm_shuffle_ps(_mm_sub_ps(B(0), v4), B(0), 50),
         136);
  v10 = _mm_add_ps(_mm_mul_ps(_mm_shuffle_ps(*(self + 1), *(self + 1), 26), v9), _mm_mul_ps(*(self + 1), v7));
  *self = _mm_add_ps(_mm_mul_ps(_mm_shuffle_ps(*self, *self, 26), v9), _mm_mul_ps(*self, v7));
  *(self + 1) = v10;
  *(self + 2) = _mm_add_ps(_mm_mul_ps(_mm_shuffle_ps(v5, v5, 26), v9), _mm_mul_ps(v7, v5));
  *(self + 3) = _mm_add_ps(_mm_mul_ps(_mm_shuffle_ps(v8, v8, 26), v9), _mm_mul_ps(v8, v7));
  return result;
}


int Kernel5128C0(ts3::ui::MatrixAccumulator *matrix, unsigned int a2)
{
  auto self = reinterpret_cast<__m128*>(matrix);
  int result;
  __m128 v3;
  __m128 v4;
  __m128 v5;
  __m128 v6;
  __m128 v7;
  __m128 v8;
  __m128 v9;
  __m128 v10;
  unsigned int v11;
  unsigned int v12;
  float v13;

  result = 2;
  if ( *((int *)self + 16) < 2 )
    *((uint32_t *)self + 16) = 2;
  v13 = _mm_shuffle_ps(B(a2), B(a2), 0).m128_f32[0];
  v3 = _mm_shuffle_ps(B(0x3F800000u), B(0x3F800000u), 0);
  *(float *)&v11 = Sin(v13);
  v4 = _mm_shuffle_ps(B(v11), B(v11), 0);
  *(float *)&v12 = Cos(v13);
  v5 = *(self + 2);
  v6 = _mm_shuffle_ps(B(v12), B(v12), 0);
  v7 = _mm_shuffle_ps(_mm_shuffle_ps(v6, v3, 16), _mm_shuffle_ps(v6, v3, 50), 136);
  v8 = *(self + 3);
  v9 = _mm_shuffle_ps(
         _mm_shuffle_ps(_mm_sub_ps(B(0), v4), B(0), 16),
         _mm_shuffle_ps(v4, B(0), 50),
         136);
  v10 = _mm_add_ps(_mm_mul_ps(_mm_shuffle_ps(*(self + 1), *(self + 1), 2), v9), _mm_mul_ps(*(self + 1), v7));
  *self = _mm_add_ps(_mm_mul_ps(_mm_shuffle_ps(*self, *self, 2), v9), _mm_mul_ps(*self, v7));
  *(self + 1) = v10;
  *(self + 2) = _mm_add_ps(_mm_mul_ps(_mm_shuffle_ps(v5, v5, 2), v9), _mm_mul_ps(v7, v5));
  *(self + 3) = _mm_add_ps(_mm_mul_ps(_mm_shuffle_ps(v8, v8, 2), v9), _mm_mul_ps(v8, v7));
  return result;
}


int Kernel5129B0(ts3::ui::MatrixAccumulator *matrix, unsigned int a2)
{
  auto self = reinterpret_cast<__m128*>(matrix);
  int result;
  __m128 v3;
  __m128 v4;
  __m128 v5;
  __m128 v6;
  __m128 v7;
  __m128 v8;
  __m128 v9;
  __m128 v10;
  unsigned int v11;
  unsigned int v12;
  float v13;

  result = 2;
  if ( *((int *)self + 16) < 2 )
    *((uint32_t *)self + 16) = 2;
  v13 = _mm_shuffle_ps(B(a2), B(a2), 0).m128_f32[0];
  v3 = _mm_shuffle_ps(B(0x3F800000u), B(0x3F800000u), 0);
  *(float *)&v11 = Sin(v13);
  v4 = _mm_shuffle_ps(B(v11), B(v11), 0);
  *(float *)&v12 = Cos(v13);
  v5 = *(self + 2);
  v6 = *(self + 3);
  v7 = _mm_shuffle_ps(B(v12), B(v12), 0);
  v8 = _mm_shuffle_ps(_mm_shuffle_ps(v7, v7, 16), _mm_shuffle_ps(v3, v3, 50), 136);
  v9 = _mm_shuffle_ps(
         _mm_shuffle_ps(v4, _mm_sub_ps(B(0), v4), 16),
         _mm_shuffle_ps(B(0), B(0), 50),
         136);
  v10 = _mm_add_ps(_mm_mul_ps(_mm_shuffle_ps(*(self + 1), *(self + 1), 17), v9), _mm_mul_ps(*(self + 1), v8));
  *self = _mm_add_ps(_mm_mul_ps(_mm_shuffle_ps(*self, *self, 17), v9), _mm_mul_ps(*self, v8));
  *(self + 1) = v10;
  *(self + 2) = _mm_add_ps(_mm_mul_ps(_mm_shuffle_ps(v5, v5, 17), v9), _mm_mul_ps(v8, v5));
  *(self + 3) = _mm_add_ps(_mm_mul_ps(_mm_shuffle_ps(v6, v6, 17), v9), _mm_mul_ps(v6, v8));
  return result;
}


__m128 *Kernel512AA0(ts3::ui::MatrixAccumulator *matrix, __m128 *a2, unsigned int a3)
{
  auto self = reinterpret_cast<__m128*>(matrix);
  float v3;
  __m128 v5;
  __m128 v6;
  __m128 v7;
  __m128 v8;
  __m128 v9;
  __m128 v10;
  __m128 v11;
  __m128 v12;
  float v13;
  float v14;
  float v15;
  __m128 v16;
  __m128 v17;
  __m128 v18;
  __m128 v19;
  __m128 v20;
  __m128 v21;
  __m128 v22;
  __m128 v23;
  __m128 v24;
  __m128 v25;
  __m128 v26;
  __m128 v27;
  unsigned int v28;
  unsigned int v29;
  float v30;
  float v31;
  float v32;
  float v33;

  if ( *((int *)self + 16) < 2 )
    *((uint32_t *)self + 16) = 2;
  v3 = _mm_shuffle_ps(B(a3), B(a3), 0).m128_f32[0];
  v5 = _mm_mul_ps(*a2, *a2);
  v6 = _mm_add_ps(_mm_add_ps(_mm_shuffle_ps(v5, v5, 2), _mm_shuffle_ps(v5, v5, 1)), v5);
  v7 = _mm_shuffle_ps(v6, v6, 0);
  v8 = _mm_rsqrt_ps(v7);
  v9 = _mm_shuffle_ps(B(0x3F800000u), B(0x3F800000u), 0);
  v10 = _mm_shuffle_ps(B(0x3F000000u), B(0x3F000000u), 0);
  v11 = _mm_add_ps(_mm_mul_ps(_mm_sub_ps(v9, _mm_mul_ps(_mm_mul_ps(v8, v8), v7)), _mm_mul_ps(v10, v8)), v8);
  *(float *)&v28 = Sin(v3);
  v8.m128_f32[0] = _mm_shuffle_ps(B(v28), B(v28), 0).m128_f32[0];
  *(float *)&v29 = Cos(v3);
  v12 = _mm_mul_ps(
          _mm_add_ps(_mm_mul_ps(_mm_sub_ps(v9, _mm_mul_ps(_mm_mul_ps(v11, v11), v7)), _mm_mul_ps(v10, v11)), v11),
          *a2);
  v7.m128_f32[0] = _mm_shuffle_ps(v12, v12, 0).m128_f32[0];
  v13 = _mm_shuffle_ps(B(v29), B(v29), 0).m128_f32[0];
  v32 = v7.m128_f32[0] * (float)(v9.m128_f32[0] - v13);
  v11.m128_f32[0] = _mm_shuffle_ps(v12, v12, 85).m128_f32[0];
  v12.m128_f32[0] = _mm_shuffle_ps(v12, v12, 170).m128_f32[0];
  v31 = v11.m128_f32[0] * (float)(v9.m128_f32[0] - v13);
  v14 = v12.m128_f32[0] * (float)(v9.m128_f32[0] - v13);
  v30 = v7.m128_f32[0] * v8.m128_f32[0];
  v15 = v12.m128_f32[0] * v8.m128_f32[0];
  v33 = (float)(v7.m128_f32[0] * v14) + (float)(v11.m128_f32[0] * v8.m128_f32[0]);
  v27.m128_u64[1] = U((float)(v12.m128_f32[0] * v32) - (float)(v11.m128_f32[0] * v8.m128_f32[0]));
  v27.m128_f32[1] = (float)(v11.m128_f32[0] * v32) + (float)(v12.m128_f32[0] * v8.m128_f32[0]);
  v27.m128_f32[0] = (float)(v7.m128_f32[0] * v32) + v13;
  v16 = v27;
  v27.m128_u64[1] = U((float)(v12.m128_f32[0] * v31) + v30);
  v27.m128_f32[1] = (float)(v11.m128_f32[0] * v31) + v13;
  v27.m128_f32[0] = (float)(v7.m128_f32[0] * v31) - v15;
  v17 = v27;
  v27.m128_u64[1] = U((float)(v12.m128_f32[0] * v14) + v13);
  v27.m128_f32[1] = (float)(v11.m128_f32[0] * v14) - v30;
  v27.m128_f32[0] = v33;
  v18 = v27;
  v27.m128_i32[3] = v9.m128_i32[0];
  *(uint64_t *)((char *)v27.m128_u64 + 4) = 0;
  v27.m128_i32[0] = 0;
  v19 = *(self + 1);
  v20 = _mm_add_ps(
          _mm_add_ps(_mm_mul_ps(_mm_shuffle_ps(v19, v19, 85), v17), _mm_mul_ps(_mm_shuffle_ps(v19, v19, 0), v16)),
          _mm_mul_ps(_mm_shuffle_ps(v19, v19, 170), v18));
  v21 = _mm_shuffle_ps(v19, v19, 255);
  v22 = *(self + 2);
  v23 = _mm_add_ps(v20, _mm_mul_ps(v21, v27));
  v24 = _mm_add_ps(
          _mm_add_ps(_mm_mul_ps(_mm_shuffle_ps(v22, v22, 85), v17), _mm_mul_ps(_mm_shuffle_ps(v22, v22, 0), v16)),
          _mm_mul_ps(_mm_shuffle_ps(v22, v22, 170), v18));
  v25 = _mm_mul_ps(_mm_shuffle_ps(v22, v22, 255), v27);
  v26 = *(self + 3);
  *self = _mm_add_ps(
            _mm_add_ps(
              _mm_add_ps(
                _mm_mul_ps(_mm_shuffle_ps(*self, *self, 85), v17),
                _mm_mul_ps(_mm_shuffle_ps(*self, *self, 0), v16)),
              _mm_mul_ps(_mm_shuffle_ps(*self, *self, 170), v18)),
            _mm_mul_ps(_mm_shuffle_ps(*self, *self, 255), v27));
  *(self + 1) = v23;
  *(self + 2) = _mm_add_ps(v24, v25);
  *(self + 3) = _mm_add_ps(
                  _mm_add_ps(
                    _mm_add_ps(
                      _mm_mul_ps(_mm_shuffle_ps(v26, v26, 85), v17),
                      _mm_mul_ps(_mm_shuffle_ps(v26, v26, 0), v16)),
                    _mm_mul_ps(_mm_shuffle_ps(v26, v26, 170), v18)),
                  _mm_mul_ps(_mm_shuffle_ps(v26, v26, 255), v27));
  return a2;
}


int Kernel512E70(ts3::ui::MatrixAccumulator *matrix, float a2)
{
  auto self = reinterpret_cast<__m128*>(matrix);
  int result;
  __m128 v3;
  __m128 v4;
  __m128 v5;
  __m128 v6;
  __m128 v7;
  __m128 v8;

  result = 3;
  if ( *((int *)self + 16) < 3 )
    *((uint32_t *)self + 16) = 3;
  v3 = B(0x3F800000u);
  v3.m128_f32[0] = 1.0 / a2;
  v4 = v3;
  v4.m128_f32[0] = (float)((float)(1.0 / a2) * self->m128_f32[2]) + self->m128_f32[3];
  *self = _mm_shuffle_ps(*self, _mm_shuffle_ps(*self, _mm_shuffle_ps(v4, v4, 0), 10), 132);
  v7 = *(self + 1);
  v5 = v3;
  v5.m128_f32[0] = (float)((float)(1.0 / a2) * v7.m128_f32[2]) + v7.m128_f32[3];
  *(self + 1) = _mm_shuffle_ps(v7, _mm_shuffle_ps(v7, _mm_shuffle_ps(v5, v5, 0), 10), 132);
  v8 = *(self + 2);
  v6 = v3;
  v6.m128_f32[0] = (float)((float)(1.0 / a2) * v8.m128_f32[2]) + v8.m128_f32[3];
  *(self + 2) = _mm_shuffle_ps(v8, _mm_shuffle_ps(v8, _mm_shuffle_ps(v6, v6, 0), 10), 132);
  v3.m128_f32[0] = (float)((float)(1.0 / a2) * *((float *)self + 14)) + *((float *)self + 15);
  *(self + 3) = _mm_shuffle_ps(*(self + 3), _mm_shuffle_ps(*(self + 3), _mm_shuffle_ps(v3, v3, 0), 10), 132);
  return result;
}


__m128 *Kernel561E60(__m128 *a1, __m128 *a2, __m128 *a3)
{
  __m128 v3;
  __m128 v4;
  __m128 v5;
  __m128 v6;
  __m128 v7;
  __m128 v8;
  __m128 v9;
  __m128 v10;
  __m128 v11;
  __m128 v12;
  __m128 v13;
  __m128 v14;
  __m128 v15;
  __m128 v16;
  __m128 v17;
  __m128 v18;
  __m128 v20;
  __m128 v21;
  __m128 v22;
  __m128 v23;
  __m128 v24;
  __m128 v25;
  __m128 v26;
  __m128 v27;
  __m128 v28;
  __m128 v29;
  __m128 v30;
  __m128 v31;
  __m128 v32;
  __m128 v33;
  __m128 v34;
  __m128 v35;
  __m128 v36;

  v3 = _mm_shuffle_ps(a2[2], a2[2], 57);
  v4 = _mm_shuffle_ps(*a2, *a2, 57);
  v29 = _mm_shuffle_ps(v4, v4, 57);
  v28 = _mm_shuffle_ps(v29, v29, 57);
  v34 = v4;
  v30 = _mm_shuffle_ps(a2[3], a2[3], 57);
  v5 = _mm_shuffle_ps(v30, v30, 57);
  v6 = _mm_shuffle_ps(v3, v3, 57);
  v33 = _mm_shuffle_ps(v6, v6, 57);
  v31 = _mm_shuffle_ps(v5, v5, 57);
  v36 = v3;
  v7 = v3;
  v27 = _mm_shuffle_ps(a2[1], a2[1], 57);
  v8 = _mm_shuffle_ps(v27, v27, 57);
  v9 = _mm_add_ps(
         _mm_mul_ps(_mm_sub_ps(_mm_mul_ps(v31, v6), _mm_mul_ps(v33, v5)), v27),
         _mm_mul_ps(_mm_sub_ps(_mm_mul_ps(v33, v30), _mm_mul_ps(v31, v3)), v8));
  v10 = _mm_sub_ps(_mm_mul_ps(v5, v3), _mm_mul_ps(v6, v30));
  v26.m128_i32[0] = 0;
  v11 = _mm_shuffle_ps(v8, v8, 57);
  v26.m128_i32[3] = 0x80000000;
  *(uint64_t *)((char *)v26.m128_u64 + 4) = 0x80000000LL;
  v12 = _mm_xor_ps(_mm_add_ps(v9, _mm_mul_ps(v10, v11)), v26);
  v13 = _mm_mul_ps(*a2, v12);
  v14 = _mm_add_ps(_mm_shuffle_ps(v13, v13, 14), v13);
  v15 = _mm_add_ps(_mm_shuffle_ps(v14, v14, 1), v14);
  v32 = _mm_shuffle_ps(v15, v15, 0);
  *a3 = v32;
  v16 = v34;
  v26.m128_u64[1] = 0x80000000LL;
  v26.m128_u64[0] = 0x80000000LL;
  v35 = _mm_xor_ps(
          _mm_add_ps(
            _mm_add_ps(
              _mm_mul_ps(_mm_sub_ps(_mm_mul_ps(v28, v5), _mm_mul_ps(v31, v29)), v7),
              _mm_mul_ps(_mm_sub_ps(_mm_mul_ps(v31, v34), _mm_mul_ps(v28, v30)), v6)),
            _mm_mul_ps(_mm_sub_ps(_mm_mul_ps(v29, v30), _mm_mul_ps(v5, v34)), v33)),
          v26);
  v26.m128_i32[0] = 0;
  v26.m128_i32[3] = 0x80000000;
  *(uint64_t *)((char *)v26.m128_u64 + 4) = 0x80000000LL;
  v17 = _mm_xor_ps(
          _mm_add_ps(
            _mm_add_ps(
              _mm_mul_ps(_mm_sub_ps(_mm_mul_ps(v11, v29), _mm_mul_ps(v28, v8)), v30),
              _mm_mul_ps(_mm_sub_ps(_mm_mul_ps(v28, v27), _mm_mul_ps(v11, v16)), v5)),
            _mm_mul_ps(_mm_sub_ps(_mm_mul_ps(v8, v16), _mm_mul_ps(v29, v27)), v31)),
          v26);
  v18 = _mm_add_ps(
          _mm_add_ps(
            _mm_mul_ps(_mm_sub_ps(_mm_mul_ps(v33, v8), _mm_mul_ps(v11, v6)), v16),
            _mm_mul_ps(_mm_sub_ps(_mm_mul_ps(v11, v36), _mm_mul_ps(v33, v27)), v29)),
          _mm_mul_ps(_mm_sub_ps(_mm_mul_ps(v6, v27), _mm_mul_ps(v8, v36)), v28));
  v27.m128_u64[1] = 0x80000000LL;
  v27.m128_u64[0] = 0x80000000LL;
  v20 = _mm_xor_ps(v18, v27);
  v21 = _mm_shuffle_ps(v12, v35, 68);
  v22 = _mm_shuffle_ps(v12, v35, 238);
  v23 = v17;
  v24 = _mm_shuffle_ps(v17, v20, 238);
  v25 = _mm_shuffle_ps(v23, v20, 68);
  a1[1] = _mm_div_ps(_mm_shuffle_ps(v21, v25, 221), v32);
  *a1 = _mm_div_ps(_mm_shuffle_ps(v21, v25, 136), v32);
  a1[2] = _mm_div_ps(_mm_shuffle_ps(v22, v24, 136), v32);
  a1[3] = _mm_div_ps(_mm_shuffle_ps(v22, v24, 221), v32);
  return a1;
}


double Kernel5621D0(__m128 *a1)
{
  __m128 v1;
  __m128 v2;
  __m128 v3;
  __m128 v4;
  __m128 v5;
  __m128 v6;
  __m128 v7;
  __m128 v8;
  __m128 v9;
  __m128 v10;
  __m128 v11;
  __m128 v12;
  __m128 v13;
  __m128 v14;
  __m128 v15;
  __m128 v16;
  __m128 v17;
  __m128 v18;
  __m128 v19;
  __m128 v20;
  __m128 v21;
  __m128 v22;
  float v23;
  float v25;
  unsigned int v26;
  __m128 v27;
  float v28;

  v1 = a1[1];
  v2 = a1[3];
  v3 = _mm_shuffle_ps(*a1, v1, 238);
  v4 = _mm_shuffle_ps(*a1, v1, 68);
  v5 = _mm_shuffle_ps(a1[2], v2, 68);
  v6 = _mm_shuffle_ps(a1[2], v2, 238);
  v7 = _mm_setzero_ps();
  v8 = _mm_add_ps(
         _mm_add_ps(
           _mm_mul_ps(
             _mm_shuffle_ps(B(0x3F80000000000000ULL), B(0x3F80000000000000ULL), 85),
             _mm_shuffle_ps(v4, v5, 221)),
           _mm_mul_ps(
             _mm_shuffle_ps(B(0x3F80000000000000ULL), B(0x3F80000000000000ULL), 0),
             _mm_shuffle_ps(v4, v5, 136))),
         _mm_mul_ps(
           _mm_shuffle_ps(B(0x3F80000000000000ULL), B(0x3F80000000000000ULL), 170),
           _mm_shuffle_ps(v3, v6, 136)));
  v9 = _mm_mul_ps(B(0x3F800000), v8);
  v10 = _mm_add_ps(_mm_add_ps(_mm_shuffle_ps(v9, v9, 2), _mm_shuffle_ps(v9, v9, 1)), v9);
  v10.m128_f32[0] = _mm_shuffle_ps(v10, v10, 0).m128_f32[0];
  v11 = _mm_sub_ps(
          v8,
          _mm_mul_ps(_mm_shuffle_ps(B(v10.m128_u32[0]), B(v10.m128_u32[0]), 0), B(0x3F800000)));
  v12 = _mm_mul_ps(v11, v11);
  v13 = _mm_add_ps(_mm_add_ps(_mm_shuffle_ps(v12, v12, 2), _mm_shuffle_ps(v12, v12, 1)), v12);
  v14 = _mm_shuffle_ps(v13, v13, 0);
  v15 = v14;
  v16 = _mm_rsqrt_ps(v14);
  v14.m128_f32[0] = 1.0;
  v27 = v15;
  v17 = _mm_shuffle_ps(v14, v14, 0);
  v18 = _mm_shuffle_ps(B(0x3F000000u), B(0x3F000000u), 0);
  v19 = _mm_add_ps(_mm_mul_ps(_mm_sub_ps(v17, _mm_mul_ps(_mm_mul_ps(v16, v16), v15)), _mm_mul_ps(v18, v16)), v16);
  v20 = _mm_mul_ps(
          _mm_add_ps(_mm_mul_ps(_mm_sub_ps(v17, _mm_mul_ps(_mm_mul_ps(v19, v19), v27)), _mm_mul_ps(v18, v19)), v19),
          v11);
  v21 = _mm_mul_ps(B(0x3F80000000000000ULL), v20);
  v22 = _mm_add_ps(_mm_add_ps(_mm_shuffle_ps(v21, v21, 2), _mm_shuffle_ps(v21, v21, 1)), v21);
  v28 = _mm_shuffle_ps(v22, v22, 0).m128_f32[0];
  v23 = 0.0;
  v25 = 0.0;
  if ( v28 > -1.0 && v28 < 1.0 )
  {
    *(float *)&v26 = acos(_mm_shuffle_ps(B(U(v28)), B(U(v28)), 0).m128_f32[0]);
    v7 = _mm_setzero_ps();
    v23 = _mm_shuffle_ps(B(v26), B(v26), 0).m128_f32[0];
    v25 = v23;
  }
  if ( _mm_shuffle_ps(v20, v20, 170).m128_f32[0] > _mm_shuffle_ps(v7, v7, 0).m128_f32[0] )
    return (float)(-0.0 - v23);
  return v25;
}


float *Kernel7A1840(float *self, float *a2, float *a3)
{
  float v4;
  float v5;
  float v6;
  float v7;
  float v8;
  float *result;
  float v10;
  float v11;
  float v12;
  float v13;
  float v14;
  float v15;
  float v16;
  float v17;
  float v18;
  float v19;
  float v20;
  float v21;
  float v22;
  float v23;
  float v24;
  float v25;
  float v26;
  float v27;
  float v28;
  float v29;
  float v30;
  float v31;
  float v32;
  float v33;
  float v34;
  float v35;
  float v36;
  float v37;
  float v38;
  float v39;

  v4 = *(self + 1);
  v5 = a3[3];
  v6 = *a3;
  v7 = *(self + 2);
  v8 = a3[6];
  result = a2;
  v10 = (float)(a3[1] * *self) + (float)(a3[4] * v4);
  *a2 = (float)((float)(*self * *a3) + (float)(v4 * v5)) + (float)(v7 * v8);
  v11 = a3[7];
  v12 = a3[2];
  a2[1] = v10 + (float)(v11 * v7);
  v13 = v12 * *self;
  v14 = a3[5] * v4;
  v15 = *(self + 4);
  v16 = v13 + v14;
  v17 = a3[8] * v7;
  v18 = *(self + 3);
  v19 = v16 + v17;
  v20 = *(self + 5);
  a2[2] = v19;
  v21 = (float)((float)(v18 * v6) + (float)(v15 * v5)) + (float)(v20 * v8);
  v22 = a3[1];
  a2[3] = v21;
  v23 = v20 * v11;
  v24 = v18 * v22;
  v25 = a3[4];
  v26 = (float)(v23 + v24) + (float)(v15 * v25);
  v27 = v18 * a3[2];
  v39 = a3[2];
  v28 = a3[5];
  v29 = v27 + (float)(v15 * v28);
  v30 = *(self + 7);
  a2[4] = v26;
  v38 = v28;
  v31 = a3[8];
  v32 = v29 + (float)(v20 * v31);
  v33 = *(self + 8);
  result[5] = v32;
  v34 = (float)((float)(v30 * v5) + (float)(v33 * v8)) + (float)(*(self + 6) * *a3);
  v35 = (float)(*(self + 6) * a3[1]) + (float)(v30 * v25);
  v36 = v33 * a3[7];
  v37 = (float)((float)(*(self + 6) * v39) + (float)(v30 * v38)) + (float)(v33 * v31);
  result[6] = v34;
  result[7] = v35 + v36;
  result[8] = v37;
  return result;
}


float* Kernel7C23E0(float *self, float* output, float *a3)
{
  auto a2 = reinterpret_cast<uintptr_t>(output);
  float v5;
  float v6;
  float v7;
  float v8;
  float v9;
  float v10;
  float v11;
  float v12;
  float v13;
  float v14;
  float v15;
  float v16;
  float v17;
  float v18;
  float v19;
  float v20;
  float v21;
  float v22;
  float v23;
  float v24;
  float v25;
  float v26;
  float v27;
  float v28;
  float v29;
  float v30;
  float v31;

  v5 = *(self + 1);
  v6 = *self;
  v7 = *a3;
  v8 = a3[4];
  v9 = a3[8];
  *(uint32_t *)(a2 + 12) = 0;
  *(uint32_t *)(a2 + 28) = 0;
  *(uint32_t *)(a2 + 44) = 0;
  *(uint32_t *)(a2 + 48) = 0;
  *(uint32_t *)(a2 + 52) = 0;
  *(uint32_t *)(a2 + 56) = 0;
  *(uint32_t *)(a2 + 60) = 1065353216;
  v10 = *(self + 2);
  v11 = a3[5];
  *(float *)a2 = (float)((float)(v6 * v7) + (float)(v5 * v8)) + (float)(v10 * v9);
  v12 = (float)((float)(v11 * v5) + (float)(a3[9] * v10)) + (float)(a3[1] * v6);
  v13 = a3[10] * v10;
  v14 = a3[2];
  *(float *)(a2 + 4) = v12;
  v15 = a3[6] * v5;
  v16 = *(self + 6);
  v17 = v14 * v6;
  v18 = *(self + 5);
  v19 = (float)(v15 + v13) + v17;
  v20 = *(self + 4);
  *(float *)(a2 + 8) = v19;
  v21 = (float)((float)(v16 * v9) + (float)(v20 * v7)) + (float)(v18 * v8);
  v22 = a3[1];
  *(float *)(a2 + 16) = v21;
  v23 = v20 * v22;
  v24 = a3[9];
  *(float *)(a2 + 20) = (float)(v23 + (float)(v18 * a3[5])) + (float)(v16 * v24);
  v25 = (float)(v20 * a3[2]) + (float)(v18 * a3[6]);
  v26 = *(self + 9);
  v27 = v25 + (float)(v16 * a3[10]);
  v28 = *(self + 10);
  *(float *)(a2 + 24) = v27;
  v29 = (float)((float)(*(self + 8) * *a3) + (float)(v26 * v8)) + (float)(v28 * v9);
  v30 = (float)((float)(*(self + 8) * a3[1]) + (float)(v26 * a3[5])) + (float)(v28 * v24);
  v31 = (float)((float)(*(self + 8) * a3[2]) + (float)(v26 * a3[6])) + (float)(v28 * a3[10]);
  *(float *)(a2 + 32) = v29;
  *(float *)(a2 + 36) = v30;
  *(float *)(a2 + 40) = v31;
  return output;
}


float *Kernel46E2F0(float *a1, float *a2, float a3)
{
  float v3;
  float v4;
  float v5;
  float v6;
  float v7;
  float v8;
  long double v9;
  long double v10;
  float v11;
  float v12;
  float v13;
  int v14;
  float v15;
  float v16;
  long double v17;
  long double v18;
  long double v19;
  int v20;
  float v22;
  float v23;
  float v24;
  float v25;
  float v26;
  float v27;

  v3 = a1[4];
  v4 = a1[1];
  v5 = a1[2];
  v6 = *a1 - a3;
  v7 = a1[3] - a3;
  v8 = a1[5] - a3;
  v26 = (float)(v8 * v7) - (float)(v3 * v3);
  v9 = fabs(v26);
  v27 = (float)(v3 * v5) - (float)(v8 * v4);
  v10 = fabs(v27);
  v11 = (float)(v3 * v4) - (float)(v7 * v5);
  v12 = v5 * v5;
  v13 = (float)(v5 * v4) - (float)(v3 * v6);
  v14 = 0;
  v15 = (float)(v8 * v6) - v12;
  v16 = (float)(v7 * v6) - (float)(v4 * v4);
  if ( v10 > v9 )
  {
    v22 = v10;
    v9 = v22;
    v14 = 1;
  }
  v17 = fabs(v11);
  if ( v17 > v9 )
  {
    v23 = v17;
    v9 = v23;
    v14 = 2;
  }
  v18 = fabs(v15);
  if ( v18 > v9 )
  {
    v24 = v18;
    v9 = v24;
    v14 = 3;
  }
  v19 = fabs(v13);
  if ( v19 > v9 )
  {
    v25 = v19;
    v9 = v25;
    v14 = 4;
  }
  if ( fabs(v16) > v9 )
    v14 = 5;
  if ( v14 )
  {
    v20 = v14 - 1;
    if ( !v20 || v20 == 2 )
    {
      *a2 = v27;
      a2[1] = v15;
      a2[2] = v13;
      return a2;
    }
    else
    {
      *a2 = v11;
      a2[2] = v16;
      a2[1] = v13;
      return a2;
    }
  }
  else
  {
    *a2 = v26;
    a2[2] = v11;
    a2[1] = v27;
    return a2;
  }
}



float* Kernel46E4B0(float* a, float* out, float lambda) {
    // x87 keeps the first subtraction extended while selecting the largest entry.
    double first=static_cast<double>(a[0])-lambda;
    float x=static_cast<float>(first), y=a[3]-lambda, z=a[5]-lambda;
    double best=std::fabs(first); int index=0;
    float v[6]={x,a[1],a[2],y,a[4],z};
    for(int i=1;i<6;++i) if(std::fabs(static_cast<double>(v[i]))>best) { best=std::fabs(static_cast<double>(v[i])); index=i; }
    if(index==5) { out[0]=0; out[1]=-0.0f-z; out[2]=v[4]; }
    else if(index<2) { out[0]=-0.0f-v[1]; out[1]=x; out[2]=0; }
    else if(index==2) { out[0]=v[2]; out[1]=0; out[2]=-0.0f-x; }
    else { out[0]=0; out[1]=-0.0f-v[4]; out[2]=y; }
    return out;
}
float* Kernel46E650(float* out, float* a2) {
    float v3=a2[3], v4=a2[5], v5=a2[4];
    float v6=((*a2+v3)*v4)+(*a2*v3);
    float sum=(v4+*a2)+v3;
    float pair=((v6-a2[1]*a2[1])-a2[2]*a2[2])-v5*v5;
    float p=pair-(sum*sum)*0.33333334f;
    float q=((pair*0.33333334f-(sum*sum)*0.074074075f)*sum)
        -((((((v5*a2[2])*a2[1])*2.0f+(a2[5]*a2[0])*v3)-(v5*v5)*a2[0])-(a2[2]*a2[2])*v3)-(a2[1]*a2[1])*a2[5]);
    float q2=(q*q)*0.25f;
    float disc=((p*p)*p)*0.037037037f+q2;
    if(disc>0.00000011920929f) { out[0]=out[1]=out[2]=1; return out; }
    float third=sum*0.33333334f;
    if(disc < -0.00000011920929f) {
        float angle=static_cast<float>(std::atan2(std::sqrt(-static_cast<double>(disc)),static_cast<double>(q)*-0.5));
        float radius=static_cast<float>(std::pow(std::sqrt(static_cast<double>(q2)-disc),0.3333333432674408));
        double theta=static_cast<double>(angle)*0.33333334f;
        float c=static_cast<float>(std::cos(theta));
        float t=static_cast<float>(std::sin(theta)*std::sqrt(3.0));
        float l0=(c*radius)*2.0f+third;
        float l1=third-(t+c)*radius;
        float l2=third-(c-t)*radius;
        if(std::fabs(l1)>std::fabs(l0)) l0=l1;
        if(std::fabs(l2)>std::fabs(l0)) l0=l2;
        return Kernel46E2F0(a2,out,l0);
    }
    float root=static_cast<float>(q<0 ? -std::pow(static_cast<double>(q)*-0.5,0.3333333432674408) : std::pow(static_cast<double>(q)*0.5,0.3333333432674408));
    float l0=third+root, l1=third-root*2.0f;
    if(std::fabs(l0)>std::fabs(l1)) return Kernel46E4B0(a2,out,l0);
    return Kernel46E2F0(a2,out,l1);
}
float* Kernel92BE60(float* out,float divisor) {
    float step=1.0f/divisor;
    for(int i=0;i<16;++i) {
        double angle=static_cast<double>(i)*step;
#if defined(_M_IX86)
        float sineValue,cosineValue;
        __asm {
 fld angle
 fsin
 fstp sineValue
 fld angle
 fcos
 fstp cosineValue
 }
        out[2*i]=sineValue; out[2*i+1]=cosineValue;
#else
        out[2*i]=static_cast<float>(std::sin(angle));
        out[2*i+1]=static_cast<float>(std::cos(angle));
#endif
    }
    return out;
}

#if defined(_M_IX86)
// The two symmetric leaves use custom register ABIs, not thiscall. These
// thunks also retain EDX/ECX respectively, as their original caller requires.
__declspec(naked) void Entry46E2F0() {
    __asm {
        push edx
        sub esp,4
        movss dword ptr [esp],xmm6
        push edx
        push eax
        call Kernel46E2F0
        add esp,12
        pop edx
        ret
    }
}
__declspec(naked) void Entry46E4B0() {
    __asm {
        push ecx
        push dword ptr [esp+8]
        push ecx
        push eax
        call Kernel46E4B0
        add esp,12
        pop ecx
        ret
    }
}
float* __fastcall Entry92BE60(float* self,void*) {
    return Kernel92BE60(self,*reinterpret_cast<const float*>(0x011DC058));
}
__m128* __fastcall Entry511BB0(ts3::ui::MatrixAccumulator* self,void*,__m128* input) { return Kernel511BB0(self,input); }
__m128* __fastcall Entry511D60(ts3::ui::MatrixAccumulator* self,void*,__m128* out,__m128* input) { return Kernel511D60(self,out,input); }
int __fastcall Entry5127D0(ts3::ui::MatrixAccumulator* self,void*,unsigned angle) { return Kernel5127D0(self,angle); }
int __fastcall Entry5128C0(ts3::ui::MatrixAccumulator* self,void*,unsigned angle) { return Kernel5128C0(self,angle); }
int __fastcall Entry5129B0(ts3::ui::MatrixAccumulator* self,void*,unsigned angle) { return Kernel5129B0(self,angle); }
__m128* __fastcall Entry512AA0(ts3::ui::MatrixAccumulator* self,void*,__m128* axis,unsigned angle) { return Kernel512AA0(self,axis,angle); }
int __fastcall Entry512E70(ts3::ui::MatrixAccumulator* self,void*,float distance) { return Kernel512E70(self,distance); }
float* __fastcall Entry7A1840(float* self,void*,float* out,float* right) { return Kernel7A1840(self,out,right); }
float* __fastcall Entry7C23E0(float* self,void*,float* out,float* right) { return Kernel7C23E0(self,out,right); }
#endif
}
