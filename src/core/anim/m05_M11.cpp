#include "ts3/anim/m05_M11.h"
#include "ts3/math/m04_B11.h"
#include <cstring>
#include <cmath>
#include <emmintrin.h>
#pragma float_control(precise, on, push)
namespace ts3::anim::m11 {
namespace {
void Add(Cursor* s,uint32_t n) { s->position=static_cast<int32_t>(static_cast<uint32_t>(s->position)+n); }
uintptr_t Table() {
#if defined(_M_IX86)
    return 0x00fc138c;
#else
    static void* table[11]{}; return reinterpret_cast<uintptr_t>(table);
#endif
}
// Keep legacy two-operand SSE NaN precedence.
__declspec(noinline) __m128 Mul(__m128 a,__m128 b) { return _mm_mul_ps(a,b); }
__declspec(noinline) __m128 Plus(__m128 a,__m128 b) { return _mm_add_ps(a,b); }
uint32_t Bits(float f) { uint32_t u; std::memcpy(&u,&f,4); return u; }
float Float(uint32_t u) { float f; std::memcpy(&f,&u,4); return f; }
// Do not inline: _CIacos has volatile XMM registers invisible to inline-asm analysis.
__declspec(noinline) float Trig(float f,int op) {
#if defined(_M_IX86)
    float result;
    if(op==0) {
        const uintptr_t target=0x00f7edde; // _CIacos, x87 operand ABI
        __asm { fld f
            call target
            fstp result }
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
    return static_cast<float>(op==0?std::acos(double(f)):op==1?std::sin(double(f)):std::cos(double(f)));
#endif
}
}
void* __fastcall Query(void* self,void*,uint32_t id) {
    if(id==0x73deb8c8u || id==0x87eb0331u) return self;
    return nullptr;
}
Evaluator* __fastcall Construct(Evaluator* self,void*,void* host) {
    b01::Construct(&self->base,host);
    *reinterpret_cast<uintptr_t*>(self)=Table();
    std::memset(self->direction,0,16);
    self->key[0]=self->key[1]=0;
    self->index=0xffffffff;
    return self;
}
Evaluator* __fastcall Destroy(Evaluator* self,void*,uint32_t flags) {
    *reinterpret_cast<uintptr_t*>(self)=Table();
    b01::Reset(&self->base);
    if(flags&1u) CallGameFunction<void,0x004e3f70>(self);
    return self;
}
uintptr_t __fastcall Save(Evaluator* self,void*,Cursor* s) {
    uintptr_t eax=0;
    const bool refill=s->position>=-12;
    if(refill) eax=CallGameMethod<uintptr_t,0x00594140>(s,0);
    if(refill && !(eax&255)) s->writeError=1;
    else {
        auto* out=s->output+s->position+1; eax=reinterpret_cast<uintptr_t>(out);
        std::memcpy(out,self->direction,12); Add(s,12);
    }
    // Capture key before the refill callback, but index after it.
    uint32_t key[2]={self->key[0],self->key[1]};
    Add(s,8);
    if(s->position>=0) eax=CallGameMethod<uintptr_t,0x00594140>(s,8);
    if(s->position<0) {
        eax=reinterpret_cast<uintptr_t>(s->output);
        std::memcpy(s->output+s->position-7,key,8);
    }
    const uint32_t index=self->index;
    Add(s,4);
    if(s->position>=0) eax=CallGameMethod<uintptr_t,0x00594140>(s,4);
    if(s->position<0) {
        eax=reinterpret_cast<uintptr_t>(s->output);
        std::memcpy(s->output+s->position-3,&index,4);
    }
    return eax;
}
uintptr_t __fastcall Load(Evaluator* self,void*,Cursor* s) {
    uintptr_t eax=0;
    const bool refill=s->position>=-12;
    if(refill) eax=CallGameMethod<uintptr_t,0x00593ff0>(s,0);
    if(refill && !(eax&255)) s->readError=1;
    else {
        const auto* in=s->input+s->position+1; eax=reinterpret_cast<uintptr_t>(in);
        std::memcpy(self->direction,in,12); Add(s,12);
    }
    Add(s,8);
    if(s->position>=0) eax=CallGameMethod<uintptr_t,0x00593ff0>(s,8);
    if(s->position<0) {
        std::memcpy(self->key,s->input+s->position-7,8); eax=self->key[1];
    }
    Add(s,4);
    if(s->position>=0) eax=CallGameMethod<uintptr_t,0x00593ff0>(s,4);
    if(s->position<0) { std::memcpy(&self->index,s->input+s->position-3,4); eax=self->index; }
    return eax;
}
__m128 *__cdecl Aim(__m128 *a1, __m128 *a2, __m128 *a3, __m128 *a4)
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
  __m128 v29;
  __m128 v30;
  __m128 v31;
  __m128 v32;
  __m128 v33;
  __m128 v34;
  float v35;
  __m128 v36;
  __m128 v37;
  __m128 v38;
  __m128 v39;
  __m128 v40;
  __m128 v42;
  __m128 v43;
  __m128 v44;
  __m128 v45;
  __m128 v46;
  int v47; // [esp+8h] [ebp-F8h]
  unsigned int v48; // [esp+8h] [ebp-F8h]
  unsigned int v49; // [esp+8h] [ebp-F8h]
  unsigned int v50; // [esp+Ch] [ebp-F4h]
  __m128 v51; // [esp+10h] [ebp-F0h]
  __m128 v52; // [esp+10h] [ebp-F0h]
  float v53; // [esp+10h] [ebp-F0h]
  __m128 v54; // [esp+20h] [ebp-E0h] BYREF
  __m128 v55; // [esp+30h] [ebp-D0h]
  __m128 v56; // [esp+40h] [ebp-C0h]
  __m128 v57; // [esp+50h] [ebp-B0h]
  __m128 v58; // [esp+60h] [ebp-A0h] BYREF
  __m128 v59; // [esp+70h] [ebp-90h] BYREF
  float v60; // [esp+80h] [ebp-80h]
  __m128 v61; // [esp+90h] [ebp-70h] BYREF
  __m128 v62; // [esp+A0h] [ebp-60h]
  __m128 v63; // [esp+B0h] [ebp-50h]
  __m128 v64; // [esp+C0h] [ebp-40h]
  alignas(16) __m128 v65[3]; // [esp+D0h] [ebp-30h] BYREF

