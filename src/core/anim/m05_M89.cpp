#include "ts3/anim/m05_M89.h"
#include <cstring>
#include <cmath>
#include <emmintrin.h>
#include "ts3/math/m04_B09.h"
namespace ts3::anim::m89 {
namespace {
void* Table(uintptr_t address) {
#if defined(_M_IX86)
    return reinterpret_cast<void*>(address);
#else
    static void* tables[2][8]{}; return tables[address==0x00fcb508u];
#endif
}
}
Event* ConstructEvent2(Event* p,uint32_t argument) {
    p->time=0.0f; p->interval=0.008333333767950535f;
    p->argument=argument; p->type=0xc1e40002u; p->zero=0;
    p->duration=0.03333333507180214f; p->mode=38;
    p->reserved0=0; p->reserved1=0; p->vtable=Table(0x00fcb558u);
    return p;
}
Event* ConstructEvent3(Event* p,uint32_t argument) {
    p->time=0.0f; p->duration=0.03333333507180214f;
    p->argument=argument; p->type=0xc1e40003u; p->zero=0;
    p->mode=0; p->reserved0=0; p->reserved1=0;
    p->vtable=Table(0x00fcb508u); p->interval=0.30000001192092896f; p->flag=0;
    return p;
}
uintptr_t Reset(Pose* p) {
    std::memset(p->translation,0,sizeof p->translation);
    std::memset(p->rotation,0,sizeof p->rotation); p->weight=0.0f;
    p->workspace=p->inlineWorkspace;
    for(unsigned i=0;i<6;++i) {
        p->workspace[i].end=p->workspace[i].begin;
        p->workspace[i].weight=0.0f;
    }
    return reinterpret_cast<uintptr_t>(p->workspace+6)+offsetof(Workspace,weight);
}
uintptr_t AccumulateSimple(float weight,Pose* source,Pose* dest) {
    // CD80 executes LAHF after comparing the old destination weight with zero.
    // E730 enters it with EAX = source+10; only AH is overwritten.
    const unsigned flags=std::isnan(dest->weight)?0x47u:(dest->weight==0.0f?0x42u:(dest->weight<0.0f?0x03u:0x02u));
    CallGameFunction<void,0x0057cd80>(dest,source->translation,weight);
    uintptr_t result=(reinterpret_cast<uintptr_t>(source->translation)&~uintptr_t(0xff00))|(uintptr_t(flags)<<8);
    if(source->workspace && !dest->workspace) return 0;
    if(source->workspace && dest->workspace) {
        result=reinterpret_cast<uintptr_t>(dest->workspace)-reinterpret_cast<uintptr_t>(source->workspace);
        for(unsigned i=0;i<6;++i) {
            auto& s=source->workspace[i]; auto& d=dest->workspace[i];
            for(Packet* p=s.begin;p!=s.end;++p) {
                alignas(16) Packet copy;
                CallGameMethod<Packet*,0x00575200>(&copy,p);
                copy.weight=copy.weight*weight;
                const bool accepted=copy.weight>0.0f && copy.total>0.0f;
                // Rejection retains EAX from the preceding clone.
                const uintptr_t rejected=reinterpret_cast<uintptr_t>(&copy);
                const uintptr_t appended=CallGameMethod<uintptr_t,0x005753c0>(&d,&copy);
                result=accepted?appended:rejected;
            }
        }
    }
    return result;
}
uintptr_t Accumulate(const float* first,const float* second,float weight,Pose* source,Pose* dest) {
    float root=weight;
    if(first) root=first[0]*weight;
    if(second) root=second[0]*root;
    CallGameFunction<uintptr_t,0x0057cd80>(dest,source->translation,root);
    std::memcpy(dest->saved,source->saved,sizeof dest->saved);
    uintptr_t result=reinterpret_cast<uintptr_t>(source->workspace);
    if(source->workspace && dest->workspace) {
        for(unsigned i=0;i<6;++i) {
            auto& s=source->workspace[i]; auto& d=dest->workspace[i];
            result=reinterpret_cast<uintptr_t>(&d);
            for(Packet* p=s.begin;p!=s.end;++p) {
                alignas(16) Packet copy;
                CallGameMethod<Packet*,0x00575200>(&copy,p);
                result=static_cast<uintptr_t>(static_cast<intptr_t>(copy.index));
                if(copy.index!=-1) {
                    float w=copy.weight*weight;
                    if(first) w=first[copy.index]*w;
                    if(second) w=second[copy.index]*w;
                    copy.weight=w;
                    for(Element* e=copy.begin;e!=copy.end;++e) {
                        if(first) e->weight=first[e->index]*e->weight;
                        if(second) e->weight=second[e->index]*e->weight;
                        e->weight=weight*e->weight;
                    }
                    CallGameMethod<void,0x00574860>(&copy);
                    const bool accepted=copy.weight>0.0f && copy.total>0.0f;
                    // Rejection retains the element-end cursor from 574860 in EAX.
                    const uintptr_t rejected=reinterpret_cast<uintptr_t>(copy.end);
                    const uintptr_t appended=CallGameMethod<uintptr_t,0x005753c0>(&d,&copy);
                    result=accepted?appended:rejected;
                }
            }
        }
    }
    return result;
}

namespace {
uint32_t FloatBits(float value) { uint32_t bits; std::memcpy(&bits,&value,4); return bits; }
__m128 LaneBits(uint32_t bits) { return _mm_castsi128_ps(_mm_cvtsi32_si128(static_cast<int>(bits))); }
int32_t SelectedBone(const IkState* state,int32_t mode) {
    // The indexed load starts at orientation.w, not at first. Mode zero uses its bit pattern as an index.
    int32_t values[4]; std::memcpy(values,state->orientation.lane+3,4);
    values[1]=state->first;values[2]=state->middle;values[3]=state->last;
    return values[mode];
}
void ClearPoseDirty(Pose* pose) {
    uint16_t flags;std::memcpy(&flags,pose->opaque4c+4,2);flags&=0xfffe;
    std::memcpy(pose->opaque4c+4,&flags,2);
}
float SinOriginal(float value) {
#if defined(_M_IX86)
    float result; __asm { fld value } __asm { fsin } __asm { fstp result } return result;
#else
    return std::sin(value);
#endif
}
float CosOriginal(float value) {
#if defined(_M_IX86)
    float result; __asm { fld value } __asm { fcos } __asm { fstp result } return result;
#else
    return std::cos(value);
#endif
}
float SqrtRatioOriginal(float numerator,float denominator) {
#if defined(_M_IX86)
    float result; __asm { fld numerator } __asm { fdiv denominator } __asm { fsqrt } __asm { fstp result } return result;
#else
    return static_cast<float>(std::sqrt(static_cast<double>(numerator)/denominator));
#endif
}
}
// Register-shaped temporaries intentionally retain the instruction-level grouping:
// two RSQRTPS refinements, no fused operations, and the original quaternion lane order.
extern "C" uintptr_t M89SolveWithResidue(IkState* state,const ts3::math::Float4Value* residue) {
  alignas(16) __m128 block159[4];
  __m128& v159=block159[0];
  __m128& v160=block159[1];
  __m128& v161=block159[2];
  __m128& v162=block159[3];
  alignas(16) __m128 block172[4];
  __m128& v172=block172[0];
  __m128& v173=block172[1];
  __m128& v174=block172[2];
  alignas(16) __m128 block167[4];
  __m128& v167=block167[0];
  __m128& v168=block167[1];
  __m128& v169=block167[2];
  __m128& v170=block167[3];
  alignas(16) __m128 block163[4];
  __m128& v163=block163[0];
  __m128& v164=block163[1];
  __m128& v165=block163[2];
  __m128& v166=block163[3];

  uintptr_t v1;
  int32_t v2;
  int32_t v3;
  __m128 v4;
  __m128 v5;
  __m128 *v6;
  __m128 v7;
  __m128 v8;
  __m128 v9;
  __m128 v10;
  uintptr_t v11;
  int32_t v12;
  int v13;
  __m128 v14;
  __m128 v15;
  __m128 *v16;
  __m128 v17;
  __m128 v18;
  __m128 v19;
  __m128 v20;
  __m128 v21;
  __m128 v22;
  __m128 *v23;
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
  __m128 v37;
  __m128 v38;
  __m128 v39;
  __m128 v40;
  __m128 v41;
  __m128 v42;
  __m128 v43;
  __m128 v44;
  __m128 v45;
  __m128 v46;
  float v47;
  __m128 v48;
  __m128 v49;
  float v50;
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
  __m128 v66;
  __m128 v67;
  __m128 v68;
  int32_t v69;
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
  uintptr_t v83;
  __m128 v84;
  int32_t v85;
  int v86;
  __m128 v87;
  __m128 v88;
  __m128 v89;
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
  int32_t v102;
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
  __m128 v116;
  __m128 v117;
  __m128 v118;
  uintptr_t result;
  unsigned int v120;
  unsigned int v121;
  float v122;
  __m128 v123;
  uintptr_t v124;
  __m128 *v125;
  unsigned int v126;
  uintptr_t v127;
  int v128;
  int v129;
  float v130;
  __m128 v131;
  __m128 v132;
  __m128 v133;
  __m128 v134;
  __m128 v135;
  __m128 v136;
  __m128 v137;
  __m128 v138;
  __m128 v139;
  __m128 v140;
  __m128 v141;
  __m128 v142;
  __m128 v143;
  int32_t v144;
  int32_t v145;
  int32_t v146;
  __m128 v147;
  int32_t v148;
  int32_t v149;
  __m128 v150;
  __m128 v151;
  int32_t v152;
  float v153;
  __m128 v154;
  __m128 v155;
  __m128 v156;
  __m128 v157;
  __m128 v158;
  alignas(16) __m128 v171;
  __m128 v175;
  __m128 v176;
  __m128 v177;
  alignas(16) __m128 v178[4];
  alignas(16) __m128 v179[4];
  alignas(16) __m128 v180[4];

  block159[state->mode]=_mm_loadu_ps(residue->lane);
  v1 = reinterpret_cast<uintptr_t>(state->world);
  v124 = reinterpret_cast<uintptr_t>(state->local);
  v2 = state->mode;
  v3 = SelectedBone(state, v2);
  v127 = v1;
  if ( v2 == 1 )
  {
    const int32_t parent=b21::ChainIndex(state->skeleton,v3);
    v123.m128_i32[3] = 0;
    v123.m128_i32[0] = 0;
    *(uint64_t *)((char *)v123.m128_u64 + 4) = 1065353216;
    v4 = v123;
    v123.m128_u64[1] = 1065353216;
    v123.m128_u64[0] = 0;
    v131 = v4;
    v5 = v123;
    v123.m128_u64[1] = 0x3F80000000000000LL;
    v132 = v5;
    CallGameMethod<void,0x00577240>(state->world+parent,v178);
    v157 = _mm_shuffle_ps(_mm_load_ps(state->target.lane), _mm_load_ps(state->target.lane), 0);
    v123.m128_i32[0] = v157.m128_i32[0];
    v150 = _mm_shuffle_ps(_mm_load_ps(state->target.lane), _mm_load_ps(state->target.lane), 85);
    v142 = v123;
    v142.m128_i32[1] = v150.m128_i32[0];
    v130 = _mm_shuffle_ps(_mm_load_ps(state->target.lane), _mm_load_ps(state->target.lane), 170).m128_f32[0];
    v143 = v142;
    v143.m128_f32[2] = v130;
    v6 = (__m128 *)ts3::math::b09::Kernel561E60(v179,v178,&v171);
    v7 = v6[1];
    v8 = v6[2];
    v9 = v6[3];
    v10 = _mm_shuffle_ps(
            _mm_setzero_ps(),
            _mm_shuffle_ps(_mm_setzero_ps(), _mm_shuffle_ps(LaneBits(0x3F800000u), LaneBits(0x3F800000u), 0), 10),
            132);
    *(__m128 *)(48 * v3 + v124) = _mm_add_ps(
                                    _mm_add_ps(
                                      _mm_add_ps(
                                        _mm_mul_ps(
                                          _mm_shuffle_ps(v10, v10, 0),
                                          _mm_add_ps(
                                            _mm_add_ps(
                                              _mm_add_ps(
                                                _mm_mul_ps(_mm_shuffle_ps(*v6, *v6, 85), v131),
                                                _mm_mul_ps(_mm_shuffle_ps(*v6, *v6, 0), LaneBits(0x3F800000u))),
                                              _mm_mul_ps(_mm_shuffle_ps(*v6, *v6, 170), v132)),
                                            _mm_mul_ps(_mm_shuffle_ps(*v6, *v6, 255), v143))),
                                        _mm_mul_ps(
                                          _mm_shuffle_ps(v10, v10, 85),
                                          _mm_add_ps(
                                            _mm_add_ps(
                                              _mm_add_ps(
                                                _mm_mul_ps(_mm_shuffle_ps(v7, v7, 85), v131),
                                                _mm_mul_ps(_mm_shuffle_ps(v7, v7, 0), LaneBits(0x3F800000u))),
                                              _mm_mul_ps(_mm_shuffle_ps(v7, v7, 170), v132)),
                                            _mm_mul_ps(_mm_shuffle_ps(v7, v7, 255), v143)))),
                                      _mm_mul_ps(
                                        _mm_shuffle_ps(v10, v10, 170),
                                        _mm_add_ps(
                                          _mm_add_ps(
                                            _mm_add_ps(
                                              _mm_mul_ps(_mm_shuffle_ps(v8, v8, 85), v131),
                                              _mm_mul_ps(_mm_shuffle_ps(v8, v8, 0), LaneBits(0x3F800000u))),
                                            _mm_mul_ps(_mm_shuffle_ps(v8, v8, 170), v132)),
                                          _mm_mul_ps(_mm_shuffle_ps(v8, v8, 255), v143)))),
                                    _mm_mul_ps(
                                      _mm_shuffle_ps(v10, v10, 255),
                                      _mm_add_ps(
                                        _mm_add_ps(
                                          _mm_add_ps(
                                            _mm_mul_ps(_mm_shuffle_ps(v9, v9, 85), v131),
                                            _mm_mul_ps(_mm_shuffle_ps(v9, v9, 0), LaneBits(0x3F800000u))),
                                          _mm_mul_ps(_mm_shuffle_ps(v9, v9, 170), v132)),
                                        _mm_mul_ps(_mm_shuffle_ps(v9, v9, 255), v143))));
    v11 = reinterpret_cast<uintptr_t>(state->pose);
    v160 = _mm_load_ps(state->orientation.lane);
    ClearPoseDirty(state->pose);
    CallGameMethod<uintptr_t,0x00574500>(state->pose,state->first, v3 - state->first + 1);
  }
  else if ( v2 != 2 )
  {
    v12 = state->middle;
    v13 = 48 * state->first;
    v14 = *(__m128 *)(v13 + v1);
    v15 = _mm_sub_ps(_mm_load_ps(state->target.lane), v14);
    v16 = (__m128 *)(v13 + v1);
    v17 = _mm_mul_ps(v15, v15);
    v18 = _mm_load_ps(state->pole.lane);
    v128 = v13;
    v19 = _mm_add_ps(_mm_add_ps(_mm_shuffle_ps(v17, v17, 2), _mm_shuffle_ps(v17, v17, 1)), v17);
    v155 = _mm_shuffle_ps(v19, v19, 0);
    v20 = *(__m128 *)(v1 + 48 * v12);
    v21 = _mm_sub_ps(v20, v14);
    v22 = _mm_mul_ps(v21, v21);
    v23 = (__m128 *)(48 * v12 + v1);
    v24 = _mm_add_ps(_mm_add_ps(_mm_shuffle_ps(v22, v22, 2), _mm_shuffle_ps(v22, v22, 1)), v22);
    v156 = _mm_shuffle_ps(v24, v24, 0);
    v129 = 48 * v12;
    v25 = _mm_sub_ps(*(__m128 *)(v127 + 48 * v3), v20);
    v26 = _mm_mul_ps(v25, v25);
    v27 = _mm_add_ps(_mm_add_ps(_mm_shuffle_ps(v26, v26, 2), _mm_shuffle_ps(v26, v26, 1)), v26);
    v139 = _mm_shuffle_ps(v27, v27, 0);
    v134 = v15;
    v28 = _mm_sub_ps(v18, v14);
    v136 = _mm_shuffle_ps(v15, v15, 18);
    v137 = _mm_shuffle_ps(v15, v15, 9);
    v29 = _mm_sub_ps(_mm_mul_ps(_mm_shuffle_ps(v28, v28, 9), v136), _mm_mul_ps(_mm_shuffle_ps(v28, v28, 18), v137));
    v30 = _mm_mul_ps(v29, v29);
    v31 = _mm_add_ps(_mm_add_ps(_mm_shuffle_ps(v30, v30, 2), _mm_shuffle_ps(v30, v30, 1)), v30);
    v32 = _mm_shuffle_ps(v31, v31, 0);
    v125 = v16;
    v33 = _mm_rsqrt_ps(v32);
    v34 = _mm_shuffle_ps(LaneBits(0x3F800000u), LaneBits(0x3F800000u), 0);
    v35 = _mm_shuffle_ps(LaneBits(0x3F000000u), LaneBits(0x3F000000u), 0);
    v36 = _mm_add_ps(_mm_mul_ps(_mm_sub_ps(v34, _mm_mul_ps(_mm_mul_ps(v33, v33), v32)), _mm_mul_ps(v35, v33)), v33);
    v37 = _mm_add_ps(_mm_mul_ps(_mm_sub_ps(v34, _mm_mul_ps(_mm_mul_ps(v36, v36), v32)), _mm_mul_ps(v35, v36)), v36);
    v38 = _mm_shuffle_ps(LaneBits(0x40000000u), LaneBits(0x40000000u), 0);
    v151 = _mm_div_ps(_mm_shuffle_ps(_mm_set_ss(state->angle), _mm_set_ss(state->angle), 0), v38);
    v39 = _mm_mul_ps(v37, v29);
    v120=FloatBits(SinOriginal(v151.m128_f32[0]));
    v40 = _mm_shuffle_ps(LaneBits(v120), LaneBits(v120), 0);
    v177 = _mm_mul_ps(_mm_shuffle_ps(v134, v134, 0), v40);
    v176 = _mm_mul_ps(_mm_shuffle_ps(v134, v134, 170), v40);
    v175 = _mm_mul_ps(_mm_shuffle_ps(v134, v134, 85), v40);
    v121=FloatBits(CosOriginal(v151.m128_f32[0]));
    v41 = _mm_shuffle_ps(
            _mm_shuffle_ps(v177, v175, 16),
            _mm_shuffle_ps(v176, _mm_shuffle_ps(LaneBits(v121), LaneBits(v121), 0), 50),
            136);
    v42 = _mm_shuffle_ps(v41, v41, 18);
    v43 = _mm_shuffle_ps(v41, v41, 9);
    v44 = _mm_add_ps(
            _mm_sub_ps(_mm_mul_ps(_mm_shuffle_ps(v39, v39, 18), v43), _mm_mul_ps(_mm_shuffle_ps(v39, v39, 9), v42)),
            _mm_mul_ps(_mm_shuffle_ps(v41, v41, 255), v39));
    v45 = LaneBits(0x2B8CBCCCu);
    v46 = _mm_add_ps(
            _mm_mul_ps(
              _mm_sub_ps(_mm_mul_ps(_mm_shuffle_ps(v44, v44, 18), v43), _mm_mul_ps(_mm_shuffle_ps(v44, v44, 9), v42)),
              v38),
            v39);
    v47 = 0.0;
    if ( v155.m128_f32[0] > 1.0e-12f )
      v47 = (float)((float)((float)(v156.m128_f32[0] - v139.m128_f32[0]) / v155.m128_f32[0]) + 1.0) * 0.5;
    v45.m128_f32[0] = v47;
    v139 = _mm_add_ps(_mm_mul_ps(_mm_shuffle_ps(v45, v45, 0), v134), v14);
    v136 = _mm_sub_ps(_mm_mul_ps(_mm_shuffle_ps(v46, v46, 18), v137), _mm_mul_ps(_mm_shuffle_ps(v46, v46, 9), v136));
    v48 = _mm_mul_ps(v136, v136);
    v49 = _mm_add_ps(_mm_add_ps(_mm_shuffle_ps(v48, v48, 2), _mm_shuffle_ps(v48, v48, 1)), v48);
    v137 = _mm_shuffle_ps(v49, v49, 0);
    v50 = v156.m128_f32[0] - (float)((float)(v155.m128_f32[0] * v47) * v47);
    v122 = v50;
    if ( (float)(v156.m128_f32[0] * state->threshold) >= v50 )
    {
      v122 = (float)((float)(v50 - (float)(v156.m128_f32[0] * state->threshold)) * state->relaxation)
           + (float)(v156.m128_f32[0] * state->threshold);
      if ( v122 < 0.0 )
        v122 = 0.0;
    }
    CallGameMethod<void,0x00577240>(reinterpret_cast<ts3::math::SrtValue*>(v125),block167);
    v51 = *v125;
    v52 = _mm_shuffle_ps(v169, v170, 68);
    v53 = _mm_shuffle_ps(v167, v168, 68);
    v54 = _mm_shuffle_ps(v53, v52, 221);
    v55 = _mm_shuffle_ps(v53, v52, 136);
    v56 = v54;
    v57 = _mm_shuffle_ps(_mm_shuffle_ps(v167, v168, 238), _mm_shuffle_ps(v169, v170, 238), 136);
    v126=FloatBits(SqrtRatioOriginal(v122,v137.m128_f32[0]));
    v58 = _mm_sub_ps(_mm_add_ps(_mm_mul_ps(_mm_shuffle_ps(LaneBits(v126), LaneBits(v126), 0), v136), v139), v51);
    v59 = _mm_add_ps(_mm_mul_ps(_mm_shuffle_ps(v58, v58, 85), v56), _mm_mul_ps(_mm_shuffle_ps(v58, v58, 0), v55));
    v60 = _mm_shuffle_ps(v58, v58, 170);
    v61 = _mm_sub_ps(*v23, v51);
    v154 = _mm_add_ps(v59, _mm_mul_ps(v60, v57));
    v147 = _mm_add_ps(
             _mm_add_ps(_mm_mul_ps(_mm_shuffle_ps(v61, v61, 85), v56), _mm_mul_ps(_mm_shuffle_ps(v61, v61, 0), v55)),
             _mm_mul_ps(_mm_shuffle_ps(v61, v61, 170), v57));
    CallGameFunction<void,0x0057d060>(&v147, &v154, &v135);
    v62 = *(__m128 *)(v128 + v124 + 16);
    v63 = _mm_shuffle_ps(v62, v62, 255);
    v64 = _mm_shuffle_ps(v135, v135, 255);
    v65 = _mm_mul_ps(v135, v62);
    v66 = _mm_add_ps(
            _mm_add_ps(
              _mm_sub_ps(
                _mm_mul_ps(_mm_shuffle_ps(v62, v62, 9), _mm_shuffle_ps(v135, v135, 18)),
                _mm_mul_ps(_mm_shuffle_ps(v62, v62, 18), _mm_shuffle_ps(v135, v135, 9))),
              _mm_mul_ps(v135, v63)),
            _mm_mul_ps(v64, v62));
    v67 = _mm_add_ps(_mm_add_ps(_mm_shuffle_ps(v65, v65, 2), _mm_shuffle_ps(v65, v65, 1)), v65);
    v68 = _mm_shuffle_ps(
            v66,
            _mm_shuffle_ps(v66, _mm_sub_ps(_mm_mul_ps(v64, v63), _mm_shuffle_ps(v67, v67, 0)), 10),
            132);
    *(__m128 *)(v128 + v124 + 16) = v68;
    ClearPoseDirty(state->pose);
    v69 = state->first;
    v160 = v68;
    CallGameMethod<uintptr_t,0x00574500>(state->pose,v69, v3 - v69 + 1);
    CallGameMethod<void,0x00577240>(state->world+state->middle,block163);
    v70 = _mm_shuffle_ps(v163, v164, 68);
    v71 = _mm_shuffle_ps(v165, v166, 68);
    v72 = _mm_shuffle_ps(v70, v71, 136);
    v73 = _mm_shuffle_ps(v70, v71, 221);
    v74 = _mm_sub_ps(_mm_load_ps(state->target.lane), *v23);
    v135 = *v23;
    v133 = _mm_shuffle_ps(_mm_shuffle_ps(v163, v164, 238), _mm_shuffle_ps(v165, v166, 238), 136);
    v158 = _mm_add_ps(
             _mm_add_ps(_mm_mul_ps(_mm_shuffle_ps(v74, v74, 0), v72), _mm_mul_ps(_mm_shuffle_ps(v74, v74, 85), v73)),
             _mm_mul_ps(_mm_shuffle_ps(v74, v74, 170), v133));
    CallGameMethod<void,0x00577240>(state->world+state->middle,v180);
    v75 = _mm_sub_ps(*(__m128 *)(48 * v3 + v127), v135);
    v141 = _mm_add_ps(
             _mm_add_ps(_mm_mul_ps(_mm_shuffle_ps(v75, v75, 0), v72), _mm_mul_ps(_mm_shuffle_ps(v75, v75, 85), v73)),
             _mm_mul_ps(_mm_shuffle_ps(v75, v75, 170), v133));
    CallGameFunction<void,0x0057d060>(&v141, &v158, &v140);
    v76 = *(__m128 *)(v129 + v124 + 16);
    v77 = _mm_shuffle_ps(v76, v76, 255);
    v78 = _mm_shuffle_ps(v140, v140, 255);
    v79 = _mm_mul_ps(v140, v76);
    v80 = _mm_add_ps(
            _mm_add_ps(
              _mm_sub_ps(
                _mm_mul_ps(_mm_shuffle_ps(v76, v76, 9), _mm_shuffle_ps(v140, v140, 18)),
                _mm_mul_ps(_mm_shuffle_ps(v76, v76, 18), _mm_shuffle_ps(v140, v140, 9))),
              _mm_mul_ps(v140, v77)),
            _mm_mul_ps(v78, v76));
    v81 = _mm_add_ps(_mm_add_ps(_mm_shuffle_ps(v79, v79, 2), _mm_shuffle_ps(v79, v79, 1)), v79);
    v82 = _mm_shuffle_ps(
            v80,
            _mm_shuffle_ps(v80, _mm_sub_ps(_mm_mul_ps(v78, v77), _mm_shuffle_ps(v81, v81, 0)), 10),
            132);
    *(__m128 *)(v129 + v124 + 16) = v82;
    v83 = reinterpret_cast<uintptr_t>(state->pose);
    v161 = v82;
    v84 = _mm_load_ps(state->orientation.lane);
    ClearPoseDirty(state->pose);
    v85 = state->first;
    v162 = v84;
    CallGameMethod<uintptr_t,0x00574500>(state->pose,v85, v3 - v85 + 1);
  }
  v86 = 48 * v3;
  CallGameMethod<void,0x00577240>(state->world+v3,block172);
  v87 = _mm_shuffle_ps(v172, v172, 0);
  v88 = _mm_shuffle_ps(v173, v173, 85);
  v89 = _mm_add_ps(v87, v88);
  v90 = _mm_shuffle_ps(v174, v174, 170);
  v140 = _mm_add_ps(v89, v90);
  v159.m128_i32[0] = v140.m128_i32[0];
  v153 = _mm_shuffle_ps(_mm_setzero_ps(), _mm_setzero_ps(), 0).m128_f32[0];
  v141 = v89;
  v91 = _mm_shuffle_ps(LaneBits(0x3F000000u), LaneBits(0x3F000000u), 0);
  if ( v140.m128_f32[0] > v153 )
  {
    v92 = _mm_sqrt_ps(_mm_add_ps(_mm_shuffle_ps(LaneBits(0x3F800000u), LaneBits(0x3F800000u), 0), v140));
    v93 = _mm_div_ps(v91, v92);
    v94 = _mm_shuffle_ps(
            _mm_shuffle_ps(
              _mm_mul_ps(_mm_sub_ps(_mm_shuffle_ps(v173, v173, 170), _mm_shuffle_ps(v174, v174, 85)), v93),
              _mm_mul_ps(_mm_sub_ps(_mm_shuffle_ps(v174, v174, 0), _mm_shuffle_ps(v172, v172, 170)), v93),
              16),
            _mm_shuffle_ps(
              _mm_mul_ps(_mm_sub_ps(_mm_shuffle_ps(v172, v172, 85), _mm_shuffle_ps(v173, v173, 0)), v93),
              _mm_mul_ps(v92, v91),
              50),
            136);
LABEL_18:
    v97 = v94;
    goto LABEL_19;
  }
  v144 = v87.m128_i32[0];
  v149 = v88.m128_i32[0];
  if ( !(v87.m128_f32[0] > v88.m128_f32[0])
    || (v148 = v87.m128_i32[0], v152 = v90.m128_i32[0], !(v87.m128_f32[0] > v90.m128_f32[0])) )
  {
    v145 = v88.m128_i32[0];
    v146 = v90.m128_i32[0];
    if ( !(v88.m128_f32[0] > v90.m128_f32[0]) )
    {
      v100 = _mm_sqrt_ps(_mm_add_ps(_mm_shuffle_ps(LaneBits(0x3F800000u), LaneBits(0x3F800000u), 0), _mm_sub_ps(v90, v141)));
      v101 = _mm_div_ps(v91, v100);
      v94 = _mm_shuffle_ps(
              _mm_shuffle_ps(
                _mm_mul_ps(_mm_add_ps(_mm_shuffle_ps(v172, v172, 170), _mm_shuffle_ps(v174, v174, 0)), v101),
                _mm_mul_ps(_mm_add_ps(_mm_shuffle_ps(v173, v173, 170), _mm_shuffle_ps(v174, v174, 85)), v101),
                16),
              _mm_shuffle_ps(
                _mm_mul_ps(v91, v100),
                _mm_mul_ps(_mm_sub_ps(_mm_shuffle_ps(v172, v172, 85), _mm_shuffle_ps(v173, v173, 0)), v101),
                50),
              136);
      goto LABEL_18;
    }
    v98 = _mm_sqrt_ps(
            _mm_add_ps(
              _mm_shuffle_ps(LaneBits(0x3F800000u), LaneBits(0x3F800000u), 0),
              _mm_sub_ps(v88, _mm_add_ps(v90, v87))));
    v99 = _mm_div_ps(v91, v98);
    v97 = _mm_shuffle_ps(
            _mm_shuffle_ps(
              _mm_mul_ps(_mm_add_ps(_mm_shuffle_ps(v172, v172, 85), _mm_shuffle_ps(v173, v173, 0)), v99),
              _mm_mul_ps(v91, v98),
              16),
            _mm_shuffle_ps(
              _mm_mul_ps(_mm_add_ps(_mm_shuffle_ps(v173, v173, 170), _mm_shuffle_ps(v174, v174, 85)), v99),
              _mm_mul_ps(_mm_sub_ps(_mm_shuffle_ps(v174, v174, 0), _mm_shuffle_ps(v172, v172, 170)), v99),
              50),
            136);
  }
  else
  {
    v95 = _mm_sqrt_ps(
            _mm_add_ps(
              _mm_shuffle_ps(LaneBits(0x3F800000u), LaneBits(0x3F800000u), 0),
              _mm_sub_ps(v87, _mm_add_ps(v90, v88))));
    v96 = _mm_div_ps(v91, v95);
    v97 = _mm_shuffle_ps(
            _mm_shuffle_ps(
              _mm_mul_ps(v95, v91),
              _mm_mul_ps(v96, _mm_add_ps(_mm_shuffle_ps(v172, v172, 85), _mm_shuffle_ps(v173, v173, 0))),
              16),
            _mm_shuffle_ps(
              _mm_mul_ps(_mm_add_ps(_mm_shuffle_ps(v172, v172, 170), _mm_shuffle_ps(v174, v174, 0)), v96),
              _mm_mul_ps(_mm_sub_ps(_mm_shuffle_ps(v173, v173, 170), _mm_shuffle_ps(v174, v174, 85)), v96),
              50),
            136);
  }
LABEL_19:
  v102 = state->mode;
  v138 = v97;
  v103 = _mm_shuffle_ps(
           _mm_shuffle_ps(
             _mm_sub_ps(_mm_setzero_ps(), _mm_shuffle_ps(v97, v97, 0)),
             _mm_sub_ps(_mm_setzero_ps(), _mm_shuffle_ps(v97, v97, 85)),
             16),
           _mm_shuffle_ps(_mm_sub_ps(_mm_setzero_ps(), _mm_shuffle_ps(v97, v97, 170)), _mm_shuffle_ps(v97, v97, 255), 50),
           136);
  v104 = _mm_shuffle_ps(v103, v103, 255);
  v105 = block159[v102];
  v106 = _mm_add_ps(
           _mm_sub_ps(
             _mm_mul_ps(_mm_shuffle_ps(v103, v103, 9), _mm_shuffle_ps(v105, v105, 18)),
             _mm_mul_ps(_mm_shuffle_ps(v103, v103, 18), _mm_shuffle_ps(v105, v105, 9))),
           _mm_mul_ps(v105, v104));
  v107 = _mm_shuffle_ps(v105, v105, 255);
  v108 = _mm_mul_ps(v105, v103);
  v109 = _mm_add_ps(v106, _mm_mul_ps(v107, v103));
  v110 = _mm_add_ps(_mm_add_ps(_mm_shuffle_ps(v108, v108, 2), _mm_shuffle_ps(v108, v108, 1)), v108);
  v111 = _mm_shuffle_ps(
           v109,
           _mm_shuffle_ps(v109, _mm_sub_ps(_mm_mul_ps(v107, v104), _mm_shuffle_ps(v110, v110, 0)), 10),
           132);
  v112 = *(__m128 *)(v86 + v124 + 16);
  v113 = _mm_shuffle_ps(v112, v112, 255);
  v114 = _mm_add_ps(
           _mm_sub_ps(
             _mm_mul_ps(_mm_shuffle_ps(v111, v111, 18), _mm_shuffle_ps(v112, v112, 9)),
             _mm_mul_ps(_mm_shuffle_ps(v111, v111, 9), _mm_shuffle_ps(v112, v112, 18))),
           _mm_mul_ps(v113, v111));
  v115 = _mm_shuffle_ps(v111, v111, 255);
  v116 = _mm_mul_ps(v112, v111);
  v117 = _mm_add_ps(v114, _mm_mul_ps(*(__m128 *)(v86 + v124 + 16), v115));
  v118 = _mm_add_ps(_mm_add_ps(_mm_shuffle_ps(v116, v116, 2), _mm_shuffle_ps(v116, v116, 1)), v116);
  *(__m128 *)(v86 + v124 + 16) = _mm_shuffle_ps(
                                   v117,
                                   _mm_shuffle_ps(
                                     v117,
                                     _mm_sub_ps(_mm_mul_ps(v115, v113), _mm_shuffle_ps(v118, v118, 0)),
                                     10),
                                   132);
  result = CallGameMethod<uintptr_t,0x00574500>(state->pose,state->first, state->count);
  ClearPoseDirty(state->pose);
  return result;

}
#if defined(_M_IX86)
// Capture the original function's untouched local quaternion BEFORE the C++ frame
// can overwrite that memory. Original frame = align_down(entryESP-4,16)-0x450.
__declspec(naked) uintptr_t Solve(IkState*) {
    __asm {
        mov eax,esp
        sub eax,4
        and eax,0fffffff0h
        sub eax,450h
        mov ecx,[esp+4]
        mov edx,[ecx+7ch]
        shl edx,4
        movaps xmm0,[eax+edx+250h]
        sub esp,10h
        movups [esp],xmm0
        mov eax,esp
        push eax
        push ecx
        call M89SolveWithResidue
        add esp,18h
        ret
    }
}
#else
uintptr_t Solve(IkState* state) {
    // Native callers with residue-dependent modes use the explicit native form;
    // an x64 caller has no original x86 frame from which to recover these bytes.
    if(state->mode!=1 && state->mode!=3) NotRewritten("0057D670: native caller must supply x86 stack residue via M89SolveWithResidue");
    const ts3::math::Float4Value unused{};
    return M89SolveWithResidue(state,&unused);
}
#endif
namespace hooks {
#if defined(_M_IX86)
Event* __fastcall ConstructEvent2(Event* p,void*,uint32_t a) {return m89::ConstructEvent2(p,a);}
Event* __fastcall ConstructEvent3(Event* p,void*,uint32_t a) {return m89::ConstructEvent3(p,a);}
#endif
}
}
