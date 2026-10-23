#include "ts3/anim/m05_M14.h"
#include "ts3/math/m04_srt.h"
#include "ts3/math/m04_B11.h"
#include <cstring>
#include <cmath>
#include <emmintrin.h>
#include "ts3/anim/m05_M04.h"
#include "ts3/math/m04_B01.h"
#include "ts3/math/m04_sign.h"
#include "ts3/math/m04_maxref.h"
#include <xmmintrin.h>
namespace ts3::anim::m14 {
namespace {
float SelectSign(float test,float negative,float nonnegative) {
    uint32_t t,n,p;
    std::memcpy(&t,&test,4); std::memcpy(&n,&negative,4); std::memcpy(&p,&nonnegative,4);
    uint32_t mask=0u-(t>>31), result=(n&mask)|(p&~mask);
    float f;std::memcpy(&f,&result,4);return f;
}
float Scalar(__m128 f) {return _mm_cvtss_f32(f);}
__m128 Row(__m128 v,const Matrix44Value& m,bool reverseXY=false,bool reverseSum=false) {
    auto x=_mm_mul_ps(_mm_shuffle_ps(v,v,0),_mm_load_ps(m.vector[0].lane));
    auto y=_mm_mul_ps(_mm_shuffle_ps(v,v,0x55),_mm_load_ps(m.vector[1].lane));
    auto z=_mm_mul_ps(_mm_shuffle_ps(v,v,0xaa),_mm_load_ps(m.vector[2].lane));
    auto w=_mm_mul_ps(_mm_shuffle_ps(v,v,0xff),_mm_load_ps(m.vector[3].lane));
    auto sum=reverseXY?_mm_add_ps(y,x):_mm_add_ps(x,y);
    sum=reverseSum?_mm_add_ps(z,sum):_mm_add_ps(sum,z);
    return reverseSum?_mm_add_ps(w,sum):_mm_add_ps(sum,w);
}
__m128 BoundPoint(Evaluator* self,BoundTarget* object) {
    Matrix44Value world;
    CallGameMethod<Matrix44Value*,0x006faf40>(object,&world);
    auto target=_mm_load_ps(world.vector[3].lane);
    auto offset=_mm_load_ps(self->offset.lane);
    auto magnitude=_mm_max_ps(_mm_mul_ps(_mm_set1_ps(-1.0f),offset),offset);
    if(self->targetIndex==-1 && (_mm_movemask_ps(_mm_cmple_ps(magnitude,_mm_set1_ps(0x1p-23f)))&7)==7) {
        auto minimum=_mm_load_ps(object->minimum.lane);
        auto extent=_mm_sub_ps(_mm_load_ps(object->maximum.lane),minimum);
        auto x=_mm_mul_ps(_mm_shuffle_ps(minimum,minimum,0),_mm_load_ps(world.vector[0].lane));
        auto y=_mm_mul_ps(_mm_shuffle_ps(minimum,minimum,0x55),_mm_load_ps(world.vector[1].lane));
        auto z=_mm_mul_ps(_mm_shuffle_ps(minimum,minimum,0xaa),_mm_load_ps(world.vector[2].lane));
        auto base=_mm_add_ps(_mm_add_ps(_mm_add_ps(x,target),y),z);
        auto height=_mm_mul_ps(_mm_shuffle_ps(extent,extent,0x55),_mm_set1_ps(0.75f));
        auto level=_mm_add_ps(_mm_shuffle_ps(base,base,0x55),height);
        auto mixed=_mm_shuffle_ps(target,level,0);
        return _mm_shuffle_ps(mixed,target,0xe8);
    }
    if(self->targetIndex!=-1) {
        Matrix44Value binding;
        if(CallGameVirtual<uint8_t,11>(object,self->targetIndex,&binding)) {
            target=Row(_mm_load_ps(binding.vector[3].lane),world);
            volatile __m128 discarded2=Row(_mm_load_ps(binding.vector[2].lane),world);
            volatile __m128 discarded1=Row(_mm_load_ps(binding.vector[1].lane),world);
            volatile __m128 discarded0=Row(_mm_load_ps(binding.vector[0].lane),world,true);
            (void)discarded2;(void)discarded1;(void)discarded0;
        }
    }
    SrtValue transform;
    Float4Value rotated;
    math::DecomposeSrt(&transform,&world);
    math::b11::RotateVector(reinterpret_cast<__m128*>(&self->offset),reinterpret_cast<__m128*>(&transform),reinterpret_cast<__m128*>(&rotated));
    return _mm_add_ps(_mm_load_ps(rotated.lane),target);
}
}

namespace kernel {
using _DWORD=int32_t;
uint32_t FloatBits(float x){uint32_t b;std::memcpy(&b,&x,4);return b;}
float COERCE_FLOAT(uint32_t b){float x;std::memcpy(&x,&b,4);return x;}
int32_t COERCE_INT(float x){return static_cast<int32_t>(FloatBits(x));}
uint32_t COERCE_UNSIGNED_INT(float x){return FloatBits(x);}
template<class T> uint32_t& LODWORD(T& x){return *reinterpret_cast<uint32_t*>(&x);}
__m128 Low(uint64_t b){return _mm_castsi128_ps(_mm_set_epi32(0,0,static_cast<int32_t>(b>>32),static_cast<int32_t>(b)));}
__m128 Low(float f){return Low(static_cast<uint64_t>(FloatBits(f)));}
__m128 Low(int b){return Low(static_cast<uint64_t>(static_cast<uint32_t>(b)));}
__m128 Low(uint32_t b){return Low(static_cast<uint64_t>(b));}
float Weight(const uint64_t* p){return *reinterpret_cast<const float*>(p);}
template<unsigned EA> float Tuning(float fallback){
#if defined(_M_IX86)
    (void)fallback;return *reinterpret_cast<const float*>(EA);
#else
    return fallback;
#endif
}
template<class T> __m128* V(T* p){return reinterpret_cast<__m128*>(p);}
__m128* V(uintptr_t p){return reinterpret_cast<__m128*>(p);}
template<class A,class B,class C> void Rotate(A a,B b,C c){math::b11::RotateVector(V(a),V(b),V(c));}
template<class A,class B,class C> void Point(A a,B b,C c){math::b11::TransformPoint(V(a),V(b),V(c));}
template<class A,class B> void Inverse(A a,B b){math::b11::InvertPose(V(a),V(b));}
uintptr_t Srts(m04::Pose* p){return reinterpret_cast<uintptr_t>(CallGameMethod<SrtValue*,0x00574190>(p));}
uintptr_t WorldSrts(m04::Pose* p){return reinterpret_cast<uintptr_t>(CallGameMethod<SrtValue*,0x005749b0>(p));}
void Prepare(m04::Pose* p){CallGameMethod<void,0x00574990>(p);}
void Update(m04::Pose* p,int32_t start,int32_t count){CallGameMethod<void,0x00574500>(p,start,count);}
template<class A,class B,class C> uintptr_t Apply(A a,B b,C c,float w){return CallGameFunction<uintptr_t,0x00555ce0>(V(a),V(b),V(c),w);}
float Min(float a,float b){return _mm_cvtss_f32(_mm_min_ss(_mm_set_ss(a),_mm_set_ss(b)));}
float Max(float a,float b){return _mm_cvtss_f32(_mm_max_ss(_mm_set_ss(a),_mm_set_ss(b)));}
// The hidden CRT call clobbers volatile XMM registers. Keep a C++ call boundary
// so the solver cannot retain SIMD temporaries across this inline-asm block.
__declspec(noinline) float Acos(float x){
#if defined(_M_IX86)
    float out;uintptr_t address=0x00f7edde;
    __asm {fld x
        call address
        fstp out}
    return out;
#else
    return std::acos(x);
#endif
}
__declspec(noinline) float Sin(float x){
#if defined(_M_IX86)
    float out;__asm {fld x
        fsin
        fstp out}
    return out;
#else
    return std::sin(x);
#endif
}
__declspec(noinline) float Cos(float x){
#if defined(_M_IX86)
    float out;__asm {fld x
        fcos
        fstp out}
    return out;
#else
    return std::cos(x);
#endif
}
double Floor(double x){return CallGameFunction<double,0x00f7edf0>(x);}
void Debug(Context* c,void* from,void* to,float weight){
#if defined(_M_IX86)
    auto state=*reinterpret_cast<unsigned char**>(0x011cd800);
    if(state && state[48]) {
        auto renderer=CallGameFunction<unsigned char*,0x006e8330>();
        auto alpha=static_cast<int64_t>(static_cast<double>(weight)*255.0);
        auto key=c->renderer?CallGameMethod<uint64_t,0x0054dcb0>(c->renderer)%6u:0u;
        uint32_t color=(static_cast<uint32_t>(alpha)<<24)|reinterpret_cast<const uint32_t*>(0x00fc13e0)[key];
        CallGameMethod<void,0x0054f260>(renderer+0x328,from,to,color);
        CallGameMethod<void,0x006c87f0>(renderer+0x328,to,0.1f,color);
    }
#else
    (void)c;(void)from;(void)to;(void)weight;
#endif
}
}


namespace kernel {
uintptr_t Solve(Evaluator* self,Context* context,m04::Pose* pose,const SrtValue& local,const SrtValue& inverse,const Float4Value& target,const Float4Value& point,float weight) {
    alignas(16) unsigned char scratch[0xa00];
    uintptr_t result,middlePose;
    double X;
    auto secondIndex=reinterpret_cast<int32_t*>(context->rig->opaquebc)+1;
    uintptr_t v24;
    int v25;
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
    __m128 v66;
    __m128 v67;
    __m128 v68;
    double v70;
    float v71;
    float v72;
    __m128 v73;
    __m128 v74;
    __m128 v75;
    float v76;
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
    __m128 v102;
    __m128 v103;
    __m128 v104;
    double v105;
    double v106;
    float v107;
    float v108;
    __m128 v109;
    __m128 v110;
    float v111;
    __m128 v112;
    __m128 v113;
    __m128 v114;
    __m128 v115;
    __m128 v116;
    __m128 v117;
    __m128 v118;
    __m128 v119;
    __m128 v120;
    __m128 v121;
    __m128 v122;
    __m128 v123;
    __m128 v124;
    __m128 v125;
    __m128 v126;
    __m128 v127;
    __m128 v128;
    __m128 v129;
    __m128 v130;
    __m128 v131;
    __m128 v132;
    __m128 v133;
    __m128 v134;
    __m128 v135;
    __m128 v136;
    __m128 v137;
    __m128 v138;
    double v139;
    double v140;
    float v141;
    float v142;
    float v143;
    float v144;
    double v145;
    double v146;
    float v147;
    __m128 v148;
    __m128 v149;
    __m128 v150;
    __m128 v151;
    __m128 v152;
    __m128 v153;
    __m128 v154;
    __m128 v155;
    __m128 v156;
    __m128 v157;
    __m128 v158;
    __m128 v159;
    __m128 v160;
    __m128 v161;
    __m128 v162;
    __m128 v163;
    __m128 v164;
    __m128 v165;
    __m128 v166;
    __m128 v167;
    __m128 v168;
    __m128 v169;
    __m128 v170;
    __m128 v171;
    __m128 v172;
    __m128 v173;
    double v174;
    double v175;
    float v176;
    float v177;
    bool v178;
    float v180;
    double v181;
    double v182;
    __m128 v184;
    __m128 v185;
    uintptr_t v186;
    __m128 v187;
    __m128 v188;
    uintptr_t v189;
    int v190;
    __m128 v191;
    __m128 v192;
    __m128 v193;
    __m128 v194;
    __m128 v195;
    __m128 v196;
    __m128 v197;
    __m128 v198;
    __m128 v199;
    __m128 v200;
    __m128 v201;
    __m128 v202;
    __m128 v203;
    __m128 v204;
    __m128 v205;
    __m128 v206;
    __m128 v207;
    __m128 v208;
    float v209;
    __m128 v210;
    __m128 v211;
    __m128 v212;
    __m128 v213;
    __m128 v214;
    __m128 v215;
    __m128 v216;
    __m128 v217;
    __m128 v218;
    __m128 v219;
    __m128 v220;
    __m128 v221;
    __m128 v222;
    __m128 v223;
    double v224;
    __m128 v225;
    __m128 v226;
    __m128 v227;
    uintptr_t v229;
    __m128 v230;
    __m128 v231;
    __m128 v232;
    __m128 v233;
    __m128 v234;
    __m128 v235;
    __m128 v236;
    __m128 v237;
    __m128 v238;
    __m128 v239;
    __m128 v240;
    __m128 v241;
    __m128 v242;
    __m128 v243;
    __m128 v244;
    __m128 v245;
    __m128 v246;
    __m128 v247;
    __m128 v248;
    __m128 v249;
    __m128 v250;
    __m128 v251;
    __m128 v252;
    __m128 v253;
    __m128 v254;
    __m128 v255;
    __m128 v256;
    __m128 v257;
    __m128 v258;
    float v259;
    __m128 v260;
    __m128 v261;
    __m128 v262;
    __m128 v263;
    __m128 v264;
    __m128 v265;
    __m128 v266;
    __m128 v267;
    __m128 v268;
    __m128 v269;
    __m128 v270;
    __m128 v271;
    __m128 v272;
    __m128 v273;
    double v274;
    __m128 v275;
    __m128 v276;
    __m128 v277;
    uintptr_t v281;
    uintptr_t v282;
    uintptr_t v283;
    uintptr_t v284;
    uintptr_t v288;
    uintptr_t v330;
    uintptr_t v334;
    uintptr_t v354;
    int32_t* v23;
    int32_t* v228;
    int32_t* v279;
    const int32_t* v280;
    float* v179;
    float* v183;
    int32_t* v351;
    int32_t* v343;
    auto& v285=*reinterpret_cast<float*>(scratch+0xc);
    auto& v286=*reinterpret_cast<float*>(scratch+0xc);
    auto& v287=*reinterpret_cast<float*>(scratch+0xc);
    auto& v294=*reinterpret_cast<float*>(scratch+0x34);
    auto& v295=*reinterpret_cast<float*>(scratch+0x34);
    auto& v296=*reinterpret_cast<unsigned int*>(scratch+0x34);
    auto& v297=*reinterpret_cast<float*>(scratch+0x34);
    auto& v298=*reinterpret_cast<float*>(scratch+0x34);
    auto& v299=*reinterpret_cast<unsigned int*>(scratch+0x34);
    auto& v300=*reinterpret_cast<float*>(scratch+0x34);
    auto& v301=*reinterpret_cast<float*>(scratch+0x34);
    auto& v302=*reinterpret_cast<float*>(scratch+0x34);
    auto& v303=*reinterpret_cast<float*>(scratch+0x34);
    auto& v304=*reinterpret_cast<float*>(scratch+0x38);
    auto& v305=*reinterpret_cast<unsigned int*>(scratch+0x38);
    auto& v306=*reinterpret_cast<unsigned int*>(scratch+0x38);
    auto& v307=*reinterpret_cast<unsigned int*>(scratch+0x38);
    auto& v308=*reinterpret_cast<unsigned int*>(scratch+0x38);
    auto& v309=*reinterpret_cast<unsigned int*>(scratch+0x38);
    auto& v310=*reinterpret_cast<unsigned int*>(scratch+0x38);
    auto& v311=*reinterpret_cast<unsigned int*>(scratch+0x38);
    auto& v312=*reinterpret_cast<unsigned int*>(scratch+0x38);
    auto& v313=*reinterpret_cast<unsigned int*>(scratch+0x38);
    auto& v314=*reinterpret_cast<unsigned int*>(scratch+0x38);
    auto& v315=*reinterpret_cast<unsigned int*>(scratch+0x38);
    auto& v316=*reinterpret_cast<unsigned int*>(scratch+0x38);
    auto& v317=*reinterpret_cast<unsigned int*>(scratch+0x38);
    auto& v318=*reinterpret_cast<unsigned int*>(scratch+0x38);
    auto& v319=*reinterpret_cast<unsigned int*>(scratch+0x38);
    auto& v320=*reinterpret_cast<unsigned int*>(scratch+0x38);
    auto& v321=*reinterpret_cast<unsigned int*>(scratch+0x38);
    auto& v322=*reinterpret_cast<unsigned int*>(scratch+0x38);
    auto& v323=*reinterpret_cast<unsigned int*>(scratch+0x38);
    auto& v324=*reinterpret_cast<unsigned int*>(scratch+0x38);
    auto& v325=*reinterpret_cast<float*>(scratch+0x3c);
    auto& v326=*reinterpret_cast<float*>(scratch+0x3c);
    auto& v327=*reinterpret_cast<float*>(scratch+0x3c);
    auto& v328=*reinterpret_cast<float*>(scratch+0x3c);
    auto& v331=*reinterpret_cast<unsigned int*>(scratch+0x40);
    auto& v332=*reinterpret_cast<unsigned int*>(scratch+0x40);
    auto& v333=*reinterpret_cast<unsigned int*>(scratch+0x40);
    auto& v335=*reinterpret_cast<unsigned int*>(scratch+0x40);
    auto& v336=*reinterpret_cast<float*>(scratch+0x40);
    auto& v337=*reinterpret_cast<int*>(scratch+0x40);
    auto& v338=*reinterpret_cast<int*>(scratch+0x40);
    auto& v339=*reinterpret_cast<float*>(scratch+0x40);
    auto& v340=*reinterpret_cast<unsigned int*>(scratch+0x40);
    auto& v341=*reinterpret_cast<float*>(scratch+0x40);
    auto& v344=*reinterpret_cast<float*>(scratch+0x50);
    auto& v345=*reinterpret_cast<float*>(scratch+0x50);
    auto& v346=*reinterpret_cast<float*>(scratch+0x54);
    auto& v347=*reinterpret_cast<float*>(scratch+0x54);
    auto& v348=*reinterpret_cast<uint64_t (*)[3]>(scratch+0x58);
    auto& v349=*reinterpret_cast<float*>(scratch+0x74);
    auto& v350=*reinterpret_cast<float*>(scratch+0x78);
    auto& v352=*reinterpret_cast<__m128*>(scratch+0x80);
    auto& v353=*reinterpret_cast<float*>(scratch+0x98);
    auto& v355=*reinterpret_cast<__m128*>(scratch+0xa0);
    auto& v356=*reinterpret_cast<__m128*>(scratch+0xb0);
    auto& v358=*reinterpret_cast<__m128*>(scratch+0xd0);
    auto& v359=*reinterpret_cast<__m128*>(scratch+0xe0);
    auto& v360=*reinterpret_cast<__m128*>(scratch+0xf0);
    auto& v361=*reinterpret_cast<__m128*>(scratch+0x100);
    auto& v362=*reinterpret_cast<__m128*>(scratch+0x110);
    auto& v363=*reinterpret_cast<__m128*>(scratch+0x120);
    auto& v364=*reinterpret_cast<float (*)[8]>(scratch+0x138);
    auto& v365=*reinterpret_cast<float (*)[8]>(scratch+0x158);
    auto& v366=*reinterpret_cast<float*>(scratch+0x178);
    auto& v367=*reinterpret_cast<float*>(scratch+0x17c);
    auto& v368=*reinterpret_cast<__m128*>(scratch+0x180);
    auto& v369=*reinterpret_cast<__m128*>(scratch+0x190);
    auto& v370=*reinterpret_cast<__m128*>(scratch+0x1a0);
    auto& v371=*reinterpret_cast<__m128*>(scratch+0x1b0);
    auto& v372=*reinterpret_cast<__m128*>(scratch+0x1c0);
    auto& v373=*reinterpret_cast<__m128*>(scratch+0x1d0);
    auto& v374=*reinterpret_cast<__m128*>(scratch+0x1e0);
    auto& v375=*reinterpret_cast<__m128*>(scratch+0x1f0);
    auto& v376=*reinterpret_cast<__m128*>(scratch+0x200);
    auto& v377=*reinterpret_cast<uint32_t (*)[8]>(scratch+0x218);
    auto& v378=*reinterpret_cast<uint32_t (*)[8]>(scratch+0x238);
    auto& v379=*reinterpret_cast<uint32_t (*)[9]>(scratch+0x258);
    auto& v387=*reinterpret_cast<uint32_t (*)[7]>(scratch+0x298);
    auto& v389=*reinterpret_cast<uint64_t (*)[3]>(scratch+0x2b8);
    auto& v396=*reinterpret_cast<__m128*>(scratch+0x320);
    auto& v397=*reinterpret_cast<unsigned char (*)[8]>(scratch+0x330);
    auto& v398=*reinterpret_cast<__m128*>(scratch+0x340);
    auto& v399=*reinterpret_cast<__m128*>(scratch+0x350);
    auto& v400=*reinterpret_cast<__m128*>(scratch+0x360);
    auto& v401=*reinterpret_cast<__m128*>(scratch+0x370);
    auto& v402=*reinterpret_cast<__m128 (*)[3]>(scratch+0x380);
    auto& v403=*reinterpret_cast<__m128*>(scratch+0x3b0);
    auto& v404=*reinterpret_cast<float*>(scratch+0x3c0);
    auto& v405=*reinterpret_cast<__m128*>(scratch+0x3d0);
    auto& v406=*reinterpret_cast<__m128*>(scratch+0x3e0);
    auto& v407=*reinterpret_cast<__m128*>(scratch+0x3f0);
    auto& v408=*reinterpret_cast<__m128*>(scratch+0x400);
    auto& v409=*reinterpret_cast<__m128*>(scratch+0x410);
    auto& v411=*reinterpret_cast<float*>(scratch+0x430);
    auto& v412=*reinterpret_cast<float*>(scratch+0x440);
    auto& v413=*reinterpret_cast<float*>(scratch+0x450);
    auto& v414=*reinterpret_cast<float*>(scratch+0x460);
    auto& v415=*reinterpret_cast<int*>(scratch+0x470);
    auto& v416=*reinterpret_cast<float*>(scratch+0x480);
    auto& v417=*reinterpret_cast<float*>(scratch+0x490);
    auto& v418=*reinterpret_cast<int32_t*>(scratch+0x4a0);
    auto& v419=*reinterpret_cast<float*>(scratch+0x4b0);
    auto& v420=*reinterpret_cast<__m128 (*)[2]>(scratch+0x4c0);
    auto& v421=*reinterpret_cast<int32_t*>(scratch+0x4e0);
    auto& v422=*reinterpret_cast<__m128*>(scratch+0x4f0);
    auto& v423=*reinterpret_cast<__m128*>(scratch+0x500);
    auto& v424=*reinterpret_cast<int32_t*>(scratch+0x510);
    auto& v425=*reinterpret_cast<float*>(scratch+0x520);
    auto& v426=*reinterpret_cast<float*>(scratch+0x530);
    auto& v427=*reinterpret_cast<float*>(scratch+0x540);
    auto& v429=*reinterpret_cast<int32_t*>(scratch+0x560);
    auto& v430=*reinterpret_cast<__m128 (*)[2]>(scratch+0x570);
    auto& v432=*reinterpret_cast<float*>(scratch+0x5a0);
    auto& v433=*reinterpret_cast<int32_t*>(scratch+0x5b0);
    auto& v434=*reinterpret_cast<float*>(scratch+0x5c0);
    auto& v435=*reinterpret_cast<uint64_t (*)[3]>(scratch+0x5c8);
    auto& v436=*reinterpret_cast<float*>(scratch+0x5e0);
    auto& v437=*reinterpret_cast<int32_t*>(scratch+0x5f0);
    auto& v438=*reinterpret_cast<int32_t*>(scratch+0x600);
    auto& v439=*reinterpret_cast<int32_t*>(scratch+0x610);
    auto& v440=*reinterpret_cast<uint64_t (*)[3]>(scratch+0x618);
    auto& v441=*reinterpret_cast<float*>(scratch+0x630);
    auto& v442=*reinterpret_cast<float*>(scratch+0x640);
    auto& v443=*reinterpret_cast<float*>(scratch+0x650);
    auto& v444=*reinterpret_cast<float*>(scratch+0x660);
    auto& v445=*reinterpret_cast<float*>(scratch+0x670);
    auto& v446=*reinterpret_cast<uint64_t (*)[3]>(scratch+0x678);
    auto& v447=*reinterpret_cast<int32_t*>(scratch+0x690);
    auto& v448=*reinterpret_cast<float*>(scratch+0x6a0);
    auto& v449=*reinterpret_cast<unsigned char (*)[40]>(scratch+0x6b0);
    auto& v456=*reinterpret_cast<__m128*>(scratch+0x770);
    auto& v457=*reinterpret_cast<unsigned char (*)[8]>(scratch+0x780);
    auto& v458=*reinterpret_cast<__m128*>(scratch+0x790);
    auto& v459=*reinterpret_cast<__m128*>(scratch+0x7a0);
    auto& v461=*reinterpret_cast<__m128*>(scratch+0x7c0);
    auto& v462=*reinterpret_cast<unsigned char (*)[40]>(scratch+0x7d0);
    auto& v463=*reinterpret_cast<uint64_t (*)[3]>(scratch+0x7f8);
    auto& v464=*reinterpret_cast<__m128*>(scratch+0x810);
    auto& v469=*reinterpret_cast<__m128*>(scratch+0x860);
    auto& v470=*reinterpret_cast<__m128*>(scratch+0x870);
    auto& v471=*reinterpret_cast<__m128*>(scratch+0x880);
    auto& v472=*reinterpret_cast<__m128*>(scratch+0x890);
    auto& v473=*reinterpret_cast<__m128*>(scratch+0x8a0);
    auto& v474=*reinterpret_cast<__m128*>(scratch+0x8b0);
    auto& v475=*reinterpret_cast<__m128*>(scratch+0x8c0);
    auto& v476=*reinterpret_cast<__m128*>(scratch+0x8d0);
    auto& v477=*reinterpret_cast<__m128*>(scratch+0x8e0);
    auto& v479=*reinterpret_cast<unsigned char (*)[48]>(scratch+0x920);
    auto& v480=*reinterpret_cast<unsigned char (*)[48]>(scratch+0x950);
    v351=reinterpret_cast<int32_t*>(context->rig->opaquebc);
    v23=v351+2;
    *reinterpret_cast<float*>(v348)=weight;
    std::memcpy(&v358,&target,16);
    std::memcpy(v449,&local,48);
    std::memcpy(v462,&inverse,48);
    std::memcpy(v397,&point,16);
    v354 = WorldSrts(pose);
    Prepare(pose);
    v24 = reinterpret_cast<uintptr_t>(context->rig->worldBegin);
    v25 = 3 * *v351;
    v330 = v24 + 48 * *v23;
    v361.m128_u64[1] = 1065353216;
    v361.m128_u64[0] = 0;
    v371 = v361;
    v422 = v361;
    Rotate(&v422, v24 + 16 * v25, &v409);
    v405 = v371;
    Rotate(&v405, v330, &v407);
    v370 = _mm_sub_ps(
             _mm_mul_ps(_mm_shuffle_ps(v407, v407, 18), _mm_shuffle_ps(v409, v409, 9)),
             _mm_mul_ps(_mm_shuffle_ps(v407, v407, 9), _mm_shuffle_ps(v409, v409, 18)));
    v26 = _mm_mul_ps(v409, v409);
    v27 = _mm_add_ps(_mm_add_ps(_mm_shuffle_ps(v26, v26, 2), _mm_shuffle_ps(v26, v26, 1)), v26);
    v28 = _mm_shuffle_ps(v27, v27, 0);
    v29 = _mm_rsqrt_ps(v28);
    v363 = _mm_shuffle_ps(Low(0x3F800000u), Low(0x3F800000u), 0);
    *(__m128 *)&v365[2] = _mm_shuffle_ps(Low(0x3F000000u), Low(0x3F000000u), 0);
    v30 = _mm_mul_ps(
            _mm_add_ps(
              _mm_mul_ps(_mm_sub_ps(v363, _mm_mul_ps(_mm_mul_ps(v29, v29), v28)), _mm_mul_ps(v29, *(__m128 *)&v365[2])),
              v29),
            v409);
    v31 = _mm_mul_ps(v407, v407);
    v32 = _mm_add_ps(_mm_add_ps(_mm_shuffle_ps(v31, v31, 2), _mm_shuffle_ps(v31, v31, 1)), v31);
    v33 = _mm_shuffle_ps(v32, v32, 0);
    v34 = _mm_rsqrt_ps(v33);
    v362 = _mm_mul_ps(
             _mm_add_ps(
               _mm_mul_ps(_mm_sub_ps(v363, _mm_mul_ps(_mm_mul_ps(v34, v34), v33)), _mm_mul_ps(v34, *(__m128 *)&v365[2])),
               v34),
             v407);
    v35 = _mm_mul_ps(v362, v30);
    v36 = _mm_add_ps(_mm_add_ps(_mm_shuffle_ps(v35, v35, 2), _mm_shuffle_ps(v35, v35, 1)), v35);
    v355 = v30;
    v356 = _mm_shuffle_ps(Low(0xBF800000), Low(0xBF800000), 0);
    *(float *)&v331 = Acos(Min(Max(_mm_shuffle_ps(v36, v36, 0).m128_f32[0], v356.m128_f32[0]), v363.m128_f32[0]));
    v37 = _mm_mul_ps(
            _mm_sub_ps(
              _mm_mul_ps(_mm_shuffle_ps(v362, v362, 18), _mm_shuffle_ps(v30, v30, 9)),
              _mm_mul_ps(_mm_shuffle_ps(v362, v362, 9), _mm_shuffle_ps(v30, v30, 18))),
            v370);
    v38 = _mm_add_ps(_mm_add_ps(_mm_shuffle_ps(v37, v37, 2), _mm_shuffle_ps(v37, v37, 1)), v37);
    v369 = _mm_setzero_ps();
    v415 = 0;
    v436 = _mm_shuffle_ps(v38, v38, 0).m128_f32[0];
    v39 = _mm_shuffle_ps(Low(v331), Low(v331), 0);
    if ( v436 < 0.0 )
      v39 = _mm_sub_ps(_mm_shuffle_ps(Low(0x40C90FDBu), Low(0x40C90FDBu), 0), v39);
    v40 = _mm_shuffle_ps(Low(0x40000000u), Low(0x40000000u), 0);
    v402[2] = v39;
    v352 = _mm_div_ps(_mm_shuffle_ps(Low(v39.m128_u32[0]), Low(v39.m128_u32[0]), 0), v40);
    v361 = v40;
    *(float *)&v332 = Sin(v352.m128_f32[0]);
    v41 = _mm_shuffle_ps(Low(v332), Low(v332), 0);
    *(float *)&v333 = Cos(v352.m128_f32[0]);
    v42 = Low(v333);



    v477 = _mm_mul_ps(_mm_shuffle_ps(v370, v370, 0), v41);
    v473 = _mm_mul_ps(_mm_shuffle_ps(v370, v370, 170), v41);
    v475 = _mm_mul_ps(_mm_shuffle_ps(v370, v370, 85), v41);
    v334 = v354 + 48 * *v351;
    middlePose = v354 + 48 * *v23;
    v288 = v354 + 48 * v351[4];
    v352 = _mm_shuffle_ps(_mm_shuffle_ps(v477, v475, 16), _mm_shuffle_ps(v473, _mm_shuffle_ps(v42, v42, 0), 50), 136);
    Point(v288, v449, &v396);
    v43 = _mm_sub_ps(v358, v396);
    v44 = _mm_mul_ps(v43, v43);
    v45 = _mm_add_ps(_mm_add_ps(_mm_shuffle_ps(v44, v44, 2), _mm_shuffle_ps(v44, v44, 1)), v44);
    v426 = _mm_shuffle_ps(v45, v45, 0).m128_f32[0];
    v399 = v43;
    v404 = _mm_shuffle_ps(Low(0), Low(0), 0).m128_f32[0];
    if ( v426 > v404 )
    {
      v46 = _mm_add_ps(_mm_add_ps(_mm_shuffle_ps(v44, v44, 2), _mm_shuffle_ps(v44, v44, 1)), v44);
      v47 = _mm_shuffle_ps(v46, v46, 0);
      v48 = _mm_rsqrt_ps(v47);
      v49 = _mm_add_ps(
              _mm_mul_ps(_mm_sub_ps(v363, _mm_mul_ps(_mm_mul_ps(v48, v48), v47)), _mm_mul_ps(*(__m128 *)&v365[2], v48)),
              v48);
      v399 = _mm_mul_ps(
               _mm_add_ps(
                 _mm_mul_ps(
                   _mm_sub_ps(v363, _mm_mul_ps(_mm_mul_ps(v49, v49), v47)),
                   _mm_mul_ps(*(__m128 *)&v365[2], v49)),
                 v49),
               v43);
    }
    Rotate(&v399, v462, &v400);
    v402[0] = v371;
    Rotate(v402, v334, &v408);
    v362 = Low(0x3F80000000000000uLL);
    v406 = Low(0x3F80000000000000uLL);
    Rotate(&v406, middlePose, v457);
    v430[0] = v371;
    Rotate(v430, middlePose, &v376);
    v50 = _mm_shuffle_ps(v352, v352, 18);
    v51 = _mm_shuffle_ps(v352, v352, 9);
    v52 = _mm_add_ps(
            _mm_sub_ps(_mm_mul_ps(_mm_shuffle_ps(v408, v408, 18), v51), _mm_mul_ps(_mm_shuffle_ps(v408, v408, 9), v50)),
            _mm_mul_ps(_mm_shuffle_ps(v352, v352, 255), v408));
    v350 = 0.0;
    v53 = _mm_add_ps(
            _mm_mul_ps(
              _mm_sub_ps(_mm_mul_ps(_mm_shuffle_ps(v52, v52, 18), v51), _mm_mul_ps(_mm_shuffle_ps(v52, v52, 9), v50)),
              v361),
            v408);
    v325 = Tuning<0x0114f2b8>(65.0f) * 0.01745329238474369049f;
    v346 = Tuning<0x0114f2bc>(20.0f) * 0.01745329238474369049f;
    v54 = _mm_mul_ps(v53, v53);
    v355 = v53;
    v55 = _mm_add_ps(_mm_add_ps(_mm_shuffle_ps(v54, v54, 2), _mm_shuffle_ps(v54, v54, 1)), v54);
    v56 = _mm_shuffle_ps(v55, v55, 0);
    v57 = _mm_rsqrt_ps(v56);
    v58 = _mm_mul_ps(
            _mm_add_ps(
              _mm_mul_ps(_mm_sub_ps(v363, _mm_mul_ps(_mm_mul_ps(v57, v57), v56)), _mm_mul_ps(*(__m128 *)&v365[2], v57)),
              v57),
            v53);
    v59 = _mm_mul_ps(v376, v376);
    v60 = _mm_add_ps(_mm_add_ps(_mm_shuffle_ps(v59, v59, 2), _mm_shuffle_ps(v59, v59, 1)), v59);
    v61 = _mm_shuffle_ps(v60, v60, 0);
    v62 = _mm_rsqrt_ps(v61);
    v359 = _mm_mul_ps(
             _mm_add_ps(
               _mm_mul_ps(_mm_sub_ps(v363, _mm_mul_ps(_mm_mul_ps(v62, v62), v61)), _mm_mul_ps(*(__m128 *)&v365[2], v62)),
               v62),
             v376);
    v63 = _mm_mul_ps(v359, v58);
    v64 = _mm_add_ps(_mm_add_ps(_mm_shuffle_ps(v63, v63, 2), _mm_shuffle_ps(v63, v63, 1)), v63);
    *(float *)&v335 = Acos(Min(Max(_mm_shuffle_ps(v64, v64, 0).m128_f32[0], v356.m128_f32[0]), v363.m128_f32[0]));
    v65 = _mm_shuffle_ps(Low(v335), Low(v335), 0);
    v66 = _mm_mul_ps(
            _mm_sub_ps(
              _mm_mul_ps(_mm_shuffle_ps(v376, v376, 18), _mm_shuffle_ps(v355, v355, 9)),
              _mm_mul_ps(_mm_shuffle_ps(v376, v376, 9), _mm_shuffle_ps(v355, v355, 18))),
            _mm_sub_ps(
              _mm_mul_ps(_mm_shuffle_ps(v359, v359, 18), _mm_shuffle_ps(v58, v58, 9)),
              _mm_mul_ps(_mm_shuffle_ps(v359, v359, 9), _mm_shuffle_ps(v58, v58, 18))));
    v67 = _mm_add_ps(_mm_add_ps(_mm_shuffle_ps(v66, v66, 2), _mm_shuffle_ps(v66, v66, 1)), v66);
    v433 = v369.m128_i32[0];
    v412 = _mm_shuffle_ps(v67, v67, 0).m128_f32[0];
    if ( v369.m128_f32[0] <= v412 )
      v68 = v65;
    else
      v68 = _mm_sub_ps(_mm_shuffle_ps(Low(0x40C90FDBu), Low(0x40C90FDBu), 0), v65);
    *(__m128 *)&v348[1] = v68;
    X = static_cast<double>(v68.m128_f32[0]) * 0.15915493667125701904f;
    *(__m128 *)&v435[1] = v68;
    v294 = X;
    v70 = v294 - Floor(X);
    v295 = v70;
    v71 = v295;
    if ( v70 > 0.5 )
      v71 = v295 - 1.0;
    v72 = v71 * 6.2831854820251464844f;
    v336 = v72;
    if ( v72 > 3.1415927410125732422f )
      v336 = v72 - 6.2831854820251464844f;
    v401 = v376;
    *(__m128 *)&v377[2] = v400;
    v401.m128_i32[0] = 0;
    v73 = _mm_mul_ps(v401, v401);
    v74 = _mm_add_ps(_mm_add_ps(_mm_shuffle_ps(v73, v73, 2), _mm_shuffle_ps(v73, v73, 1)), v73);
    v414 = _mm_shuffle_ps(v74, v74, 0).m128_f32[0];
    v75 = _mm_shuffle_ps(Low(0x38D1B717u), Low(0x38D1B717u), 0);
    v437 = v75.m128_i32[0];
    v76 = 0.0;
    v344 = 0.0;
    v377[2] = 0;
    v352 = v75;
    if ( v414 <= v75.m128_f32[0]
      || (v77 = _mm_mul_ps(*(__m128 *)&v377[2], *(__m128 *)&v377[2]),
          v78 = _mm_add_ps(_mm_add_ps(_mm_shuffle_ps(v77, v77, 2), _mm_shuffle_ps(v77, v77, 1)), v77),
          v416 = _mm_shuffle_ps(v78, v78, 0).m128_f32[0],
          v439 = v75.m128_i32[0],
          v416 <= v75.m128_f32[0]) )
    {
      v108 = v346;
    }
    else
    {
      v79 = _mm_add_ps(_mm_add_ps(_mm_shuffle_ps(v73, v73, 2), _mm_shuffle_ps(v73, v73, 1)), v73);
      v80 = _mm_shuffle_ps(v79, v79, 0);
      v81 = _mm_rsqrt_ps(v80);
      v82 = _mm_add_ps(
              _mm_mul_ps(_mm_sub_ps(v363, _mm_mul_ps(_mm_mul_ps(v81, v81), v80)), _mm_mul_ps(*(__m128 *)&v365[2], v81)),
              v81);
      v83 = _mm_add_ps(
              _mm_mul_ps(_mm_sub_ps(v363, _mm_mul_ps(_mm_mul_ps(v82, v82), v80)), _mm_mul_ps(*(__m128 *)&v365[2], v82)),
              v82);
      v84 = _mm_add_ps(_mm_add_ps(_mm_shuffle_ps(v77, v77, 2), _mm_shuffle_ps(v77, v77, 1)), v77);
      v85 = _mm_mul_ps(v83, v401);
      v86 = _mm_shuffle_ps(v84, v84, 0);
      v87 = _mm_rsqrt_ps(v86);
      v88 = _mm_add_ps(
              _mm_mul_ps(_mm_sub_ps(v363, _mm_mul_ps(_mm_mul_ps(v87, v87), v86)), _mm_mul_ps(*(__m128 *)&v365[2], v87)),
              v87);
      v89 = _mm_mul_ps(
              _mm_add_ps(
                _mm_mul_ps(
                  _mm_sub_ps(v363, _mm_mul_ps(_mm_mul_ps(v88, v88), v86)),
                  _mm_mul_ps(v88, *(__m128 *)&v365[2])),
                v88),
              *(__m128 *)&v377[2]);
      v90 = _mm_mul_ps(v85, v85);
      v91 = _mm_add_ps(_mm_add_ps(_mm_shuffle_ps(v90, v90, 2), _mm_shuffle_ps(v90, v90, 1)), v90);
      v92 = _mm_shuffle_ps(v91, v91, 0);
      v93 = _mm_rsqrt_ps(v92);
      v94 = _mm_mul_ps(
              _mm_add_ps(
                _mm_mul_ps(
                  _mm_sub_ps(v363, _mm_mul_ps(_mm_mul_ps(v93, v93), v92)),
                  _mm_mul_ps(v93, *(__m128 *)&v365[2])),
                v93),
              v85);
      v95 = _mm_mul_ps(v89, v89);
      v96 = _mm_add_ps(_mm_add_ps(_mm_shuffle_ps(v95, v95, 2), _mm_shuffle_ps(v95, v95, 1)), v95);
      v97 = _mm_shuffle_ps(v96, v96, 0);
      v98 = _mm_rsqrt_ps(v97);
      v359 = _mm_mul_ps(
               _mm_add_ps(
                 _mm_mul_ps(
                   _mm_sub_ps(v363, _mm_mul_ps(_mm_mul_ps(v98, v98), v97)),
                   _mm_mul_ps(v98, *(__m128 *)&v365[2])),
                 v98),
               v89);
      v99 = _mm_mul_ps(v359, v94);
      v100 = _mm_add_ps(_mm_add_ps(_mm_shuffle_ps(v99, v99, 2), _mm_shuffle_ps(v99, v99, 1)), v99);
      *(float *)&v296 = Acos(Min(Max(_mm_shuffle_ps(v100, v100, 0).m128_f32[0], v356.m128_f32[0]), v363.m128_f32[0]));
      v355.m128_u64[1] = 0;
      v355.m128_u64[0] = 1065353216;
      v101 = _mm_mul_ps(
               _mm_sub_ps(
                 _mm_mul_ps(_mm_shuffle_ps(v359, v359, 18), _mm_shuffle_ps(v94, v94, 9)),
                 _mm_mul_ps(_mm_shuffle_ps(v359, v359, 9), _mm_shuffle_ps(v94, v94, 18))),
               Low(0x3F800000uLL));
      v102 = _mm_add_ps(_mm_add_ps(_mm_shuffle_ps(v101, v101, 2), _mm_shuffle_ps(v101, v101, 1)), v101);
      v418 = v369.m128_i32[0];
      v441 = _mm_shuffle_ps(v102, v102, 0).m128_f32[0];
      v103 = _mm_shuffle_ps(Low(v296), Low(v296), 0);
      if ( v369.m128_f32[0] <= v441 )
        v104 = v103;
      else
        v104 = _mm_sub_ps(_mm_shuffle_ps(Low(0x40C90FDBu), Low(0x40C90FDBu), 0), v103);
      *(__m128 *)&v348[1] = v104;
      v105 = static_cast<double>(v104.m128_f32[0]) * 0.15915493667125701904f;
      v420[0] = v104;
      v297 = v105;
      v106 = v297 - Floor(v105);
      v298 = v106;
      v107 = v298;
      if ( v106 > 0.5 )
        v107 = v298 - 1.0;
      v76 = v107 * 6.2831854820251464844f;
      v344 = v76;
      if ( v76 > 3.1415927410125732422f )
      {
        v76 = v76 - 6.2831854820251464844f;
        v344 = v76;
      }
      v108 = v346;
      if ( COERCE_FLOAT(LODWORD(v76) & 0x7FFFFFFF) > v346 )
      {
        if ( v76 >= 0.0 )
        {
          v76 = v346;
          v344 = v346;
        }
        else
        {
          v76 = -0.0 - v346;
          v344 = -0.0 - v346;
        }
      }
    }
    *(__m128 *)&v378[2] = v376;
    *(__m128 *)&v379[2] = v400;
    v378[3] = 0;
    v109 = _mm_mul_ps(*(__m128 *)&v378[2], *(__m128 *)&v378[2]);
    v110 = _mm_add_ps(_mm_add_ps(_mm_shuffle_ps(v109, v109, 2), _mm_shuffle_ps(v109, v109, 1)), v109);
    v443 = _mm_shuffle_ps(v110, v110, 0).m128_f32[0];
    v421 = v352.m128_i32[0];
    v111 = 0.0;
    v379[3] = 0;
    if ( v443 <= v352.m128_f32[0]
      || (v112 = _mm_mul_ps(*(__m128 *)&v379[2], *(__m128 *)&v379[2]),
          v113 = _mm_add_ps(_mm_add_ps(_mm_shuffle_ps(v112, v112, 2), _mm_shuffle_ps(v112, v112, 1)), v112),
          v445 = _mm_shuffle_ps(v113, v113, 0).m128_f32[0],
          v447 = v352.m128_i32[0],
          v445 <= v352.m128_f32[0]) )
    {
      v142 = v325;
    }
    else
    {
      v114 = _mm_add_ps(_mm_add_ps(_mm_shuffle_ps(v109, v109, 2), _mm_shuffle_ps(v109, v109, 1)), v109);
      v115 = _mm_shuffle_ps(v114, v114, 0);
      v116 = _mm_rsqrt_ps(v115);
      v117 = _mm_add_ps(
               _mm_mul_ps(
                 _mm_sub_ps(v363, _mm_mul_ps(_mm_mul_ps(v116, v116), v115)),
                 _mm_mul_ps(v116, *(__m128 *)&v365[2])),
               v116);
      v118 = _mm_mul_ps(
               _mm_add_ps(
                 _mm_mul_ps(
                   _mm_sub_ps(v363, _mm_mul_ps(_mm_mul_ps(v117, v117), v115)),
                   _mm_mul_ps(v117, *(__m128 *)&v365[2])),
                 v117),
               *(__m128 *)&v378[2]);
      v119 = _mm_add_ps(_mm_add_ps(_mm_shuffle_ps(v112, v112, 2), _mm_shuffle_ps(v112, v112, 1)), v112);
      v120 = _mm_shuffle_ps(v119, v119, 0);
      v121 = _mm_rsqrt_ps(v120);
      v122 = _mm_add_ps(
               _mm_mul_ps(
                 _mm_sub_ps(v363, _mm_mul_ps(_mm_mul_ps(v121, v121), v120)),
                 _mm_mul_ps(v121, *(__m128 *)&v365[2])),
               v121);
      v123 = _mm_mul_ps(
               _mm_add_ps(
                 _mm_mul_ps(
                   _mm_sub_ps(v363, _mm_mul_ps(_mm_mul_ps(v122, v122), v120)),
                   _mm_mul_ps(v122, *(__m128 *)&v365[2])),
                 v122),
               *(__m128 *)&v379[2]);
      v124 = _mm_mul_ps(v118, v118);
      v125 = _mm_add_ps(_mm_add_ps(_mm_shuffle_ps(v124, v124, 2), _mm_shuffle_ps(v124, v124, 1)), v124);
      v126 = _mm_shuffle_ps(v125, v125, 0);
      v127 = _mm_rsqrt_ps(v126);
      v128 = _mm_mul_ps(
               _mm_add_ps(
                 _mm_mul_ps(
                   _mm_sub_ps(v363, _mm_mul_ps(_mm_mul_ps(v127, v127), v126)),
                   _mm_mul_ps(v127, *(__m128 *)&v365[2])),
                 v127),
               v118);
      v129 = _mm_mul_ps(v123, v123);
      v355 = v128;
      v130 = _mm_add_ps(_mm_add_ps(_mm_shuffle_ps(v129, v129, 2), _mm_shuffle_ps(v129, v129, 1)), v129);
      v131 = _mm_shuffle_ps(v130, v130, 0);
      v132 = _mm_rsqrt_ps(v131);
      v359 = _mm_mul_ps(
               _mm_add_ps(
                 _mm_mul_ps(
                   _mm_sub_ps(v363, _mm_mul_ps(_mm_mul_ps(v132, v132), v131)),
                   _mm_mul_ps(v132, *(__m128 *)&v365[2])),
                 v132),
               v123);
      v133 = _mm_mul_ps(v359, v128);
      v134 = _mm_add_ps(_mm_add_ps(_mm_shuffle_ps(v133, v133, 2), _mm_shuffle_ps(v133, v133, 1)), v133);
      *(float *)&v299 = Acos(Min(Max(_mm_shuffle_ps(v134, v134, 0).m128_f32[0], v356.m128_f32[0]), v363.m128_f32[0]));
      v135 = _mm_mul_ps(
               _mm_sub_ps(
                 _mm_mul_ps(_mm_shuffle_ps(v359, v359, 18), _mm_shuffle_ps(v128, v128, 9)),
                 _mm_mul_ps(_mm_shuffle_ps(v359, v359, 9), _mm_shuffle_ps(v128, v128, 18))),
               v362);
      v136 = _mm_add_ps(_mm_add_ps(_mm_shuffle_ps(v135, v135, 2), _mm_shuffle_ps(v135, v135, 1)), v135);
      v424 = v369.m128_i32[0];
      v425 = _mm_shuffle_ps(v136, v136, 0).m128_f32[0];
      v137 = _mm_shuffle_ps(Low(v299), Low(v299), 0);
      if ( v369.m128_f32[0] <= v425 )
        v138 = v137;
      else
        v138 = _mm_sub_ps(_mm_shuffle_ps(Low(0x40C90FDBu), Low(0x40C90FDBu), 0), v137);
      *(__m128 *)&v348[1] = v138;
      v139 = static_cast<double>(v138.m128_f32[0]) * 0.15915493667125701904f;
      *(__m128 *)&v446[1] = v138;
      v300 = v139;
      v140 = v300 - Floor(v139);
      v301 = v140;
      v141 = v301;
      if ( v140 > 0.5 )
        v141 = v301 - 1.0;
      v111 = v141 * 6.2831854820251464844f;
      if ( v111 > 3.1415927410125732422f )
        v111 = v111 - 6.2831854820251464844f;
      v142 = v325;
      v108 = v346;
      v76 = v344;
      if ( COERCE_FLOAT(LODWORD(v111) & 0x7FFFFFFF) > v325 )
      {
        if ( v111 >= 0.0 )
          v111 = v325;
        else
          v111 = -0.0 - v325;
      }
    }
    if ( v76 > 0.0 )
      v76 = (float)((float)((float)(v76 * 0.1f) - v76)
                  * (float)((float)(COERCE_FLOAT(LODWORD(v111) & 0x7FFFFFFF) / v142)
                          * (float)(COERCE_FLOAT(LODWORD(v111) & 0x7FFFFFFF) / v142)))
          + v76;
    v143 = fabs(v336 * 0.63661974668502807617f);
    v337 = (COERCE_INT(-0.0 - v143) >> 31) & LODWORD(v143);
    v338 = (COERCE_INT(
              -0.0
            - COERCE_FLOAT(
                (COERCE_INT(1.0 - *(float *)&v337) >> 31) & 0x3F800000
              | v337 & ~(COERCE_INT(1.0 - *(float *)&v337) >> 31))) >> 31)
         & ((COERCE_INT(1.0 - *(float *)&v337) >> 31) & 0x3F800000 | v337 & ~(COERCE_INT(1.0 - *(float *)&v337) >> 31));
    LODWORD(v144) = (COERCE_INT(1.0 - *(float *)&v338) >> 31) & 0x3F800000
                  | v338 & ~(COERCE_INT(1.0 - *(float *)&v338) >> 31);
    context->dirty = 1;
    context->attenuation = v144;
    v347 = v144 * Tuning<0x0114f2d8>(1.0f);
    v145 = context->pitch;
    v365[1] = v108 * 2.0;
    v285 = v145;
    v304 = math::b01::Approach(
             v285,
             (float)((float)(-0.0 - v76) * (float)(v144 * Tuning<0x0114f2d8>(1.0f))) + v76,
             context->delta,
             Tuning<0x0114f2b4>(0.6f),
             &v365[1]);
    context->dirty = 1;
    context->evaluatedPitch = v304;
    v146 = context->yaw;
    v367 = v325 * 2.0;
    v286 = v146;
    v147 = v325;
    v302 = math::b01::Approach(
             v286,
             (float)((float)(-0.0 - v111) * v347) + v111,
             context->delta,
             Tuning<0x0114f2b4>(0.6f),
             &v367);
    LODWORD(v339) = (COERCE_INT((float)(-0.0 - v147) - v302) >> 31) & LODWORD(v302)
                  | COERCE_UNSIGNED_INT(-0.0 - v147) & ~(COERCE_INT((float)(-0.0 - v147) - v302) >> 31);
    LODWORD(v303) = (COERCE_INT(v147 - v339) >> 31) & LODWORD(v147) | LODWORD(v339) & ~(COERCE_INT(v147 - v339) >> 31);
    context->dirty = 1;
    context->evaluatedYaw = v303;
    v345 = Tuning<0x0114f2c0>(10.0f) * 0.01745329238474369049f;
    v148 = *(__m128 *)(48 * *v23 + Srts(pose) + 16);
    v149 = _mm_shuffle_ps(v148, v148, 9);
    v150 = _mm_shuffle_ps(v148, v148, 18);
    v151 = _mm_add_ps(
             _mm_sub_ps(
               _mm_mul_ps(_mm_shuffle_ps(v362, v362, 18), v149),
               _mm_mul_ps(_mm_shuffle_ps(v362, v362, 9), v150)),
             _mm_mul_ps(_mm_shuffle_ps(v148, v148, 255), v362));
    v398 = _mm_add_ps(
             _mm_mul_ps(
               _mm_sub_ps(
                 _mm_mul_ps(_mm_shuffle_ps(v151, v151, 18), v149),
                 _mm_mul_ps(_mm_shuffle_ps(v151, v151, 9), v150)),
               v361),
             v362);
    v398.m128_i32[2] = 0;
    v152 = _mm_mul_ps(v398, v398);
    v153 = _mm_add_ps(_mm_add_ps(_mm_shuffle_ps(v152, v152, 2), _mm_shuffle_ps(v152, v152, 1)), v152);
    v427 = _mm_shuffle_ps(v153, v153, 0).m128_f32[0];
    v438 = v352.m128_i32[0];
    if ( v427 > v352.m128_f32[0] )
    {
      v154 = _mm_add_ps(_mm_add_ps(_mm_shuffle_ps(v152, v152, 2), _mm_shuffle_ps(v152, v152, 1)), v152);
      v155 = _mm_shuffle_ps(v154, v154, 0);
      v156 = _mm_rsqrt_ps(v155);
      v157 = _mm_add_ps(
               _mm_mul_ps(
                 _mm_sub_ps(v363, _mm_mul_ps(_mm_mul_ps(v156, v156), v155)),
                 _mm_mul_ps(*(__m128 *)&v365[2], v156)),
               v156);
      v158 = _mm_mul_ps(
               _mm_add_ps(
                 _mm_mul_ps(
                   _mm_sub_ps(v363, _mm_mul_ps(_mm_mul_ps(v157, v157), v155)),
                   _mm_mul_ps(*(__m128 *)&v365[2], v157)),
                 v157),
               v398);
      v159 = _mm_mul_ps(v362, v362);
      v160 = _mm_add_ps(_mm_add_ps(_mm_shuffle_ps(v159, v159, 2), _mm_shuffle_ps(v159, v159, 1)), v159);
      v161 = _mm_shuffle_ps(v160, v160, 0);
      v162 = _mm_rsqrt_ps(v161);
      v163 = _mm_mul_ps(
               _mm_add_ps(
                 _mm_mul_ps(
                   _mm_sub_ps(v363, _mm_mul_ps(_mm_mul_ps(v162, v162), v161)),
                   _mm_mul_ps(*(__m128 *)&v365[2], v162)),
                 v162),
               v362);
      v164 = _mm_mul_ps(v158, v158);
      v165 = _mm_add_ps(_mm_add_ps(_mm_shuffle_ps(v164, v164, 2), _mm_shuffle_ps(v164, v164, 1)), v164);
      v166 = _mm_shuffle_ps(v165, v165, 0);
      v167 = _mm_rsqrt_ps(v166);
      v359 = _mm_mul_ps(
               _mm_add_ps(
                 _mm_mul_ps(
                   _mm_sub_ps(v363, _mm_mul_ps(_mm_mul_ps(v167, v167), v166)),
                   _mm_mul_ps(*(__m128 *)&v365[2], v167)),
                 v167),
               v158);
      v168 = _mm_mul_ps(v359, v163);
      v169 = _mm_add_ps(_mm_add_ps(_mm_shuffle_ps(v168, v168, 2), _mm_shuffle_ps(v168, v168, 1)), v168);
      v352 = v163;
      *(float *)&v340 = Acos(Min(Max(_mm_shuffle_ps(v169, v169, 0).m128_f32[0], v356.m128_f32[0]), v363.m128_f32[0]));
      v170 = _mm_mul_ps(
               _mm_sub_ps(
                 _mm_mul_ps(_mm_shuffle_ps(v359, v359, 18), _mm_shuffle_ps(v163, v163, 9)),
                 _mm_mul_ps(_mm_shuffle_ps(v359, v359, 9), _mm_shuffle_ps(v163, v163, 18))),
               v371);
      v171 = _mm_add_ps(_mm_add_ps(_mm_shuffle_ps(v170, v170, 2), _mm_shuffle_ps(v170, v170, 1)), v170);
      v429 = v369.m128_i32[0];
      v444 = _mm_shuffle_ps(v171, v171, 0).m128_f32[0];
      v172 = _mm_shuffle_ps(Low(v340), Low(v340), 0);
      if ( v369.m128_f32[0] <= v444 )
        v173 = v172;
      else
        v173 = _mm_sub_ps(_mm_shuffle_ps(Low(0x40C90FDBu), Low(0x40C90FDBu), 0), v172);
      *(__m128 *)&v348[1] = v173;
      v174 = static_cast<double>(v173.m128_f32[0]) * 0.15915493667125701904f;
      v430[1] = v173;
      v326 = v174;
      v175 = v326 - Floor(v174);
      v327 = v175;
      v176 = v327;
      if ( v175 > 0.5 )
        v176 = v327 - 1.0;
      v177 = v176 * 6.2831854820251464844f;
      v353 = v177;
      if ( v177 > 3.1415927410125732422f )
        v353 = v177 - 6.2831854820251464844f;
      v328 = fabs((float)(v303 / Tuning<0x0114f2b8>(65.0f)) * Tuning<0x0114f2d4>(1.0f));
      v349 = math::FloatSign((float)(v303 / Tuning<0x0114f2b8>(65.0f)) * Tuning<0x0114f2d4>(1.0f))
           * -1.0
           * (1.0 - v328)
           * v345;
      v341 = math::FloatSign(v349);
      if ( v341 == math::FloatSign(v353) )
      {
        if ( v341 >= 0.0 )
          v178 = v353 <= v349;
        else
          v178 = v349 <= v353;
        v179 = &v353;
        if ( v178 )
          v179 = &v349;
        v180 = *v179;
        v349 = *v179;
      }
      else
      {
        v180 = v349;
      }
      v350 = v180 - v353;
    }
    v181 = context->roll;
    v366 = v345 * 2.0;
    v287 = v181;
    v182 = math::b01::Approach(v287, v350, context->delta, Tuning<0x0114f2b4>(0.6f), &v366);
    context->dirty = 1;
    v350 = v182;
    context->evaluatedRoll = v182;
    v364[1] = 1.0 - v347;
    v183 = (float *)v348;
    if ( (float)(1.0 - v347) <= *(float *)v348 )
      v183 = &v364[1];
    *(float *)v348 = *v183;
    Debug(context, &v396, &v358, Weight(v348));
    v356.m128_u64[1] = 0;
    v356.m128_u64[0] = 1065353216;
    v375 = Low(0x3F800000u);
    v184 = Low(LODWORD(v304));
    v184.m128_f32[0] = v304 * 0.5;
    *(__m128 *)&v348[1] = _mm_div_ps(_mm_shuffle_ps(v184, v184, 0), v361);
    *(float *)&v305 = Sin(*(float *)&v348[1]);
    v185 = _mm_shuffle_ps(Low(v305), Low(v305), 0);
    *(float *)&v306 = Cos(*(float *)&v348[1]);


    v472 = _mm_mul_ps(_mm_shuffle_ps(Low(0x3F800000u), Low(0x3F800000u), 0), v185);

    v474 = _mm_mul_ps(_mm_shuffle_ps(Low(0x3F800000u), Low(0x3F800000u), 170), v185);
    v459 = _mm_mul_ps(_mm_shuffle_ps(Low(0x3F800000u), Low(0x3F800000u), 85), v185);
    v186 = v354 + 48 * *secondIndex;
    v356 = _mm_shuffle_ps(
             _mm_shuffle_ps(v472, v459, 16),
             _mm_shuffle_ps(v474, _mm_shuffle_ps(Low(v306), Low(v306), 0), 50),
             136);
    Inverse(v186, v480);
    *(__m128 *)&v440[1] = v362;
    Rotate(&v440[1], v480, &v374);
    v187 = Low(LODWORD(v303));
    v187.m128_f32[0] = v303 * 0.5;
    v360 = _mm_div_ps(_mm_shuffle_ps(v187, v187, 0), v361);
    *(float *)&v307 = Sin(v360.m128_f32[0]);
    v188 = _mm_shuffle_ps(Low(v307), Low(v307), 0);
    *(float *)&v308 = Cos(v360.m128_f32[0]);
    *(__m128 *)&v463[1] = _mm_mul_ps(_mm_shuffle_ps(v374, v374, 0), v188);
    v461 = _mm_mul_ps(_mm_shuffle_ps(v374, v374, 170), v188);


    v464 = _mm_mul_ps(_mm_shuffle_ps(v374, v374, 85), v188);

    *(__m128 *)&v348[1] = _mm_shuffle_ps(
                            _mm_shuffle_ps(*(__m128 *)&v463[1], v464, 16),
                            _mm_shuffle_ps(v461, _mm_shuffle_ps(Low(v308), Low(v308), 0), 50),
                            136);
    v189 = Srts(pose);
    v190 = 48 * *secondIndex;
    v191 = *(__m128 *)(v189 + v190 + 16);
    *(__m128 *)&v389[1] = _mm_shuffle_ps(v356, v356, 9);
    v192 = _mm_shuffle_ps(v356, v356, 255);
    v368 = _mm_shuffle_ps(v356, v356, 18);
    v193 = _mm_shuffle_ps(v191, v191, 255);
    v194 = _mm_add_ps(
             _mm_add_ps(
               _mm_sub_ps(
                 _mm_mul_ps(_mm_shuffle_ps(v191, v191, 18), *(__m128 *)&v389[1]),
                 _mm_mul_ps(_mm_shuffle_ps(v191, v191, 9), v368)),
               _mm_mul_ps(v192, v191)),
             _mm_mul_ps(v193, v356));
    v195 = _mm_mul_ps(v191, v356);
    v196 = _mm_add_ps(_mm_add_ps(_mm_shuffle_ps(v195, v195, 2), _mm_shuffle_ps(v195, v195, 1)), v195);
    v355 = v192;
    v197 = _mm_shuffle_ps(
             v194,
             _mm_shuffle_ps(v194, _mm_sub_ps(_mm_mul_ps(v192, v193), _mm_shuffle_ps(v196, v196, 0)), 10),
             132);
    v198 = _mm_shuffle_ps(*(__m128 *)&v348[1], *(__m128 *)&v348[1], 255);
    v199 = _mm_shuffle_ps(v197, v197, 255);
    v200 = _mm_sub_ps(
             _mm_mul_ps(_mm_shuffle_ps(v197, v197, 18), _mm_shuffle_ps(*(__m128 *)&v348[1], *(__m128 *)&v348[1], 9)),
             _mm_mul_ps(_mm_shuffle_ps(v197, v197, 9), _mm_shuffle_ps(*(__m128 *)&v348[1], *(__m128 *)&v348[1], 18)));
    v201 = _mm_mul_ps(v198, v197);
    v202 = _mm_mul_ps(v197, *(__m128 *)&v348[1]);
    v203 = _mm_add_ps(_mm_add_ps(v200, v201), _mm_mul_ps(v199, *(__m128 *)&v348[1]));
    v204 = _mm_add_ps(_mm_add_ps(_mm_shuffle_ps(v202, v202, 2), _mm_shuffle_ps(v202, v202, 1)), v202);
    v205 = _mm_shuffle_ps(
             v203,
             _mm_shuffle_ps(v203, _mm_sub_ps(_mm_mul_ps(v199, v198), _mm_shuffle_ps(v204, v204, 0)), 10),
             132);
    v206 = _mm_mul_ps(v191, v205);
    v207 = _mm_add_ps(_mm_shuffle_ps(v206, v206, 14), v206);
    v208 = _mm_add_ps(_mm_shuffle_ps(v207, v207, 1), v207);
    v209 = _mm_shuffle_ps(v208, v208, 0).m128_f32[0];
    v432 = v369.m128_f32[0];
    v448 = v209;
    v210 = _mm_shuffle_ps(Low(LODWORD(v348[0])), Low(LODWORD(v348[0])), 0);
    *(__m128 *)&v387[2] = v205;
    v359 = v210;
    v352 = v191;
    if ( v369.m128_f32[0] > v209 )
    {
      v191 = _mm_sub_ps(v369, v191);
      v352 = v191;
      v209 = v369.m128_f32[0] - v209;
    }
    *(__m128 *)&v348[1] = _mm_shuffle_ps(Low(0x3DB2B8C3u), Low(0x3DB2B8C3u), 0);
    v434 = v209;
    *(__m128 *)&v364[2] = *(__m128 *)&v348[1];
    *(float *)&v309 = Cos(*(float *)&v348[1]);
    v442 = _mm_shuffle_ps(Low(v309), Low(v309), 0).m128_f32[0];
    if ( v209 <= v442 )
    {
      *(float *)&v310 = Acos(v209);
      v223 = _mm_shuffle_ps(Low(v310), Low(v310), 0);
      *(__m128 *)&v348[1] = _mm_mul_ps(_mm_sub_ps(v363, v359), v223);
      v423 = v223;
      v359 = _mm_mul_ps(v223, v359);
      *(float *)&v311 = Sin(*(float *)&v348[1]);
      v224 = Sin(v223.m128_f32[0]);
      v225 = _mm_shuffle_ps(Low(v311), Low(v311), 0);
      *(float *)&v312 = v224;
      v226 = _mm_mul_ps(_mm_div_ps(v225, _mm_shuffle_ps(Low(v312), Low(v312), 0)), v352);
      *(float *)&v313 = Sin(v359.m128_f32[0]);
      v227 = _mm_shuffle_ps(Low(v313), Low(v313), 0);
      *(float *)&v314 = Sin(v423.m128_f32[0]);
      v222 = _mm_add_ps(
               v226,
               _mm_mul_ps(_mm_div_ps(v227, _mm_shuffle_ps(Low(v314), Low(v314), 0)), *(__m128 *)&v387[2]));
    }
    else
    {
      v211 = _mm_mul_ps(v191, v205);
      v212 = _mm_add_ps(_mm_shuffle_ps(v211, v211, 14), v211);
      v213 = _mm_add_ps(_mm_shuffle_ps(v212, v212, 1), v212);
      v214 = _mm_cmplt_ps(v369, _mm_shuffle_ps(v213, v213, 0));
      v215 = _mm_or_ps(
               _mm_andnot_ps(v214, _mm_sub_ps(v191, _mm_mul_ps(_mm_add_ps(v191, v205), v210))),
               _mm_and_ps(_mm_add_ps(_mm_mul_ps(_mm_sub_ps(v205, v191), v210), v191), v214));
      v216 = _mm_mul_ps(v215, v215);
      v217 = _mm_add_ps(_mm_shuffle_ps(v216, v216, 14), v216);
      v218 = _mm_add_ps(_mm_shuffle_ps(v217, v217, 1), v217);
      v219 = _mm_shuffle_ps(v218, v218, 0);
      v220 = _mm_rsqrt_ps(v219);
      v221 = _mm_add_ps(
               _mm_mul_ps(
                 _mm_sub_ps(v363, _mm_mul_ps(_mm_mul_ps(v220, v220), v219)),
                 _mm_mul_ps(v220, *(__m128 *)&v365[2])),
               v220);
      v222 = _mm_mul_ps(
               _mm_add_ps(
                 _mm_mul_ps(
                   _mm_sub_ps(v363, _mm_mul_ps(_mm_mul_ps(v221, v221), v219)),
                   _mm_mul_ps(v221, *(__m128 *)&v365[2])),
                 v221),
               v215);
    }
    v228 = secondIndex;
    v402[1] = v222;
    *(__m128 *)(v190 + v189 + 16) = v222;
    pose->flags &= ~1u;
    Update(pose,*v228, *v23 - *v228 + 1);
    Inverse(v354 + 48 * *v23, v479);
    v229 = 48 * *v23 + Srts(pose);
    v403 = v362;
    Rotate(&v403, v479, &v372);
    *(float *)&v315 = Sin(v360.m128_f32[0]);
    v230 = _mm_shuffle_ps(Low(v315), Low(v315), 0);
    *(float *)&v316 = Cos(v360.m128_f32[0]);
    v469 = _mm_mul_ps(_mm_shuffle_ps(v372, v372, 170), v230);
    v476 = _mm_mul_ps(_mm_shuffle_ps(v372, v372, 0), v230);
    v231 = _mm_mul_ps(_mm_shuffle_ps(v372, v372, 85), v230);
    v232 = _mm_shuffle_ps(
             _mm_shuffle_ps(v476, v231, 16),
             _mm_shuffle_ps(v469, _mm_shuffle_ps(Low(v316), Low(v316), 0), 50),
             136);
    v373 = v371;
    v233 = _mm_div_ps(_mm_shuffle_ps(Low(v350), Low(v350), 0), v361);


    v471 = v231;

    *(float *)&v317 = Sin(v233.m128_f32[0]);
    v360 = v233;
    v234 = _mm_shuffle_ps(Low(v317), Low(v317), 0);
    *(float *)&v318 = Cos(v233.m128_f32[0]);
    v458 = _mm_mul_ps(_mm_shuffle_ps(v371, v371, 0), v234);
    v470 = _mm_mul_ps(_mm_shuffle_ps(v371, v371, 170), v234);
    v456 = _mm_mul_ps(_mm_shuffle_ps(v371, v371, 85), v234);
    v235 = _mm_shuffle_ps(
             _mm_shuffle_ps(v458, v456, 16),
             _mm_shuffle_ps(v470, _mm_shuffle_ps(Low(v318), Low(v318), 0), 50),
             136);



    v236 = *(__m128 *)(v229 + 16);
    v237 = _mm_shuffle_ps(v236, v236, 255);
    v238 = _mm_add_ps(
             _mm_add_ps(
               _mm_sub_ps(
                 _mm_mul_ps(_mm_shuffle_ps(v236, v236, 18), *(__m128 *)&v389[1]),
                 _mm_mul_ps(_mm_shuffle_ps(v236, v236, 9), v368)),
               _mm_mul_ps(v236, v355)),
             _mm_mul_ps(v356, v237));
    v239 = _mm_mul_ps(v236, v356);
    v240 = _mm_add_ps(_mm_add_ps(_mm_shuffle_ps(v239, v239, 2), _mm_shuffle_ps(v239, v239, 1)), v239);
    v241 = _mm_shuffle_ps(
             v238,
             _mm_shuffle_ps(v238, _mm_sub_ps(_mm_mul_ps(v355, v237), _mm_shuffle_ps(v240, v240, 0)), 10),
             132);
    v368 = _mm_shuffle_ps(v232, v232, 255);
    v242 = _mm_shuffle_ps(v241, v241, 255);
    v243 = _mm_add_ps(
             _mm_sub_ps(
               _mm_mul_ps(_mm_shuffle_ps(v232, v232, 9), _mm_shuffle_ps(v241, v241, 18)),
               _mm_mul_ps(_mm_shuffle_ps(v232, v232, 18), _mm_shuffle_ps(v241, v241, 9))),
             _mm_mul_ps(v368, v241));
    v244 = v232;
    v245 = _mm_mul_ps(v232, v241);
    v246 = _mm_add_ps(v243, _mm_mul_ps(v244, v242));
    v247 = _mm_add_ps(_mm_add_ps(_mm_shuffle_ps(v245, v245, 2), _mm_shuffle_ps(v245, v245, 1)), v245);
    v248 = _mm_shuffle_ps(
             v246,
             _mm_shuffle_ps(v246, _mm_sub_ps(_mm_mul_ps(v242, v368), _mm_shuffle_ps(v247, v247, 0)), 10),
             132);
    v249 = _mm_shuffle_ps(v235, v235, 255);
    v250 = _mm_shuffle_ps(v248, v248, 255);
    v251 = _mm_add_ps(
             _mm_sub_ps(
               _mm_mul_ps(_mm_shuffle_ps(v248, v248, 18), _mm_shuffle_ps(v235, v235, 9)),
               _mm_mul_ps(_mm_shuffle_ps(v248, v248, 9), _mm_shuffle_ps(v235, v235, 18))),
             _mm_mul_ps(v249, v248));
    v252 = _mm_mul_ps(v248, v235);
    v253 = _mm_add_ps(v251, _mm_mul_ps(v250, v235));
    v254 = _mm_add_ps(_mm_add_ps(_mm_shuffle_ps(v252, v252, 2), _mm_shuffle_ps(v252, v252, 1)), v252);
    v255 = _mm_shuffle_ps(
             v253,
             _mm_shuffle_ps(v253, _mm_sub_ps(_mm_mul_ps(v250, v249), _mm_shuffle_ps(v254, v254, 0)), 10),
             132);
    v256 = _mm_mul_ps(v236, v255);
    v257 = _mm_add_ps(_mm_shuffle_ps(v256, v256, 14), v256);
    v258 = _mm_add_ps(_mm_shuffle_ps(v257, v257, 1), v257);
    v259 = _mm_shuffle_ps(v258, v258, 0).m128_f32[0];
    v411 = v369.m128_f32[0];
    v413 = v259;
    v260 = _mm_shuffle_ps(Low(LODWORD(v348[0])), Low(LODWORD(v348[0])), 0);
    *(__m128 *)&v387[2] = v255;
    v368 = v260;
    v352 = v236;
    if ( v369.m128_f32[0] > v259 )
    {
      v236 = _mm_sub_ps(v369, v236);
      v352 = v236;
      v259 = v369.m128_f32[0] - v259;
    }
    v417 = v259;
    *(float *)&v319 = Cos(v364[2]);
    v419 = _mm_shuffle_ps(Low(v319), Low(v319), 0).m128_f32[0];
    if ( v259 <= v419 )
    {
      *(float *)&v320 = Acos(v259);
      v273 = _mm_shuffle_ps(Low(v320), Low(v320), 0);
      *(__m128 *)&v364[2] = _mm_mul_ps(_mm_sub_ps(v363, v368), v273);
      *(__m128 *)&v389[1] = v273;
      v360 = v273;
      v368 = _mm_mul_ps(v273, v368);
      *(float *)&v321 = Sin(v364[2]);
      v274 = Sin(v273.m128_f32[0]);
      v275 = _mm_shuffle_ps(Low(v321), Low(v321), 0);
      *(float *)&v322 = v274;
      v276 = _mm_mul_ps(_mm_div_ps(v275, _mm_shuffle_ps(Low(v322), Low(v322), 0)), v352);
      *(float *)&v323 = Sin(v368.m128_f32[0]);
      v277 = _mm_shuffle_ps(Low(v323), Low(v323), 0);
      *(float *)&v324 = Sin(v360.m128_f32[0]);
      v272 = _mm_add_ps(
               v276,
               _mm_mul_ps(_mm_div_ps(v277, _mm_shuffle_ps(Low(v324), Low(v324), 0)), *(__m128 *)&v387[2]));
    }
    else
    {
      v261 = _mm_mul_ps(v236, v255);
      v262 = _mm_add_ps(_mm_shuffle_ps(v261, v261, 14), v261);
      v263 = _mm_add_ps(_mm_shuffle_ps(v262, v262, 1), v262);
      v264 = _mm_cmplt_ps(v369, _mm_shuffle_ps(v263, v263, 0));
      v265 = _mm_or_ps(
               _mm_andnot_ps(v264, _mm_sub_ps(v236, _mm_mul_ps(_mm_add_ps(v236, v255), v260))),
               _mm_and_ps(_mm_add_ps(_mm_mul_ps(_mm_sub_ps(v255, v236), v260), v236), v264));
      v266 = _mm_mul_ps(v265, v265);
      v267 = _mm_add_ps(_mm_shuffle_ps(v266, v266, 14), v266);
      v268 = _mm_add_ps(_mm_shuffle_ps(v267, v267, 1), v267);
      v269 = _mm_shuffle_ps(v268, v268, 0);
      v270 = _mm_rsqrt_ps(v269);
      v271 = _mm_add_ps(
               _mm_mul_ps(
                 _mm_sub_ps(v363, _mm_mul_ps(_mm_mul_ps(v270, v270), v269)),
                 _mm_mul_ps(*(__m128 *)&v365[2], v270)),
               v270);
      v272 = _mm_mul_ps(
               _mm_add_ps(
                 _mm_mul_ps(
                   _mm_sub_ps(v363, _mm_mul_ps(_mm_mul_ps(v271, v271), v269)),
                   _mm_mul_ps(*(__m128 *)&v365[2], v271)),
                 v271),
               v265);
    }
    result = reinterpret_cast<uintptr_t>(v351);
    v420[1] = v272;
    v279 = v351 + 5;
    *(__m128 *)(v229 + 16) = v272;
    if ( *v279 >= 0 )
    {
      result += 24;
      v343 = (int32_t *)result;
      if ( *(int *)result >= 0 )
      {
        pose->flags &= ~1u;
        v280 = (int32_t *)math::MaxSignedReference(v279,reinterpret_cast<int32_t*>(result));
        Update(pose,*secondIndex, *v280 - *secondIndex + 1);
        v281 = v354 + 48 * *v279;
        v282 = Srts(pose);
        Apply(v397, v281, 48 * *v279 + v282, Weight(v348));
        v283 = v354 + 48 * *v343;
        v284 = Srts(pose);
        return Apply(v397, v283, 48 * *v343 + v284, Weight(v348));
      }
    }

  return result;
}
}

uintptr_t EvaluateUnbound(Evaluator* self,Context* context,void* pose) {
    auto target=_mm_load_ps(context->target.lane);
    bool keyed=(self->key[0]|self->key[1])!=0;
    if(keyed) {
        auto object=CallGameVirtual<BoundTarget*,0>(context->registry,self->key[0],self->key[1]);
        if(object) target=BoundPoint(self,object);
    }
    auto step=_mm_div_ss(_mm_set_ss(context->delta),_mm_set_ss(0.5f));
    float weight=keyed?Scalar(_mm_add_ss(step,_mm_set_ss(context->weight))):Scalar(_mm_sub_ss(_mm_set_ss(context->weight),step));
    // These are integer sign-bit selections, not MAXSS/MINSS or ordered comparisons.
    float lowTest=Scalar(_mm_sub_ss(_mm_set_ss(-0.0f),_mm_set_ss(weight)));
    weight=SelectSign(lowTest,weight,0.0f);
    float highTest=Scalar(_mm_sub_ss(_mm_set_ss(1.0f),_mm_set_ss(weight)));
    weight=SelectSign(highTest,1.0f,weight);
    context->dirty=1;
    context->evaluatedWeight=weight;
    context->dirty=1;
    _mm_store_ps(context->evaluatedTarget.lane,target);

    auto head=&self->base.children;
    // The original tests the previous link for emptiness and evaluates ONLY the first child.
    if(head->previous!=head) {
        auto link=head->next;
        auto child=link?reinterpret_cast<b01::Node*>(reinterpret_cast<unsigned char*>(link)-offsetof(b01::Node,link)):nullptr;
        CallGameVirtual<void,2>(child,context,pose);
    }
    // Preserve the unconditional matrix conversion/inversion/point-transform work, including FP effects.
    SrtValue local,inverse;
    Float4Value point,savedTarget;
    _mm_store_ps(savedTarget.lane,target);
    Matrix44Value composed;
    auto matrix=&context->matrix;
    if(context->renderer && CallGameMethod<uint8_t,0x006fac20>(context->renderer)) {
        Matrix44Value storage;
        auto rendererMatrix=CallGameMethod<Matrix44Value*,0x006fb0e0>(context->renderer,&storage);
        for(int i=3;i>=0;--i)
            _mm_store_ps(composed.vector[i].lane,Row(_mm_load_ps(context->matrix.vector[i].lane),*rendererMatrix,true,true));
        matrix=&composed;
    }
    math::DecomposeSrt(&local,matrix);
    math::b11::InvertPose(reinterpret_cast<__m128*>(&local),reinterpret_cast<__m128*>(&inverse));
    math::b11::TransformPoint(reinterpret_cast<__m128*>(&savedTarget),reinterpret_cast<__m128*>(&inverse),reinterpret_cast<__m128*>(&point));
    // Reload after child dispatch: it may have changed the borrowed rig/chain.
    int32_t chain[11];
    std::memcpy(chain,context->rig->opaquebc,sizeof chain);
    bool complete=chain[4]!=-1 && chain[0]!=-1 && chain[1]!=-1 && chain[2]!=-1 && chain[3]!=-1;
    if(complete)
        return kernel::Solve(self,context,static_cast<m04::Pose*>(pose),local,inverse,savedTarget,point,weight);
    int32_t neck;
    std::memcpy(&neck,context->rig->opaquebc+0x1c,4);
    if(neck!=-1)
        return CallGameMethod<uintptr_t,0x005561c0>(self,context,pose,&savedTarget,weight);
    // Every missing-limb branch reloads the chain base into EAX at 0055B9F2.
    return reinterpret_cast<uintptr_t>(context->rig->opaquebc);
}
}

#if defined(_M_IX86)
namespace ts3::anim::m14::hooks {
uintptr_t __fastcall Evaluate(Evaluator* e,void*,Context* c,void* p) {return EvaluateUnbound(e,c,p);}
}
#endif
