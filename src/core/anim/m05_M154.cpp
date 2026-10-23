#include "ts3/anim/m05_M154.h"
#include "ts3/layout.h"
#include <cstring>
#include <cmath>
#include <emmintrin.h>
#include "ts3/anim/m05_B02.h"
#include "ts3/anim/m05_M01.h"
#include "ts3/anim/m05_B26.h"
#include "ts3/anim/m05_B29.h"
#include "ts3/anim/m05_M100.h"
#include "ts3/resourcesystem/clu_4a0a80_ResourceSystem.h"
#include "ts3/animation/cls_7f1620_Animation.h"
#include "ts3/misc/cls_4a6c80_x.h"
#include "ts3/scripting/clu_5826b0_Scripting.h"
namespace ts3::anim::m154 {
namespace {
void Identity(float* lanes) {
    lanes[0]=0.0f; lanes[1]=0.0f; lanes[2]=0.0f; lanes[3]=1.0f;
}
void Zero(float* lanes) { std::memset(lanes,0,16); }
}
bool TryUpdateIdle(Component* self, float delta, Pose*& result) {
    // Do not partially execute the function when the active evaluator is required.
    if (self->pose && self->pose->manager && delta>0.0f) return false;
    if (self->pose) {
        Identity(self->pose->vectors[3]);
        Zero(self->pose->vectors[4]);
        Zero(self->pose->vectors[5]);
    }
    if (self->followup) {
        ts3::CallGameMethod<void,0x005813c0>(self->followup,delta);
    }
    // The callback may replace the pose. EA reloads it rather than retaining the entry pointer.
    result=self->pose;
    if (result) {
        Identity(self->pose->vectors[6]);
        Zero(self->pose->vectors[7]);
        self->pose->weights[1]=1.0f;
        self->pose->weights[0]=1.0f;
        Identity(self->pose->vectors[8]);
        Zero(self->pose->vectors[9]);
        result=self->pose;
        result->weights[3]=0.0f;
        self->pose->weights[2]=0.0f;
    }
    return true;
}
namespace {
template<class T,unsigned A> T& Global() {
#if defined(_M_IX86)
    return *reinterpret_cast<T*>(uintptr_t(A));
#else
    AbortNotRewritten("M154 game global");static T unused{};return unused;
#endif
}
const char* Descriptor() {
#if defined(_M_IX86)
    return reinterpret_cast<const char*>(0x00fc10d8);
#else
    return "Animation/RigPose";
#endif
}
float FloatBits(uint32_t bits) {float f;std::memcpy(&f,&bits,4);return f;}
void SetFloatBits(uint32_t& bits,float f) {std::memcpy(&bits,&f,4);}
__m128 LowBits(uint64_t bits) {
    // Keep literal zero lanes opaque: folding 0-x into a sign-bit flip changes
    // the sign of zero in EA's matrix-to-quaternion cross products.
    volatile uint64_t stored=bits;
    uint64_t value=stored;
    return _mm_castsi128_ps(_mm_set_epi32(0,0,static_cast<int32_t>(value>>32),static_cast<int32_t>(value)));
}
float Cos(float value) {
#if defined(_M_IX86)
    float result;__asm { fld value } __asm { fcos } __asm { fstp result } return result;
#else
    return std::cos(value);
#endif
}
float Sin(float value) {
#if defined(_M_IX86)
    float result;__asm { fld value } __asm { fsin } __asm { fstp result } return result;
#else
    return std::sin(value);
#endif
}
float Acos(float value) {
#if defined(_M_IX86)
    float result;uintptr_t target=0x00f7edde;
    // The compiler cannot see the register clobbers of an x87-only import.
    // Preserve them explicitly rather than letting interprocedural analysis
    // assume this inline-assembly helper leaves live SIMD temporaries intact.
    alignas(16) __m128 saved[8];
    __asm {
        movaps saved[0],xmm0
        movaps saved[16],xmm1
        movaps saved[32],xmm2
        movaps saved[48],xmm3
        movaps saved[64],xmm4
        movaps saved[80],xmm5
        movaps saved[96],xmm6
        movaps saved[112],xmm7
        fld value
        push eax
        push ecx
        push edx
        mov eax,target
        call eax
        pop edx
        pop ecx
        pop eax
        fstp result
        movaps xmm0,saved[0]
        movaps xmm1,saved[16]
        movaps xmm2,saved[32]
        movaps xmm3,saved[48]
        movaps xmm4,saved[64]
        movaps xmm5,saved[80]
        movaps xmm6,saved[96]
        movaps xmm7,saved[112]
    }
    return result;
#else
    return std::acos(value);
#endif
}
}

namespace {
void ApplyMotion(Component* self,Pose* pose,void* v11,void* projector,float a2) {
    __m128* v9=reinterpret_cast<__m128*>(pose->vectors);
    __m128 constantTmp,savedQuat;
    __m128 v18;
    __m128 v40;
    __m128 v41;
    __m128 v42;
    __m128 v43;
    __m128 v44;
    __m128 v45;
    float v46;
    __m128 v47;
    __m128 v48;
    __m128 v49;
    __m128 v50;
    __m128 v51;
    __m128 v52;
    __m128 v53;
    __m128 v54;
    __m128 v55;
    __m128 v56;
    __m128 v57;
    __m128 v58;
    __m128 v59;
    __m128 v60;
    __m128 v61;
    __m128 v62;
    __m128 v63;
    __m128 v64;
    __m128 v65;
    float v66;
    __m128 v67;
    __m128 v68;
    __m128 v69;
    __m128 v70;
    __m128 v71;
    __m128 v72;
    __m128 v73;
    __m128 v74;
    __m128 v75;
    __m128 v76;
    __m128 v77;
    __m128 v78;
    __m128 v79;
    __m128 v80;
    __m128 v81;
    __m128 v82;
    __m128 v83;
    __m128 v84;
    __m128 v85;
    __m128 v86;
    __m128 v87;
    __m128 v88;
    int v89;
    __m128 v90;
    __m128 v91;
    __m128 v92;
    __m128 v93;
    __m128 v94;
    __m128 v95;
    __m128 v96;
    __m128 v97;
    __m128 v98;
    __m128 v99;
    __m128 v100;
    __m128 v101;
    __m128 v102;
    __m128 v103;
    __m128 v104;
    __m128 v105;
    __m128 v106;
    __m128 v107;
    __m128 v108;
    __m128 v109;
    __m128 v110;
    __m128 v111;
    __m128 v112;
    __m128 v113;
    __m128 v114;
    __m128 v115;
    unsigned int v120;
    __m128 v122;
    __m128 v124;
    __m128 v125;
    __m128 v126[2];

    float v137;

    float v139;

    __m128 v142[2];
    __m128 v143[3];
    // Preserve the disassembly's shuffle/lane and arithmetic order. The first
    // stage blends transient +60/+70 inputs against the previous +80/+90 state;
    // the second rotates motion into world space, composes the quaternion and
    // optionally projects it onto the horizontal plane.
    v18=_mm_setzero_ps(); v122=_mm_setzero_ps(); v124=_mm_set_ps(1,0,0,0);
    v40 = v9[6];
    v41 = v9[8];
    v42 = _mm_set_ss(pose->weights[1]);
    v43 = _mm_mul_ps(v40, v41);
    v44 = _mm_add_ps(_mm_shuffle_ps(v43, v43, 14), v43);
    v45 = _mm_add_ps(_mm_shuffle_ps(v44, v44, 1), v44);
    v46 = _mm_shuffle_ps(v45, v45, 0).m128_f32[0];

    v47 = _mm_shuffle_ps(v42, v42, 0);
    v126[0] = v47;
    v125 = v41;
    savedQuat = v40;
    if ( v18.m128_f32[0] > v46 )
    {
      v41 = _mm_sub_ps(v18, v41);
      v125 = v41;
      v46 = v18.m128_f32[0] - v46;
    }
    constantTmp = _mm_shuffle_ps(LowBits(0x3DB2B8C3ULL), LowBits(0x3DB2B8C3ULL), 0);

    SetFloatBits(v120, Cos(constantTmp.m128_f32[0]));
    v137 = _mm_shuffle_ps(LowBits(v120), LowBits(v120), 0).m128_f32[0];
    if ( !(v46 > v137) )
    {
      SetFloatBits(v120, Acos(v46));
      constantTmp = _mm_shuffle_ps(LowBits(0x3F800000ULL), LowBits(0x3F800000ULL), 0);
      v62 = _mm_shuffle_ps(LowBits(v120), LowBits(v120), 0);
      v63 = _mm_mul_ps(_mm_sub_ps(constantTmp, v126[0]), v62);

      v126[0] = _mm_mul_ps(v62, v126[0]);
      v126[1] = v63;
      v18 = v122;
      SetFloatBits(v120, Sin(v126[0].m128_f32[0]));
      v64 = _mm_shuffle_ps(LowBits(v120), LowBits(v120), 0);
      SetFloatBits(v120, Sin(v62.m128_f32[0]));
      v65 = _mm_mul_ps(_mm_div_ps(v64, _mm_shuffle_ps(LowBits(v120), LowBits(v120), 0)), savedQuat);
      SetFloatBits(v120, Sin(v63.m128_f32[0]));
      v66 = Sin(v62.m128_f32[0]);
      v67 = _mm_shuffle_ps(LowBits(v120), LowBits(v120), 0);
      SetFloatBits(v120, v66);
      v61 = _mm_add_ps(v65, _mm_mul_ps(_mm_div_ps(v67, _mm_shuffle_ps(LowBits(v120), LowBits(v120), 0)), v125));
    }
    else
    {
      v48 = _mm_mul_ps(v40, v41);
      v49 = _mm_add_ps(_mm_shuffle_ps(v48, v48, 14), v48);
      v50 = _mm_add_ps(_mm_shuffle_ps(v49, v49, 1), v49);
      v51 = _mm_cmplt_ps(v18, _mm_shuffle_ps(v50, v50, 0));
      v52 = _mm_or_ps(
              _mm_andnot_ps(v51, _mm_sub_ps(v41, _mm_mul_ps(_mm_add_ps(v40, v41), v47))),
              _mm_and_ps(_mm_add_ps(_mm_mul_ps(_mm_sub_ps(v40, v41), v47), v41), v51));
      v53 = _mm_mul_ps(v52, v52);
      v54 = _mm_add_ps(_mm_shuffle_ps(v53, v53, 14), v53);
      v55 = _mm_add_ps(_mm_shuffle_ps(v54, v54, 1), v54);
      v56 = _mm_shuffle_ps(v55, v55, 0);
      v57 = _mm_rsqrt_ps(v56);
      v58 = _mm_shuffle_ps(LowBits(0x3F800000ULL), LowBits(0x3F800000ULL), 0);
      v59 = _mm_shuffle_ps(LowBits(0x3F000000ULL), LowBits(0x3F000000ULL), 0);
      v60 = _mm_add_ps(
              _mm_mul_ps(_mm_sub_ps(v58, _mm_mul_ps(_mm_mul_ps(v57, v57), v56)), _mm_mul_ps(v57, v59)),
              v57);
      constantTmp = v58;
      v61 = _mm_mul_ps(
              _mm_add_ps(
                _mm_mul_ps(_mm_sub_ps(v58, _mm_mul_ps(_mm_mul_ps(v60, v60), v56)), _mm_mul_ps(v60, v59)),
                v60),
              v52);
    }
    v9[3] = v61;
    v9[4] = _mm_add_ps(
              _mm_mul_ps(_mm_shuffle_ps(_mm_set_ss(pose->weights[2]), _mm_set_ss(pose->weights[2]), 0), v9[9]),
              _mm_mul_ps(_mm_shuffle_ps(_mm_set_ss(pose->weights[0]), _mm_set_ss(pose->weights[0]), 0), v9[7]));
    v9[4] = _mm_mul_ps(
              _mm_shuffle_ps(
                _mm_set_ss(self->pose->motionScale),
                _mm_set_ss(self->pose->motionScale),
                0),
              v9[4]);
    v68 = _mm_sub_ps(v9[4], v18);
    v69 = _mm_shuffle_ps(LowBits(0xBF800000ULL), LowBits(0xBF800000ULL), 0);
    v70 = _mm_shuffle_ps(LowBits(0x37800000ULL), LowBits(0x37800000ULL), 0);
    if ( (_mm_movemask_ps(_mm_cmple_ps(_mm_max_ps(_mm_mul_ps(v68, v69), v68), v70)) & 7) != 7
      || (v71 = _mm_sub_ps(v9[3], v124),
          _mm_movemask_ps(_mm_cmple_ps(_mm_max_ps(_mm_mul_ps(v71, v69), v71), v70)) != 15) )
    {
      if ( v11 )
      {
        v72 = v9[4];
        v73 = v9[1];
        v74 = _mm_shuffle_ps(v73, v73, 18);
        v75 = _mm_shuffle_ps(v73, v73, 9);
        v76 = _mm_add_ps(
                _mm_sub_ps(
                  _mm_mul_ps(_mm_shuffle_ps(v72, v72, 18), v75),
                  _mm_mul_ps(_mm_shuffle_ps(v72, v72, 9), v74)),
                _mm_mul_ps(_mm_shuffle_ps(v73, v73, 255), v72));
        v77 = _mm_shuffle_ps(LowBits(0x40000000ULL), LowBits(0x40000000ULL), 0);
        v78 = _mm_add_ps(
                _mm_mul_ps(
                  _mm_sub_ps(
                    _mm_mul_ps(_mm_shuffle_ps(v76, v76, 18), v75),
                    _mm_mul_ps(_mm_shuffle_ps(v76, v76, 9), v74)),
                  v77),
                v72);
        *v9 = _mm_add_ps(v78, *v9);
        v79 = LowBits(0x3F800000ULL);
        v79 = _mm_div_ss(_mm_set_ss(1.0f),_mm_set_ss(a2));
        v9[5] = v78;
        v80 = v9[5];
        v124 = v77;
        v9[5] = _mm_mul_ps(_mm_shuffle_ps(v79, v79, 0), v80);
        v81 = v9[3];
        v82 = v9[1];
        v83 = _mm_shuffle_ps(v81, v81, 255);
        v84 = _mm_shuffle_ps(v82, v82, 255);
        v85 = _mm_add_ps(
                _mm_sub_ps(
                  _mm_mul_ps(_mm_shuffle_ps(v82, v82, 18), _mm_shuffle_ps(v81, v81, 9)),
                  _mm_mul_ps(_mm_shuffle_ps(v82, v82, 9), _mm_shuffle_ps(v81, v81, 18))),
                _mm_mul_ps(v83, v82));
        v86 = _mm_mul_ps(v82, v81);
        v87 = _mm_add_ps(v85, _mm_mul_ps(v84, v81));
        v88 = _mm_add_ps(_mm_add_ps(_mm_shuffle_ps(v86, v86, 2), _mm_shuffle_ps(v86, v86, 1)), v86);
        v9[1] = _mm_shuffle_ps(
                  v87,
                  _mm_shuffle_ps(v87, _mm_sub_ps(_mm_mul_ps(v84, v83), _mm_shuffle_ps(v88, v88, 0)), 10),
                  132);
        ts3::scripting::FindSelectedBinding(self->host);
        if (auto* service=CallGameFunction<void*,0x005eb7c0>())
          v89 = CallGameMethod<uint32_t,0x005ec9e0>(service,static_cast<unsigned char*>(self->context)+2*sizeof(void*));
        else
          v89 = 0;
        if ( projector || v89 && CallGameFunction<uint8_t,0x0055da60>(v89 & 0xCF00) )
        {
          if ( (Global<uint32_t,0x011dc4a0>() & 1) == 0 )
          {
            Global<uint32_t,0x011dc4a0>() |= 1u;
            v122.m128_u64[1] = 0x3F8000003F800000LL;
            v122.m128_u64[0] = 1065353216;
            Global<__m128,0x011dc490>() = v122;
          }
          v90 = v9[1];
          v91 = _mm_shuffle_ps(v90, v90, 9);
          v92 = _mm_shuffle_ps(v90, v90, 18);
          v122.m128_i32[0] = 0;
          v93 = LowBits(0x3F80000000000000ULL);
          v122.m128_i32[3] = 0;
          *(unsigned __int64 *)((char *)v122.m128_u64 + 4) = 0x3F80000000000000LL;
          v94 = _mm_add_ps(
                  _mm_sub_ps(
                    _mm_mul_ps(_mm_shuffle_ps(v122, v122, 18), v91),
                    _mm_mul_ps(_mm_shuffle_ps(v122, v122, 9), v92)),
                  _mm_mul_ps(_mm_shuffle_ps(v90, v90, 255), v122));
          v95 = _mm_mul_ps(
                  _mm_add_ps(
                    _mm_mul_ps(
                      _mm_sub_ps(
                        _mm_mul_ps(_mm_shuffle_ps(v94, v94, 18), v91),
                        _mm_mul_ps(_mm_shuffle_ps(v94, v94, 9), v92)),
                      v124),
                    v122),
                  (__m128)Global<__m128,0x011dc490>());
          v96 = _mm_mul_ps(v95, v95);
          v97 = _mm_add_ps(_mm_add_ps(_mm_shuffle_ps(v96, v96, 2), _mm_shuffle_ps(v96, v96, 1)), v96);
          v98 = _mm_shuffle_ps(v97, v97, 0);
          v99 = _mm_rsqrt_ps(v98);
          v100 = _mm_shuffle_ps(LowBits(0x3F000000ULL), LowBits(0x3F000000ULL), 0);
          v101 = _mm_add_ps(
                   _mm_mul_ps(
                     _mm_sub_ps(constantTmp, _mm_mul_ps(_mm_mul_ps(v99, v99), v98)),
                     _mm_mul_ps(v99, v100)),
                   v99);
          v102 = _mm_mul_ps(
                   _mm_add_ps(
                     _mm_mul_ps(
                       _mm_sub_ps(constantTmp, _mm_mul_ps(_mm_mul_ps(v101, v101), v98)),
                       _mm_mul_ps(v101, v100)),
                     v101),
                   v95);
          v103 = _mm_sub_ps(
                   _mm_mul_ps(_mm_shuffle_ps(v102, v102, 18), _mm_shuffle_ps(v93, v93, 9)),
                   _mm_mul_ps(_mm_shuffle_ps(v102, v102, 9), _mm_shuffle_ps(v93, v93, 18)));
          v122 = _mm_shuffle_ps(v93, v93, 85);
          v124 = _mm_shuffle_ps(v103, v103, 0);
          v104 = _mm_add_ps(v124, v122);
          v105 = _mm_shuffle_ps(v102, v102, 170);
          v106 = _mm_add_ps(v105, v104);
          v139 = _mm_shuffle_ps(LowBits(0ULL), LowBits(0ULL), 0).m128_f32[0];

          if ( !(v106.m128_f32[0] > v139) )
          {

            if ( !(v124.m128_f32[0] > v122.m128_f32[0])
              || !(v124.m128_f32[0] > v105.m128_f32[0]) )
            {

              if ( !(v122.m128_f32[0] > v105.m128_f32[0]) )
              {
                v114 = _mm_sqrt_ps(_mm_add_ps(_mm_sub_ps(v105, v104), constantTmp));
                v115 = _mm_div_ps(v100, v114);
                v109 = _mm_shuffle_ps(
                         _mm_shuffle_ps(
                           _mm_mul_ps(
                             _mm_add_ps(_mm_shuffle_ps(v103, v103, 170), _mm_shuffle_ps(v102, v102, 0)),
                             v115),
                           _mm_mul_ps(
                             _mm_add_ps(
                               _mm_shuffle_ps(v102, v102, 85),
                               _mm_shuffle_ps(LowBits(0x3F80000000000000ULL), LowBits(0x3F80000000000000ULL), 170)),
                             v115),
                           16),
                         _mm_shuffle_ps(
                           _mm_mul_ps(v114, v100),
                           _mm_mul_ps(
                             _mm_sub_ps(
                               _mm_shuffle_ps(v103, v103, 85),
                               _mm_shuffle_ps(LowBits(0x3F80000000000000ULL), LowBits(0x3F80000000000000ULL), 0)),
                             v115),
                           50),
                         136);
              }
              else
              {
                v112 = _mm_sqrt_ps(_mm_add_ps(_mm_sub_ps(v122, _mm_add_ps(v105, v124)), constantTmp));
                v113 = _mm_div_ps(v100, v112);
                v109 = _mm_shuffle_ps(
                         _mm_shuffle_ps(
                           _mm_mul_ps(
                             _mm_add_ps(
                               _mm_shuffle_ps(v103, v103, 85),
                               _mm_shuffle_ps(LowBits(0x3F80000000000000ULL), LowBits(0x3F80000000000000ULL), 0)),
                             v113),
                           _mm_mul_ps(v112, v100),
                           16),
                         _mm_shuffle_ps(
                           _mm_mul_ps(
                             _mm_add_ps(
                               _mm_shuffle_ps(v102, v102, 85),
                               _mm_shuffle_ps(LowBits(0x3F80000000000000ULL), LowBits(0x3F80000000000000ULL), 170)),
                             v113),
                           _mm_mul_ps(
                             _mm_sub_ps(_mm_shuffle_ps(v102, v102, 0), _mm_shuffle_ps(v103, v103, 170)),
                             v113),
                           50),
                         136);
              }
            }
            else
            {
              v110 = _mm_sqrt_ps(_mm_add_ps(constantTmp, _mm_sub_ps(v124, _mm_add_ps(v105, v122))));
              v111 = _mm_div_ps(v100, v110);
              v109 = _mm_shuffle_ps(
                       _mm_shuffle_ps(
                         _mm_mul_ps(v110, v100),
                         _mm_mul_ps(
                           _mm_add_ps(
                             _mm_shuffle_ps(v103, v103, 85),
                             _mm_shuffle_ps(LowBits(0x3F80000000000000ULL), LowBits(0x3F80000000000000ULL), 0)),
                           v111),
                         16),
                       _mm_shuffle_ps(
                         _mm_mul_ps(
                           _mm_add_ps(_mm_shuffle_ps(v103, v103, 170), _mm_shuffle_ps(v102, v102, 0)),
                           v111),
                         _mm_mul_ps(
                           _mm_sub_ps(
                             _mm_shuffle_ps(LowBits(0x3F80000000000000ULL), LowBits(0x3F80000000000000ULL), 170),
                             _mm_shuffle_ps(v102, v102, 85)),
                           v111),
                         50),
                       136);
            }
          }
          else
          {
            v107 = _mm_sqrt_ps(_mm_add_ps(constantTmp, v106));
            v108 = _mm_div_ps(v100, v107);
            v109 = _mm_shuffle_ps(
                     _mm_shuffle_ps(
                       _mm_mul_ps(
                         _mm_sub_ps(
                           _mm_shuffle_ps(LowBits(0x3F80000000000000ULL), LowBits(0x3F80000000000000ULL), 170),
                           _mm_shuffle_ps(v102, v102, 85)),
                         v108),
                       _mm_mul_ps(
                         _mm_sub_ps(_mm_shuffle_ps(v102, v102, 0), _mm_shuffle_ps(v103, v103, 170)),
                         v108),
                       16),
                     _mm_shuffle_ps(
                       _mm_mul_ps(
                         _mm_sub_ps(
                           _mm_shuffle_ps(v103, v103, 85),
                           _mm_shuffle_ps(LowBits(0x3F80000000000000ULL), LowBits(0x3F80000000000000ULL), 0)),
                         v108),
                       _mm_mul_ps(v107, v100),
                       50),
                     136);
          }
          v9[2] = v109;
          v143[0] = v9[2];
          v143[1] = *v9;
          CallGameMethod<void,0x00959670>(v11,v143);
        }
        else
        {
          v9[2] = v9[1];
          v142[0] = v9[1];
          v142[1] = *v9;
          CallGameMethod<void,0x00959670>(v11,v142);
        }
      }
    }
}
}

namespace {
void* Interface(void* host,uint32_t mask) {
    return CallGameMethod<void*,0x00935770>(host,mask);
}
void EvaluateTracks(Component* self,Pose* pose,float delta,uint8_t first) {
    if (!first) return;
    b02::Registry registry;
    b02::InitRegistry(&registry);
    auto* providers=reinterpret_cast<ts3::resourcesystem::ResourceProviders*>(self->pose?self->pose->manager:nullptr);
    auto* node=providers->BuildAggregate(reinterpret_cast<ts3::resourcesystem::ResourceObjectPool*>(&registry),reinterpret_cast<uintptr_t>(self->host));
    b02::SetCurrent(&registry,node);
    if (b02::GetCurrent(&registry)) {
        auto* secondary=&self->secondaryVtable;
        if (CallGameVirtual<void*,8>(secondary)) {
            // 0083B080 constructs the opaque evaluation context. Only its observed
            // matrix/rig/time/flags prefix is written here; the rest stays opaque.
            alignas(16) unsigned char context[0x170];
            CallGameMethod<void,0x0083b080>(context);
            std::memcpy(context+0x50,&delta,4);
            void* rig=CallGameVirtual<void*,8>(secondary);
            __m128 q=_mm_load_ps(pose->vectors[1]);
            __m128 v25=_mm_mul_ps(_mm_set1_ps(FloatBits(0x3fb504f3)),q);
            __m128 v26=_mm_add_ps(_mm_mul_ps(_mm_sub_ps(_mm_setzero_ps(),v25),v25),_mm_set1_ps(0.5f));
            __m128 v27=_mm_mul_ps(_mm_shuffle_ps(v25,v25,9),v25);
            __m128 v28=_mm_mul_ps(_mm_shuffle_ps(v25,v25,255),_mm_shuffle_ps(v25,v25,18));
            __m128 v29=_mm_add_ps(v28,v27);
            __m128 v30=_mm_sub_ps(v27,v28);
            __m128 v31=_mm_add_ps(_mm_shuffle_ps(v26,v26,9),v26);
            __m128 v32=_mm_shuffle_ps(v31,v29,1);
            __m128 v33=_mm_shuffle_ps(_mm_shuffle_ps(v30,v31,32),v29,24);
            __m128 v34=_mm_shuffle_ps(_mm_shuffle_ps(v29,v30,18),v31,8);
            __m128 mask=Global<__m128,0x011af670>();
            __m128 v35=_mm_and_ps(_mm_shuffle_ps(v32,v30,40),mask);
            __m128 t=_mm_load_ps(pose->vectors[0]);
            __m128 one=_mm_set1_ps(1.0f);
            __m128 v36=_mm_shuffle_ps(t,_mm_shuffle_ps(t,one,250),132);
            auto* matrix=reinterpret_cast<__m128*>(context);
            matrix[0]=v35;matrix[1]=_mm_and_ps(v33,mask);matrix[2]=_mm_and_ps(v34,mask);matrix[3]=v36;
            std::memcpy(context+0x40,&rig,sizeof rig);
            uint32_t flag=1;std::memcpy(context+0x54,&flag,4);
            if (rig) {
                void* allocation=CallGameFunction<void*,0x005748a0>(0x19b30,Descriptor(),0,0,0,0);
                void* temp=nullptr;
                if (allocation) {
                    void* value=CallGameVirtual<void*,8>(secondary);
                    void* holder;
                    CallGameMethod<void,0x0074b950>(&holder,value);
                    temp=CallGameMethod<void*,0x00574f50>(allocation,holder);
                }
                if (temp) {
                    auto* refs=reinterpret_cast<int32_t*>(static_cast<unsigned char*>(temp)+sizeof(void*));
                    *refs=static_cast<int32_t>(static_cast<uint32_t>(*refs)+1u);
                }
                b02::Evaluate(&registry,reinterpret_cast<uintptr_t>(context),reinterpret_cast<uintptr_t>(temp));
                _mm_store_ps(pose->vectors[6],_mm_load_ps(reinterpret_cast<float*>(static_cast<unsigned char*>(temp)+0x20)));
                _mm_store_ps(pose->vectors[7],_mm_load_ps(reinterpret_cast<float*>(static_cast<unsigned char*>(temp)+0x10)));
                CallGameMethod<void,0x0055cfa0>(&temp);
            }
        }
    }
    m01::Destroy(&registry);
}
}
Pose* Update(Component* self,float delta) {
    if (self->pose) {
        Identity(self->pose->vectors[3]);Zero(self->pose->vectors[4]);Zero(self->pose->vectors[5]);
    }
    if (self->pose && self->pose->manager && delta>0.0f) {
        uint32_t hostFlags;
        // Host's published animation flags are a scalar, not a pointer-sized field.
        std::memcpy(&hostFlags,static_cast<unsigned char*>(self->host)+0x2c,4);
        bool diagnostic=(Global<uint32_t,0x011e90fc>()&hostFlags)!=0;
        auto* manager=static_cast<m100::Manager*>(self->pose->manager);
        if ((self->flags&0x80000000u) || b29::Count(reinterpret_cast<b29::Group*>(manager),nullptr)) {
            if (!(self->flags&0x40) && self->host) {
                if (auto* target=Interface(self->host,Global<uint32_t,0x011e9158>())) CallGameMethod<void,0x00959170>(target,1);
            }
            self->pose->idleTicks=0;
            uint8_t first=0,second=0;
            auto count=m100::Update(self->pose?static_cast<m100::Manager*>(self->pose->manager):nullptr,delta,&first,&second);
            if (self->host) {
                if (auto* target=Interface(self->host,Global<uint32_t,0x011e8fa8>())) {
                    uint32_t flag;std::memcpy(&flag,static_cast<unsigned char*>(target)+0x2c,4);
                    if (flag) CallGameMethod<void,0x00947590>(target,first==0,0);
                }
            }
            if (count) {
                self->flags&=~2u;
                if (second) CallGameMethod<void,0x00935820>(self->host);
                Pose* pose=self->pose;
                void* transform=self->host?Interface(self->host,Global<uint32_t,0x011e9158>()):nullptr;
                if (transform) {
                    __m128 translation=_mm_load_ps(static_cast<ts3::animation::ClipBinding*>(transform)->GetMatrix());
                    __m128 rotation=_mm_load_ps(reinterpret_cast<float*>(static_cast<ts3::misc::RecordKeyView*>(transform)->Key()));
                    __m128 diff=_mm_sub_ps(translation,_mm_load_ps(pose->vectors[0]));
                    __m128 minus=_mm_set1_ps(-1.0f),epsilon=_mm_set1_ps(FloatBits(0x37800000));
                    bool same=(_mm_movemask_ps(_mm_cmple_ps(_mm_max_ps(_mm_mul_ps(minus,diff),diff),epsilon))&7)==7;
                    diff=_mm_sub_ps(rotation,_mm_load_ps(pose->vectors[2])); // EA compares against +20, not the +10 destination.
                    if (!same || _mm_movemask_ps(_mm_cmple_ps(_mm_max_ps(_mm_mul_ps(minus,diff),diff),epsilon))!=15) {
                        _mm_store_ps(pose->vectors[0],translation);_mm_store_ps(pose->vectors[1],rotation);
                    }
                }
                Identity(pose->vectors[6]);Zero(pose->vectors[7]);
                void* projector=self->host?Interface(self->host,Global<uint32_t,0x011e90fc>()):nullptr;
                bool suppress=(self->flags&0x4000)!=0;self->flags&=~0x4000u;
                if (!suppress) EvaluateTracks(self,pose,delta,first);
                ApplyMotion(self,pose,transform,projector,delta);
            }
        } else {
            ++self->pose->idleTicks;
            if (self->pose->idleTicks>=Global<uint32_t,0x0117b7e8>() && !diagnostic) CallGameMethod<void,0x00940310>(self);
        }
    }
    if (self->followup) b26::Update(static_cast<b26::Manager*>(self->followup),nullptr,delta);
    Pose* result=self->pose;
    if (result) {
        Identity(self->pose->vectors[6]);Zero(self->pose->vectors[7]);
        self->pose->weights[1]=1.0f;self->pose->weights[0]=1.0f;
        Identity(self->pose->vectors[8]);Zero(self->pose->vectors[9]);
        result=self->pose;result->weights[3]=0.0f;self->pose->weights[2]=0.0f;
    }
    return result;
}
#if defined(_M_IX86)
Pose* __fastcall HookUpdate(Component* self,void*,float delta) { return Update(self,delta); }
#endif

}
