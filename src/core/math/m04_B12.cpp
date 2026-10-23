#include "ts3/math/m04_B12.h"
#include "ts3/layout.h"
#include <cmath>
#include <cstring>
#pragma float_control(precise, on, push)
namespace ts3::math::b12 {
namespace {
__m128 Bits64(uint64_t x) { alignas(16) uint64_t v[2]={x,0}; return _mm_load_ps(reinterpret_cast<const float*>(v)); }
}
float* Project(float* a1, float* a2)
{
  float v3;
  float v4;
  float v5;
  float v6;
  float v7;
  float v8;

  v3 = a2[2];
  v4 = a2[1];
  v5 = (float)((float)((float)(a1[4] * v4) + (float)(a1[8] * v3)) + (float)(*a1 * *a2)) + a1[12];
  v6 = (float)((float)((float)(a1[1] * *a2) + (float)(a1[5] * v4)) + (float)(a1[9] * v3)) + a1[13];
  v7 = (float)((float)((float)(a1[2] * *a2) + (float)(a1[6] * v4)) + (float)(a1[10] * v3)) + a1[14];
  v8 = 1.0f / (float)((float)((float)((float)(a1[3] * *a2) + (float)(a1[7] * v4)) + (float)(a1[11] * v3)) + a1[15]);
  *a2 = v8 * v5;
  a2[1] = v8 * v6;
  a2[2] = v8 * v7;
  return a1;
}


float* CubeUV(float* a1, int* a2, float* a3, float* a4)
{
  long double v5;
  long double v6;
  long double v7;
  float v8;
  float v9;
  float v10;
  float v11;
  int v12;
  int v13;
  float v15;
  float v16;
  float v17;
  float v18;
  float v19;
  float v20;

  v5 = *a1;
  v20 = *a1;
  v6 = fabs(v5);
  v19 = v6;
  v16 = a1[1];
  v17 = fabs(v16);
  v15 = a1[2];
  v7 = fabs(v15);
  v18 = v7;
  if ( !(v7 >= v6) || !(v18 >= v17) )
  {
    if ( !(v17 >= v19) )
    {
      v9 = v15 / v19;
      v10 = v16 / *a1;
      v13 = 4;
      if ( !(*a1 >= 0.0f) )
        v13 = 5;
      *a2 = v13;
      if ( v13 == 4 )
      {
        v9 = -0.0f - v9;
        v10 = -0.0f - v10;
      }
    }
    else
    {
      v11 = a1[1];
      v9 = v20 / v17;
      v10 = v15 / v11;
      v12 = 3;
      if ( !(v11 >= 0.0f) )
        v12 = 1;
      *a2 = v12;
    }
  }
  else
  {
    v8 = a1[2];
    v9 = -0.0f - (float)(v20 / v8);
    v10 = v16 / v18;
    if ( !(v8 >= 0.0f) )
      *a2 = 2;
    else
      *a2 = 0;
  }
  *a3 = (float)(v9 + 1.0f) * 0.5f;
  *a4 = (float)(v10 + 1.0f) * 0.5f;
  return a4;
}


int CubeFace(float a1, float a2, float a3)
{
  long double v3;
  long double v4;
  int result;
  float v6;
  float v7;
  float v8;

  v3 = fabs(a1);
  v4 = fabs(a2);
  v8 = fabs(a3);
  if ( !(v3 > v4) )
  {
    v7 = v4;
    if ( v7 > v8 )
    {
      if ( !(a2 > 0.0f) )
        return 4;
      else
        return 1;
    }
  }
  else
  {
    v6 = v3;
    if ( v6 > v8 )
    {
      if ( !(a1 > 0.0f) )
        return 3;
      else
        return 0;
    }
  }
  result = 2;
  if ( !(a3 > 0.0f) )
    return 5;
  return result;
}


float* Rotate(float* a1, float* a2, float* a3)
{
  float v3;
  float v4;
  float v5;
  float v6;
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

  v3 = *a3;
  v4 = a3[1];
  v5 = a3[2];
  v6 = a3[3];
  v20 = v3 * v3;
  v23 = v4 * *a3;
  v9 = v5 * v3;
  v10 = v5 * v5;
  v19 = v5 * v4;
  v11 = v6 * v3;
  v12 = v6 * v5;
  v21 = v6 * v4;
  v22 = (float)((float)((float)(v6 * v4) + v9) * a2[2]) + (float)((float)(v23 - v12) * a2[1]);
  v13 = (float)(v10 + (float)(v4 * v4)) * 2.0f;
  v14 = (float)(v4 * v4) + (float)(v3 * v3);
  v15 = *a2;
  v16 = a2[2];
  *a1 = (float)(v22 * 2.0f) + (float)((float)(1.0f - v13) * *a2);
  v17 = 1.0f - (float)((float)(v10 + v20) * 2.0f);
  v18 = a2[1];
  a1[1] = (float)((float)((float)((float)(v12 + v23) * v15) + (float)((float)(v19 - v11) * v16)) * 2.0f)
        + (float)(v17 * v18);
  a1[2] = (float)((float)((float)((float)(v9 - v21) * v15) + (float)((float)(v11 + v19) * v18)) * 2.0f)
        + (float)((float)(1.0f - (float)(v14 * 2.0f)) * v16);
  return a1;
}


bool DirectionThreshold(__m128* a2, __m128* a3)
{
  __m128 a1 = _mm_setzero_ps();
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
  __m128 v22;
  __m128 v23;
  __m128 v24;
  __m128 v25;
  __m128 v26;
  __m128 v27;
  __m128 v28;
  __m128 v29;

  v3 = *a2;
  v4 = _mm_mul_ps(v3, v3);
  v5 = _mm_add_ps(_mm_add_ps(_mm_shuffle_ps(v4, v4, 2), _mm_shuffle_ps(v4, v4, 1)), v4);
  if ( !(_mm_shuffle_ps(v5, v5, 0).m128_f32[0] > _mm_shuffle_ps(Bits64(0x3A83126Full), Bits64(0x3A83126Full), 0).m128_f32[0]) )
    return 0;
  v6 = _mm_add_ps(_mm_add_ps(_mm_shuffle_ps(v4, v4, 2), _mm_shuffle_ps(v4, v4, 1)), v4);
  v7 = _mm_shuffle_ps(v6, v6, 0);
  v8 = _mm_rsqrt_ps(v7);
  a1.m128_f32[0] = 1.0f;
  v9 = _mm_shuffle_ps(a1, a1, 0);
  v10 = _mm_shuffle_ps(Bits64(0x3F000000ull), Bits64(0x3F000000ull), 0);
  v11 = _mm_add_ps(_mm_mul_ps(_mm_sub_ps(v9, _mm_mul_ps(_mm_mul_ps(v8, v8), v7)), _mm_mul_ps(v10, v8)), v8);
  v12 = _mm_mul_ps(
          _mm_add_ps(_mm_mul_ps(_mm_sub_ps(v9, _mm_mul_ps(_mm_mul_ps(v11, v11), v7)), _mm_mul_ps(v10, v11)), v11),
          v3);
  v13 = _mm_shuffle_ps(v12, v12, 9);
  v14 = _mm_shuffle_ps(v12, v12, 18);
  v15 = _mm_sub_ps(
          _mm_mul_ps(_mm_shuffle_ps(Bits64(0x3F80000000000000ull), Bits64(0x3F80000000000000ull), 9), v14),
          _mm_mul_ps(_mm_shuffle_ps(Bits64(0x3F80000000000000ull), Bits64(0x3F80000000000000ull), 18), v13));
  v16 = _mm_sub_ps(_mm_mul_ps(_mm_shuffle_ps(v15, v15, 18), v13), _mm_mul_ps(_mm_shuffle_ps(v15, v15, 9), v14));
  v17 = _mm_shuffle_ps(v16, v16, 85);
  v28 = _mm_shuffle_ps(v15, v15, 0);
  v18 = _mm_shuffle_ps(v12, v12, 170);
  v29 = _mm_add_ps(_mm_add_ps(v17, v28), v18);
  if ( !(v29.m128_f32[0] > _mm_shuffle_ps(_mm_setzero_ps(), _mm_setzero_ps(), 0).m128_f32[0]) )
  {
    if ( !(v28.m128_f32[0] > v17.m128_f32[0]) || !(v28.m128_f32[0] > v18.m128_f32[0]) )
    {
      if ( !(v17.m128_f32[0] > v18.m128_f32[0]) )
      {
        v26 = _mm_sqrt_ps(_mm_add_ps(_mm_sub_ps(v18, _mm_add_ps(v17, v28)), v9));
        v27 = _mm_div_ps(v10, v26);
        *a3 = _mm_shuffle_ps(
                _mm_shuffle_ps(
                  _mm_mul_ps(_mm_add_ps(_mm_shuffle_ps(v15, v15, 170), _mm_shuffle_ps(v12, v12, 0)), v27),
                  _mm_mul_ps(_mm_add_ps(_mm_shuffle_ps(v16, v16, 170), _mm_shuffle_ps(v12, v12, 85)), v27),
                  16),
                _mm_shuffle_ps(
                  _mm_mul_ps(v26, v10),
                  _mm_mul_ps(_mm_sub_ps(_mm_shuffle_ps(v15, v15, 85), _mm_shuffle_ps(v16, v16, 0)), v27),
                  50),
                136);
      }
      else
      {
        v24 = _mm_sqrt_ps(_mm_add_ps(v9, _mm_sub_ps(v17, _mm_add_ps(v18, v28))));
        v25 = _mm_div_ps(v10, v24);
        *a3 = _mm_shuffle_ps(
                _mm_shuffle_ps(
                  _mm_mul_ps(_mm_add_ps(_mm_shuffle_ps(v16, v16, 0), _mm_shuffle_ps(v15, v15, 85)), v25),
                  _mm_mul_ps(v24, v10),
                  16),
                _mm_shuffle_ps(
                  _mm_mul_ps(_mm_add_ps(_mm_shuffle_ps(v16, v16, 170), _mm_shuffle_ps(v12, v12, 85)), v25),
                  _mm_mul_ps(_mm_sub_ps(_mm_shuffle_ps(v12, v12, 0), _mm_shuffle_ps(v15, v15, 170)), v25),
                  50),
                136);
      }
      return 1;
    }
    else
    {
      v22 = _mm_sqrt_ps(_mm_add_ps(v9, _mm_sub_ps(v28, _mm_add_ps(v18, v17))));
      v23 = _mm_div_ps(v10, v22);
      *a3 = _mm_shuffle_ps(
              _mm_shuffle_ps(
                _mm_mul_ps(v22, v10),
                _mm_mul_ps(_mm_add_ps(_mm_shuffle_ps(v16, v16, 0), _mm_shuffle_ps(v15, v15, 85)), v23),
                16),
              _mm_shuffle_ps(
                _mm_mul_ps(_mm_add_ps(_mm_shuffle_ps(v15, v15, 170), _mm_shuffle_ps(v12, v12, 0)), v23),
                _mm_mul_ps(_mm_sub_ps(_mm_shuffle_ps(v16, v16, 170), _mm_shuffle_ps(v12, v12, 85)), v23),
                50),
              136);
      return 1;
    }
  }
  else
  {
    v19 = _mm_sqrt_ps(_mm_add_ps(v9, v29));
    v20 = _mm_div_ps(v10, v19);
    *a3 = _mm_shuffle_ps(
            _mm_shuffle_ps(
              _mm_mul_ps(_mm_sub_ps(_mm_shuffle_ps(v16, v16, 170), _mm_shuffle_ps(v12, v12, 85)), v20),
              _mm_mul_ps(_mm_sub_ps(_mm_shuffle_ps(v12, v12, 0), _mm_shuffle_ps(v15, v15, 170)), v20),
              16),
            _mm_shuffle_ps(
              _mm_mul_ps(_mm_sub_ps(_mm_shuffle_ps(v15, v15, 85), _mm_shuffle_ps(v16, v16, 0)), v20),
              _mm_mul_ps(v19, v10),
              50),
            136);
    return 1;
  }
}


bool Direction(__m128* a1, __m128* a2)
{
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
  __m128 v23;
  __m128 v24;
  __m128 v25;
  __m128 v26;
  __m128 v27;
  __m128 v28;
  float v29;
  __m128 v30;
  __m128 v31;
  float v32;
  __m128 v33;

  v2 = *a1;
  v3 = _mm_mul_ps(v2, v2);
  v4 = _mm_add_ps(_mm_add_ps(_mm_shuffle_ps(v3, v3, 2), _mm_shuffle_ps(v3, v3, 1)), v3);
  v5 = _mm_shuffle_ps(v4, v4, 0);
  v29 = v5.m128_f32[0];
  v5.m128_f32[0] = 0.0f;
  v32 = _mm_shuffle_ps(v5, v5, 0).m128_f32[0];
  if ( !(v29 > v32) )
    return 0;
  v6 = _mm_mul_ps(v2, v2);
  v7 = _mm_add_ps(_mm_add_ps(_mm_shuffle_ps(v6, v6, 2), _mm_shuffle_ps(v6, v6, 1)), v6);
  v8 = _mm_shuffle_ps(v7, v7, 0);
  v9 = _mm_rsqrt_ps(v8);
  v10 = _mm_shuffle_ps(Bits64(0x3F800000ull), Bits64(0x3F800000ull), 0);
  v11 = _mm_shuffle_ps(Bits64(0x3F000000ull), Bits64(0x3F000000ull), 0);
  v12 = _mm_add_ps(_mm_mul_ps(_mm_sub_ps(v10, _mm_mul_ps(_mm_mul_ps(v9, v9), v8)), _mm_mul_ps(v11, v9)), v9);
  v30.m128_u64[1] = 0;
  v30.m128_u64[0] = 0x3F80000000000000LL;
  v13 = _mm_mul_ps(
          _mm_add_ps(_mm_mul_ps(_mm_sub_ps(v10, _mm_mul_ps(_mm_mul_ps(v12, v12), v8)), _mm_mul_ps(v11, v12)), v12),
          v2);
  v14 = _mm_shuffle_ps(v13, v13, 18);
  v15 = _mm_shuffle_ps(v13, v13, 9);
  v16 = _mm_sub_ps(_mm_mul_ps(_mm_shuffle_ps(v30, v30, 9), v14), _mm_mul_ps(_mm_shuffle_ps(v30, v30, 18), v15));
  v17 = _mm_sub_ps(_mm_mul_ps(_mm_shuffle_ps(v16, v16, 18), v15), _mm_mul_ps(_mm_shuffle_ps(v16, v16, 9), v14));
  v18 = _mm_shuffle_ps(v16, v16, 0);
  v31 = _mm_shuffle_ps(v17, v17, 85);
  v19 = _mm_shuffle_ps(v13, v13, 170);
  v33 = _mm_add_ps(_mm_add_ps(v18, v31), v19);
  if ( !(v33.m128_f32[0] > v32) )
  {
    if ( !(v18.m128_f32[0] > v31.m128_f32[0]) || !(v18.m128_f32[0] > v19.m128_f32[0]) )
    {
      if ( !(v31.m128_f32[0] > v19.m128_f32[0]) )
      {
        v27 = _mm_sqrt_ps(_mm_add_ps(_mm_sub_ps(v19, _mm_add_ps(v18, v31)), v10));
        v28 = _mm_div_ps(v11, v27);
        *a2 = _mm_shuffle_ps(
                _mm_shuffle_ps(
                  _mm_mul_ps(_mm_add_ps(_mm_shuffle_ps(v16, v16, 170), _mm_shuffle_ps(v13, v13, 0)), v28),
                  _mm_mul_ps(_mm_add_ps(_mm_shuffle_ps(v17, v17, 170), _mm_shuffle_ps(v13, v13, 85)), v28),
                  16),
                _mm_shuffle_ps(
                  _mm_mul_ps(v27, v11),
                  _mm_mul_ps(_mm_sub_ps(_mm_shuffle_ps(v16, v16, 85), _mm_shuffle_ps(v17, v17, 0)), v28),
                  50),
                136);
      }
      else
      {
        v25 = _mm_sqrt_ps(_mm_add_ps(v10, _mm_sub_ps(v31, _mm_add_ps(v19, v18))));
        v26 = _mm_div_ps(v11, v25);
        *a2 = _mm_shuffle_ps(
                _mm_shuffle_ps(
                  _mm_mul_ps(_mm_add_ps(_mm_shuffle_ps(v17, v17, 0), _mm_shuffle_ps(v16, v16, 85)), v26),
                  _mm_mul_ps(v25, v11),
                  16),
                _mm_shuffle_ps(
                  _mm_mul_ps(_mm_add_ps(_mm_shuffle_ps(v17, v17, 170), _mm_shuffle_ps(v13, v13, 85)), v26),
                  _mm_mul_ps(_mm_sub_ps(_mm_shuffle_ps(v13, v13, 0), _mm_shuffle_ps(v16, v16, 170)), v26),
                  50),
                136);
      }
      return 1;
    }
    else
    {
      v23 = _mm_sqrt_ps(_mm_add_ps(v10, _mm_sub_ps(v18, _mm_add_ps(v19, v31))));
      v24 = _mm_div_ps(v11, v23);
      *a2 = _mm_shuffle_ps(
              _mm_shuffle_ps(
                _mm_mul_ps(v23, v11),
                _mm_mul_ps(_mm_add_ps(_mm_shuffle_ps(v17, v17, 0), _mm_shuffle_ps(v16, v16, 85)), v24),
                16),
              _mm_shuffle_ps(
                _mm_mul_ps(_mm_add_ps(_mm_shuffle_ps(v16, v16, 170), _mm_shuffle_ps(v13, v13, 0)), v24),
                _mm_mul_ps(_mm_sub_ps(_mm_shuffle_ps(v17, v17, 170), _mm_shuffle_ps(v13, v13, 85)), v24),
                50),
              136);
      return 1;
    }
  }
  else
  {
    v20 = _mm_sqrt_ps(_mm_add_ps(v10, v33));
    v21 = _mm_div_ps(v11, v20);
    *a2 = _mm_shuffle_ps(
            _mm_shuffle_ps(
              _mm_mul_ps(_mm_sub_ps(_mm_shuffle_ps(v17, v17, 170), _mm_shuffle_ps(v13, v13, 85)), v21),
              _mm_mul_ps(_mm_sub_ps(_mm_shuffle_ps(v13, v13, 0), _mm_shuffle_ps(v16, v16, 170)), v21),
              16),
            _mm_shuffle_ps(
              _mm_mul_ps(_mm_sub_ps(_mm_shuffle_ps(v16, v16, 85), _mm_shuffle_ps(v17, v17, 0)), v21),
              _mm_mul_ps(v20, v11),
              50),
            136);
    return 1;
  }
}


float* ToMatrix(float* a1, float* a2)
{
  float v2;
  float v3;
  float v4;
  float v6;
  float v7;
  float v8;
  float v9;
  float v10;
  float v11;

  v2 = *a2;
  v3 = a2[1];
  v4 = a2[2];
  v8 = a2[3];
  v6 = v3 * *a2;
  v11 = v4 * *a2;
  v10 = v8 * v2;
  v7 = v8 * v3;
  v9 = v8 * v4;
  *a1 = 1.0f - (float)((float)((float)(v4 * v4) + (float)(v3 * v3)) * 2.0f);
  a1[1] = (float)(v9 + v6) * 2.0f;
  a1[2] = (float)(v11 - v7) * 2.0f;
  a1[3] = (float)(v6 - v9) * 2.0f;
  a1[4] = 1.0f - (float)((float)((float)(v4 * v4) + (float)(v2 * v2)) * 2.0f);
  a1[5] = (float)(v10 + (float)(v4 * v3)) * 2.0f;
  a1[6] = (float)(v7 + v11) * 2.0f;
  a1[7] = (float)((float)(v4 * v3) - v10) * 2.0f;
  a1[8] = 1.0f - (float)((float)((float)(v3 * v3) + (float)(v2 * v2)) * 2.0f);
  return a1;
}


float* Multiply(float* a1, float* a2, float* a3)
{
  float v3;
  float v4;
  float v5;
  float v6;
  float v7;
  float v8;
  float v10;
  float v11;
  float v12;
  float v13;
  float v14;
  float v15;
  float v16;
  float v17;
  float v18;

  v3 = a3[1];
  v4 = a3[2];
  v5 = a2[1];
  v6 = a2[2];
  v7 = *a3;
  v8 = *a2;
  v16 = (float)(v4 * v5) - (float)(v3 * v6);
  v17 = (float)(*a3 * v6) - (float)(v4 * *a2);
  v10 = a3[3];
  v18 = v10 * v5;
  v11 = a2[3];
  v12 = (float)((float)(v11 * v4) + (float)(v10 * v6)) + (float)((float)(*a2 * v3) - (float)(*a3 * v5));
  v13 = v6;
  v14 = (float)((float)(v11 * *a3) + (float)(a3[3] * *a2)) + v16;
  a1[2] = v12;
  v15 = (float)(a2[3] * a3[3]) - (float)((float)((float)(v7 * v8) + (float)(v4 * v13)) + (float)(v3 * a2[1]));
  *a1 = v14;
  a1[1] = (float)((float)(v11 * v3) + v18) + v17;
  a1[3] = v15;
  return a1;
}


__m128* OwnerBasis(__m128* self, __m128* a2, __m128* a3)
{
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
  __m128 v23;
  __m128 v24;
  __m128 v25;
  __m128 v26;
  __m128 v27;
  __m128 v28;

  v4 = *a2;
  v5 = _mm_shuffle_ps(v4, v4, 18);
  v6 = _mm_shuffle_ps(v4, v4, 9);
  v7 = _mm_sub_ps(_mm_mul_ps(_mm_shuffle_ps(*a3, *a3, 9), v5), _mm_mul_ps(_mm_shuffle_ps(*a3, *a3, 18), v6));
  v8 = _mm_mul_ps(v7, v7);
  v24 = v6;
  v9 = _mm_add_ps(_mm_add_ps(_mm_shuffle_ps(v8, v8, 2), _mm_shuffle_ps(v8, v8, 1)), v8);
  v10 = _mm_shuffle_ps(v9, v9, 0);
  v11 = _mm_rsqrt_ps(v10);
  v12 = _mm_shuffle_ps(Bits64(0x3F800000ull), Bits64(0x3F800000ull), 0);
  v13 = _mm_shuffle_ps(Bits64(0x3F000000ull), Bits64(0x3F000000ull), 0);
  v14 = _mm_add_ps(_mm_mul_ps(_mm_sub_ps(v12, _mm_mul_ps(_mm_mul_ps(v11, v11), v10)), _mm_mul_ps(v13, v11)), v11);
  v15 = _mm_mul_ps(
          _mm_add_ps(_mm_mul_ps(_mm_sub_ps(v12, _mm_mul_ps(_mm_mul_ps(v14, v14), v10)), _mm_mul_ps(v13, v14)), v14),
          v7);
  v28 = _mm_shuffle_ps(v15, v15, 9);
  v16 = _mm_shuffle_ps(v15, v15, 18);
  v17 = _mm_sub_ps(_mm_mul_ps(v16, v24), _mm_mul_ps(v28, v5));
  v18 = _mm_mul_ps(v17, v17);
  v19 = _mm_add_ps(_mm_add_ps(_mm_shuffle_ps(v18, v18, 2), _mm_shuffle_ps(v18, v18, 1)), v18);
  v20 = _mm_shuffle_ps(v19, v19, 0);
  v21 = _mm_rsqrt_ps(v20);
  v22 = _mm_add_ps(_mm_mul_ps(_mm_sub_ps(v12, _mm_mul_ps(_mm_mul_ps(v21, v21), v20)), _mm_mul_ps(v13, v21)), v21);
  v25 = v15;
  v23 = _mm_mul_ps(
          _mm_add_ps(_mm_mul_ps(_mm_sub_ps(v12, _mm_mul_ps(_mm_mul_ps(v22, v22), v20)), _mm_mul_ps(v13, v22)), v22),
          v17);
  v25.m128_i32[3] = 0;
  *(self + 98) = v25;
  v26 = v23;
  v26.m128_i32[3] = 0;
  *(self + 99) = v26;
  v27 = _mm_sub_ps(_mm_mul_ps(_mm_shuffle_ps(v23, v23, 18), v28), _mm_mul_ps(_mm_shuffle_ps(v23, v23, 9), v16));
  v27.m128_i32[3] = 0;
  *(self + 100) = v27;
  return a3;
}


namespace {
// FSQRT stays in x87 until both the explicit root spill and the derived spill.
void RootPair(float a,float b,float c,int kind,float& root,float& derived) {
    const float half=0.5f;
#if defined(_M_IX86)
    float r,d;
    __asm { fld a }
    if(kind==0) { __asm { fmul half } }
    if(kind==2) {
        __asm { fld b }
        __asm { fadd c }
        __asm { fsubp st(1), st(0) }
        __asm { fld1 }
        __asm { faddp st(1), st(0) }
    }
    __asm { fsqrt }
    __asm { fst r }
    if(kind==0) {
        __asm { fld st(0) }
        __asm { faddp st(1), st(0) }
    } else if(kind==1) {
        __asm { fld1 }
        __asm { fdivrp st(1), st(0) }
    } else { __asm { fmul half } }
    __asm { fstp d }
    root=r; derived=d;
#else
    double r=std::sqrt(kind==0?double(a)*0.5f:kind==2?double(a)-(double(b)+c)+1.0f:double(a));
    root=static_cast<float>(r); derived=static_cast<float>(kind==0?r+r:kind==1?1.0f/r:r*0.5f);
#endif
}
}
__m128* Between(__m128* out,__m128* from,__m128* to) {
    __m128 a=*from,b=*to,p=_mm_mul_ps(b,a);
    __m128 sum=_mm_add_ps(_mm_add_ps(_mm_shuffle_ps(p,p,2),_mm_shuffle_ps(p,p,1)),p);
    float t=_mm_cvtss_f32(sum)+1.0f;
    float root,scale;
    if(!(t<0.00000011920928955078125f)) {
        RootPair(t,0,0,0,root,scale);
        __m128 cross=_mm_sub_ps(_mm_mul_ps(_mm_shuffle_ps(b,b,18),_mm_shuffle_ps(a,a,9)),_mm_mul_ps(_mm_shuffle_ps(b,b,9),_mm_shuffle_ps(a,a,18)));
        __m128 v=_mm_div_ps(cross,_mm_set1_ps(scale));
        // Every source needed after these stores is already in registers.
        alignas(16) float lanes[4]; _mm_store_ps(lanes,v); lanes[3]=root; *out=_mm_load_ps(lanes);
    } else {
        alignas(16) float v[4]; _mm_store_ps(v,a);
        if(!(std::fabs(v[0])>=0.6000000238418579f)) {
            RootPair(1.0f-v[0]*v[0],0,0,1,root,scale);
            *out=_mm_setr_ps(0.0f,v[2]*scale,(0.0f-v[1])*scale,0.0f);
        } else if(!(std::fabs(v[1])>=0.6000000238418579f)) {
            RootPair(1.0f-v[1]*v[1],0,0,1,root,scale);
            *out=_mm_setr_ps((0.0f-v[2])*scale,0.0f,v[0]*scale,0.0f);
        } else {
            RootPair(1.0f-v[2]*v[2],0,0,1,root,scale);
            *out=_mm_setr_ps(v[1]*scale,(0.0f-v[0])*scale,0.0f,0.0f);
        }
    }
    return out;
}
float* FromMatrix(float* out,float* m,float threshold) {
    float yy=m[4],zz=m[8],xx=m[0],sum=yy+zz,trace=xx+sum;
    float root,halfroot;
    if(trace>threshold) {
        // The addition is extended, not an SSE trace+1 spill.
        RootPair(trace,0.0f,0.0f,2,root,halfroot);
        float s=0.5f/root;
        out[0]=(m[5]-m[7])*s; float v=m[6];
        out[3]=halfroot; out[1]=(v-m[2])*s; out[2]=(m[1]-m[3])*s;
    } else if(xx>yy && xx>zz) {
        RootPair(xx,sum,0,2,root,halfroot); float s=0.5f/root;
        out[1]=(m[3]+m[1])*s; float v=m[6]; out[0]=halfroot;
        out[2]=(v+m[2])*s; out[3]=(m[5]-m[7])*s;
    } else if(yy>zz) {
        RootPair(yy,xx,zz,2,root,halfroot); float s=0.5f/root;
        out[0]=(m[3]+m[1])*s; float v=m[7]; out[1]=halfroot;
        out[2]=(v+m[5])*s; out[3]=(m[6]-m[2])*s;
    } else {
        RootPair(zz,xx,yy,2,root,halfroot); float s=0.5f/root;
        out[0]=(m[6]+m[2])*s; float v=m[7]; out[2]=halfroot;
        out[1]=(v+m[5])*s; out[3]=(m[1]-m[3])*s;
    }
    return out;
}
float* Angles(float* input,float* heading,float* elevation) {
    float y=input[1],x=input[0],xy=y*y+x*x;
    if(!(xy<0.000001f)) {
#if defined(_M_IX86)
        *heading=CallGameFunction<float,0x008E4700>(y,x);
        float z=input[2],value;
        float* residue;
        const uintptr_t asinAddress=0x00F7EDE4;
        __asm {
            fld z
            fmul st(0),st(0)
            fadd xy
            fsqrt
            fdivr z
            mov eax,heading
            call asinAddress
            mov residue,eax
            fstp value
        }
        *elevation=value;
        return residue;
#else
        *heading=std::atan2(y,x);
        *elevation=static_cast<float>(std::asin(double(input[2])/std::sqrt(double(input[2])*input[2]+xy)));
#endif
    } else {
        *heading=0.0f;
#if defined(_M_IX86)
        const float halfpi=*reinterpret_cast<const float*>(uintptr_t(0x0116AC38));
#else
        const float halfpi=1.5707963705062866f;
#endif
        *elevation=input[2]<0?(-0.0f-halfpi):halfpi;
    }
    return heading;
}
}
#pragma float_control(pop)
