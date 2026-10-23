#include "ts3/math/m04_B10.h"
#include <cmath>
#include <cstring>
#include <xmmintrin.h>
namespace ts3::math::b10 {
namespace {
__declspec(noinline) float Add(float a, float b) {
#if defined(_M_IX86)
    float r;
    __asm { movss xmm0,a }
    __asm { addss xmm0,b }
    __asm { movss r,xmm0 }
    return r;
#else
    // Preserve source-operand NaN selection even if the compiler commutes SSE.
    uint32_t bits; std::memcpy(&bits,&a,4);
    if ((bits&0x7fffffff)>0x7f800000) { bits|=0x400000; std::memcpy(&a,&bits,4); return a; }
    std::memcpy(&bits,&b,4);
    if ((bits&0x7fffffff)>0x7f800000) { bits|=0x400000; std::memcpy(&b,&bits,4); return b; }
    return _mm_cvtss_f32(_mm_add_ss(_mm_set_ss(a),_mm_set_ss(b)));
#endif
}
__declspec(noinline) float Sub(float a, float b) {
#if defined(_M_IX86)
    float r;
    __asm { movss xmm0,a }
    __asm { subss xmm0,b }
    __asm { movss r,xmm0 }
    return r;
#else
    // Preserve source-operand NaN selection even if the compiler commutes SSE.
    uint32_t bits; std::memcpy(&bits,&a,4);
    if ((bits&0x7fffffff)>0x7f800000) { bits|=0x400000; std::memcpy(&a,&bits,4); return a; }
    std::memcpy(&bits,&b,4);
    if ((bits&0x7fffffff)>0x7f800000) { bits|=0x400000; std::memcpy(&b,&bits,4); return b; }
    return _mm_cvtss_f32(_mm_sub_ss(_mm_set_ss(a),_mm_set_ss(b)));
#endif
}
__declspec(noinline) float Mul(float a, float b) {
#if defined(_M_IX86)
    float r;
    __asm { movss xmm0,a }
    __asm { mulss xmm0,b }
    __asm { movss r,xmm0 }
    return r;
#else
    // Preserve source-operand NaN selection even if the compiler commutes SSE.
    uint32_t bits; std::memcpy(&bits,&a,4);
    if ((bits&0x7fffffff)>0x7f800000) { bits|=0x400000; std::memcpy(&a,&bits,4); return a; }
    std::memcpy(&bits,&b,4);
    if ((bits&0x7fffffff)>0x7f800000) { bits|=0x400000; std::memcpy(&b,&bits,4); return b; }
    return _mm_cvtss_f32(_mm_mul_ss(_mm_set_ss(a),_mm_set_ss(b)));
#endif
}
__declspec(noinline) float Div(float a, float b) {
#if defined(_M_IX86)
    float r;
    __asm { movss xmm0,a }
    __asm { divss xmm0,b }
    __asm { movss r,xmm0 }
    return r;
#else
    // Preserve source-operand NaN selection even if the compiler commutes SSE.
    uint32_t bits; std::memcpy(&bits,&a,4);
    if ((bits&0x7fffffff)>0x7f800000) { bits|=0x400000; std::memcpy(&a,&bits,4); return a; }
    std::memcpy(&bits,&b,4);
    if ((bits&0x7fffffff)>0x7f800000) { bits|=0x400000; std::memcpy(&b,&bits,4); return b; }
    return _mm_cvtss_f32(_mm_div_ss(_mm_set_ss(a),_mm_set_ss(b)));
#endif
}
float X87Copy(float x) {
#if defined(_M_IX86)
    float y;
    __asm { fld x }
    __asm { fstp y }
    return y;
#else
    return Mul(x,1.0f);
#endif
}

// These accessors refer to the two packed 16-bit flags/revision fields.
uint16_t& Word(float* p, unsigned n=0) { return reinterpret_cast<uint16_t*>(p)[n]; }
unsigned char Byte(float* p) { return *reinterpret_cast<unsigned char*>(p); }
void SinCos(float x, float& s, float& c) {
#if defined(_M_IX86)
    float sv,cv;
    __asm { fld x }
    __asm { fld st(0) }
    __asm { fsin }
    __asm { fstp sv }
    __asm { fcos }
    __asm { fstp cv }
    s=sv; c=cv;
#else
    // FSIN/FCOS leave finite out-of-reduction-range operands unchanged.
    if (std::isfinite(x) && std::fabs(x)>=9223372036854775808.0f) { s=x; c=x; return; }
    s=static_cast<float>(std::sin(static_cast<double>(x)));
    c=static_cast<float>(std::cos(static_cast<double>(x)));
#endif
}
float ReciprocalNorm(float x, float y) {
#if defined(_M_IX86)
    float r;
    __asm {
        fld x
        fmul st(0), st(0)
        fld y
        fmul st(0), st(0)
        faddp st(1), st(0)
        fsqrt
        fld1
        fdivrp st(1), st(0)
        fstp r
    }
    return r;
#else
    return static_cast<float>(1.0/std::sqrt(double(x)*x+double(y)*y));
#endif
}
}
// TS3W.exe 0x0071ADE0
void RotateX(float* self, float a2)
{
  double v2; // st7
  float v3; // xmm4_4
  float v4; // xmm5_4
  float v5; // xmm6_4
  float v6; // [esp+0h] [ebp-1Ch]
  float v7; // [esp+4h] [ebp-18h]
  float v8; // [esp+8h] [ebp-14h]
  float v9; // [esp+Ch] [ebp-10h]
  float v10; // [esp+20h] [ebp+4h]


  v3 = *(self + 8);
  v4 = *(self + 9);
  v5 = *(self + 10);
  v8 = *(self + 12);
  v7 = *(self + 11);
  v9 = *(self + 13);
  SinCos(a2, v10, v6);
  *(self + 8) = (float)(v3 * v6) + (float)(v7 * v10);
  *(self + 9) = (float)(v4 * v6) + (float)(v8 * v10);
  *(self + 10) = (float)(v5 * v6) + (float)(v9 * v10);
  *(self + 11) = (float)(v3 * (float)(-0.0f - v10)) + (float)(v7 * v6);
  *(self + 12) = (float)(v4 * (float)(-0.0f - v10)) + (float)(v8 * v6);
  *(self + 13) = (float)(v5 * (float)(-0.0f - v10)) + (float)(v9 * v6);
  Word(self) |= 2u;
  ++Word(self,1);
}


// TS3W.exe 0x0071AEE0
float* Export44(float* self, float* a2)
{
  float v2; // xmm0_4
  float v3; // xmm2_4
  float v4; // xmm3_4
  float v6; // xmm0_4
  float v7; // xmm2_4
  float v8; // xmm3_4
  float v9; // xmm0_4
  float v10; // xmm2_4
  float v11; // xmm3_4

  v2 = *(self + 4);
  v3 = *(self + 6);
  v4 = *(self + 7);
  a2[0] = *(self + 5) * v2;
  a2[1] = v3 * v2;
  a2[2] = v4 * v2;
  v6 = *(self + 4);
  v7 = *(self + 9) * v6;
  v8 = *(self + 10) * v6;
  a2[4] = v6 * *(self + 8);
  a2[5] = v7;
  a2[6] = v8;
  v9 = *(self + 4);
  v10 = *(self + 12);
  v11 = *(self + 13);
  a2[8] = v9 * *(self + 11);
  a2[9] = v10 * v9;
  a2[10] = v11 * v9;
  a2[12] = *(self + 1);
  a2[13] = *(self + 2);
  a2[14] = *(self + 3);
  a2[3] = 0.0f;
  a2[7] = 0.0f;
  a2[11] = 0.0f;
  a2[15] = 1.0f;
  return a2;
}


// TS3W.exe 0x008C4440
uintptr_t InversePoint(float* self, float* a2, uintptr_t incomingEax)
{
    // Scalar SSE dataflow, with explicit source order and original reloads.
    uintptr_t residue=incomingEax;
    float x0=0,x1=0,x2=0,x3=0,x4=0,x5=0,x6=0,x7=0;
    float spill[20]{};
    bool equal=false; uint16_t flags=0; float copy=0;
    x1 = a2[1];
    x1 = Sub(x1, self[2]);
    x2 = a2[2];
    x2 = Sub(x2, self[3]);
    x0 = a2[0];
    x0 = Sub(x0, self[1]);
    a2[0] = x0;
    x0 = 1.0f;
    a2[1] = x1;
    a2[2] = x2;
    x3 = self[4];
    equal = (x3 == x0);
    const unsigned ah=std::isnan(x3)?0x47:(equal?0x42:(x3<x0?0x03:0x02));
    residue=(incomingEax & ~uintptr_t(0xff00)) | (uintptr_t(ah)<<8);
    if(equal) goto L008C44AA;
    x0 = Div(x0, x3);
    x3 = a2[0];
    x3 = Mul(x3, x0);
    a2[0] = x3;
    x3 = x0;
    x3 = Mul(x3, x1);
    x0 = Mul(x0, x2);
    a2[1] = x3;
    a2[2] = x0;
L008C44AA:;
    if(!(Byte(self)&2)) goto L008C4534;
    x4 = a2[2];
    x5 = a2[1];
    x1 = self[7];
    x2 = self[6];
    x0 = a2[0];
    x3 = self[9];
    x2 = Mul(x2, x5);
    x1 = Mul(x1, x4);
    x1 = Add(x1, x2);
    x2 = self[5];
    x2 = Mul(x2, x0);
    x1 = Add(x1, x2);
    x2 = self[10];
    x3 = Mul(x3, x5);
    x2 = Mul(x2, x4);
    x2 = Add(x2, x3);
    x3 = self[8];
    x3 = Mul(x3, x0);
    x0 = Mul(x0, self[11]);
    x2 = Add(x2, x3);
    x3 = self[13];
    x3 = Mul(x3, x4);
    x4 = self[12];
    x4 = Mul(x4, x5);
    x3 = Add(x3, x4);
    x3 = Add(x3, x0);
    a2[0] = x1;
    a2[1] = x2;
    a2[2] = x3;
L008C4534:;
    return residue;
}


// TS3W.exe 0x008C4540
int MatrixDifferent(float* a1, float* a2)
{
  return *a1 != *a2
      || a1[1] != a2[1]
      || a1[2] != a2[2]
      || a1[3] != a2[3]
      || a1[4] != a2[4]
      || a1[5] != a2[5]
      || a1[6] != a2[6]
      || a1[7] != a2[7]
      || a1[8] != a2[8];
}


// TS3W.exe 0x008C4CD0
float* TransformPoint(float* self, float* a2)
{
    // Scalar SSE dataflow, with explicit source order and original reloads.
    float x0=0,x1=0,x2=0,x3=0,x4=0,x5=0,x6=0,x7=0;
    float spill[20]{};
    bool equal=false; uint16_t flags=0; float copy=0;
    if(!(Byte(self)&2)) goto L008C4D62;
    x4 = a2[1];
    x3 = a2[2];
    x0 = self[8];
    x1 = self[11];
    x5 = a2[0];
    x2 = self[9];
    x1 = Mul(x1, x3);
    x0 = Mul(x0, x4);
    x0 = Add(x0, x1);
    x1 = self[5];
    x1 = Mul(x1, x5);
    x2 = Mul(x2, x4);
    x0 = Add(x0, x1);
    x1 = self[6];
    x1 = Mul(x1, x5);
    x1 = Add(x1, x2);
    x2 = self[12];
    x2 = Mul(x2, x3);
    x1 = Add(x1, x2);
    x2 = self[7];
    x2 = Mul(x2, x5);
    x5 = self[10];
    x5 = Mul(x5, x4);
    x4 = self[13];
    x2 = Add(x2, x5);
    x4 = Mul(x4, x3);
    x2 = Add(x2, x4);
    a2[0] = x0;
    a2[1] = x1;
    a2[2] = x2;
L008C4D62:;
    x2 = self[4];
    x0 = a2[1];
    x3 = a2[0];
    x0 = Mul(x0, x2);
    x1 = x2;
    x1 = Mul(x1, a2[2]);
    x3 = Mul(x3, x2);
    a2[1] = x0;
    a2[2] = x1;
    a2[0] = x3;
    x2 = self[2];
    x2 = Add(x2, x0);
    x0 = self[3];
    x0 = Add(x0, x1);
    x1 = self[1];
    x1 = Add(x1, x3);
    a2[0] = x1;
    a2[1] = x2;
    a2[2] = x0;
    return a2;
}


// TS3W.exe 0x008C7F90
void RotateY(float* self, float a2)
{
  double v2; // st7
  float v3; // xmm4_4
  float v4; // xmm5_4
  float v5; // xmm6_4
  float v6; // [esp+0h] [ebp-1Ch]
  float v7; // [esp+4h] [ebp-18h]
  float v8; // [esp+8h] [ebp-14h]
  float v9; // [esp+Ch] [ebp-10h]
  float v10; // [esp+20h] [ebp+4h]


  v3 = *(self + 11);
  v4 = *(self + 12);
  v5 = *(self + 13);
  v8 = *(self + 6);
  v7 = *(self + 5);
  v9 = *(self + 7);
  SinCos(a2, v10, v6);
  *(self + 11) = (float)(v3 * v6) + (float)(v7 * v10);
  *(self + 12) = (float)(v4 * v6) + (float)(v8 * v10);
  *(self + 13) = (float)(v5 * v6) + (float)(v9 * v10);
  *(self + 5) = (float)(v3 * (float)(-0.0f - v10)) + (float)(v7 * v6);
  *(self + 6) = (float)(v4 * (float)(-0.0f - v10)) + (float)(v8 * v6);
  *(self + 7) = (float)(v5 * (float)(-0.0f - v10)) + (float)(v9 * v6);
  Word(self) |= 2u;
  ++Word(self,1);
}


// TS3W.exe 0x008C8090
void RotateZ(float* self, float a2)
{
  double v2; // st7
  float v3; // xmm4_4
  float v4; // xmm5_4
  float v5; // xmm6_4
  float v6; // [esp+0h] [ebp-1Ch]
  float v7; // [esp+4h] [ebp-18h]
  float v8; // [esp+8h] [ebp-14h]
  float v9; // [esp+Ch] [ebp-10h]
  float v10; // [esp+20h] [ebp+4h]


  v3 = *(self + 5);
  v4 = *(self + 6);
  v5 = *(self + 7);
  v8 = *(self + 9);
  v7 = *(self + 8);
  v9 = *(self + 10);
  SinCos(a2, v10, v6);
  *(self + 5) = (float)(v3 * v6) + (float)(v7 * v10);
  *(self + 6) = (float)(v4 * v6) + (float)(v8 * v10);
  *(self + 7) = (float)(v5 * v6) + (float)(v9 * v10);
  *(self + 8) = (float)(v3 * (float)(-0.0f - v10)) + (float)(v7 * v6);
  *(self + 9) = (float)(v4 * (float)(-0.0f - v10)) + (float)(v8 * v6);
  *(self + 10) = (float)(v5 * (float)(-0.0f - v10)) + (float)(v9 * v6);
  Word(self) |= 2u;
  ++Word(self,1);
}


// TS3W.exe 0x008C8470
uintptr_t PostCompose(float* self, float* a2)
{
    // Scalar SSE dataflow, with explicit source order and original reloads.
    float x0=0,x1=0,x2=0,x3=0,x4=0,x5=0,x6=0,x7=0;
    float spill[20]{};
    bool equal=false; uint16_t flags=0; float copy=0;
    x3 = self[4];
    x0 = a2[1];
    x1 = a2[2];
    x2 = a2[3];
    x4 = self[8];
    x5 = self[9];
    x2 = Mul(x2, x3);
    x0 = Mul(x0, x3);
    x1 = Mul(x1, x3);
    x3 = self[11];
    x3 = Mul(x3, x2);
    x4 = Mul(x4, x1);
    x5 = Mul(x5, x1);
    x3 = Add(x3, x4);
    x4 = x0;
    x4 = Mul(x4, self[5]);
    x3 = Add(x3, x4);
    x4 = self[12];
    x4 = Mul(x4, x2);
    x4 = Add(x4, x5);
    x5 = self[6];
    x5 = Mul(x5, x0);
    x4 = Add(x4, x5);
    x5 = self[13];
    x5 = Mul(x5, x2);
    x2 = self[10];
    x2 = Mul(x2, x1);
    x1 = self[7];
    x1 = Mul(x1, x0);
    x0 = self[1];
    x5 = Add(x5, x2);
    x0 = Add(x0, x3);
    self[1] = x0;
    x0 = self[2];
    x5 = Add(x5, x1);
    x0 = Add(x0, x4);
    self[2] = x0;
    x0 = self[3];
    x0 = Add(x0, x5);
    self[3] = x0;
    x6 = a2[6];
    x7 = a2[5];
    x4 = self[8];
    x0 = self[5];
    x1 = a2[7];
    x3 = self[11];
    x5 = x0;
    x5 = Mul(x5, x7);
    x2 = x4;
    x2 = Mul(x2, x6);
    x5 = Add(x5, x2);
    x2 = x3;
    x2 = Mul(x2, x1);
    x1 = self[12];
    x5 = Add(x5, x2);
    spill[1] = x1;
    x1 = self[9];
    spill[0] = x1;
    x1 = self[6];
    x2 = x1;
    x2 = Mul(x2, x7);
    x7 = spill[0];
    x7 = Mul(x7, x6);
    x6 = a2[7];
    x2 = Add(x2, x7);
    x7 = spill[1];
    x7 = Mul(x7, x6);
    x6 = a2[5];
    x2 = Add(x2, x7);
    spill[5] = x2;
    x2 = self[13];
    spill[3] = x2;
    x2 = self[10];
    spill[2] = x2;
    x2 = self[7];
    x7 = x2;
    x7 = Mul(x7, x6);
    x6 = a2[6];
    spill[14] = x7;
    x7 = spill[2];
    x7 = Mul(x7, x6);
    x6 = spill[14];
    x6 = Add(x6, x7);
    x7 = spill[3];
    spill[14] = x6;
    x6 = a2[7];
    x7 = Mul(x7, x6);
    x6 = spill[14];
    x6 = Add(x6, x7);
    spill[6] = x6;
    x6 = a2[8];
    x7 = x0;
    x7 = Mul(x7, x6);
    x6 = a2[9];
    spill[14] = x7;
    x7 = x4;
    x7 = Mul(x7, x6);
    x6 = spill[14];
    x6 = Add(x6, x7);
    spill[14] = x6;
    x6 = a2[10];
    x7 = x3;
    x7 = Mul(x7, x6);
    x6 = spill[14];
    x6 = Add(x6, x7);
    spill[7] = x6;
    x6 = a2[8];
    x7 = x1;
    x7 = Mul(x7, x6);
    x6 = a2[9];
    spill[14] = x7;
    x7 = spill[0];
    x7 = Mul(x7, x6);
    x6 = spill[14];
    x6 = Add(x6, x7);
    x7 = spill[1];
    spill[14] = x6;
    x6 = a2[10];
    x7 = Mul(x7, x6);
    x6 = spill[14];
    x6 = Add(x6, x7);
    spill[8] = x6;
    x6 = a2[8];
    x7 = x2;
    x7 = Mul(x7, x6);
    x6 = a2[9];
    spill[14] = x7;
    x7 = spill[2];
    x7 = Mul(x7, x6);
    x6 = spill[14];
    x6 = Add(x6, x7);
    x7 = spill[3];
    spill[14] = x6;
    x6 = a2[10];
    x7 = Mul(x7, x6);
    x6 = spill[14];
    x6 = Add(x6, x7);
    x7 = a2[11];
    x0 = Mul(x0, x7);
    x7 = a2[12];
    x4 = Mul(x4, x7);
    x0 = Add(x0, x4);
    x4 = a2[13];
    x3 = Mul(x3, x4);
    x0 = Add(x0, x3);
    x3 = spill[0];
    x3 = Mul(x3, x7);
    spill[10] = x0;
    x0 = a2[11];
    x1 = Mul(x1, x0);
    x1 = Add(x1, x3);
    x3 = spill[1];
    x3 = Mul(x3, x4);
    x1 = Add(x1, x3);
    x3 = spill[2];
    x3 = Mul(x3, x7);
    x2 = Mul(x2, x0);
    x2 = Add(x2, x3);
    x3 = spill[3];
    x3 = Mul(x3, x4);
    x2 = Add(x2, x3);
    x0 = spill[5];
    self[6] = x0;
    x0 = spill[6];
    self[7] = x0;
    x0 = spill[7];
    self[5] = x5;
    self[8] = x0;
    x0 = spill[8];
    self[9] = x0;
    x0 = spill[10];
    self[10] = x6;
    self[11] = x0;
    self[12] = x1;
    self[13] = x2;
    x0 = a2[4];
    x0 = Mul(x0, self[4]);
    self[4] = x0;
    flags = Word(a2);
    Word(self) |= flags;
    ++Word(self,1);
    return (reinterpret_cast<uintptr_t>(a2)&~uintptr_t(0xffff)) | flags;
}


// TS3W.exe 0x008CF5A0
uintptr_t PreCompose(float* self, float* a2)
{
    // Scalar SSE dataflow, with explicit source order and original reloads.
    float x0=0,x1=0,x2=0,x3=0,x4=0,x5=0,x6=0,x7=0;
    float spill[20]{};
    bool equal=false; uint16_t flags=0; float copy=0;
    x0 = a2[4];
    x1 = x0;
    x1 = Mul(x1, self[2]);
    self[2] = x1;
    x4 = x1;
    x2 = x0;
    x2 = Mul(x2, self[3]);
    x0 = Mul(x0, self[1]);
    self[3] = x2;
    self[1] = x0;
    x1 = a2[11];
    x3 = x2;
    x2 = a2[9];
    x2 = Mul(x2, x4);
    x1 = Mul(x1, x3);
    x5 = x0;
    x0 = a2[8];
    x0 = Mul(x0, x4);
    x0 = Add(x0, x1);
    x1 = x5;
    x1 = Mul(x1, a2[5]);
    x0 = Add(x0, x1);
    x1 = a2[6];
    x1 = Mul(x1, x5);
    x1 = Add(x1, x2);
    x2 = a2[12];
    x2 = Mul(x2, x3);
    x1 = Add(x1, x2);
    x2 = a2[7];
    x2 = Mul(x2, x5);
    x5 = a2[10];
    x5 = Mul(x5, x4);
    x4 = a2[13];
    self[1] = x0;
    self[2] = x1;
    x2 = Add(x2, x5);
    x4 = Mul(x4, x3);
    x2 = Add(x2, x4);
    self[3] = x2;
    x0 = a2[2];
    x0 = Add(x0, x1);
    x1 = a2[3];
    x1 = Add(x1, x2);
    x2 = a2[1];
    x2 = Add(x2, self[1]);
    self[1] = x2;
    self[2] = x0;
    self[3] = x1;
    x6 = self[6];
    x7 = self[5];
    x4 = a2[8];
    x0 = a2[5];
    x1 = self[7];
    x3 = a2[11];
    x5 = x0;
    x5 = Mul(x5, x7);
    x2 = x4;
    x2 = Mul(x2, x6);
    x5 = Add(x5, x2);
    x2 = x3;
    x2 = Mul(x2, x1);
    x1 = a2[12];
    x5 = Add(x5, x2);
    spill[1] = x1;
    x1 = a2[9];
    spill[0] = x1;
    x1 = a2[6];
    x2 = x1;
    x2 = Mul(x2, x7);
    x7 = spill[0];
    x7 = Mul(x7, x6);
    x6 = self[7];
    x2 = Add(x2, x7);
    x7 = spill[1];
    x7 = Mul(x7, x6);
    x2 = Add(x2, x7);
    spill[5] = x2;
    x2 = a2[13];
    spill[3] = x2;
    x2 = a2[10];
    x6 = self[5];
    spill[2] = x2;
    x2 = a2[7];
    x7 = x2;
    x7 = Mul(x7, x6);
    x6 = self[6];
    spill[14] = x7;
    x7 = spill[2];
    x7 = Mul(x7, x6);
    x6 = spill[14];
    x6 = Add(x6, x7);
    x7 = spill[3];
    spill[14] = x6;
    x6 = self[7];
    x7 = Mul(x7, x6);
    x6 = spill[14];
    x6 = Add(x6, x7);
    spill[6] = x6;
    x6 = self[8];
    x7 = x0;
    x7 = Mul(x7, x6);
    x6 = self[9];
    spill[14] = x7;
    x7 = x4;
    x7 = Mul(x7, x6);
    x6 = spill[14];
    x6 = Add(x6, x7);
    spill[14] = x6;
    x6 = self[10];
    x7 = x3;
    x7 = Mul(x7, x6);
    x6 = spill[14];
    x6 = Add(x6, x7);
    spill[7] = x6;
    x6 = self[8];
    x7 = x1;
    x7 = Mul(x7, x6);
    x6 = self[9];
    spill[14] = x7;
    x7 = spill[0];
    x7 = Mul(x7, x6);
    x6 = spill[14];
    x6 = Add(x6, x7);
    x7 = spill[1];
    spill[14] = x6;
    x6 = self[10];
    x7 = Mul(x7, x6);
    x6 = spill[14];
    x6 = Add(x6, x7);
    spill[8] = x6;
    x6 = self[8];
    x7 = x2;
    x7 = Mul(x7, x6);
    x6 = self[9];
    spill[14] = x7;
    x7 = spill[2];
    x7 = Mul(x7, x6);
    x6 = spill[14];
    x6 = Add(x6, x7);
    x7 = spill[3];
    spill[14] = x6;
    x6 = self[10];
    x7 = Mul(x7, x6);
    x6 = spill[14];
    x6 = Add(x6, x7);
    x7 = self[11];
    x0 = Mul(x0, x7);
    x7 = self[12];
    x4 = Mul(x4, x7);
    x0 = Add(x0, x4);
    x4 = self[13];
    x3 = Mul(x3, x4);
    x0 = Add(x0, x3);
    x3 = spill[0];
    spill[10] = x0;
    x0 = self[11];
    x3 = Mul(x3, x7);
    x1 = Mul(x1, x0);
    x1 = Add(x1, x3);
    x3 = spill[1];
    x3 = Mul(x3, x4);
    x2 = Mul(x2, x0);
    x0 = spill[5];
    self[6] = x0;
    x0 = spill[6];
    self[7] = x0;
    x0 = spill[7];
    self[5] = x5;
    self[8] = x0;
    x0 = spill[8];
    x1 = Add(x1, x3);
    x3 = spill[2];
    self[9] = x0;
    x0 = spill[10];
    self[10] = x6;
    x3 = Mul(x3, x7);
    x2 = Add(x2, x3);
    x3 = spill[3];
    self[11] = x0;
    self[12] = x1;
    x3 = Mul(x3, x4);
    x2 = Add(x2, x3);
    self[13] = x2;
    x0 = self[4];
    x0 = Mul(x0, a2[4]);
    self[4] = x0;
    flags = Word(a2);
    Word(self) |= flags;
    ++Word(self,1);
    return (reinterpret_cast<uintptr_t>(a2)&~uintptr_t(0xffff)) | flags;
}


// TS3W.exe 0x008D9C40
float* InverseVector(float* self, float* a2)
{
    // Scalar SSE dataflow, with explicit source order and original reloads.
    float x0=0,x1=0,x2=0,x3=0,x4=0,x5=0,x6=0,x7=0;
    float spill[20]{};
    bool equal=false; uint16_t flags=0; float copy=0;
    x1 = self[4];
    x0 = 1.0f;
    equal = (x1 == x0);
    if(equal) goto L008D9C81;
    x0 = Div(x0, x1);
    x1 = a2[0];
    x1 = Mul(x1, x0);
    a2[0] = x1;
    x1 = x0;
    x1 = Mul(x1, a2[1]);
    x0 = Mul(x0, a2[2]);
    a2[1] = x1;
    a2[2] = x0;
L008D9C81:;
    if(!(Byte(self)&2)) goto L008D9D0F;
    x3 = a2[2];
    x4 = a2[1];
    x0 = self[7];
    x1 = self[6];
    x5 = a2[0];
    x2 = self[9];
    x1 = Mul(x1, x4);
    x0 = Mul(x0, x3);
    x0 = Add(x0, x1);
    x1 = self[5];
    x1 = Mul(x1, x5);
    x0 = Add(x0, x1);
    x1 = self[10];
    x2 = Mul(x2, x4);
    x1 = Mul(x1, x3);
    x1 = Add(x1, x2);
    x2 = self[8];
    x2 = Mul(x2, x5);
    x1 = Add(x1, x2);
    x2 = self[13];
    x2 = Mul(x2, x3);
    x3 = self[12];
    x3 = Mul(x3, x4);
    x2 = Add(x2, x3);
    x3 = self[11];
    x3 = Mul(x3, x5);
    x2 = Add(x2, x3);
    a2[0] = x0;
    a2[1] = x1;
    a2[2] = x2;
L008D9D0F:;
    return a2;
}


// TS3W.exe 0x008DA360
float* TransformVector(float* self, float* a2)
{
    // Scalar SSE dataflow, with explicit source order and original reloads.
    float x0=0,x1=0,x2=0,x3=0,x4=0,x5=0,x6=0,x7=0;
    float spill[20]{};
    bool equal=false; uint16_t flags=0; float copy=0;
    if(!(Byte(self)&2)) goto L008DA3F2;
    x4 = a2[1];
    x3 = a2[2];
    x0 = self[8];
    x1 = self[11];
    x5 = a2[0];
    x2 = self[9];
    x1 = Mul(x1, x3);
    x0 = Mul(x0, x4);
    x0 = Add(x0, x1);
    x1 = self[5];
    x1 = Mul(x1, x5);
    x2 = Mul(x2, x4);
    x0 = Add(x0, x1);
    x1 = self[6];
    x1 = Mul(x1, x5);
    x1 = Add(x1, x2);
    x2 = self[12];
    x2 = Mul(x2, x3);
    x1 = Add(x1, x2);
    x2 = self[7];
    x2 = Mul(x2, x5);
    x5 = self[10];
    x5 = Mul(x5, x4);
    x4 = self[13];
    x2 = Add(x2, x5);
    x4 = Mul(x4, x3);
    x2 = Add(x2, x4);
    a2[0] = x0;
    a2[1] = x1;
    a2[2] = x2;
L008DA3F2:;
    x0 = self[4];
    x1 = a2[1];
    x3 = a2[0];
    x2 = x0;
    x2 = Mul(x2, a2[2]);
    x1 = Mul(x1, x0);
    x3 = Mul(x3, x0);
    a2[0] = x3;
    a2[1] = x1;
    a2[2] = x2;
    return a2;
}


// TS3W.exe 0x008E4FB0
uintptr_t TransposeVector(float* self, float* a2, uintptr_t incomingEax)
{
    // Scalar SSE dataflow, with explicit source order and original reloads.
    uintptr_t residue=incomingEax;
    float x0=0,x1=0,x2=0,x3=0,x4=0,x5=0,x6=0,x7=0;
    float spill[20]{};
    bool equal=false; uint16_t flags=0; float copy=0;
    if(!(Byte(self)&2)) goto L008E503E;
    residue=reinterpret_cast<uintptr_t>(a2);
    x5 = a2[1];
    x4 = a2[2];
    x1 = self[6];
    x2 = self[7];
    x0 = a2[0];
    x3 = self[10];
    x2 = Mul(x2, x4);
    x1 = Mul(x1, x5);
    x1 = Add(x1, x2);
    x2 = self[5];
    x2 = Mul(x2, x0);
    x1 = Add(x1, x2);
    x2 = self[9];
    x3 = Mul(x3, x4);
    x2 = Mul(x2, x5);
    x2 = Add(x2, x3);
    x3 = self[8];
    x3 = Mul(x3, x0);
    x0 = Mul(x0, self[11]);
    x2 = Add(x2, x3);
    x3 = self[12];
    x3 = Mul(x3, x5);
    x5 = self[13];
    x5 = Mul(x5, x4);
    x3 = Add(x3, x5);
    x3 = Add(x3, x0);
    a2[0] = x1;
    a2[1] = x2;
    a2[2] = x3;
L008E503E:;
    return residue;
}


// TS3W.exe 0x008F6630
float* AppendMatrix(float* self, float* a2)
{
    // Scalar SSE dataflow, with explicit source order and original reloads.
    float x0=0,x1=0,x2=0,x3=0,x4=0,x5=0,x6=0,x7=0;
    float spill[20]{};
    bool equal=false; uint16_t flags=0; float copy=0;
    if(!(Byte(self)&2)) goto L008F68BA;
    x6 = a2[1];
    x3 = self[8];
    x0 = a2[0];
    x4 = a2[2];
    x1 = self[5];
    x5 = self[11];
    x7 = x6;
    x2 = x0;
    x7 = Mul(x7, x3);
    spill[2] = x3;
    x3 = x4;
    x3 = Mul(x3, x5);
    x2 = Mul(x2, x1);
    x2 = Add(x2, x7);
    x7 = self[9];
    x2 = Add(x2, x3);
    x3 = x0;
    spill[6] = x2;
    x2 = self[12];
    spill[0] = x2;
    x2 = self[6];
    x3 = Mul(x3, x2);
    spill[16] = x3;
    x3 = x6;
    x3 = Mul(x3, x7);
    spill[3] = x7;
    x7 = spill[16];
    x7 = Add(x7, x3);
    x3 = x4;
    x3 = Mul(x3, spill[0]);
    spill[16] = x7;
    x7 = Add(x7, x3);
    x3 = self[13];
    spill[1] = x3;
    x4 = Mul(x4, spill[1]);
    x3 = self[7];
    x0 = Mul(x0, x3);
    spill[7] = x7;
    x7 = self[10];
    x6 = Mul(x6, x7);
    x0 = Add(x0, x6);
    x6 = a2[3];
    x0 = Add(x0, x4);
    spill[8] = x0;
    x0 = a2[4];
    x4 = x1;
    x4 = Mul(x4, x6);
    x6 = spill[2];
    x6 = Mul(x6, x0);
    x0 = a2[5];
    x4 = Add(x4, x6);
    x6 = x5;
    x6 = Mul(x6, x0);
    x0 = a2[3];
    x4 = Add(x4, x6);
    x6 = x2;
    x6 = Mul(x6, x0);
    x0 = a2[4];
    spill[16] = x6;
    x6 = spill[3];
    x6 = Mul(x6, x0);
    x0 = spill[16];
    x0 = Add(x0, x6);
    x6 = spill[0];
    spill[16] = x0;
    x0 = a2[5];
    x6 = Mul(x6, x0);
    x0 = spill[16];
    x0 = Add(x0, x6);
    spill[10] = x0;
    x0 = a2[3];
    x6 = x3;
    x6 = Mul(x6, x0);
    x0 = a2[4];
    spill[16] = x6;
    x6 = x7;
    x6 = Mul(x6, x0);
    x0 = spill[16];
    x0 = Add(x0, x6);
    x6 = spill[1];
    spill[16] = x0;
    x0 = a2[5];
    x6 = Mul(x6, x0);
    x0 = spill[16];
    x0 = Add(x0, x6);
    x6 = a2[7];
    spill[11] = x0;
    x0 = a2[8];
    spill[16] = x0;
    x0 = a2[6];
    x1 = Mul(x1, x0);
    spill[4] = x0;
    x0 = spill[2];
    x0 = Mul(x0, x6);
    x1 = Add(x1, x0);
    x0 = spill[3];
    spill[5] = x6;
    x6 = spill[16];
    x5 = Mul(x5, x6);
    x1 = Add(x1, x5);
    x5 = spill[4];
    spill[12] = x1;
    x1 = spill[5];
    x0 = Mul(x0, x1);
    x2 = Mul(x2, x5);
    x2 = Add(x2, x0);
    x0 = spill[0];
    x0 = Mul(x0, x6);
    x2 = Add(x2, x0);
    x0 = spill[1];
    x0 = Mul(x0, x6);
    x3 = Mul(x3, x5);
    x7 = Mul(x7, x1);
    x3 = Add(x3, x7);
    x3 = Add(x3, x0);
    x0 = spill[6];
    self[5] = x0;
    x0 = spill[7];
    self[6] = x0;
    x0 = spill[8];
    self[7] = x0;
    x0 = spill[10];
    self[9] = x0;
    x0 = spill[11];
    self[10] = x0;
    x0 = spill[12];
    self[8] = x4;
    self[11] = x0;
    self[12] = x2;
    self[13] = x3;
    ++Word(self,1);
    return a2;
L008F68BA:;
    copy = X87Copy(a2[0]);
    self[5] = copy;
    copy = X87Copy(a2[1]);
    self[6] = copy;
    copy = X87Copy(a2[2]);
    self[7] = copy;
    copy = X87Copy(a2[3]);
    self[8] = copy;
    copy = X87Copy(a2[4]);
    self[9] = copy;
    copy = X87Copy(a2[5]);
    self[10] = copy;
    copy = X87Copy(a2[6]);
    self[11] = copy;
    copy = X87Copy(a2[7]);
    self[12] = copy;
    copy = X87Copy(a2[8]);
    self[13] = copy;
    Word(self) |= 2;
    ++Word(self,1);
    return a2;
}


// TS3W.exe 0x0091C4B0
float* MovePivot(float* self, float* a2)
{
    // Scalar SSE dataflow, with explicit source order and original reloads.
    float x0=0,x1=0,x2=0,x3=0,x4=0,x5=0,x6=0,x7=0;
    float spill[20]{};
    bool equal=false; uint16_t flags=0; float copy=0;
    x3 = self[4];
    x6 = a2[0];
    x7 = a2[1];
    x2 = a2[2];
    x4 = self[8];
    x5 = self[6];
    spill[1] = x2;
    x2 = Mul(x2, x3);
    x0 = x6;
    x0 = Mul(x0, x3);
    x1 = x7;
    x1 = Mul(x1, x3);
    x3 = self[11];
    x4 = Mul(x4, x1);
    x5 = Mul(x5, x0);
    x3 = Mul(x3, x2);
    x3 = Add(x3, x4);
    x4 = self[5];
    x4 = Mul(x4, x0);
    x3 = Add(x3, x4);
    x4 = self[12];
    x4 = Mul(x4, x2);
    x4 = Add(x4, x5);
    x5 = self[9];
    x5 = Mul(x5, x1);
    x4 = Add(x4, x5);
    x5 = self[13];
    x5 = Mul(x5, x2);
    x2 = self[7];
    x2 = Mul(x2, x0);
    x0 = self[10];
    x0 = Mul(x0, x1);
    x1 = self[1];
    x6 = Sub(x6, x3);
    x1 = Add(x1, x6);
    self[1] = x1;
    x1 = self[2];
    x5 = Add(x5, x2);
    x5 = Add(x5, x0);
    x0 = spill[1];
    x7 = Sub(x7, x4);
    x1 = Add(x1, x7);
    self[2] = x1;
    x1 = self[3];
    x0 = Sub(x0, x5);
    x1 = Add(x1, x0);
    self[3] = x1;
    Word(self) |= 4;
    ++Word(self,1);
    return a2;
}


// TS3W.exe 0x0092C430
void BasisFromAxis(float* a1, float* a2)
{
    float x0=0,x1=0,x2=0,x3=0,x4=0,x5=0,x6=0,x7=0;
    float spill[6]{};
    a2[6]=X87Copy(a1[0]); a2[7]=X87Copy(a1[1]); a2[8]=X87Copy(a1[2]);
    spill[5]=a1[0]; spill[4]=a1[1];
    if(!(std::fabs(a1[0])>std::fabs(a1[1]))) goto L0092C527;
    x3 = -0.0f;
    x1 = spill[5];
    x0 = x3;
    x0 = Sub(x0,a1[2]);
    x2 = x3;
    x2 = Sub(x2,x0);
    x0 = x3;
    x0 = Sub(x0,x1);
    spill[2] = x0;
    spill[0] = x2;
    x5 = a1[1];
    x6 = x5;
    spill[5] = ReciprocalNorm(spill[2],spill[0]);
    x4 = spill[5];
    x1 = x4;
    x1 = Mul(x1,x2);
    x2 = x4;
    x0 = Mul(x0,x4);
    x2 = Mul(x2,x3);
    x3 = a1[2];
    x4 = x3;
    x4 = Mul(x4,x2);
    x6 = Mul(x6,x0);
    x6 = Sub(x6,x4);
    x4 = a1[0];
    x7 = x4;
    x3 = Mul(x3,x1);
    x7 = Mul(x7,x0);
    x4 = Mul(x4,x2);
    x5 = Mul(x5,x1);
    a2[3] = x6;
    x3 = Sub(x3,x7);
    a2[4] = x3;
    x4 = Sub(x4,x5);
    a2[5] = x4;
    a2[0] = x1;
    a2[1] = x2;
    a2[2] = x0;
    return;
L0092C527:;
    x2 = a1[2];
    x0 = -0.0f;
    x0 = Sub(x0,spill[4]);
    spill[1] = x2;
    spill[2] = x0;
    spill[5] = ReciprocalNorm(spill[2],spill[1]);
    x3 = spill[5];
    x2 = Mul(x2,x3);
    x7 = x2;
    x2 = a1[1];
    x0 = Mul(x0,x3);
    x1 = x3;
    x3 = a1[2];
    x1 = Mul(x1,0.0f);
    x5 = x2;
    x4 = x3;
    x5 = Mul(x5,x0);
    x4 = Mul(x4,x7);
    x4 = Sub(x4,x5);
    x5 = a1[0];
    x6 = x5;
    x6 = Mul(x6,x0);
    x3 = Mul(x3,x1);
    x2 = Mul(x2,x1);
    x5 = Mul(x5,x7);
    a2[0] = x4;
    x6 = Sub(x6,x3);
    a2[1] = x6;
    x2 = Sub(x2,x5);
    a2[2] = x2;
    a2[3] = x1;
    a2[4] = x7;
    a2[5] = x0;
    return;
}


#if defined(_M_IX86)
void __fastcall RotateXEntry(float* self, void*, float a2) { RotateX(self,a2); }
float* __fastcall Export44Entry(float* self, void*, float* a2) { return Export44(self,a2); }
// Capture the otherwise unexpressible incoming-EAX component of the return.
__declspec(naked) uintptr_t __fastcall InversePointEntry(float*, void*, float*) {
    __asm {
        push eax
        push dword ptr [esp+8]
        push ecx
        call InversePoint
        add esp,12
        ret 4
    }
}
float* __fastcall TransformPointEntry(float* self, void*, float* a2) { return TransformPoint(self,a2); }
void __fastcall RotateYEntry(float* self, void*, float a2) { RotateY(self,a2); }
void __fastcall RotateZEntry(float* self, void*, float a2) { RotateZ(self,a2); }
uintptr_t __fastcall PostComposeEntry(float* self, void*, float* a2) { return PostCompose(self,a2); }
uintptr_t __fastcall PreComposeEntry(float* self, void*, float* a2) { return PreCompose(self,a2); }
float* __fastcall InverseVectorEntry(float* self, void*, float* a2) { return InverseVector(self,a2); }
float* __fastcall TransformVectorEntry(float* self, void*, float* a2) { return TransformVector(self,a2); }
// Capture the otherwise unexpressible incoming-EAX component of the return.
__declspec(naked) uintptr_t __fastcall TransposeVectorEntry(float*, void*, float*) {
    __asm {
        push eax
        push dword ptr [esp+8]
        push ecx
        call TransposeVector
        add esp,12
        ret 4
    }
}
float* __fastcall AppendMatrixEntry(float* self, void*, float* a2) { return AppendMatrix(self,a2); }
float* __fastcall MovePivotEntry(float* self, void*, float* a2) { return MovePivot(self,a2); }
#endif
}
