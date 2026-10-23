#include "ts3/anim/m05_M21.h"
#include "ts3/anim/m05_B07.h"
#include "ts3/anim/m05_B20.h"
#include "ts3/math/m04_B09.h"
#include "ts3/layout.h"
#include <emmintrin.h>
#include <cmath>
#include <cstring>

namespace ts3::anim::m21 {
namespace {
template<class T, uint32_t Address> T& Global() {
#if defined(_M_IX86)
    return *reinterpret_cast<T*>(Address);
#else
    NotRewritten("m05_M21 game global");
#endif
}
uint32_t& Bits(float& f) { return reinterpret_cast<uint32_t&>(f); }
uint32_t Bits(float&& f) { uint32_t bits; std::memcpy(&bits,&f,4); return bits; }
int32_t SignedBits(float f) { int32_t bits; std::memcpy(&bits,&f,4); return bits; }
float FromBits(uint32_t b) { float f; std::memcpy(&f,&b,4); return f; }
__m128 Bits128(uint64_t b) { return _mm_castsi128_ps(_mm_set_epi32(0,0,static_cast<int32_t>(b>>32),static_cast<int32_t>(b))); }
float Sin(float value) {
#if defined(_M_IX86)
    float output;
    __asm { fld value }
    __asm { fsin }
    __asm { fstp output }
    return output;
#else
    return std::sin(value);
#endif
}
float Cos(float value) {
#if defined(_M_IX86)
    float output;
    __asm { fld value }
    __asm { fcos }
    __asm { fstp output }
    return output;
#else
    return std::cos(value);
#endif
}
}
// SIMD temporary numbering follows the original kernel to make shuffle and
// rounding order auditable against 005625B0.asm. No algebraic reassociation.
uintptr_t Evaluate(Node* node, Context* context, Pose* pose)
{
    auto* children = &node->children;
    if (children->previous != children) {
        auto* link = children->next;
        auto* child = link ? reinterpret_cast<Node*>(reinterpret_cast<unsigned char*>(link)-offsetof(Node,link)) : nullptr;
        CallGameVirtual<void,2>(child,context,pose);
    }
    if (!(context->flags & 2)) {
        pose->flags &= 0xfffe;
        return reinterpret_cast<uintptr_t>(pose);
    }
    auto* self = reinterpret_cast<Evaluator*>(node);
    // RigResource embeds Skeleton at +28 on x86.
    auto joint = [context](const char* name) {
        return CallGameMethod<int32_t, 0x00575E30>(&context->rig->skeleton, name);
    };
    auto rotate = [](__m128* local, const __m128* bind, int32_t index, const void* axis, uint32_t angle) {
        b07::Rotate23F0(local, static_cast<const void*>(bind), index, static_cast<const Float4Value*>(axis), FromBits(angle));
    };
  alignas(16) __m128 inverse1[4];
  alignas(16) __m128 inverse0[4];
  alignas(16) __m128 samples[4];
  __m128 v3; // xmm1
  __m128 v4; // xmm2
  __m128 *v8; // esi
  const __m128* v9; // edi
  int v10; // eax
  __m128 v11; // xmm3
  int v12; // xmm4_4
  int f_34; // eax
  __m128 v14; // xmm1
  __m128 v15; // xmm0
  uintptr_t v16; // eax
  float v17; // xmm2_4
  float v18; // xmm1_4
  float v19; // xmm2_4
  float v20; // xmm2_4
  float v21; // xmm2_4
  __m128 v22; // xmm2
  float v23; // xmm1_4
  float v24; // xmm4_4
  float v25; // xmm0_4
  float v26; // xmm0_4
  float v27; // xmm0_4
  float v28; // xmm1_4
  float v29; // xmm0_4
  float v30; // xmm2_4
  float v31; // xmm0_4
  float v32; // xmm2_4
  float v33; // xmm2_4
  float v34; // xmm2_4
  int v35; // eax
  int v36; // eax
  int v37; // eax
  int v38; // eax
  uintptr_t result; // eax
  char f_40; // al
  int v41; // xmm0_4
  int v42; // xmm0_4
  int v43; // xmm0_4
  float v44; // xmm0_4
  void* v45; // eax
  __m128 v46; // xmm1
  __m128 v47; // xmm0
  __m128 v48; // xmm0
  __m128 v49; // xmm2
  __m128 v50; // xmm3
  __m128 v51; // xmm4
  __m128 v52; // xmm1
  __m128 v53; // xmm2
  __m128 v54; // xmm1
  __m128 v55; // xmm0
  __m128 v56; // xmm0
  __m128 v57; // xmm5
  __m128 v58; // xmm1
  __m128 v59; // xmm1
  __m128 v60; // xmm0
  __m128 v61; // xmm0
  __m128 v62; // xmm6
  __m128 v63; // xmm1
  __m128 v64; // xmm0
  __m128 v65; // xmm1
  __m128 v66; // xmm4
  __m128 v67; // xmm0
  __m128 v68; // xmm2
  bool v69; // cf
  __m128 v70; // xmm3
  __m128 v71; // xmm4
  __m128 v72; // xmm0
  __m128 v73; // xmm2
  __m128 v74; // xmm1
  __m128 v75; // xmm0
  __m128 v76; // xmm2
  __m128 *v77; // eax
  __m128 v78; // xmm6
  __m128 v79; // xmm0
  __m128 v80; // xmm1
  __m128 v81; // xmm0
  __m128 v82; // xmm0
  __m128 v83; // xmm3
  __m128 v84; // xmm1
  __m128 v85; // xmm2
  __m128 v86; // xmm1
  __m128 v87; // xmm3
  double v88; // st7
  int v89; // xmm0_4
  double v90; // st7
  __m128 v91; // xmm0
  __m128 v92; // xmm1
  __m128 v93; // xmm2
  __m128 v94; // xmm1
  int v95; // eax
  __m128 v96; // xmm2
  __m128 v97; // xmm4
  __m128 v98; // xmm3
  __m128 v99; // xmm0
  __m128 v100; // xmm1
  __m128 v101; // xmm2
  double v102; // st7
  __m128 v103; // xmm0
  __m128 v104; // xmm1
  float v105; // xmm1_4
  int v106; // xmm0_4
  float v107; // xmm2_4
  float v108; // xmm2_4
  float v109; // xmm2_4
  float v110; // xmm1_4
  __m128 v111; // xmm2
  __m128 v112; // xmm1
  float v113; // xmm2_4
  float v114; // xmm1_4
  float v115; // xmm2_4
  float v116; // xmm2_4
  float v117; // xmm2_4
  float v118; // xmm2_4
  float v119; // xmm2_4
  float v120; // xmm2_4
  float v121; // xmm1_4
  float v122; // xmm2_4
  float v123; // xmm2_4
  float v124; // xmm2_4
  float v125; // xmm1_4
  float v126; // xmm2_4
  float v127; // xmm2_4
  float v128; // xmm2_4
  float v129; // xmm2_4
  float v130; // xmm2_4
  float v131; // xmm2_4
  float v132; // xmm1_4
  int v133; // eax
  int v134; // eax
  int v135; // eax
  int v136; // eax
  int v137; // eax
  int v138; // eax
  int v139; // eax
  int v140; // eax
  __m128 v141; // xmm2
  __m128 v142; // xmm3
  __m128 v143; // xmm0
  int v144; // xmm1_4
  __m128 v145; // xmm1
  __m128 v146; // xmm1
  float v147; // xmm3_4
  float v148; // xmm3_4
  float v149; // xmm3_4
  float v150; // xmm3_4
  float v151; // xmm2_4
  __m128 v152; // xmm3
  __m128 v153; // xmm2
  float v154; // xmm2_4
  __m128 v155; // xmm1
  __m128 v156; // xmm3
  __m128 v157; // xmm1
  __m128 v158; // xmm1
  __m128 v159; // xmm1
  __m128 v160; // xmm5
  float v161; // xmm3_4
  float v162; // xmm3_4
  float v163; // xmm3_4
  float v164; // xmm3_4
  float v165; // xmm2_4
  __m128 v166; // xmm2
  float v167; // xmm0_4
  __m128 v168; // xmm2
  __m128 v169; // xmm3
  __m128 v170; // xmm4
  __m128 v171; // xmm1
  __m128 v172; // xmm3
  __m128 v173; // xmm4
  __m128 v174; // xmm0
  int v175; // eax
  __m128 v176; // xmm1
  __m128 v177; // xmm3
  __m128 v178; // xmm4
  __m128 v179; // xmm0
  int v180; // eax
  __m128 v181; // xmm1
  __m128 v182; // xmm2
  __m128 v183; // xmm3
  __m128 v184; // xmm0
  int v185; // eax
  int v186; // eax
  int v187; // eax
  int v188; // eax
  int v189; // eax
  float v190; // [esp+14h] [ebp-6F0h]
  int v191; // [esp+3Ch] [ebp-6C8h]
  float *v192; // [esp+3Ch] [ebp-6C8h]
  float v193; // [esp+3Ch] [ebp-6C8h]
  int v194; // [esp+3Ch] [ebp-6C8h]
  int v196; // [esp+40h] [ebp-6C4h]
  __m128 v197; // [esp+44h] [ebp-6C0h]
  __m128 v198; // [esp+44h] [ebp-6C0h]
  int v199; // [esp+54h] [ebp-6B0h]
  int v200; // [esp+54h] [ebp-6B0h]
  int v201; // [esp+54h] [ebp-6B0h]
  int v202; // [esp+58h] [ebp-6ACh]
  int v203; // [esp+58h] [ebp-6ACh]
  int *p_f_34; // [esp+5Ch] [ebp-6A8h]
  unsigned int v205; // [esp+5Ch] [ebp-6A8h]
  unsigned int v206; // [esp+5Ch] [ebp-6A8h]
  unsigned int v207; // [esp+5Ch] [ebp-6A8h]
  unsigned int v208; // [esp+5Ch] [ebp-6A8h]
  __int32 v209; // [esp+5Ch] [ebp-6A8h]
  float v210; // [esp+60h] [ebp-6A4h]
  unsigned int v211; // [esp+60h] [ebp-6A4h]
  float v212; // [esp+60h] [ebp-6A4h]
  float v213; // [esp+60h] [ebp-6A4h]
  int v214; // [esp+60h] [ebp-6A4h]
  float v215; // [esp+64h] [ebp-6A0h]
  unsigned int v216; // [esp+64h] [ebp-6A0h]
  unsigned int v217; // [esp+64h] [ebp-6A0h]
  unsigned int v218; // [esp+64h] [ebp-6A0h]
  unsigned int v219; // [esp+64h] [ebp-6A0h]
  float v220; // [esp+64h] [ebp-6A0h]
  unsigned int v221; // [esp+74h] [ebp-690h]
  unsigned int v222; // [esp+74h] [ebp-690h]
  __m128 v223; // [esp+74h] [ebp-690h]
  __m128 v224; // [esp+74h] [ebp-690h]
  __m128 v225; // [esp+74h] [ebp-690h]
  __m128 v226; // [esp+84h] [ebp-680h]
  __m128 v227; // [esp+84h] [ebp-680h]
  __m128 v228; // [esp+84h] [ebp-680h]
  __m128 v229; // [esp+84h] [ebp-680h]
  __m128 v230; // [esp+84h] [ebp-680h]
  float v231; // [esp+A0h] [ebp-664h]
  float v232; // [esp+A0h] [ebp-664h]
  __m128 v233; // [esp+A4h] [ebp-660h]
  __m128 v234; // [esp+A4h] [ebp-660h]
  __m128 v235; // [esp+A4h] [ebp-660h]
  __m128 v236; // [esp+A4h] [ebp-660h]
  __m128 v237; // [esp+A4h] [ebp-660h]
  void* v238; // [esp+C0h] [ebp-644h]
  float v239; // [esp+C4h] [ebp-640h]
  __m128 v240; // [esp+D4h] [ebp-630h]
  float smoothingRate;
  __m128 rootDelta;
  uint32_t routeHit[3];
  __m128& v244 = samples[0];
  __m128& v245 = samples[1];
  __m128& v246 = samples[2];
  __m128& v247 = samples[3];
  __m128 v248; // [esp+154h] [ebp-5B0h]
  __m128 v249; // [esp+164h] [ebp-5A0h] BYREF
  __m128 v250; // [esp+174h] [ebp-590h] BYREF
  __m128 v251; // [esp+184h] [ebp-580h] BYREF
  __m128 *v252; // [esp+1A0h] [ebp-564h]
  __m128 *v253; // [esp+1A4h] [ebp-560h]
  __m128 *v254; // [esp+1A8h] [ebp-55Ch]
  __m128 *v255; // [esp+1ACh] [ebp-558h]
  __m128 *v256; // [esp+1B0h] [ebp-554h]
  __m128 v257; // [esp+1B4h] [ebp-550h]
  __m128 *v258; // [esp+1D0h] [ebp-534h]
  __m128 v259; // [esp+1D4h] [ebp-530h]
  __m128 *v260; // [esp+1F0h] [ebp-514h]
  __m128 v261; // [esp+1F4h] [ebp-510h]
  __m128 *v262; // [esp+210h] [ebp-4F4h]
  __m128 v263; // [esp+214h] [ebp-4F0h]
  __m128 *v264; // [esp+230h] [ebp-4D4h]
  __m128 v265; // [esp+234h] [ebp-4D0h]
  __m128 *v266; // [esp+24Ch] [ebp-4B8h]
  __m128 *v267; // [esp+250h] [ebp-4B4h]
  __m128 *v268; // [esp+254h] [ebp-4B0h]
  __m128 *v269; // [esp+258h] [ebp-4ACh]
  __m128 *v270; // [esp+25Ch] [ebp-4A8h]
  __m128 *v271; // [esp+260h] [ebp-4A4h]
  __m128 v272; // [esp+264h] [ebp-4A0h]
  __m128 v273; // [esp+274h] [ebp-490h] BYREF
  alignas(16) __m128 inputMatrix[4];
  // Keep all matrix/sample rows contiguous: original helpers consume arrays.
  __m128& v274 = inputMatrix[0]; // [esp+284h] [ebp-480h] BYREF
  __m128& v275 = inputMatrix[1]; // [esp+294h] [ebp-470h]
  __m128& v276 = inputMatrix[2]; // [esp+2A4h] [ebp-460h]
  __m128& v277 = inputMatrix[3]; // [esp+2B4h] [ebp-450h]
  __m128 v278; // [esp+2C4h] [ebp-440h]
  __m128 v279; // [esp+2D4h] [ebp-430h]
  __m128 v280; // [esp+2E4h] [ebp-420h]
  __m128 v281; // [esp+2F4h] [ebp-410h]
  __m128 v282; // [esp+304h] [ebp-400h]
  __m128 v283; // [esp+314h] [ebp-3F0h]
  __m128 v284; // [esp+324h] [ebp-3E0h] BYREF
  __m128 v285; // [esp+334h] [ebp-3D0h] BYREF
  __m128 v286; // [esp+344h] [ebp-3C0h] BYREF
  __m128 v287; // [esp+354h] [ebp-3B0h] BYREF
  __m128 v288; // [esp+364h] [ebp-3A0h] BYREF
  __m128 v289; // [esp+374h] [ebp-390h] BYREF
  float v290; // [esp+384h] [ebp-380h]
  __m128 neckAxis;
  __m128 v292; // [esp+3A4h] [ebp-360h] BYREF
  __m128 v293; // [esp+3B4h] [ebp-350h] BYREF
  __m128 v294; // [esp+3C4h] [ebp-340h] BYREF
  __m128 v295; // [esp+3D4h] [ebp-330h] BYREF
  __m128 v296; // [esp+3E4h] [ebp-320h] BYREF
  __m128 v297; // [esp+3F4h] [ebp-310h] BYREF
  __m128 v298; // [esp+404h] [ebp-300h] BYREF
  __m128 v299; // [esp+414h] [ebp-2F0h] BYREF
  __m128 v300; // [esp+424h] [ebp-2E0h] BYREF
  __m128 v301; // [esp+434h] [ebp-2D0h] BYREF
  float v302; // [esp+444h] [ebp-2C0h]
  __m128 v303; // [esp+454h] [ebp-2B0h] BYREF
  __m128& v304 = inverse0[0];
  __m128& v305 = inverse0[1];
  __m128& v306 = inverse0[2];
  __m128& v307 = inverse0[3];
  __m128 v308; // [esp+4A4h] [ebp-260h] BYREF
  __m128 v309; // [esp+4B4h] [ebp-250h] BYREF
  __m128 v310; // [esp+4C4h] [ebp-240h] BYREF
  __m128& v311 = inverse1[0];
  __m128& v312 = inverse1[1];
  __m128& v313 = inverse1[2];
  __m128& v314 = inverse1[3];
  __m128 v315[4]; // [esp+514h] [ebp-1F0h] BYREF
  __m128 v316[4]; // [esp+554h] [ebp-1B0h] BYREF
  __m128 v317[4]; // [esp+594h] [ebp-170h]
  __m128 v318[5]; // [esp+5D4h] [ebp-130h] BYREF
  __m128 v319; // [esp+624h] [ebp-E0h]
  __m128 v320; // [esp+634h] [ebp-D0h]
  __m128 v321; // [esp+644h] [ebp-C0h]
  __m128 v322; // [esp+654h] [ebp-B0h]
  __m128 v323; // [esp+664h] [ebp-A0h]
  __m128 v324; // [esp+674h] [ebp-90h] BYREF
  __m128 v325; // [esp+684h] [ebp-80h] BYREF
  __m128 v326; // [esp+694h] [ebp-70h]
  __m128 v327; // [esp+6A4h] [ebp-60h]
  __m128 v328; // [esp+6B4h] [ebp-50h]
  __m128 v329; // [esp+6C4h] [ebp-40h]
  __m128 v330; // [esp+6D4h] [ebp-30h]
  __m128 v331[2]; // [esp+6E4h] [ebp-20h]

  v8 = (__m128 *)reinterpret_cast<__m128*>(b20::Local(pose));
  CallGameMethod<void, 0x00574990>(pose);
  v9 = CallGameMethod<__m128*, 0x005749B0>(pose);
  v10 = Global<uint32_t, 0x011CD4D8>();
  if ( (Global<uint32_t, 0x011CD4D8>() & 1) == 0 )
  {
    v4.m128_f32[0] = 0.017453292;
    v4 = _mm_shuffle_ps(v4, v4, 0);
    *(float *)&v221 = _mm_shuffle_ps(Bits128(Global<uint32_t, 0x0114F424>()), Bits128(Global<uint32_t, 0x0114F424>()), 0).m128_f32[0]
                    * v4.m128_f32[0];
    v3 = Bits128(v221);
    v10 = Global<uint32_t, 0x011CD4D8>() | 1;
    Global<uint32_t, 0x011CD4D8>() |= 1u;
    Global<uint32_t, 0x011CD4D4>() = v221;
  }
  if ( (v10 & 2) != 0 )
  {
    v11 = Bits128(Global<uint32_t, 0x011CD4D0>());
  }
  else
  {
    v3.m128_f32[0] = 0.017453292;
    v4 = _mm_shuffle_ps(Bits128(0x41700000u), Bits128(0x41700000u), 0);
    *(float *)&v222 = _mm_shuffle_ps(v3, v3, 0).m128_f32[0] * v4.m128_f32[0];
    v11 = Bits128(v222);
    v10 |= 2u;
    Global<uint32_t, 0x011CD4D8>() = v10;
    Global<uint32_t, 0x011CD4D0>() = v222;
  }
  if ( (v10 & 4) == 0 )
  {
    v4.m128_f32[0] = 0.017453292;
    v10 |= 4u;
    Global<uint32_t, 0x011CD4D8>() = v10;
    *(float *)&Global<uint32_t, 0x011CD4CC>() = _mm_shuffle_ps(
                                 Bits128(Global<uint32_t, 0x0114F428>()),
                                 Bits128(Global<uint32_t, 0x0114F428>()),
                                 0).m128_f32[0]
                             * _mm_shuffle_ps(v4, v4, 0).m128_f32[0];
  }
  if ( (v10 & 8) != 0 )
  {
    v12 = Global<uint32_t, 0x011CD4C8>();
  }
  else
  {
    *(float *)&v12 = _mm_shuffle_ps(Bits128(0x3C8EFA35u), Bits128(0x3C8EFA35u), 0).m128_f32[0]
                   * _mm_shuffle_ps(Bits128(0x41F00000u), Bits128(0x41F00000u), 0).m128_f32[0];
    Global<uint32_t, 0x011CD4D8>() = v10 | 8;
    Global<uint32_t, 0x011CD4C8>() = v12;
  }
  // 00562704: species without terrain fitting use scene yaw and four weights.
  p_f_34 = reinterpret_cast<int*>(&self->f_34);
  f_34 = self->f_34;
  if ( (f_34 == 256 || f_34 == 0x10000 || f_34 == 2304 || f_34 == 2560 || f_34 == 2816) && !self->f_40 )
  {
    v14 = _mm_setzero_ps();
    v15 = _mm_setzero_ps();
    if ( (uint64_t(self->f_38) | (uint64_t(self->f_3C) << 32)) )
    {
      v16 = reinterpret_cast<uintptr_t>(CallGameVirtual<void*, 0>(context->scene, self->f_38, self->f_3C));
      void* render = v16 ? CallGameMethod<void*, 0x006F4A50>(reinterpret_cast<void*>(v16)) : nullptr;
      if (render && CallGameMethod<void*, 0x00573D40>(render, uint32_t(0)))
        v15 = Bits128(*reinterpret_cast<uint32_t*>(static_cast<unsigned char*>(CallGameMethod<void*, 0x00573D40>(render, uint32_t(0))) + 96));
      else
        v15 = _mm_setzero_ps();
      v14 = _mm_setzero_ps();
      v12 = Global<uint32_t, 0x011CD4C8>();
      v11 = Bits128(Global<uint32_t, 0x011CD4D0>());
    }
    v196 = v191 = v199 = 0;
    // COMISS/Jcc's unordered path reaches 00562BC7 with all four weights
    // still zero; it is not the negative-angle saturation path.
    if (std::isnan(v15.m128_f32[0])) {
      v15 = _mm_setzero_ps();
    }
    else if ( (float)(-0.0 - *(float *)&v12) <= v15.m128_f32[0] )
    {
      if ( (float)(-0.0 - v11.m128_f32[0]) <= v15.m128_f32[0] )
      {
        if ( v14.m128_f32[0] <= v15.m128_f32[0] )
        {
          if ( v11.m128_f32[0] <= v15.m128_f32[0] )
          {
            if ( *(float *)&v12 < v15.m128_f32[0] )
            {
              v196 = Global<uint32_t, 0x011CD4A4>();
              v191 = Global<uint32_t, 0x0114F4A0>();
              v199 = Global<uint32_t, 0x0114F4A4>();
              v15.m128_i32[0] = Global<uint32_t, 0x0114F4A8>();
            }
            else
            {
              v30 = _mm_shuffle_ps(Bits128(Global<uint32_t, 0x011CD4A0>()), Bits128(Global<uint32_t, 0x011CD4A0>()), 0).m128_f32[0];
              v15.m128_f32[0] = (float)(v15.m128_f32[0] - v11.m128_f32[0]) / v11.m128_f32[0];
              v31 = _mm_shuffle_ps(v15, v15, 0).m128_f32[0];
              *(float *)&v196 = (float)((float)(_mm_shuffle_ps(
                                                  Bits128(Global<uint32_t, 0x011CD4A4>()),
                                                  Bits128(Global<uint32_t, 0x011CD4A4>()),
                                                  0).m128_f32[0]
                                              - v30)
                                      * v31)
                              + v30;
              v32 = _mm_shuffle_ps(Bits128(Global<uint32_t, 0x0114F494>()), Bits128(Global<uint32_t, 0x0114F494>()), 0).m128_f32[0];
              *(float *)&v191 = (float)((float)(_mm_shuffle_ps(
                                                  Bits128(Global<uint32_t, 0x0114F4A0>()),
                                                  Bits128(Global<uint32_t, 0x0114F4A0>()),
                                                  0).m128_f32[0]
                                              - v32)
                                      * v31)
                              + v32;
              v33 = _mm_shuffle_ps(Bits128(Global<uint32_t, 0x0114F498>()), Bits128(Global<uint32_t, 0x0114F498>()), 0).m128_f32[0];
              *(float *)&v199 = (float)((float)(_mm_shuffle_ps(
                                                  Bits128(Global<uint32_t, 0x0114F4A4>()),
                                                  Bits128(Global<uint32_t, 0x0114F4A4>()),
                                                  0).m128_f32[0]
                                              - v33)
                                      * v31)
                              + v33;
              v34 = _mm_shuffle_ps(Bits128(Global<uint32_t, 0x0114F49C>()), Bits128(Global<uint32_t, 0x0114F49C>()), 0).m128_f32[0];
              v15.m128_f32[0] = (float)((float)(_mm_shuffle_ps(
                                                  Bits128(Global<uint32_t, 0x0114F4A8>()),
                                                  Bits128(Global<uint32_t, 0x0114F4A8>()),
                                                  0).m128_f32[0]
                                              - v34)
                                      * v31)
                              + v34;
            }
          }
          else
          {
            v15.m128_f32[0] = v15.m128_f32[0] / v11.m128_f32[0];
            v28 = _mm_shuffle_ps(v14, v14, 0).m128_f32[0];
            v29 = _mm_shuffle_ps(v15, v15, 0).m128_f32[0];
            *(float *)&v196 = (float)((float)(_mm_shuffle_ps(
                                                Bits128(Global<uint32_t, 0x011CD4A0>()),
                                                Bits128(Global<uint32_t, 0x011CD4A0>()),
                                                0).m128_f32[0]
                                            - v28)
                                    * v29)
                            + v28;
            *(float *)&v191 = (float)((float)(_mm_shuffle_ps(
                                                Bits128(Global<uint32_t, 0x0114F494>()),
                                                Bits128(Global<uint32_t, 0x0114F494>()),
                                                0).m128_f32[0]
                                            - v28)
                                    * v29)
                            + v28;
            *(float *)&v199 = (float)((float)(_mm_shuffle_ps(
                                                Bits128(Global<uint32_t, 0x0114F498>()),
                                                Bits128(Global<uint32_t, 0x0114F498>()),
                                                0).m128_f32[0]
                                            - v28)
                                    * v29)
                            + v28;
            v15.m128_f32[0] = (float)((float)(_mm_shuffle_ps(
                                                Bits128(Global<uint32_t, 0x0114F49C>()),
                                                Bits128(Global<uint32_t, 0x0114F49C>()),
                                                0).m128_f32[0]
                                            - v28)
                                    * v29)
                            + v28;
          }
        }
        else
        {
          v22 = v11;
          v22.m128_f32[0] = (float)(v11.m128_f32[0] + v15.m128_f32[0]) / v11.m128_f32[0];
          v23 = _mm_shuffle_ps(v14, v14, 0).m128_f32[0];
          v24 = _mm_shuffle_ps(Bits128(Global<uint32_t, 0x0114F484>()), Bits128(Global<uint32_t, 0x0114F484>()), 0).m128_f32[0];
          v22.m128_f32[0] = _mm_shuffle_ps(v22, v22, 0).m128_f32[0];
          *(float *)&v196 = (float)((float)(v23 - v24) * v22.m128_f32[0]) + v24;
          v25 = _mm_shuffle_ps(Bits128(Global<uint32_t, 0x0114F488>()), Bits128(Global<uint32_t, 0x0114F488>()), 0).m128_f32[0];
          *(float *)&v191 = (float)((float)(v23 - v25) * v22.m128_f32[0]) + v25;
          v26 = _mm_shuffle_ps(Bits128(Global<uint32_t, 0x0114F48C>()), Bits128(Global<uint32_t, 0x0114F48C>()), 0).m128_f32[0];
          *(float *)&v199 = (float)((float)(v23 - v26) * v22.m128_f32[0]) + v26;
          v27 = _mm_shuffle_ps(Bits128(Global<uint32_t, 0x0114F490>()), Bits128(Global<uint32_t, 0x0114F490>()), 0).m128_f32[0];
          v15.m128_f32[0] = (float)((float)(v23 - v27) * v22.m128_f32[0]) + v27;
        }
      }
      else
      {
        v17 = _mm_shuffle_ps(Bits128(Global<uint32_t, 0x0114F474>()), Bits128(Global<uint32_t, 0x0114F474>()), 0).m128_f32[0];
        v14.m128_f32[0] = (float)(*(float *)&v12 + v15.m128_f32[0]) / v11.m128_f32[0];
        v18 = _mm_shuffle_ps(v14, v14, 0).m128_f32[0];
        *(float *)&v196 = (float)((float)(_mm_shuffle_ps(
                                            Bits128(Global<uint32_t, 0x0114F484>()),
                                            Bits128(Global<uint32_t, 0x0114F484>()),
                                            0).m128_f32[0]
                                        - v17)
                                * v18)
                        + v17;
        v19 = _mm_shuffle_ps(Bits128(Global<uint32_t, 0x0114F478>()), Bits128(Global<uint32_t, 0x0114F478>()), 0).m128_f32[0];
        *(float *)&v191 = (float)((float)(_mm_shuffle_ps(
                                            Bits128(Global<uint32_t, 0x0114F488>()),
                                            Bits128(Global<uint32_t, 0x0114F488>()),
                                            0).m128_f32[0]
                                        - v19)
                                * v18)
                        + v19;
        v20 = _mm_shuffle_ps(Bits128(Global<uint32_t, 0x0114F47C>()), Bits128(Global<uint32_t, 0x0114F47C>()), 0).m128_f32[0];
        *(float *)&v199 = (float)((float)(_mm_shuffle_ps(
                                            Bits128(Global<uint32_t, 0x0114F48C>()),
                                            Bits128(Global<uint32_t, 0x0114F48C>()),
                                            0).m128_f32[0]
                                        - v20)
                                * v18)
                        + v20;
        v21 = _mm_shuffle_ps(Bits128(Global<uint32_t, 0x0114F480>()), Bits128(Global<uint32_t, 0x0114F480>()), 0).m128_f32[0];
        v15.m128_f32[0] = (float)((float)(_mm_shuffle_ps(
                                            Bits128(Global<uint32_t, 0x0114F490>()),
                                            Bits128(Global<uint32_t, 0x0114F490>()),
                                            0).m128_f32[0]
                                        - v21)
                                * v18)
                        + v21;
      }
    }
    else
    {
      v196 = Global<uint32_t, 0x0114F474>();
      v191 = Global<uint32_t, 0x0114F478>();
      v15.m128_i32[0] = Global<uint32_t, 0x0114F47C>();
      v199 = Global<uint32_t, 0x0114F47C>();
    }
    v35 = joint("b__Neck__");
    v226.m128_u64[0] = 0;
    v226.m128_u64[1] = 1065353216;
    v233 = _mm_sub_ps(Bits128(0LL), v226);
    v289 = v233;
    rotate(v8, v9, v35, &v289, v196);
    v36 = joint("b__Head__");
    v299 = v233;
    rotate(v8, v9, v36, &v299, v191);
    v37 = joint("b__Spine0__");
    v292 = v233;
    rotate(v8, v9, v37, &v292, v199);
    v38 = joint("b__Spine1__");
    v308 = v233;
    rotate(v8, v9, v38, &v308, v15.m128_i32[0]);
    goto LABEL_135;
  }
  // 00562CE3: rejected world/route paths return residual EAX, without cleanup.
  result = reinterpret_cast<uintptr_t>(Global<float*, 0x011ECBC4>());
  v192 = Global<float*, 0x011ECBC4>();
  if ( Global<float*, 0x011ECBC4>() )
  {
    f_40 = self->f_40;
    if ( f_40 )
      *(float *)&v41 = 1.0;
    else
      *(float *)&v41 = 0.30000001;
    v215 = *(float *)&v41;
    if ( f_40 )
      *(float *)&v42 = -1.4;
    else
      *(float *)&v42 = -1.2;
    v231 = *(float *)&v42;
    if ( f_40 )
      *(float *)&v43 = 0.5;
    else
      *(float *)&v43 = 0.125;
    v210 = *(float *)&v43;
    v238 = CallGameFunction<void*, 0x005EB7C0>();
    if ( v238 && !self->f_40 )
    {
      v215 = CallGameMethod<float, 0x005ECD80>(v238, (unsigned int)*p_f_34 | self->f_30);
      v231 = CallGameMethod<float, 0x005ECDD0>(v238, self->f_30 | (unsigned int)*p_f_34);
      v210 = CallGameMethod<float, 0x005ECE20>(v238, self->f_30 | (unsigned int)*p_f_34);
    }
    result = reinterpret_cast<uintptr_t>(v192);
    v274 = context->matrix[0];
    v275 = context->matrix[1];
    v276 = context->matrix[2];
    v44 = context->matrix[3].m128_f32[0];
    v277 = context->matrix[3];
    v250 = v277;
    if ( v277.m128_f32[0] >= v192[45]
      && v277.m128_f32[2] >= v192[46]
      && v192[47] > v44
      && (float)(v192[49] + v192[48]) > v277.m128_f32[2]
      && (_mm_movemask_ps(
            _mm_cmple_ps(
              _mm_max_ps(_mm_mul_ps(_mm_shuffle_ps(Bits128(0xBF800000), Bits128(0xBF800000), 0), v277), v277),
              _mm_shuffle_ps(Bits128(0x34000000u), Bits128(0x34000000u), 0)))
        & 7) != 7 )
    {
      v45 = CallGameMethod<void*, 0x00C68990>(v192, v277.m128_f32[0], v277.m128_f32[2]);
      routeHit[0] = routeHit[1] = 0xbf800000u;
      routeHit[2] = 0;
      if ( !v45 || (result = CallGameFunction<uintptr_t, 0x00ADD800>(v45, &v250, routeHit), !routeHit[2]) )
      {
        v278 = v276;
        v280 = v274;
        v279 = v275;
        v279.m128_i32[0] = 0;
        v278.m128_i32[1] = 0;
        v280.m128_i32[1] = 0;
        v257 = v279;
        v257.m128_i32[2] = 0;
        v46 = _mm_mul_ps(v278, v278);
        v47 = _mm_add_ps(_mm_add_ps(_mm_shuffle_ps(v46, v46, 2), _mm_shuffle_ps(v46, v46, 1)), v46);
        v48 = _mm_shuffle_ps(v47, v47, 0);
        v49 = _mm_rsqrt_ps(v48);
        v50 = _mm_shuffle_ps(Bits128(0x3F800000u), Bits128(0x3F800000u), 0);
        v51 = _mm_shuffle_ps(Bits128(0x3F000000u), Bits128(0x3F000000u), 0);
        v52 = _mm_add_ps(_mm_mul_ps(_mm_sub_ps(v50, _mm_mul_ps(_mm_mul_ps(v49, v49), v48)), _mm_mul_ps(v49, v51)), v49);
        v53 = _mm_mul_ps(
                _mm_add_ps(
                  _mm_mul_ps(_mm_sub_ps(v50, _mm_mul_ps(_mm_mul_ps(v52, v52), v48)), _mm_mul_ps(v52, v51)),
                  v52),
                v278);
        v54 = _mm_mul_ps(v280, v280);
        v55 = _mm_add_ps(_mm_add_ps(_mm_shuffle_ps(v54, v54, 2), _mm_shuffle_ps(v54, v54, 1)), v54);
        v56 = _mm_shuffle_ps(v55, v55, 0);
        v57 = _mm_rsqrt_ps(v56);
        v58 = _mm_add_ps(_mm_mul_ps(_mm_sub_ps(v50, _mm_mul_ps(_mm_mul_ps(v57, v57), v56)), _mm_mul_ps(v57, v51)), v57);
        v223 = v50;
        v240 = v51;
        v234 = v53;
        v248 = _mm_mul_ps(
                 _mm_add_ps(
                   _mm_mul_ps(_mm_sub_ps(v50, _mm_mul_ps(_mm_mul_ps(v58, v58), v56)), _mm_mul_ps(v58, v51)),
                   v58),
                 v280);
        v59 = _mm_mul_ps(v257, v257);
        v60 = _mm_add_ps(_mm_add_ps(_mm_shuffle_ps(v59, v59, 2), _mm_shuffle_ps(v59, v59, 1)), v59);
        v61 = _mm_shuffle_ps(v60, v60, 0);
        v62 = _mm_rsqrt_ps(v61);
        v63 = _mm_add_ps(_mm_mul_ps(_mm_sub_ps(v50, _mm_mul_ps(_mm_mul_ps(v62, v62), v61)), _mm_mul_ps(v62, v51)), v62);
        v197 = _mm_mul_ps(
                 _mm_add_ps(
                   _mm_mul_ps(_mm_sub_ps(v50, _mm_mul_ps(_mm_mul_ps(v63, v63), v61)), _mm_mul_ps(v63, v51)),
                   v63),
                 v257);
        v64 = _mm_add_ps(_mm_mul_ps(_mm_shuffle_ps(Bits128(Bits(v215)), Bits128(Bits(v215)), 0), v53), v250);
        v65 = _mm_mul_ps(_mm_shuffle_ps(Bits128(Bits(v210)), Bits128(Bits(v210)), 0), v248);
        v66 = _mm_add_ps(v65, v64);
        v317[1] = _mm_sub_ps(v64, v65);
        v67 = _mm_add_ps(_mm_mul_ps(_mm_shuffle_ps(Bits128(Bits(v231)), Bits128(Bits(v231)), 0), v53), v250);
        v317[0] = v66;
        v317[2] = _mm_add_ps(v67, v65);
        v317[3] = _mm_sub_ps(v67, v65);
        // 00563130: four terrain samples, then plane fitting or average-height fallback.
        v216 = 0;
        do
        {
          v227 = v317[v216 / 4];
          samples[v216 / 4] = v227;
          *(float *)&v211 = CallGameMethod<float, 0x00C61200>(v192, v227.m128_f32[0], v227.m128_f32[2], 0, 1, 1, 0, 0);
          v68 = samples[v216 / 4];
          v68.m128_f32[1] = FromBits(v211);
          samples[v216 / 4] = v68;
          v69 = v216 * 4 + 16 < 0x40;
          v216 += 4;
        }
        while ( v69 );
        v70 = v244;
        v71 = _mm_shuffle_ps(Bits128(0xC2C80000), Bits128(0xC2C80000), 0);
        v72 = v245;
        v267 = &v244;
        if ( _mm_movemask_ps(_mm_cmpeq_ps(_mm_shuffle_ps(v244, v244, 85), v71)) != 15 )
        {
          v73 = v246;
          v256 = &v245;
          if ( _mm_movemask_ps(_mm_cmpeq_ps(_mm_shuffle_ps(v245, v245, 85), v71)) == 15 )
          {
LABEL_71:
            v74 = v247;
LABEL_72:
            v75 = _mm_add_ps(_mm_shuffle_ps(v72, v72, 85), _mm_shuffle_ps(v70, v70, 85));
            v322 = _mm_add_ps(_mm_shuffle_ps(v73, v73, 85), v75);
            v76 = v223;
            v318[4] = v75;
            v326 = _mm_add_ps(v322, _mm_shuffle_ps(v74, v74, 85));
            v268 = &v247;
            v260 = &v246;
            v266 = &v245;
            v262 = &v244;
            v284 = _mm_shuffle_ps(
                     _mm_shuffle_ps(Bits128(0LL), v223, 16),
                     _mm_shuffle_ps(
                       Bits128(0LL),
                       _mm_sub_ps(
                         Bits128(0LL),
                         _mm_mul_ps(v326, _mm_shuffle_ps(Bits128(0x3E800000u), Bits128(0x3E800000u), 0))),
                       50),
                     136);
LABEL_74:
            if ( !self->f_40 || self->f_41 )
            {
              v77 = &v284;
            }
            else
            {
              v310 = Bits128(0x3F80000000000000uLL);
              v77 = (__m128 *)&v310;
            }
            v78 = *v77;
            v79 = _mm_shuffle_ps(v78, v78, 85);
            v290 = v79.m128_f32[0];
            v79.m128_f32[0] = 0.0;
            rootDelta = _mm_shuffle_ps(v79, v79, 0);
            v302 = rootDelta.m128_f32[0];
            v253 = &v273;
            if ( rootDelta.m128_f32[0] > v290 )
              v78 = _mm_sub_ps(Bits128(0LL), v78);
            v80 = _mm_mul_ps(v78, v78);
            v81 = _mm_add_ps(_mm_add_ps(_mm_shuffle_ps(v80, v80, 2), _mm_shuffle_ps(v80, v80, 1)), v80);
            v82 = _mm_shuffle_ps(v81, v81, 0);
            v83 = _mm_rsqrt_ps(v82);
            v84 = _mm_add_ps(
                    _mm_mul_ps(_mm_sub_ps(v76, _mm_mul_ps(_mm_mul_ps(v83, v83), v82)), _mm_mul_ps(v240, v83)),
                    v83);
            v85 = _mm_mul_ps(
                    _mm_add_ps(
                      _mm_mul_ps(_mm_sub_ps(v76, _mm_mul_ps(_mm_mul_ps(v84, v84), v82)), _mm_mul_ps(v240, v84)),
                      v84),
                    v78);
            v86 = _mm_shuffle_ps(v85, v85, 18);
            v87 = _mm_shuffle_ps(v85, v85, 9);
            v263 = v85;
            v273 = v85;
            v263.m128_i32[3] = 0;
            v330 = v263;
            v265 = _mm_sub_ps(
                     _mm_mul_ps(_mm_shuffle_ps(v234, v234, 18), v87),
                     _mm_mul_ps(_mm_shuffle_ps(v234, v234, 9), v86));
            v261 = _mm_sub_ps(
                     _mm_mul_ps(_mm_shuffle_ps(v265, v265, 9), v86),
                     _mm_mul_ps(_mm_shuffle_ps(v265, v265, 18), v87));
            v261.m128_i32[3] = 0;
            v331[0] = v261;
            v259 = v250;
            v265.m128_i32[3] = 0;
            v259.m128_i32[3] = 1065353216;
            v329 = v265;
            v331[1] = v259;
            math::b09::Kernel561E60(&v304, &v274, &v324);
            v316[0] = _mm_add_ps(
                        _mm_add_ps(
                          _mm_add_ps(
                            _mm_mul_ps(_mm_shuffle_ps(v265, v265, 0), v304),
                            _mm_mul_ps(_mm_shuffle_ps(v265, v265, 85), v305)),
                          _mm_mul_ps(_mm_shuffle_ps(v265, v265, 170), v306)),
                        _mm_mul_ps(_mm_shuffle_ps(v265, v265, 255), v307));
            v316[1] = _mm_add_ps(
                        _mm_add_ps(
                          _mm_add_ps(
                            _mm_mul_ps(_mm_shuffle_ps(v263, v263, 0), v304),
                            _mm_mul_ps(v305, _mm_shuffle_ps(v263, v263, 85))),
                          _mm_mul_ps(v306, _mm_shuffle_ps(v263, v263, 170))),
                        _mm_mul_ps(v307, _mm_shuffle_ps(v263, v263, 255)));
            v316[2] = _mm_add_ps(
                        _mm_add_ps(
                          _mm_add_ps(
                            _mm_mul_ps(_mm_shuffle_ps(v261, v261, 0), v304),
                            _mm_mul_ps(v305, _mm_shuffle_ps(v261, v261, 85))),
                          _mm_mul_ps(v306, _mm_shuffle_ps(v261, v261, 170))),
                        _mm_mul_ps(_mm_shuffle_ps(v261, v261, 255), v307));
            v316[3] = _mm_add_ps(
                        _mm_add_ps(
                          _mm_add_ps(
                            _mm_mul_ps(v304, _mm_shuffle_ps(v259, v259, 0)),
                            _mm_mul_ps(v305, _mm_shuffle_ps(v259, v259, 85))),
                          _mm_mul_ps(v306, _mm_shuffle_ps(v259, v259, 170))),
                        _mm_mul_ps(_mm_shuffle_ps(v259, v259, 255), v307));
            v88 = math::b09::Kernel5621D0(v316);
            // 005636D4: smooth the projected angle, then rotate local root SRT.
            v224 = context->smoothed;
            if ( self->f_40 && self->f_42 )
              v89 = Global<uint32_t, 0x0114F51C>();
            else
              v89 = Global<uint32_t, 0x0114F540>();
            Bits(smoothingRate) = v89;
            v190 = v88;
            v90 = CallGameFunction<double, 0x0055DE20>(v224.m128_f32[0], v190, context->deltaTime, 1.0f, &smoothingRate);
            context->dirty = 1;
            v212 = v90;
            *(float *)&v217 = v90;
            v91 = _mm_shuffle_ps(Bits128(v217), Bits128(v217), 0);
            context->angle = v91;
            v249 = Bits128(0x3F800000u);
            v92 = _mm_shuffle_ps(Bits128(0x40000000u), Bits128(0x40000000u), 0);
            v225 = v92;
            *(float *)&v218 = Sin((float)(v91.m128_f32[0] / v92.m128_f32[0]));
            v93 = _mm_shuffle_ps(Bits128(v218), Bits128(v218), 0);
            *(float *)&v219 = Cos((float)(v91.m128_f32[0] / v92.m128_f32[0]));
            v255 = &v249;
            v323 = _mm_mul_ps(_mm_shuffle_ps(v249, v249, 0), v93);
            v319 = _mm_mul_ps(_mm_shuffle_ps(v249, v249, 170), v93);
            v252 = &v249;
            v321 = _mm_mul_ps(_mm_shuffle_ps(v249, v249, 85), v93);
            v269 = &v249;
            v94 = _mm_shuffle_ps(
                    _mm_shuffle_ps(v323, v321, 16),
                    _mm_shuffle_ps(v319, _mm_shuffle_ps(Bits128(v219), Bits128(v219), 0), 50),
                    136);
            if ( self->f_40 )
              v95 = joint("b__ROOT_bind__");
            else
              v95 = 0;
            v96 = v8[3 * v95 + 1];
            v97 = _mm_shuffle_ps(v94, v94, 255);
            v98 = _mm_shuffle_ps(v96, v96, 255);
            v99 = _mm_add_ps(
                    _mm_add_ps(
                      _mm_sub_ps(
                        _mm_mul_ps(_mm_shuffle_ps(v96, v96, 18), _mm_shuffle_ps(v94, v94, 9)),
                        _mm_mul_ps(_mm_shuffle_ps(v96, v96, 9), _mm_shuffle_ps(v94, v94, 18))),
                      _mm_mul_ps(v97, v96)),
                    _mm_mul_ps(v98, v94));
            v100 = _mm_mul_ps(v94, v96);
            v101 = _mm_add_ps(_mm_add_ps(_mm_shuffle_ps(v100, v100, 2), _mm_shuffle_ps(v100, v100, 1)), v100);
            v8[3 * v95 + 1] = _mm_shuffle_ps(
                                v99,
                                _mm_shuffle_ps(v99, _mm_sub_ps(_mm_mul_ps(v98, v97), _mm_shuffle_ps(v101, v101, 0)), 10),
                                132);
            v282 = v248;
            v282.m128_i32[3] = 0;
            v318[0] = v282;
            v281 = v197;
            v281.m128_i32[3] = 0;
            v318[1] = v281;
            v272 = v234;
            v272.m128_i32[3] = 0;
            v318[2] = v272;
            v283 = v250;
            v283.m128_i32[3] = 1065353216;
            v318[3] = v283;
            math::b09::Kernel561E60(&v311, v318, &v325);
            v248 = _mm_shuffle_ps(v275, v275, 85);
            v315[0] = _mm_add_ps(
                        _mm_add_ps(
                          _mm_add_ps(
                            _mm_mul_ps(_mm_shuffle_ps(v274, v274, 85), v312),
                            _mm_mul_ps(_mm_shuffle_ps(v274, v274, 0), v311)),
                          _mm_mul_ps(_mm_shuffle_ps(v274, v274, 170), v313)),
                        _mm_mul_ps(_mm_shuffle_ps(v274, v274, 255), v314));
            v315[1] = _mm_add_ps(
                        _mm_add_ps(
                          _mm_add_ps(_mm_mul_ps(v312, v248), _mm_mul_ps(_mm_shuffle_ps(v275, v275, 0), v311)),
                          _mm_mul_ps(v313, _mm_shuffle_ps(v275, v275, 170))),
                        _mm_mul_ps(v314, _mm_shuffle_ps(v275, v275, 255)));
            v315[2] = _mm_add_ps(
                        _mm_add_ps(
                          _mm_add_ps(
                            _mm_mul_ps(v312, _mm_shuffle_ps(v276, v276, 85)),
                            _mm_mul_ps(_mm_shuffle_ps(v276, v276, 0), v311)),
                          _mm_mul_ps(v313, _mm_shuffle_ps(v276, v276, 170))),
                        _mm_mul_ps(_mm_shuffle_ps(v276, v276, 255), v314));
            v315[3] = _mm_add_ps(
                        _mm_add_ps(
                          _mm_add_ps(
                            _mm_mul_ps(v312, _mm_shuffle_ps(v277, v277, 85)),
                            _mm_mul_ps(v311, _mm_shuffle_ps(v277, v277, 0))),
                          _mm_mul_ps(v313, _mm_shuffle_ps(v277, v277, 170))),
                        _mm_mul_ps(_mm_shuffle_ps(v277, v277, 255), v314));
            v102 = math::b09::Kernel5621D0(v315);
            context->dirty = 1;
            v220 = v102 + v212;
            v239 = v220;
            context->combinedAngle = _mm_shuffle_ps(Bits128(Bits(v220)), Bits128(Bits(v220)), 0);
            v103 = Bits128(Bits(v220));
            v104 = Bits128(0x80000000);
            v104.m128_f32[0] = -0.0 - *(float *)&Global<uint32_t, 0x011CD4D4>();
            if ( (float)(-0.0 - *(float *)&Global<uint32_t, 0x011CD4D4>()) > v220
              || (v104 = Bits128(Global<uint32_t, 0x011CD4CC>()), v220 > *(float *)&Global<uint32_t, 0x011CD4CC>()) )
            {
              v103 = v104;
              v220 = v104.m128_f32[0];
            }
            if ( self->f_40 )
            {
              // 00563BB1..D1 initializes all weights before unordered tests.
              if (std::isnan(v220)) {
                v202 = v200 = v106 = 0;
                v193 = 0.f;
                goto LABEL_117;
              }
              v105 = -0.0 - *(float *)&Global<uint32_t, 0x011CD4C8>();
              if ( self->f_43 )
              {
                if ( v105 > v103.m128_f32[0] )
                {
                  v202 = Global<uint32_t, 0x0114F4E4>();
                  v200 = Global<uint32_t, 0x0114F4E8>();
                  v106 = Global<uint32_t, 0x0114F4EC>();
                  v193 = *(float *)&Global<uint32_t, 0x0114F4EC>();
LABEL_117:
                  v133 = joint("b__Neck__");
                  v228.m128_u64[0] = 0;
                  v228.m128_u64[1] = 1065353216;
                  v235 = _mm_sub_ps(Bits128(0LL), v228);
                  neckAxis = v235;
                  rotate(v8, v9, v133, &neckAxis, v202);
                  v134 = joint("b__Head__");
                  v303 = v235;
                  rotate(v8, v9, v134, &v303, v200);
                  v135 = joint("b__Spine0__");
                  v293 = v235;
                  rotate(v8, v9, v135, &v293, Bits(v193));
                  v136 = joint("b__Spine1__");
                  v301 = v235;
                  rotate(v8, v9, v136, &v301, v106);
LABEL_135:
                  pose->flags &= 0xfffe;
                  return reinterpret_cast<uintptr_t>(pose);
                }
                if ( (float)(-0.0 - *(float *)&Global<uint32_t, 0x011CD4D0>()) <= v103.m128_f32[0] )
                {
                  if ( v103.m128_f32[0] < 0.0 )
                  {
                    v115 = _mm_shuffle_ps(Bits128(Global<uint32_t, 0x0114F4F4>()), Bits128(Global<uint32_t, 0x0114F4F4>()), 0).m128_f32[0];
                    v103.m128_f32[0] = (float)(v103.m128_f32[0] + *(float *)&Global<uint32_t, 0x011CD4D0>()) / *(float *)&Global<uint32_t, 0x011CD4D0>();
                    v114 = rootDelta.m128_f32[0];
                    v103.m128_f32[0] = _mm_shuffle_ps(v103, v103, 0).m128_f32[0];
                    *(float *)&v202 = (float)((float)(rootDelta.m128_f32[0] - v115) * v103.m128_f32[0]) + v115;
                    v116 = _mm_shuffle_ps(Bits128(Global<uint32_t, 0x0114F4F8>()), Bits128(Global<uint32_t, 0x0114F4F8>()), 0).m128_f32[0];
                    *(float *)&v200 = (float)((float)(rootDelta.m128_f32[0] - v116) * v103.m128_f32[0]) + v116;
                    v117 = _mm_shuffle_ps(Bits128(Global<uint32_t, 0x0114F4FC>()), Bits128(Global<uint32_t, 0x0114F4FC>()), 0).m128_f32[0];
                    v193 = (float)((float)(rootDelta.m128_f32[0] - v117) * v103.m128_f32[0]) + v117;
                    v113 = _mm_shuffle_ps(Bits128(Global<uint32_t, 0x0114F500>()), Bits128(Global<uint32_t, 0x0114F500>()), 0).m128_f32[0];
                    goto LABEL_98;
                  }
                  if ( *(float *)&Global<uint32_t, 0x011CD4D0>() > v103.m128_f32[0] )
                  {
                    v113 = rootDelta.m128_f32[0];
                    v103.m128_f32[0] = v103.m128_f32[0] / *(float *)&Global<uint32_t, 0x011CD4D0>();
                    v103.m128_f32[0] = _mm_shuffle_ps(v103, v103, 0).m128_f32[0];
                    *(float *)&v202 = (float)((float)(_mm_shuffle_ps(
                                                        Bits128(Global<uint32_t, 0x011CD4B0>()),
                                                        Bits128(Global<uint32_t, 0x011CD4B0>()),
                                                        0).m128_f32[0]
                                                    - rootDelta.m128_f32[0])
                                            * v103.m128_f32[0])
                                    + rootDelta.m128_f32[0];
                    *(float *)&v200 = (float)((float)(_mm_shuffle_ps(
                                                        Bits128(Global<uint32_t, 0x0114F504>()),
                                                        Bits128(Global<uint32_t, 0x0114F504>()),
                                                        0).m128_f32[0]
                                                    - rootDelta.m128_f32[0])
                                            * v103.m128_f32[0])
                                    + rootDelta.m128_f32[0];
                    v193 = (float)((float)(_mm_shuffle_ps(
                                             Bits128(Global<uint32_t, 0x0114F508>()),
                                             Bits128(Global<uint32_t, 0x0114F508>()),
                                             0).m128_f32[0]
                                         - rootDelta.m128_f32[0])
                                 * v103.m128_f32[0])
                         + rootDelta.m128_f32[0];
                    v112 = Bits128(Global<uint32_t, 0x0114F50C>());
                    goto LABEL_97;
                  }
                  if ( *(float *)&Global<uint32_t, 0x011CD4C8>() < v103.m128_f32[0] )
                  {
                    v202 = Global<uint32_t, 0x011CD4B4>();
                    v200 = Global<uint32_t, 0x0114F510>();
                    v193 = *(float *)&Global<uint32_t, 0x0114F514>();
                    v106 = Global<uint32_t, 0x0114F518>();
                    goto LABEL_117;
                  }
                  v103.m128_f32[0] = (float)(v103.m128_f32[0] - *(float *)&Global<uint32_t, 0x011CD4D0>()) / *(float *)&Global<uint32_t, 0x011CD4D0>();
                  v118 = _mm_shuffle_ps(Bits128(Global<uint32_t, 0x011CD4B0>()), Bits128(Global<uint32_t, 0x011CD4B0>()), 0).m128_f32[0];
                  v103.m128_f32[0] = _mm_shuffle_ps(v103, v103, 0).m128_f32[0];
                  *(float *)&v202 = (float)((float)(_mm_shuffle_ps(
                                                      Bits128(Global<uint32_t, 0x011CD4B4>()),
                                                      Bits128(Global<uint32_t, 0x011CD4B4>()),
                                                      0).m128_f32[0]
                                                  - v118)
                                          * v103.m128_f32[0])
                                  + v118;
                  v119 = _mm_shuffle_ps(Bits128(Global<uint32_t, 0x0114F504>()), Bits128(Global<uint32_t, 0x0114F504>()), 0).m128_f32[0];
                  *(float *)&v200 = (float)((float)(_mm_shuffle_ps(
                                                      Bits128(Global<uint32_t, 0x0114F510>()),
                                                      Bits128(Global<uint32_t, 0x0114F510>()),
                                                      0).m128_f32[0]
                                                  - v119)
                                          * v103.m128_f32[0])
                                  + v119;
                  v120 = _mm_shuffle_ps(Bits128(Global<uint32_t, 0x0114F508>()), Bits128(Global<uint32_t, 0x0114F508>()), 0).m128_f32[0];
                  v121 = (float)((float)(_mm_shuffle_ps(
                                           Bits128(Global<uint32_t, 0x0114F514>()),
                                           Bits128(Global<uint32_t, 0x0114F514>()),
                                           0).m128_f32[0]
                                       - v120)
                               * v103.m128_f32[0])
                       + v120;
                  v111 = Bits128(Global<uint32_t, 0x0114F50C>());
                  v193 = v121;
                  v112 = Bits128(Global<uint32_t, 0x0114F518>());
                }
                else
                {
                  v107 = _mm_shuffle_ps(Bits128(Global<uint32_t, 0x0114F4E4>()), Bits128(Global<uint32_t, 0x0114F4E4>()), 0).m128_f32[0];
                  v103.m128_f32[0] = (float)(v103.m128_f32[0] + *(float *)&Global<uint32_t, 0x011CD4C8>()) / *(float *)&Global<uint32_t, 0x011CD4D0>();
                  v103.m128_f32[0] = _mm_shuffle_ps(v103, v103, 0).m128_f32[0];
                  *(float *)&v202 = (float)((float)(_mm_shuffle_ps(
                                                      Bits128(Global<uint32_t, 0x0114F4F4>()),
                                                      Bits128(Global<uint32_t, 0x0114F4F4>()),
                                                      0).m128_f32[0]
                                                  - v107)
                                          * v103.m128_f32[0])
                                  + v107;
                  v108 = _mm_shuffle_ps(Bits128(Global<uint32_t, 0x0114F4E8>()), Bits128(Global<uint32_t, 0x0114F4E8>()), 0).m128_f32[0];
                  *(float *)&v200 = (float)((float)(_mm_shuffle_ps(
                                                      Bits128(Global<uint32_t, 0x0114F4F8>()),
                                                      Bits128(Global<uint32_t, 0x0114F4F8>()),
                                                      0).m128_f32[0]
                                                  - v108)
                                          * v103.m128_f32[0])
                                  + v108;
                  v109 = _mm_shuffle_ps(Bits128(Global<uint32_t, 0x0114F4EC>()), Bits128(Global<uint32_t, 0x0114F4EC>()), 0).m128_f32[0];
                  v110 = (float)((float)(_mm_shuffle_ps(
                                           Bits128(Global<uint32_t, 0x0114F4FC>()),
                                           Bits128(Global<uint32_t, 0x0114F4FC>()),
                                           0).m128_f32[0]
                                       - v109)
                               * v103.m128_f32[0])
                       + v109;
                  v111 = Bits128(Global<uint32_t, 0x0114F4F0>());
                  v193 = v110;
                  v112 = Bits128(Global<uint32_t, 0x0114F500>());
                }
              }
              else
              {
                if ( v105 > v103.m128_f32[0] )
                {
                  v202 = Global<uint32_t, 0x0114F4AC>();
                  v200 = Global<uint32_t, 0x0114F4B0>();
                  v106 = Global<uint32_t, 0x0114F4B4>();
                  v193 = *(float *)&Global<uint32_t, 0x0114F4B4>();
                  goto LABEL_117;
                }
                if ( (float)(-0.0 - *(float *)&Global<uint32_t, 0x011CD4D0>()) <= v103.m128_f32[0] )
                {
                  if ( v103.m128_f32[0] < 0.0 )
                  {
                    v126 = _mm_shuffle_ps(Bits128(Global<uint32_t, 0x0114F4BC>()), Bits128(Global<uint32_t, 0x0114F4BC>()), 0).m128_f32[0];
                    v103.m128_f32[0] = (float)(v103.m128_f32[0] + *(float *)&Global<uint32_t, 0x011CD4D0>()) / *(float *)&Global<uint32_t, 0x011CD4D0>();
                    v114 = rootDelta.m128_f32[0];
                    v103.m128_f32[0] = _mm_shuffle_ps(v103, v103, 0).m128_f32[0];
                    *(float *)&v202 = (float)((float)(rootDelta.m128_f32[0] - v126) * v103.m128_f32[0]) + v126;
                    v127 = _mm_shuffle_ps(Bits128(Global<uint32_t, 0x0114F4C0>()), Bits128(Global<uint32_t, 0x0114F4C0>()), 0).m128_f32[0];
                    *(float *)&v200 = (float)((float)(rootDelta.m128_f32[0] - v127) * v103.m128_f32[0]) + v127;
                    v128 = _mm_shuffle_ps(Bits128(Global<uint32_t, 0x0114F4C4>()), Bits128(Global<uint32_t, 0x0114F4C4>()), 0).m128_f32[0];
                    v193 = (float)((float)(rootDelta.m128_f32[0] - v128) * v103.m128_f32[0]) + v128;
                    v113 = _mm_shuffle_ps(Bits128(Global<uint32_t, 0x0114F4C8>()), Bits128(Global<uint32_t, 0x0114F4C8>()), 0).m128_f32[0];
                    goto LABEL_98;
                  }
                  if ( *(float *)&Global<uint32_t, 0x011CD4D0>() > v103.m128_f32[0] )
                  {
                    v113 = rootDelta.m128_f32[0];
                    v103.m128_f32[0] = v103.m128_f32[0] / *(float *)&Global<uint32_t, 0x011CD4D0>();
                    v103.m128_f32[0] = _mm_shuffle_ps(v103, v103, 0).m128_f32[0];
                    *(float *)&v202 = (float)((float)(_mm_shuffle_ps(
                                                        Bits128(Global<uint32_t, 0x011CD4A8>()),
                                                        Bits128(Global<uint32_t, 0x011CD4A8>()),
                                                        0).m128_f32[0]
                                                    - rootDelta.m128_f32[0])
                                            * v103.m128_f32[0])
                                    + rootDelta.m128_f32[0];
                    *(float *)&v200 = (float)((float)(_mm_shuffle_ps(
                                                        Bits128(Global<uint32_t, 0x0114F4CC>()),
                                                        Bits128(Global<uint32_t, 0x0114F4CC>()),
                                                        0).m128_f32[0]
                                                    - rootDelta.m128_f32[0])
                                            * v103.m128_f32[0])
                                    + rootDelta.m128_f32[0];
                    v193 = (float)((float)(_mm_shuffle_ps(
                                             Bits128(Global<uint32_t, 0x0114F4D0>()),
                                             Bits128(Global<uint32_t, 0x0114F4D0>()),
                                             0).m128_f32[0]
                                         - rootDelta.m128_f32[0])
                                 * v103.m128_f32[0])
                         + rootDelta.m128_f32[0];
                    v112 = Bits128(Global<uint32_t, 0x0114F4D4>());
                    goto LABEL_97;
                  }
                  if ( *(float *)&Global<uint32_t, 0x011CD4C8>() < v103.m128_f32[0] )
                  {
                    v202 = Global<uint32_t, 0x011CD4AC>();
                    v200 = Global<uint32_t, 0x0114F4D8>();
                    v193 = *(float *)&Global<uint32_t, 0x0114F4DC>();
                    v106 = Global<uint32_t, 0x0114F4E0>();
                    goto LABEL_117;
                  }
                  v103.m128_f32[0] = (float)(v103.m128_f32[0] - *(float *)&Global<uint32_t, 0x011CD4D0>()) / *(float *)&Global<uint32_t, 0x011CD4D0>();
                  v129 = _mm_shuffle_ps(Bits128(Global<uint32_t, 0x011CD4A8>()), Bits128(Global<uint32_t, 0x011CD4A8>()), 0).m128_f32[0];
                  v103.m128_f32[0] = _mm_shuffle_ps(v103, v103, 0).m128_f32[0];
                  *(float *)&v202 = (float)((float)(_mm_shuffle_ps(
                                                      Bits128(Global<uint32_t, 0x011CD4AC>()),
                                                      Bits128(Global<uint32_t, 0x011CD4AC>()),
                                                      0).m128_f32[0]
                                                  - v129)
                                          * v103.m128_f32[0])
                                  + v129;
                  v130 = _mm_shuffle_ps(Bits128(Global<uint32_t, 0x0114F4CC>()), Bits128(Global<uint32_t, 0x0114F4CC>()), 0).m128_f32[0];
                  *(float *)&v200 = (float)((float)(_mm_shuffle_ps(
                                                      Bits128(Global<uint32_t, 0x0114F4D8>()),
                                                      Bits128(Global<uint32_t, 0x0114F4D8>()),
                                                      0).m128_f32[0]
                                                  - v130)
                                          * v103.m128_f32[0])
                                  + v130;
                  v131 = _mm_shuffle_ps(Bits128(Global<uint32_t, 0x0114F4D0>()), Bits128(Global<uint32_t, 0x0114F4D0>()), 0).m128_f32[0];
                  v132 = (float)((float)(_mm_shuffle_ps(
                                           Bits128(Global<uint32_t, 0x0114F4DC>()),
                                           Bits128(Global<uint32_t, 0x0114F4DC>()),
                                           0).m128_f32[0]
                                       - v131)
                               * v103.m128_f32[0])
                       + v131;
                  v111 = Bits128(Global<uint32_t, 0x0114F4D4>());
                  v193 = v132;
                  v112 = Bits128(Global<uint32_t, 0x0114F4E0>());
                }
                else
                {
                  v122 = _mm_shuffle_ps(Bits128(Global<uint32_t, 0x0114F4AC>()), Bits128(Global<uint32_t, 0x0114F4AC>()), 0).m128_f32[0];
                  v103.m128_f32[0] = (float)(v103.m128_f32[0] + *(float *)&Global<uint32_t, 0x011CD4C8>()) / *(float *)&Global<uint32_t, 0x011CD4D0>();
                  v103.m128_f32[0] = _mm_shuffle_ps(v103, v103, 0).m128_f32[0];
                  *(float *)&v202 = (float)((float)(_mm_shuffle_ps(
                                                      Bits128(Global<uint32_t, 0x0114F4BC>()),
                                                      Bits128(Global<uint32_t, 0x0114F4BC>()),
                                                      0).m128_f32[0]
                                                  - v122)
                                          * v103.m128_f32[0])
                                  + v122;
                  v123 = _mm_shuffle_ps(Bits128(Global<uint32_t, 0x0114F4B0>()), Bits128(Global<uint32_t, 0x0114F4B0>()), 0).m128_f32[0];
                  *(float *)&v200 = (float)((float)(_mm_shuffle_ps(
                                                      Bits128(Global<uint32_t, 0x0114F4C0>()),
                                                      Bits128(Global<uint32_t, 0x0114F4C0>()),
                                                      0).m128_f32[0]
                                                  - v123)
                                          * v103.m128_f32[0])
                                  + v123;
                  v124 = _mm_shuffle_ps(Bits128(Global<uint32_t, 0x0114F4B4>()), Bits128(Global<uint32_t, 0x0114F4B4>()), 0).m128_f32[0];
                  v125 = (float)((float)(_mm_shuffle_ps(
                                           Bits128(Global<uint32_t, 0x0114F4C4>()),
                                           Bits128(Global<uint32_t, 0x0114F4C4>()),
                                           0).m128_f32[0]
                                       - v124)
                               * v103.m128_f32[0])
                       + v124;
                  v111 = Bits128(Global<uint32_t, 0x0114F4B8>());
                  v193 = v125;
                  v112 = Bits128(Global<uint32_t, 0x0114F4C8>());
                }
              }
              v113 = _mm_shuffle_ps(v111, v111, 0).m128_f32[0];
LABEL_97:
              v114 = _mm_shuffle_ps(v112, v112, 0).m128_f32[0];
LABEL_98:
              *(float *)&v106 = (float)((float)(v114 - v113) * v103.m128_f32[0]) + v113;
              goto LABEL_117;
            }
            v137 = joint("b__R_UpperArm__");
            v213 = -v220;
            v295 = Bits128(0x3F800000u);
            rotate(v8, v9, v137, &v295, Bits(v213));
            v138 = joint("b__L_UpperArm__");
            v309 = Bits128(0x3F800000u);
            rotate(v8, v9, v138, &v309, Bits(v213));
            v139 = joint("b__R_Thigh__");
            v297 = Bits128(0x3F800000u);
            rotate(v8, v9, v139, &v297, Bits(v213));
            v140 = joint("b__L_Thigh__");
            v287 = Bits128(0x3F800000u);
            rotate(v8, v9, v140, &v287, Bits(v213));
            v229.m128_u64[1] = (unsigned int)Global<uint32_t, 0x0114F408>();
            v229.m128_i32[1] = Global<uint32_t, 0x0114F404>();
            v229.m128_i32[0] = 0;
            v236.m128_u64[1] = (unsigned int)Global<uint32_t, 0x0114F410>();
            v236.m128_i32[1] = Global<uint32_t, 0x0114F40C>();
            v236.m128_i32[0] = 0;
            v141 = v236;
            v236.m128_u64[1] = (unsigned int)Global<uint32_t, 0x0114F418>();
            v236.m128_i32[1] = Global<uint32_t, 0x0114F414>();
            v236.m128_i32[0] = 0;
            v142 = v236;
            v237.m128_u64[1] = (unsigned int)Global<uint32_t, 0x0114F420>();
            v237.m128_i32[1] = Global<uint32_t, 0x0114F41C>();
            v237.m128_i32[0] = 0;
            v143 = Bits128(Bits(v220));
            // 005645EF..0056464F: unordered leaves translation and weights zero.
            if (std::isnan(v220)) {
              v194 = v201 = v203 = v144 = 0;
              v232 = 0.f;
              v237 = _mm_setzero_ps();
              goto LABEL_131;
            }
            if ( (float)(-0.0 - *(float *)&Global<uint32_t, 0x011CD4C8>()) > v220 )
            {
              v194 = Global<uint32_t, 0x0114F42C>();
              v201 = Global<uint32_t, 0x011CD498>();
              v203 = Global<uint32_t, 0x0114F430>();
              v232 = *(float *)&Global<uint32_t, 0x0114F434>();
              v144 = Global<uint32_t, 0x0114F438>();
              v237 = v229;
LABEL_131:
              v214 = v144;
              if ( v238 )
              {
                *(float *)&v205 = CallGameMethod<float, 0x005ECE70>(v238, (unsigned int)*p_f_34 | self->f_30);
                v166 = Bits128(v205);
                v143 = Bits128(Bits(v220));
              }
              else
              {
                v166 = Bits128(0x3FAE147Bu);
              }
              v167 = _mm_shuffle_ps(v143, v143, 0).m128_f32[0];
              v142.m128_f32[0] = v166.m128_f32[0];
              *(float *)&v206 = Cos(v167);
              v166.m128_f32[0] = v166.m128_f32[0]
                               - (float)(_mm_shuffle_ps(Bits128(v206), Bits128(v206), 0).m128_f32[0]
                                       * _mm_shuffle_ps(v142, v142, 0).m128_f32[0]);
              v251 = Bits128(0x3F800000u);
              v168 = _mm_mul_ps(_mm_shuffle_ps(v166, v166, 0), _mm_sub_ps(Bits128(0LL), v273));
              *(float *)&v207 = Sin((float)(v167 / v225.m128_f32[0]));
              v169 = _mm_shuffle_ps(Bits128(v207), Bits128(v207), 0);
              *(float *)&v208 = Cos((float)(v167 / v225.m128_f32[0]));
              v170 = _mm_mul_ps(_mm_shuffle_ps(Bits128(0x3F800000u), Bits128(0x3F800000u), 85), v169);
              v328 = _mm_mul_ps(_mm_shuffle_ps(Bits128(0x3F800000u), Bits128(0x3F800000u), 0), v169);
              v327 = _mm_mul_ps(_mm_shuffle_ps(Bits128(0x3F800000u), Bits128(0x3F800000u), 170), v169);
              v230.m128_u64[1] = 0x80000000LL;
              v230.m128_u64[0] = 0x8000000080000000uLL;
              v171 = _mm_xor_ps(
                       v230,
                       _mm_shuffle_ps(
                         _mm_shuffle_ps(v328, v170, 16),
                         _mm_shuffle_ps(v327, _mm_shuffle_ps(Bits128(v208), Bits128(v208), 0), 50),
                         136));
              v320 = v170;
              v172 = _mm_shuffle_ps(v171, v171, 9);
              v173 = _mm_shuffle_ps(v171, v171, 18);
              v174 = _mm_add_ps(
                       _mm_sub_ps(
                         _mm_mul_ps(_mm_shuffle_ps(v168, v168, 18), v172),
                         _mm_mul_ps(_mm_shuffle_ps(v168, v168, 9), v173)),
                       _mm_mul_ps(_mm_shuffle_ps(v171, v171, 255), v168));
              v271 = &v251;
              v254 = &v251;
              v270 = &v251;
              rootDelta = _mm_add_ps(
                                      _mm_mul_ps(
                                        _mm_sub_ps(
                                          _mm_mul_ps(_mm_shuffle_ps(v174, v174, 18), v172),
                                          _mm_mul_ps(_mm_shuffle_ps(v174, v174, 9), v173)),
                                        v225),
                                      v168);
              v174.m128_i32[0] = (SignedBits((float)(-0.0 - v239) - (float)(-0.0 - *(float *)&Global<uint32_t, 0x011CD4C8>())) >> 31)
                               & Bits(-0.0 - *(float *)&Global<uint32_t, 0x011CD4C8>())
                               | Bits(-0.0 - v239)
                               & ~(SignedBits((float)(-0.0 - v239) - (float)(-0.0 - *(float *)&Global<uint32_t, 0x011CD4C8>())) >> 31);
              v209 = (SignedBits(v174.m128_f32[0] - *(float *)&Global<uint32_t, 0x011CD4C8>()) >> 31) & v174.m128_i32[0]
                   | Global<uint32_t, 0x011CD4C8>() & ~(SignedBits(v174.m128_f32[0] - *(float *)&Global<uint32_t, 0x011CD4C8>()) >> 31);
              v175 = joint("b__L_Spine3_slot");
              v230.m128_u64[1] = 0x80000000LL;
              v230.m128_u64[0] = 0x8000000080000000uLL;
              v176 = _mm_xor_ps(*(__m128 *)(reinterpret_cast<const unsigned char*>(v9) + 48 * v175 + 16), v230);
              v177 = _mm_shuffle_ps(v176, v176, 9);
              v178 = _mm_shuffle_ps(v176, v176, 18);
              v248 = _mm_shuffle_ps(v237, v237, 18);
              v198 = _mm_shuffle_ps(v237, v237, 9);
              v179 = _mm_add_ps(
                       _mm_sub_ps(_mm_mul_ps(v248, v177), _mm_mul_ps(v198, v178)),
                       _mm_mul_ps(_mm_shuffle_ps(v176, v176, 255), v237));
              v8[3 * v175] = _mm_add_ps(
                               _mm_add_ps(
                                 _mm_mul_ps(
                                   _mm_sub_ps(
                                     _mm_mul_ps(_mm_shuffle_ps(v179, v179, 18), v177),
                                     _mm_mul_ps(_mm_shuffle_ps(v179, v179, 9), v178)),
                                   v225),
                                 v237),
                               v8[3 * v175]);
              v288 = Bits128(0x3F800000u);
              rotate(v8, v9, v175, &v288, v209);
              v180 = joint("b__R_Spine3_slot");
              v230.m128_u64[1] = 0x80000000LL;
              v230.m128_u64[0] = 0x8000000080000000uLL;
              v181 = _mm_xor_ps(*(__m128 *)(reinterpret_cast<const unsigned char*>(v9) + 48 * v180 + 16), v230);
              v182 = _mm_shuffle_ps(v181, v181, 9);
              v183 = _mm_shuffle_ps(v181, v181, 18);
              v184 = _mm_add_ps(
                       _mm_sub_ps(_mm_mul_ps(v182, v248), _mm_mul_ps(v183, v198)),
                       _mm_mul_ps(_mm_shuffle_ps(v181, v181, 255), v237));
              v8[3 * v180] = _mm_add_ps(
                               _mm_add_ps(
                                 _mm_mul_ps(
                                   _mm_sub_ps(
                                     _mm_mul_ps(_mm_shuffle_ps(v184, v184, 18), v182),
                                     _mm_mul_ps(_mm_shuffle_ps(v184, v184, 9), v183)),
                                   v225),
                                 v237),
                               v8[3 * v180]);
              v286 = Bits128(0x3F800000u);
              rotate(v8, v9, v180, &v286, v209);
              v185 = joint("b__Neck0__");
              v285 = Bits128(0x3F800000u);
              rotate(v8, v9, v185, &v285, v194);
              v186 = joint("b__Neck1__");
              v294 = Bits128(0x3F800000u);
              rotate(v8, v9, v186, &v294, v201);
              v187 = joint("b__Head__");
              v296 = Bits128(0x3F800000u);
              rotate(v8, v9, v187, &v296, v203);
              v188 = joint("b__Tail0__");
              v298 = Bits128(0x3F800000u);
              rotate(v8, v9, v188, &v298, Bits(v232));
              v189 = joint("b__Tail2__");
              v300 = Bits128(0x3F800000u);
              rotate(v8, v9, v189, &v300, v214);
              *v8 = _mm_add_ps(*v8, rootDelta);
              goto LABEL_135;
            }
            if ( (float)(-0.0 - *(float *)&Global<uint32_t, 0x011CD4D0>()) <= v220 )
            {
              if ( v220 < 0.0 )
              {
                v155 = Bits128(Bits(v220));
                v155.m128_f32[0] = (float)(v220 + *(float *)&Global<uint32_t, 0x011CD4D0>()) / *(float *)&Global<uint32_t, 0x011CD4D0>();
                v146 = _mm_shuffle_ps(v155, v155, 0);
                v156 = _mm_add_ps(_mm_mul_ps(_mm_sub_ps(Bits128(0LL), v141), v146), v141);
                v154 = rootDelta.m128_f32[0];
                v237 = v156;
                v156.m128_f32[0] = _mm_shuffle_ps(
                                     Bits128(Global<uint32_t, 0x0114F43C>()),
                                     Bits128(Global<uint32_t, 0x0114F43C>()),
                                     0).m128_f32[0];
                *(float *)&v194 = (float)((float)(rootDelta.m128_f32[0] - v156.m128_f32[0]) * v146.m128_f32[0]) + v156.m128_f32[0];
                v156.m128_f32[0] = _mm_shuffle_ps(
                                     Bits128(Global<uint32_t, 0x011CD49C>()),
                                     Bits128(Global<uint32_t, 0x011CD49C>()),
                                     0).m128_f32[0];
                *(float *)&v201 = (float)((float)(rootDelta.m128_f32[0] - v156.m128_f32[0]) * v146.m128_f32[0]) + v156.m128_f32[0];
                v156.m128_f32[0] = _mm_shuffle_ps(
                                     Bits128(Global<uint32_t, 0x0114F440>()),
                                     Bits128(Global<uint32_t, 0x0114F440>()),
                                     0).m128_f32[0];
                *(float *)&v203 = (float)((float)(rootDelta.m128_f32[0] - v156.m128_f32[0]) * v146.m128_f32[0]) + v156.m128_f32[0];
                v156.m128_f32[0] = _mm_shuffle_ps(
                                     Bits128(Global<uint32_t, 0x0114F444>()),
                                     Bits128(Global<uint32_t, 0x0114F444>()),
                                     0).m128_f32[0];
                v232 = (float)((float)(rootDelta.m128_f32[0] - v156.m128_f32[0]) * v146.m128_f32[0]) + v156.m128_f32[0];
                v152 = Bits128(Global<uint32_t, 0x0114F448>());
                goto LABEL_123;
              }
              if ( *(float *)&Global<uint32_t, 0x011CD4D0>() > v220 )
              {
                v157 = Bits128(Bits(v220));
                v157.m128_f32[0] = v220 / *(float *)&Global<uint32_t, 0x011CD4D0>();
                v158 = _mm_shuffle_ps(v157, v157, 0);
                v237 = _mm_add_ps(_mm_mul_ps(_mm_sub_ps(v142, Bits128(0LL)), v158), Bits128(0LL));
                *(float *)&v194 = (float)((float)(_mm_shuffle_ps(
                                                    Bits128(Global<uint32_t, 0x0114F44C>()),
                                                    Bits128(Global<uint32_t, 0x0114F44C>()),
                                                    0).m128_f32[0]
                                                - rootDelta.m128_f32[0])
                                        * v158.m128_f32[0])
                                + rootDelta.m128_f32[0];
                *(float *)&v201 = (float)((float)(_mm_shuffle_ps(
                                                    Bits128(Global<uint32_t, 0x0114F450>()),
                                                    Bits128(Global<uint32_t, 0x0114F450>()),
                                                    0).m128_f32[0]
                                                - rootDelta.m128_f32[0])
                                        * v158.m128_f32[0])
                                + rootDelta.m128_f32[0];
                *(float *)&v203 = (float)((float)(_mm_shuffle_ps(
                                                    Bits128(Global<uint32_t, 0x0114F454>()),
                                                    Bits128(Global<uint32_t, 0x0114F454>()),
                                                    0).m128_f32[0]
                                                - rootDelta.m128_f32[0])
                                        * v158.m128_f32[0])
                                + rootDelta.m128_f32[0];
                v232 = (float)((float)(_mm_shuffle_ps(
                                         Bits128(Global<uint32_t, 0x0114F458>()),
                                         Bits128(Global<uint32_t, 0x0114F458>()),
                                         0).m128_f32[0]
                                     - rootDelta.m128_f32[0])
                             * v158.m128_f32[0])
                     + rootDelta.m128_f32[0];
                v142 = _mm_add_ps(
                         _mm_mul_ps(
                           _mm_sub_ps(
                             _mm_shuffle_ps(Bits128(Global<uint32_t, 0x0114F45C>()), Bits128(Global<uint32_t, 0x0114F45C>()), 0),
                             rootDelta),
                           v158),
                         rootDelta);
                v144 = v142.m128_i32[0];
                goto LABEL_131;
              }
              if ( *(float *)&Global<uint32_t, 0x011CD4C8>() < v220 )
              {
                v194 = Global<uint32_t, 0x0114F460>();
                v201 = Global<uint32_t, 0x0114F464>();
                v203 = Global<uint32_t, 0x0114F468>();
                v232 = *(float *)&Global<uint32_t, 0x0114F46C>();
                v144 = Global<uint32_t, 0x0114F470>();
                goto LABEL_131;
              }
              v159 = Bits128(Bits(v220));
              v159.m128_f32[0] = (float)(v220 - *(float *)&Global<uint32_t, 0x011CD4D0>()) / *(float *)&Global<uint32_t, 0x011CD4D0>();
              v146 = _mm_shuffle_ps(v159, v159, 0);
              v160 = _mm_add_ps(_mm_mul_ps(_mm_sub_ps(v237, v142), v146), v142);
              v161 = _mm_shuffle_ps(Bits128(Global<uint32_t, 0x0114F44C>()), Bits128(Global<uint32_t, 0x0114F44C>()), 0).m128_f32[0];
              *(float *)&v194 = (float)((float)(_mm_shuffle_ps(
                                                  Bits128(Global<uint32_t, 0x0114F460>()),
                                                  Bits128(Global<uint32_t, 0x0114F460>()),
                                                  0).m128_f32[0]
                                              - v161)
                                      * v146.m128_f32[0])
                              + v161;
              v162 = _mm_shuffle_ps(Bits128(Global<uint32_t, 0x0114F450>()), Bits128(Global<uint32_t, 0x0114F450>()), 0).m128_f32[0];
              *(float *)&v201 = (float)((float)(_mm_shuffle_ps(
                                                  Bits128(Global<uint32_t, 0x0114F464>()),
                                                  Bits128(Global<uint32_t, 0x0114F464>()),
                                                  0).m128_f32[0]
                                              - v162)
                                      * v146.m128_f32[0])
                              + v162;
              v163 = _mm_shuffle_ps(Bits128(Global<uint32_t, 0x0114F454>()), Bits128(Global<uint32_t, 0x0114F454>()), 0).m128_f32[0];
              *(float *)&v203 = (float)((float)(_mm_shuffle_ps(
                                                  Bits128(Global<uint32_t, 0x0114F468>()),
                                                  Bits128(Global<uint32_t, 0x0114F468>()),
                                                  0).m128_f32[0]
                                              - v163)
                                      * v146.m128_f32[0])
                              + v163;
              v164 = _mm_shuffle_ps(Bits128(Global<uint32_t, 0x0114F458>()), Bits128(Global<uint32_t, 0x0114F458>()), 0).m128_f32[0];
              v165 = (float)((float)(_mm_shuffle_ps(
                                       Bits128(Global<uint32_t, 0x0114F46C>()),
                                       Bits128(Global<uint32_t, 0x0114F46C>()),
                                       0).m128_f32[0]
                                   - v164)
                           * v146.m128_f32[0])
                   + v164;
              v152 = Bits128(Global<uint32_t, 0x0114F45C>());
              v232 = v165;
              v153 = Bits128(Global<uint32_t, 0x0114F470>());
              v237 = v160;
            }
            else
            {
              v145 = Bits128(Bits(v220));
              v145.m128_f32[0] = (float)(v220 + *(float *)&Global<uint32_t, 0x011CD4C8>()) / *(float *)&Global<uint32_t, 0x011CD4D0>();
              v146 = _mm_shuffle_ps(v145, v145, 0);
              v237 = _mm_add_ps(_mm_mul_ps(_mm_sub_ps(v141, v229), v146), v229);
              v147 = _mm_shuffle_ps(Bits128(Global<uint32_t, 0x0114F42C>()), Bits128(Global<uint32_t, 0x0114F42C>()), 0).m128_f32[0];
              *(float *)&v194 = (float)((float)(_mm_shuffle_ps(
                                                  Bits128(Global<uint32_t, 0x0114F43C>()),
                                                  Bits128(Global<uint32_t, 0x0114F43C>()),
                                                  0).m128_f32[0]
                                              - v147)
                                      * v146.m128_f32[0])
                              + v147;
              v148 = _mm_shuffle_ps(Bits128(Global<uint32_t, 0x011CD498>()), Bits128(Global<uint32_t, 0x011CD498>()), 0).m128_f32[0];
              *(float *)&v201 = (float)((float)(_mm_shuffle_ps(
                                                  Bits128(Global<uint32_t, 0x011CD49C>()),
                                                  Bits128(Global<uint32_t, 0x011CD49C>()),
                                                  0).m128_f32[0]
                                              - v148)
                                      * v146.m128_f32[0])
                              + v148;
              v149 = _mm_shuffle_ps(Bits128(Global<uint32_t, 0x0114F430>()), Bits128(Global<uint32_t, 0x0114F430>()), 0).m128_f32[0];
              *(float *)&v203 = (float)((float)(_mm_shuffle_ps(
                                                  Bits128(Global<uint32_t, 0x0114F440>()),
                                                  Bits128(Global<uint32_t, 0x0114F440>()),
                                                  0).m128_f32[0]
                                              - v149)
                                      * v146.m128_f32[0])
                              + v149;
              v150 = _mm_shuffle_ps(Bits128(Global<uint32_t, 0x0114F434>()), Bits128(Global<uint32_t, 0x0114F434>()), 0).m128_f32[0];
              v151 = (float)((float)(_mm_shuffle_ps(
                                       Bits128(Global<uint32_t, 0x0114F444>()),
                                       Bits128(Global<uint32_t, 0x0114F444>()),
                                       0).m128_f32[0]
                                   - v150)
                           * v146.m128_f32[0])
                   + v150;
              v152 = Bits128(Global<uint32_t, 0x0114F438>());
              v232 = v151;
              v153 = Bits128(Global<uint32_t, 0x0114F448>());
            }
            v154 = _mm_shuffle_ps(v153, v153, 0).m128_f32[0];
LABEL_123:
            v142 = _mm_shuffle_ps(v152, v152, 0);
            *(float *)&v144 = (float)((float)(v154 - v142.m128_f32[0]) * v146.m128_f32[0]) + v142.m128_f32[0];
            goto LABEL_131;
          }
          v74 = v247;
          v264 = &v246;
          if ( _mm_movemask_ps(_mm_cmpeq_ps(_mm_shuffle_ps(v246, v246, 85), v71)) == 15 )
            goto LABEL_72;
          v258 = &v247;
          if ( _mm_movemask_ps(_mm_cmpeq_ps(_mm_shuffle_ps(v247, v247, 85), v71)) == 15 )
            goto LABEL_72;
          if ( (uint8_t)CallGameFunction<uint8_t, 0x007D2240>(&v244, 4, &v284) )
          {
            v76 = v223;
            goto LABEL_74;
          }
          v72 = v245;
          v70 = v244;
        }
        v73 = v246;
        goto LABEL_71;
      }
    }
  }
  return result;
}


}
#if defined(_M_IX86)
namespace ts3::anim::m21 {
uintptr_t __fastcall HookEvaluate(Node* node, void*, Context* context, Pose* pose) { return Evaluate(node,context,pose); }
}
#endif