  float limit;
#if defined(_M_IX86)
  auto& flags=*reinterpret_cast<uint32_t*>(0x011cd418);
  auto& value=*reinterpret_cast<float*>(0x011cd414);
  if(!(flags&1)) { flags|=1; value=Float(1057360530); }
  limit=value;
#else
  limit=Float(1057360530);
#endif
  math::b11::InvertPose(a3, v65);
  v4 = _mm_sub_ps(*a2, *a3);
  v5 = Mul(v4, v4);
  v6 = Plus(Plus(_mm_shuffle_ps(v5, v5, 2), _mm_shuffle_ps(v5, v5, 1)), v5);
  v7 = _mm_shuffle_ps(v6, v6, 0);
  v8 = v7;
  v9 = _mm_rsqrt_ps(v7);
  v7.m128_f32[0] = 1.0;
  v10 = _mm_shuffle_ps(_mm_set_ss(.5f), _mm_set_ss(.5f), 0);
  v51 = _mm_shuffle_ps(v7, v7, 0);
  v11 = Plus(Mul(_mm_sub_ps(v51, Mul(Mul(v9, v9), v8)), Mul(v10, v9)), v9);
  v55 = v10;
  v56 = Mul(
          Plus(Mul(_mm_sub_ps(v51, Mul(Mul(v11, v11), v8)), Mul(v10, v11)), v11),
          v4);
  v54.m128_u64[1] = 1065353216;
  v54.m128_u64[0] = 0;
  v59 = v54;
  math::b11::RotateVector(&v59, a3, &v61);
  v12 = _mm_sub_ps(
          Mul(_mm_shuffle_ps(v61, v61, 9), _mm_shuffle_ps(v56, v56, 18)),
          Mul(_mm_shuffle_ps(v61, v61, 18), _mm_shuffle_ps(v56, v56, 9)));
  v13 = Mul(v12, v12);
  v14 = Plus(Plus(_mm_shuffle_ps(v13, v13, 2), _mm_shuffle_ps(v13, v13, 1)), v13);
  v15 = _mm_shuffle_ps(v14, v14, 0);
  v16 = v51;
  v17 = _mm_rsqrt_ps(v15);
  v18 = Plus(Mul(_mm_sub_ps(v51, Mul(Mul(v17, v17), v15)), Mul(v17, v55)), v17);
  v19 = Mul(
          Plus(Mul(_mm_sub_ps(v51, Mul(Mul(v18, v18), v15)), Mul(v18, v55)), v18),
          v12);
  v20 = Mul(v61, v61);
  v21 = Plus(Plus(_mm_shuffle_ps(v20, v20, 2), _mm_shuffle_ps(v20, v20, 1)), v20);
  v22 = _mm_shuffle_ps(v21, v21, 0);
  v23 = _mm_rsqrt_ps(v22);
  v54 = v19;
  v24 = Mul(
          Plus(Mul(_mm_sub_ps(v51, Mul(Mul(v23, v23), v22)), Mul(v23, v55)), v23),
          v61);
  v25 = Mul(v56, v56);
  v26 = Plus(Plus(_mm_shuffle_ps(v25, v25, 2), _mm_shuffle_ps(v25, v25, 1)), v25);
  v27 = _mm_shuffle_ps(v26, v26, 0);
  v28 = _mm_rsqrt_ps(v27);
  v29 = Mul(
          Plus(Mul(_mm_sub_ps(v51, Mul(Mul(v28, v28), v27)), Mul(v28, v55)), v28),
          v56);
  v56 = v24;
  v52 = v29;
  v30 = Mul(v29, v24);
  v31 = Plus(Plus(_mm_shuffle_ps(v30, v30, 2), _mm_shuffle_ps(v30, v30, 1)), v30);
  v55 = _mm_min_ps(
          _mm_max_ps(_mm_shuffle_ps(v31, v31, 0), _mm_shuffle_ps(_mm_set_ss(-1), _mm_set_ss(-1), 0)),
          v16);
  v50 = Bits(Trig(v55.m128_f32[0],0));
  v32 = Mul(
          _mm_sub_ps(
            Mul(_mm_shuffle_ps(v52, v52, 18), _mm_shuffle_ps(v56, v56, 9)),
            Mul(_mm_shuffle_ps(v52, v52, 9), _mm_shuffle_ps(v56, v56, 18))),
          v19);
  v33 = Plus(Plus(_mm_shuffle_ps(v32, v32, 2), _mm_shuffle_ps(v32, v32, 1)), v32);
  v55.m128_i32[0] = 0;
  v60 = _mm_shuffle_ps(v33, v33, 0).m128_f32[0];
  v34 = _mm_shuffle_ps(_mm_castsi128_ps(_mm_cvtsi32_si128(v50)), _mm_castsi128_ps(_mm_cvtsi32_si128(v50)), 0);
  if ( v60 < 0.0 )
    v34 = _mm_sub_ps(_mm_shuffle_ps(_mm_set_ss(Float(0x40c90fdb)), _mm_set_ss(Float(0x40c90fdb)), 0), v34);
  v57 = v34;
  const float lower=-0.0f-limit;
  const uint32_t mask=static_cast<uint32_t>(static_cast<int32_t>(Bits(lower-v34.m128_f32[0]))>>31);
  v35=Float((mask&Bits(v34.m128_f32[0]))|(~mask&Bits(lower)));
  const uint32_t mask2=static_cast<uint32_t>(static_cast<int32_t>(Bits(limit-v35))>>31);
  v47=(mask2&Bits(limit))|(~mask2&Bits(v35));
  math::b11::RotateVector(&v54, v65, &v58);
  v53 = _mm_shuffle_ps(_mm_castsi128_ps(_mm_cvtsi32_si128(v47)), _mm_castsi128_ps(_mm_cvtsi32_si128(v47)), 0).m128_f32[0]
      / _mm_shuffle_ps(_mm_set_ss(2), _mm_set_ss(2), 0).m128_f32[0];
  v48 = Bits(Trig(v53,1));
  v36 = _mm_shuffle_ps(_mm_castsi128_ps(_mm_cvtsi32_si128(v48)), _mm_castsi128_ps(_mm_cvtsi32_si128(v48)), 0);
  v62 = Mul(_mm_shuffle_ps(v58, v58, 170), v36);
  v64 = Mul(_mm_shuffle_ps(v58, v58, 85), v36);
  v63 = Mul(_mm_shuffle_ps(v58, v58, 0), v36);
  v49 = Bits(Trig(v53,2));
  v37 = _mm_shuffle_ps(
          _mm_shuffle_ps(v63, v64, 16),
          _mm_shuffle_ps(v62, _mm_shuffle_ps(_mm_castsi128_ps(_mm_cvtsi32_si128(v49)), _mm_castsi128_ps(_mm_cvtsi32_si128(v49)), 0), 50),
          136);
  v38 = a4[1];
  v39 = _mm_shuffle_ps(v37, v37, 255);
  v40 = _mm_shuffle_ps(v38, v38, 255);
  v42 = Plus(
          _mm_sub_ps(
            Mul(_mm_shuffle_ps(v38, v38, 18), _mm_shuffle_ps(v37, v37, 9)),
            Mul(_mm_shuffle_ps(v38, v38, 9), _mm_shuffle_ps(v37, v37, 18))),
          Mul(v39, v38));
  v43 = Mul(v40, v37);
  v44 = Mul(v37, v38);
  v45 = Plus(v42, v43);
  v46 = Plus(Plus(_mm_shuffle_ps(v44, v44, 2), _mm_shuffle_ps(v44, v44, 1)), v44);
  *a1 = _mm_shuffle_ps(v45, _mm_shuffle_ps(v45, _mm_sub_ps(Mul(v40, v39), _mm_shuffle_ps(v46, v46, 0)), 10), 132);
  return a1;
}
}
#pragma float_control(pop)
