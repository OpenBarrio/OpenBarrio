#include "ts3/anim/m05_M08.h"
#include "ts3/layout.h"
#include "ts3/anim/m05_B01.h"
#include "ts3/debug/clu_6c8680_Debug.h"
#include "ts3/scratch/clu_4e3f70_Scratch.h"
#include <cstring>
namespace ts3::anim {
namespace {
template<unsigned Address> uint32_t& Global() {
#if defined(_M_IX86)
 return *reinterpret_cast<uint32_t*>(Address);
#else
 AbortNotRewritten("M08 game debug globals"); static uint32_t unavailable; return unavailable;
#endif
}
template<unsigned Address> float FloatGlobal() { float f; auto b=Global<Address>(); std::memcpy(&f,&b,4); return f; }
}
void* DeleteConstraintEvaluator(void* self, uint32_t flags) {
#if defined(_M_IX86)
 *static_cast<uintptr_t*>(self)=0x00FC12D0;
#else
 AbortNotRewritten("M08 original evaluator vtable");
#endif
 b01::Reset(static_cast<b01::Node*>(self));
 if(flags & 1u) scratch::Free4E3F70(self);
 return self;
}
uintptr_t EmitConstraintDebug(const void* object, const __m128* a2)
{
  const auto self = static_cast<const uint32_t*>(object);
  debug::DebugLineBuffer* renderer = nullptr;
  uintptr_t result; // eax
  // Retain the original SSE operation order: matrix-to-quaternion branches,
  // then eight transformed corners. No normalization or scalar reassociation.
  __m128 v4; // xmm4
  __m128 v5; // xmm0
  __m128 v6; // xmm1
  __m128 v7; // xmm6
  __m128 v8; // xmm3
  __m128 v9; // xmm5
  __m128 v10; // xmm2
  __m128 v11; // xmm5
  __m128 v12; // xmm7
  __m128 v13; // xmm2
  __m128 v14; // xmm2
  __m128 v15; // xmm6
  __m128 v16; // xmm2
  __m128 v17; // xmm6
  __m128 v18; // xmm7
  __m128 v19; // xmm4
  __m128 v20; // xmm0
  __m128 v21; // xmm2
  __m128 v22; // xmm6
  __m128 v23; // xmm7
  __m128 v24; // xmm6
  __m128 v25; // xmm1
  __m128 v26; // xmm7
  __m128 v27; // xmm6
  __m128 v28; // xmm2
  __m128 v29; // xmm0
  __int32 v30; // xmm1_4
  __m128 v31; // xmm1
  __m128 v32; // xmm0
  __m128 v33; // xmm3
  __m128 v34; // xmm4
  __m128 v35; // xmm5
  __m128 v36; // xmm2
  __m128 v37; // xmm4
  __m128 v38; // [esp+10h] [ebp-160h]
  __m128 v39; // [esp+10h] [ebp-160h]
  __m128 v40; // [esp+20h] [ebp-150h]
  __m128 v41; // [esp+20h] [ebp-150h]
  __m128 v42; // [esp+30h] [ebp-140h]
  __m128 v43; // [esp+40h] [ebp-130h]
  __m128 v44; // [esp+40h] [ebp-130h]
  __m128 v45; // [esp+50h] [ebp-120h]
  __m128 v46; // [esp+50h] [ebp-120h]
  __m128 v47; // [esp+60h] [ebp-110h]
  __m128 v48; // [esp+70h] [ebp-100h]
  __m128 v49; // [esp+80h] [ebp-F0h]
  __m128 v50; // [esp+90h] [ebp-E0h]
  __m128 v51; // [esp+A0h] [ebp-D0h] BYREF
  __m128 v52; // [esp+B0h] [ebp-C0h] BYREF
  __m128 v53; // [esp+C0h] [ebp-B0h] BYREF
  __m128 v54; // [esp+D0h] [ebp-A0h] BYREF
  __m128 v55; // [esp+E0h] [ebp-90h]
  __m128 v56; // [esp+F0h] [ebp-80h]
  __m128 v57; // [esp+100h] [ebp-70h] BYREF
  __m128 v58; // [esp+110h] [ebp-60h]
  __m128 v59; // [esp+120h] [ebp-50h] BYREF
  __m128 v60; // [esp+130h] [ebp-40h]
  __m128 v61; // [esp+140h] [ebp-30h] BYREF
  __m128 v62; // [esp+150h] [ebp-20h]
  __m128 v63; // [esp+160h] [ebp-10h] BYREF

  // Disabled paths return the singleton pointer, not a boolean.
  result = Global<0x011CD800>();
  if ( Global<0x011CD800>() && *(reinterpret_cast<const uint8_t*>(uintptr_t(Global<0x011CD800>())) + 45) )
  {
    renderer = reinterpret_cast<debug::DebugLineBuffer*>(static_cast<unsigned char*>(CallGameFunction<void*,0x006E8330>()) + 0x328);
    if ( (Global<0x011CD40C>() & 1) == 0 )
    {
      Global<0x011CD40C>() |= 1u;
      Global<0x011CD408>() = 1041865114;
    }
    v4 = a2[1];
    v5 = *a2;
    v6 = a2[2];
    v7 = _mm_shuffle_ps(v4, v4, 85);
    v8 = _mm_shuffle_ps(v5, v5, 0);
    v9 = _mm_add_ps(v7, v8);
    v10 = _mm_shuffle_ps(v6, v6, 170);
    v45 = _mm_add_ps(v10, v9);
    v43 = v9;
    v11 = _mm_shuffle_ps(_mm_set_ss(0.5f), _mm_set_ss(0.5f), 0);
    if ( !(v45.m128_f32[0] > 0.0f) )
    {
      if ( !(v8.m128_f32[0] > v7.m128_f32[0]) || !(v8.m128_f32[0] > v10.m128_f32[0]) )
      {
        if ( !(v7.m128_f32[0] > v10.m128_f32[0]) )
        {
          v7.m128_f32[0] = 1.0;
          v26 = _mm_sqrt_ps(_mm_add_ps(_mm_shuffle_ps(v7, v7, 0), _mm_sub_ps(v10, v43)));
          v27 = _mm_sub_ps(_mm_shuffle_ps(v5, v5, 85), _mm_shuffle_ps(v4, v4, 0));
          v28 = _mm_add_ps(_mm_shuffle_ps(v6, v6, 0), _mm_shuffle_ps(v5, v5, 170));
          v29 = _mm_div_ps(v11, v26);
          v13 = _mm_shuffle_ps(
                  _mm_shuffle_ps(
                    _mm_mul_ps(v28, v29),
                    _mm_mul_ps(_mm_add_ps(_mm_shuffle_ps(v4, v4, 170), _mm_shuffle_ps(v6, v6, 85)), v29),
                    16),
                  _mm_shuffle_ps(_mm_mul_ps(v26, v11), _mm_mul_ps(v27, v29), 50),
                  136);
        }
        else
        {
          v21 = _mm_add_ps(v10, v8);
          v22 = _mm_sub_ps(v7, v21);
          v21.m128_f32[0] = 1.0;
          v40 = _mm_sqrt_ps(_mm_add_ps(_mm_shuffle_ps(v21, v21, 0), v22));
          v23 = _mm_sub_ps(_mm_shuffle_ps(v6, v6, 0), _mm_shuffle_ps(v5, v5, 170));
          v24 = _mm_add_ps(_mm_shuffle_ps(v4, v4, 170), _mm_shuffle_ps(v6, v6, 85));
          v25 = _mm_div_ps(v11, v40);
          v13 = _mm_shuffle_ps(
                  _mm_shuffle_ps(
                    _mm_mul_ps(_mm_add_ps(_mm_shuffle_ps(v4, v4, 0), _mm_shuffle_ps(v5, v5, 85)), v25),
                    _mm_mul_ps(v40, v11),
                    16),
                  _mm_shuffle_ps(_mm_mul_ps(v24, v25), _mm_mul_ps(v23, v25), 50),
                  136);
        }
      }
      else
      {
        v14 = _mm_add_ps(v10, v7);
        v15 = _mm_sub_ps(v8, v14);
        v14.m128_f32[0] = 1.0;
        v16 = _mm_sqrt_ps(_mm_add_ps(_mm_shuffle_ps(v14, v14, 0), v15));
        v17 = _mm_sub_ps(_mm_shuffle_ps(v4, v4, 170), _mm_shuffle_ps(v6, v6, 85));
        v18 = _mm_shuffle_ps(v5, v5, 170);
        v19 = _mm_add_ps(_mm_shuffle_ps(v4, v4, 0), _mm_shuffle_ps(v5, v5, 85));
        v20 = _mm_div_ps(v11, v16);
        v13 = _mm_shuffle_ps(
                _mm_shuffle_ps(_mm_mul_ps(v16, v11), _mm_mul_ps(v19, v20), 16),
                _mm_shuffle_ps(_mm_mul_ps(_mm_add_ps(_mm_shuffle_ps(v6, v6, 0), v18), v20), _mm_mul_ps(v17, v20), 50),
                136);
      }
    }
    else
    {
      v10.m128_f32[0] = 1.0;
      v38 = _mm_sqrt_ps(_mm_add_ps(_mm_shuffle_ps(v10, v10, 0), v45));
      v12 = _mm_div_ps(v11, v38);
      v13 = _mm_shuffle_ps(
              _mm_shuffle_ps(
                _mm_mul_ps(_mm_sub_ps(_mm_shuffle_ps(v4, v4, 170), _mm_shuffle_ps(v6, v6, 85)), v12),
                _mm_mul_ps(_mm_sub_ps(_mm_shuffle_ps(v6, v6, 0), _mm_shuffle_ps(v5, v5, 170)), v12),
                16),
              _mm_shuffle_ps(
                _mm_mul_ps(_mm_sub_ps(_mm_shuffle_ps(v5, v5, 85), _mm_shuffle_ps(v4, v4, 0)), v12),
                _mm_mul_ps(v38, v11),
                50),
              136);
    }
    v30 = self[9];
    v58 = a2[3];
    v48.m128_f32[3] = -0.0f - FloatGlobal<0x011CD408>();
    v48.m128_i32[2] = v30;
    v48.m128_i32[1] = 1038174126;
    v48.m128_f32[0] = -0.0f - FloatGlobal<0x011CD408>();
    v50.m128_u64[1] = (uint64_t(Global<0x011CD408>()) << 32) | uint32_t(v30);
    v44.m128_u64[1] = 0xBDCCCCCD3F800000uLL;
    v46.m128_u64[1] = 0x3DCCCCCD3F800000LL;
    v49.m128_u64[0] = 0x3DCCCCCDBDCCCCCDLL;
    v47.m128_u64[0] = 0x3DCCCCCD3DCCCCCDLL;
    v44.m128_u64[0] = 0x3DCCCCCDBDCCCCCDLL;
    v46.m128_u64[0] = 0x3DCCCCCD3DCCCCCDLL;
    v41.m128_u64[0] = 0x3DCCCCCDBDCCCCCDLL;
    v31 = _mm_shuffle_ps(v13, v13, 18);
    v39.m128_u64[0] = 0x3DCCCCCD3DCCCCCDLL;
    v32 = _mm_shuffle_ps(v13, v13, 9);
    v42 = _mm_shuffle_ps(v13, v13, 255);
    v41.m128_u64[1] = (unsigned int)Global<0x0114F62C>() | 0xBDCCCCCD00000000uLL;
    v39.m128_u64[1] = (unsigned int)Global<0x0114F62C>() | 0x3DCCCCCD00000000LL;
    v33 = _mm_add_ps(
            _mm_sub_ps(_mm_mul_ps(_mm_shuffle_ps(v48, v48, 18), v32), _mm_mul_ps(_mm_shuffle_ps(v48, v48, 9), v31)),
            _mm_mul_ps(v48, v42));
    v50.m128_u64[0] = (unsigned int)Global<0x011CD408>() | 0x3DE147AE00000000LL;
    v49.m128_u64[1] = (unsigned int)Global<0x0114F628>() | 0xBDCCCCCD00000000uLL;
    v34 = _mm_add_ps(
            _mm_sub_ps(_mm_mul_ps(_mm_shuffle_ps(v50, v50, 18), v32), _mm_mul_ps(_mm_shuffle_ps(v50, v50, 9), v31)),
            _mm_mul_ps(v50, v42));
    v47.m128_u64[1] = (unsigned int)Global<0x0114F628>() | 0x3DCCCCCD00000000LL;
    v35 = _mm_add_ps(
            _mm_sub_ps(_mm_mul_ps(_mm_shuffle_ps(v49, v49, 18), v32), _mm_mul_ps(_mm_shuffle_ps(v49, v49, 9), v31)),
            _mm_mul_ps(v49, v42));
    v60 = _mm_add_ps(
            _mm_sub_ps(_mm_mul_ps(_mm_shuffle_ps(v47, v47, 18), v32), _mm_mul_ps(_mm_shuffle_ps(v47, v47, 9), v31)),
            _mm_mul_ps(v47, v42));
    v55 = _mm_add_ps(
            _mm_sub_ps(_mm_mul_ps(_mm_shuffle_ps(v44, v44, 18), v32), _mm_mul_ps(_mm_shuffle_ps(v44, v44, 9), v31)),
            _mm_mul_ps(v44, v42));
    v62 = _mm_add_ps(
            _mm_sub_ps(_mm_mul_ps(_mm_shuffle_ps(v46, v46, 18), v32), _mm_mul_ps(_mm_shuffle_ps(v46, v46, 9), v31)),
            _mm_mul_ps(v46, v42));
    v56 = _mm_add_ps(
            _mm_sub_ps(_mm_mul_ps(_mm_shuffle_ps(v41, v41, 18), v32), _mm_mul_ps(_mm_shuffle_ps(v41, v41, 9), v31)),
            _mm_mul_ps(v41, v42));
    v36 = _mm_shuffle_ps(_mm_set_ss(2.0f), _mm_set_ss(2.0f), 0);
    v63 = _mm_add_ps(
            _mm_add_ps(
              _mm_mul_ps(
                _mm_sub_ps(_mm_mul_ps(_mm_shuffle_ps(v33, v33, 18), v32), _mm_mul_ps(_mm_shuffle_ps(v33, v33, 9), v31)),
                v36),
              v48),
            v58);
    v61 = _mm_add_ps(
            _mm_add_ps(
              _mm_mul_ps(
                _mm_sub_ps(_mm_mul_ps(_mm_shuffle_ps(v34, v34, 18), v32), _mm_mul_ps(_mm_shuffle_ps(v34, v34, 9), v31)),
                v36),
              v50),
            v58);
    v51 = _mm_add_ps(
            _mm_add_ps(
              _mm_mul_ps(
                _mm_sub_ps(_mm_mul_ps(_mm_shuffle_ps(v35, v35, 18), v32), _mm_mul_ps(_mm_shuffle_ps(v35, v35, 9), v31)),
                v36),
              v49),
            v58);
    v54 = _mm_add_ps(
            _mm_add_ps(
              _mm_mul_ps(
                _mm_sub_ps(_mm_mul_ps(_mm_shuffle_ps(v60, v60, 18), v32), _mm_mul_ps(_mm_shuffle_ps(v60, v60, 9), v31)),
                v36),
              v47),
            v58);
    v59 = _mm_add_ps(
            _mm_add_ps(
              _mm_mul_ps(
                _mm_sub_ps(_mm_mul_ps(_mm_shuffle_ps(v55, v55, 18), v32), _mm_mul_ps(_mm_shuffle_ps(v55, v55, 9), v31)),
                v36),
              v44),
            v58);
    v57 = _mm_add_ps(
            _mm_add_ps(
              _mm_mul_ps(
                _mm_sub_ps(_mm_mul_ps(_mm_shuffle_ps(v62, v62, 18), v32), _mm_mul_ps(_mm_shuffle_ps(v62, v62, 9), v31)),
                v36),
              v46),
            v58);
    v37 = _mm_add_ps(
            _mm_sub_ps(_mm_mul_ps(_mm_shuffle_ps(v39, v39, 18), v32), _mm_mul_ps(_mm_shuffle_ps(v39, v39, 9), v31)),
            _mm_mul_ps(v39, v42));
    v53 = _mm_add_ps(
            _mm_add_ps(
              _mm_mul_ps(
                _mm_sub_ps(_mm_mul_ps(_mm_shuffle_ps(v56, v56, 18), v32), _mm_mul_ps(_mm_shuffle_ps(v56, v56, 9), v31)),
                v36),
              v41),
            v58);
    v52 = _mm_add_ps(
            _mm_add_ps(
              _mm_mul_ps(
                _mm_sub_ps(_mm_mul_ps(_mm_shuffle_ps(v37, v37, 18), v32), _mm_mul_ps(_mm_shuffle_ps(v37, v37, 9), v31)),
                v36),
              v39),
            v58);
    reinterpret_cast<uintptr_t>(renderer->AddSegment(reinterpret_cast<const debug::Vec4*>(&v51), reinterpret_cast<const debug::Vec4*>(&v54), -16711936, -16711936));
    reinterpret_cast<uintptr_t>(renderer->AddSegment(reinterpret_cast<const debug::Vec4*>(&v59), reinterpret_cast<const debug::Vec4*>(&v57), -16711936, -16711936));
    reinterpret_cast<uintptr_t>(renderer->AddSegment(reinterpret_cast<const debug::Vec4*>(&v53), reinterpret_cast<const debug::Vec4*>(&v52), -16711936, -16711936));
    reinterpret_cast<uintptr_t>(renderer->AddSegment(reinterpret_cast<const debug::Vec4*>(&v51), reinterpret_cast<const debug::Vec4*>(&v53), -16711936, -16711936));
    reinterpret_cast<uintptr_t>(renderer->AddSegment(reinterpret_cast<const debug::Vec4*>(&v54), reinterpret_cast<const debug::Vec4*>(&v52), -16711936, -16711936));
    return reinterpret_cast<uintptr_t>(renderer->AddSegment(reinterpret_cast<const debug::Vec4*>(&v63), reinterpret_cast<const debug::Vec4*>(&v61), -65536, -65536));
  }
  return result;
}


}
#if defined(_M_IX86)
namespace ts3::anim::m08_hooks {
void* __fastcall Delete(void* p,void*,uint32_t flags) { return DeleteConstraintEvaluator(p,flags); }
uintptr_t __fastcall Emit(const void* p,void*,const __m128* matrix) { return EmitConstraintDebug(p,matrix); }
}
#endif
