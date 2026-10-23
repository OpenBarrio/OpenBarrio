#include "ts3/anim/m05_M13.h"
#include <xmmintrin.h>
#include <emmintrin.h>
#include <cmath>
#include <cstring>
namespace ts3::anim::m13 {
namespace {
__m128 VectorBits(uint64_t bits) { return _mm_castsi128_ps(_mm_set_epi32(0,0,int32_t(bits>>32),int32_t(bits))); }
__m128 VectorBits(__m128 bits) { return bits; }
template<class T> uint32_t& Low(T& value) { return *reinterpret_cast<uint32_t*>(&value); }
template<class T> uint32_t& High(T& value) { return reinterpret_cast<uint32_t*>(&value)[1]; }
int32_t SignedBits(float v) { int32_t r; std::memcpy(&r,&v,4); return r; }
float FloatBits(uint32_t v) { float r; std::memcpy(&r,&v,4); return r; }
float Minimum(float a,float b) { return _mm_cvtss_f32(_mm_min_ss(_mm_set_ss(a),_mm_set_ss(b))); }
float Maximum(float a,float b) { return _mm_cvtss_f32(_mm_max_ss(_mm_set_ss(a),_mm_set_ss(b))); }
float& StateScalar(EvaluationView* s,size_t off) { return *reinterpret_cast<float*>(s->tail+off-72); }
template<unsigned Address> float GlobalFloat() {
#if defined(_M_IX86)
 return *reinterpret_cast<const float*>(Address);
#else
 NotRewritten("constraint evaluation tuning global");
#endif
}
uintptr_t DebugGlobal() {
#if defined(_M_IX86)
 return *reinterpret_cast<const uintptr_t*>(0x011cd800);
#else
 NotRewritten("constraint evaluation debug global");
#endif
}
uint32_t DebugColour(unsigned index) {
#if defined(_M_IX86)
 return reinterpret_cast<const uint32_t*>(0x00fc13e0)[index];
#else
 (void)index; NotRewritten("constraint evaluation debug colours");
#endif
}
// EA emits x87 transcendental operations, not the host's SSE CRT approximations.
// Do not inline these opaque CRT calls: their register clobbers must be bounded
// by a real C++ call boundary rather than hidden inside the SSE kernel.
__declspec(noinline) float Sin(float x) {
#if defined(_M_IX86)
 float r; __asm { fld x
 fsin
 fstp r } return r;
#else
 return std::sin(x);
#endif
}
__declspec(noinline) float Cos(float x) {
#if defined(_M_IX86)
 float r; __asm { fld x
 fcos
 fstp r } return r;
#else
 return std::cos(x);
#endif
}
__declspec(noinline) float Atan2(float y,float x) {
#if defined(_M_IX86)
 float r; __asm { fld y
 fld x
 fpatan
 fstp r } return r;
#else
 return std::atan2(y,x);
#endif
}
__declspec(noinline) float Acos(float x) {
#if defined(_M_IX86)
 float r; uintptr_t entry=0x00f7edde; __asm { fld x
 call entry
 fstp r } return r;
#else
 return std::acos(x);
#endif
}
__declspec(noinline) float Asin(float x) {
#if defined(_M_IX86)
 float r; uintptr_t entry=0x00f7ede4; __asm { fld x
 call entry
 fstp r } return r;
#else
 return std::asin(x);
#endif
}
__declspec(noinline) double Fmod(double a,double b) {
#if defined(_M_IX86)
 double r; uintptr_t entry=0x00f7efb0; __asm { fld a
 fld b
 call entry
 fstp r } return r;
#else
 return std::fmod(a,b);
#endif
}
float Fabs(float x) { return FloatBits(Low(x)&0x7fffffffu); }
}
#define LODWORD(x) Low(x)
#define HIDWORD(x) High(x)
#define COERCE_INT(x) SignedBits(float(x))
#define COERCE_FLOAT(x) FloatBits(uint32_t(x))
#define sub_54EEF0(a,b) CallGameFunction<void,0x0054eef0>((const void*)(a),(void*)(b))
#define sub_555080(a,b,c) CallGameFunction<void,0x00555080>((const void*)(a),(const void*)(b),(void*)(c))
#define sub_555110(a,b,c) CallGameFunction<void,0x00555110>((const void*)(a),(const void*)(b),(void*)(c))
#define sub_5749B0(a) CallGameMethod<uintptr_t,0x005749b0>((void*)(a))
#define sub_574990(a) CallGameMethod<uintptr_t,0x00574990>((void*)(a))
#define sub_574190(a) CallGameMethod<uintptr_t,0x00574190>((void*)(a))
#define sub_7D29D0(a,b,c) CallGameFunction<void,0x007d29d0>((void*)(a),(const void*)(b),(const void*)(c))
#define sub_55DE20(a,b,c,d,e) CallGameFunction<float,0x0055de20>(float(a),float(b),float(c),float(d),(const void*)(e))
#define sub_6E8330() CallGameFunction<void*,0x006e8330>()
#define sub_54F260(a,b,c) CallGameMethod<void,0x0054f260>(debugContext,(const void*)(a),(const void*)(b),uint32_t(c))
#define sub_6C87F0(a,b,c) CallGameMethod<void,0x006c87f0>(debugContext,(const void*)(a),float(b),uint32_t(c))
#define sub_54C8D0(a,b,c) CallGameFunction<void,0x0054c8d0>((const void*)(a),(const void*)(b),(void*)(c))
#define sub_555060(a,b) CallGameFunction<int32_t*,0x00555060>((int32_t*)(a),(int32_t*)(b))
#define sub_555F50(a,b,c,d) CallGameFunction<uintptr_t,0x00555f50>((const void*)(a),(const void*)(b),(void*)(c),float(d))
int32_t* __stdcall Evaluate(EvaluationView* state,void* poseOpaque,const void* target,float a4) {
  auto* pose=static_cast<RigPose*>(poseOpaque);
  auto* a3=static_cast<const __m128*>(target);
  // Shared backing preserves the original temporary aliases and the SRT/vector
  // values passed by address across calls. Pointer temporaries are native-sized.
  alignas(16) unsigned char storage[0xa20];
  unsigned char* frame=storage+12;
  double X;
  int32_t* result;
  bool v5;
  uintptr_t v6;
  uintptr_t v7;
  uintptr_t v8;
  uintptr_t v9;
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
  __m128 v35;
  __m128 v36;
  __m128 v37;
  __m128 v38;
  __m128 v39;
  __m128 v40;
  __m128 v41;
  __m128 v42;
  __m128 v43;
  float v44;
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
  float v56;
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
  __m128 v69;
  float v70;
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
  double v101;
  float v102;
  float v103;
  double v104;
  long double v105;
  double v106;
  long double v107;
  float v108;
  float* v109;
  float v110;
  float* v111;
  float v112;
  float v113;
  double v114;
  long double v115;
  double v116;
  long double v117;
  float v118;
  float* v119;
  float v120;
  float* v121;
  float v122;
  float v123;
  double v124;
  long double v125;
  double v126;
  long double v127;
  float v128;
  float* v129;
  float v130;
  float* v131;
  float v132;
  float v133;
  int v134;
  float v135;
  int v136;
  float v137;
  double v138;
  double v139;
  double v140;
  double v141;
  float* v142;
  __m128 v143;
  __m128 v144;
  __m128 v145;
  __m128 v146;
  uintptr_t v147;
  __m128* v148;
  __m128 v149;
  __m128* v150;
  __m128 v151;
  __m128* v152;
  __m128 v153;
  __m128 v154;
  uintptr_t v155;
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
  __m128 v174;
  __m128 v175;
  __m128 v176;
  __m128 v177;
  __m128 v178;
  __m128 v179;
  __m128 v180;
  __m128 v181;
  __m128 v182;
  __m128 v183;
  __m128 v184;
  __m128 v185;
  __m128 v186;
  __m128 v187;
  __m128 v188;
  __m128 v189;
  __m128 v190;
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
  long double v203;
  __m128 v204;
  __m128 v205;
  __m128 v206;
  __m128 v207;
  uintptr_t v208;
  int v209;
  __m128 v210;
  __m128* v211;
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
  __m128 v224;
  __m128 v225;
  __m128 v226;
  __m128 v227;
  __m128 v228;
  __m128 v229;
  __m128 v230;
  __m128 v231;
  float v232;
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
  long double v247;
  __m128 v248;
  __m128 v249;
  __m128 v250;
  int* v251;
  int* v252;
  int32_t* v253;
  uintptr_t v254;
  uintptr_t v255;
  uintptr_t v256;
  uintptr_t v257;
  auto& v258 = *reinterpret_cast<float*>(frame + 0x0);
  auto& v259 = *reinterpret_cast<float*>(frame + 0x0);
  auto& v260 = *reinterpret_cast<float*>(frame + 0x0);
  auto& v261 = *reinterpret_cast<float*>(frame + 0x28);
  auto& v262 = *reinterpret_cast<float*>(frame + 0x28);
  auto& v263 = *reinterpret_cast<float*>(frame + 0x28);
  auto& v264 = *reinterpret_cast<unsigned int*>(frame + 0x28);
  auto& v265 = *reinterpret_cast<float*>(frame + 0x28);
  auto& v266 = *reinterpret_cast<float*>(frame + 0x28);
  auto& v267 = *reinterpret_cast<unsigned int*>(frame + 0x28);
  auto& v268 = *reinterpret_cast<unsigned int*>(frame + 0x28);
  auto& v269 = *reinterpret_cast<int*>(frame + 0x28);
  auto& v270 = *reinterpret_cast<unsigned int*>(frame + 0x28);
  auto& v271 = *reinterpret_cast<unsigned int*>(frame + 0x28);
  auto& v272 = *reinterpret_cast<float*>(frame + 0x28);
  auto& v273 = *reinterpret_cast<int*>(frame + 0x28);
  auto& v274 = *reinterpret_cast<float*>(frame + 0x28);
  auto& v275 = *reinterpret_cast<int*>(frame + 0x28);
  uintptr_t v276;
  auto& v277 = *reinterpret_cast<unsigned int*>(frame + 0x2c);
  auto& v278 = *reinterpret_cast<unsigned int*>(frame + 0x2c);
  auto& v279 = *reinterpret_cast<unsigned int*>(frame + 0x2c);
  auto& v280 = *reinterpret_cast<unsigned int*>(frame + 0x2c);
  auto& v281 = *reinterpret_cast<unsigned int*>(frame + 0x2c);
  auto& v282 = *reinterpret_cast<unsigned int*>(frame + 0x2c);
  auto& v283 = *reinterpret_cast<unsigned int*>(frame + 0x2c);
  auto& v284 = *reinterpret_cast<unsigned int*>(frame + 0x2c);
  auto& v285 = *reinterpret_cast<unsigned int*>(frame + 0x2c);
  auto& v286 = *reinterpret_cast<unsigned int*>(frame + 0x2c);
  auto& v287 = *reinterpret_cast<unsigned int*>(frame + 0x2c);
  auto& v288 = *reinterpret_cast<unsigned int*>(frame + 0x2c);
  auto& v289 = *reinterpret_cast<unsigned int*>(frame + 0x2c);
  auto& v290 = *reinterpret_cast<unsigned int*>(frame + 0x2c);
  auto& v291 = *reinterpret_cast<unsigned int*>(frame + 0x2c);
  auto& v292 = *reinterpret_cast<unsigned int*>(frame + 0x2c);
  auto& v293 = *reinterpret_cast<unsigned int*>(frame + 0x2c);
  auto& v294 = *reinterpret_cast<unsigned int*>(frame + 0x2c);
  auto& v295 = *reinterpret_cast<float*>(frame + 0x2c);
  int32_t* v296;
  auto& v297 = *reinterpret_cast<int*>(frame + 0x30);
  auto& v298 = *reinterpret_cast<unsigned int*>(frame + 0x30);
  auto& v299 = *reinterpret_cast<unsigned int*>(frame + 0x30);
  auto& v300 = *reinterpret_cast<float*>(frame + 0x30);
  auto& v301 = *reinterpret_cast<__m128*>(frame + 0x34);
  auto& v302 = *reinterpret_cast<float*>(frame + 0x34);
  auto& v303 = *reinterpret_cast<unsigned int*>(frame + 0x34);
  auto& v304 = *reinterpret_cast<unsigned int*>(frame + 0x34);
  auto& v305 = *reinterpret_cast<int64_t*>(frame + 0x34);
  auto& v306 = *reinterpret_cast<unsigned int*>(frame + 0x34);
  auto& v307 = *reinterpret_cast<unsigned int*>(frame + 0x34);
  auto& v308 = *reinterpret_cast<unsigned int*>(frame + 0x34);
  auto& v309 = *reinterpret_cast<unsigned int*>(frame + 0x34);
  auto& v310 = *reinterpret_cast<unsigned int*>(frame + 0x34);
  auto& v311 = *reinterpret_cast<unsigned int*>(frame + 0x34);
  auto& v312 = *reinterpret_cast<unsigned int*>(frame + 0x34);
  auto& v313 = *reinterpret_cast<unsigned int*>(frame + 0x34);
  auto& v314 = *reinterpret_cast<unsigned int*>(frame + 0x34);
  auto& v315 = *reinterpret_cast<unsigned int*>(frame + 0x34);
  auto& v316 = *reinterpret_cast<unsigned int*>(frame + 0x34);
  auto& v317 = *reinterpret_cast<unsigned int*>(frame + 0x34);
  auto& v318 = *reinterpret_cast<unsigned int*>(frame + 0x34);
  auto& v319 = *reinterpret_cast<unsigned int*>(frame + 0x34);
  auto& v320 = *reinterpret_cast<unsigned int*>(frame + 0x34);
  auto& v321 = *reinterpret_cast<unsigned int*>(frame + 0x34);
  auto& v322 = *reinterpret_cast<unsigned int*>(frame + 0x34);
  auto& v323 = *reinterpret_cast<unsigned int*>(frame + 0x34);
  auto& v324 = *reinterpret_cast<unsigned int*>(frame + 0x34);
  auto& v325 = *reinterpret_cast<unsigned int*>(frame + 0x34);
  auto& v326 = *reinterpret_cast<float*>(frame + 0x44);
  auto& v327 = *reinterpret_cast<float*>(frame + 0x44);
  auto& v328 = *reinterpret_cast<float*>(frame + 0x44);
  auto& v329 = *reinterpret_cast<int*>(frame + 0x44);
  auto& v330 = *reinterpret_cast<float*>(frame + 0x44);
  auto& v331 = *reinterpret_cast<unsigned int*>(frame + 0x44);
  auto& v332 = *reinterpret_cast<float*>(frame + 0x48);
  auto& v333 = *reinterpret_cast<float*>(frame + 0x48);
  __m128* v334;
  auto& v335 = *reinterpret_cast<float*>(frame + 0x4c);
  auto& v336 = *reinterpret_cast<float*>(frame + 0x4c);
  auto& v337 = *reinterpret_cast<int*>(frame + 0x4c);
  auto& v338 = *reinterpret_cast<float*>(frame + 0x50);
  auto& v339 = *reinterpret_cast<float*>(frame + 0x50);
  auto& v340 = *reinterpret_cast<float*>(frame + 0x50);
  auto& v341 = *reinterpret_cast<float*>(frame + 0x54);
  auto& v342 = *reinterpret_cast<__m128*>(frame + 0x54);
  auto& v343 = *reinterpret_cast<__m128*>(frame + 0x54);
  auto& v344 = *reinterpret_cast<__m128*>(frame + 0x54);
  auto& v345 = *reinterpret_cast<__m128*>(frame + 0x54);
  auto& v346 = *reinterpret_cast<__m128*>(frame + 0x64);
  auto& v347 = *reinterpret_cast<float*>(frame + 0x64);
  auto& v348 = *reinterpret_cast<float*>(frame + 0x64);
  auto& v349 = *reinterpret_cast<float*>(frame + 0x64);
  auto& v350 = *reinterpret_cast<__m128*>(frame + 0x64);
  auto& v351 = *reinterpret_cast<float*>(frame + 0x64);
  auto& v352 = *reinterpret_cast<float*>(frame + 0x64);
  auto& v353 = *reinterpret_cast<__m128*>(frame + 0x64);
  auto& v354 = *reinterpret_cast<__m128*>(frame + 0x74);
  auto& v355 = *reinterpret_cast<__m128*>(frame + 0x84);
  auto& v356 = *reinterpret_cast<float*>(frame + 0x84);
  auto& v357 = *reinterpret_cast<float*>(frame + 0x84);
  auto& v358 = *reinterpret_cast<float*>(frame + 0x84);
  auto& v359 = *reinterpret_cast<float*>(frame + 0x84);
  auto& v360 = *reinterpret_cast<float*>(frame + 0x84);
  auto& v361 = *reinterpret_cast<float*>(frame + 0x84);
  auto& v362 = *reinterpret_cast<__m128*>(frame + 0x84);
  auto& v363 = *reinterpret_cast<__m128*>(frame + 0x84);
  auto& v364 = *reinterpret_cast<float*>(frame + 0xa0);
  auto& v365 = *reinterpret_cast<float*>(frame + 0xa4);
  auto& v366 = *reinterpret_cast<__m128*>(frame + 0xa4);
  auto& v367 = *reinterpret_cast<__m128*>(frame + 0xa4);
  auto& v368 = *reinterpret_cast<__m128*>(frame + 0xa4);
  auto& v369 = *reinterpret_cast<float*>(frame + 0xa4);
  auto& v370 = *reinterpret_cast<__m128*>(frame + 0xa4);
  __m128* v371;
  int32_t* v372;
  auto& v373 = *reinterpret_cast<float*>(frame + 0xc8);
  int32_t* v374;
  int* v375;
  auto& v376 = *reinterpret_cast<__m128*>(frame + 0xd4);
  auto& v377 = *reinterpret_cast<__m128*>(frame + 0xd4);
  auto& v378 = *reinterpret_cast<__m128*>(frame + 0xd4);
  auto& v379 = *reinterpret_cast<__m128*>(frame + 0xe4);
  auto& v380 = *reinterpret_cast<__m128*>(frame + 0xf4);
  int* v381;
  auto& v382 = *reinterpret_cast<__m128*>(frame + 0x114);
  auto& v383 = *reinterpret_cast<__m128*>(frame + 0x124);
  auto& v384 = *reinterpret_cast<__m128*>(frame + 0x134);
  auto& v385 = *reinterpret_cast<__m128*>(frame + 0x144);
  int32_t* v386;
  auto& v387 = *reinterpret_cast<__m128*>(frame + 0x164);
  auto& v388 = *reinterpret_cast<__m128*>(frame + 0x174);
  auto& v389 = *reinterpret_cast<float*>(frame + 0x188);
  auto& v390 = *reinterpret_cast<float*>(frame + 0x18c);
  auto& v391 = *reinterpret_cast<int*>(frame + 0x190);
  auto& v392 = *reinterpret_cast<int*>(frame + 0x194);
  auto& v393 = *reinterpret_cast<int*>(frame + 0x198);
  auto& v394 = *reinterpret_cast<int*>(frame + 0x19c);
  auto& v395 = *reinterpret_cast<float*>(frame + 0x1a0);
  auto& v396 = *reinterpret_cast<__m128*>(frame + 0x1a4);
  auto& v397 = *reinterpret_cast<int*>(frame + 0x1bc);
  auto& v398 = *reinterpret_cast<int*>(frame + 0x1c0);
  auto& v399 = *reinterpret_cast<float*>(frame + 0x1c4);
  auto& v400 = *reinterpret_cast<float*>(frame + 0x1c8);
  auto& v401 = *reinterpret_cast<float*>(frame + 0x1cc);
  auto& v402 = *reinterpret_cast<float*>(frame + 0x1d0);
  auto& v403 = *reinterpret_cast<__m128*>(frame + 0x1d4);
  auto& v404 = *reinterpret_cast<float*>(frame + 0x1e8);
  auto& v405 = *reinterpret_cast<float*>(frame + 0x1ec);
  auto& v406 = *reinterpret_cast<float*>(frame + 0x1f0);
  auto& v407 = *reinterpret_cast<__m128*>(frame + 0x1f4);
  auto& v408 = *reinterpret_cast<__m128*>(frame + 0x204);
  auto& v409 = *reinterpret_cast<__m128*>(frame + 0x214);
  auto& v410 = *reinterpret_cast<__m128*>(frame + 0x224);
  auto& v411 = *reinterpret_cast<__m128*>(frame + 0x234);
  auto& v412 = *reinterpret_cast<__m128*>(frame + 0x244);
  auto& v413 = *reinterpret_cast<__m128*>(frame + 0x254);
  auto& v414 = *reinterpret_cast<__m128*>(frame + 0x264);
  auto& v415 = *reinterpret_cast<__m128*>(frame + 0x274);
  auto& v416 = *reinterpret_cast<__m128*>(frame + 0x284);
  auto& v417 = *reinterpret_cast<__m128*>(frame + 0x294);
  __m128* v418;
  __m128* v419;
  __m128* v420;
  __m128* v421;
  __m128* v422;
  __m128* v423;
  __m128* v424;
  __m128* v425;
  __m128* v426;
  __m128* v427;
  __m128* v428;
  __m128* v429;
  __m128* v430;
  __m128* v431;
  __m128* v432;
  __m128* v433;
  __m128* v434;
  __m128* v435;
  __m128* v436;
  auto& v437 = *reinterpret_cast<__m128*>(frame + 0x2f4);
  __m128* v438;
  __m128* v439;
  __m128* v440;
  __m128* v441;
  __m128* v442;
  __m128* v443;
  auto& v444 = *reinterpret_cast<__m128*>(frame + 0x324);
  auto& v445 = *reinterpret_cast<__m128*>(frame + 0x334);
  auto& v446 = *reinterpret_cast<unsigned char (*)[16]>(frame + 0x344);
  auto& v447 = *reinterpret_cast<unsigned char (*)[48]>(frame + 0x354);
  auto& v448 = *reinterpret_cast<__m128 (*)[2]>(frame + 0x384);
  auto& v449 = *reinterpret_cast<__m128*>(frame + 0x3a4);
  auto& v450 = *reinterpret_cast<__m128 (*)[2]>(frame + 0x3b4);
  auto& v451 = *reinterpret_cast<__m128*>(frame + 0x3d4);
  auto& v452 = *reinterpret_cast<__m128*>(frame + 0x3e4);
  auto& v453 = *reinterpret_cast<__m128*>(frame + 0x3f4);
  auto& v454 = *reinterpret_cast<__m128*>(frame + 0x404);
  auto& v455 = *reinterpret_cast<__m128*>(frame + 0x414);
  auto& v456 = *reinterpret_cast<float*>(frame + 0x424);
  auto& v457 = *reinterpret_cast<__m128*>(frame + 0x434);
  auto& v458 = *reinterpret_cast<float*>(frame + 0x444);
  auto& v459 = *reinterpret_cast<float*>(frame + 0x454);
  auto& v460 = *reinterpret_cast<float*>(frame + 0x464);
  auto& v461 = *reinterpret_cast<__m128*>(frame + 0x474);
  auto& v462 = *reinterpret_cast<float*>(frame + 0x484);
  auto& v463 = *reinterpret_cast<__m128*>(frame + 0x494);
  auto& v464 = *reinterpret_cast<float*>(frame + 0x4a4);
  auto& v465 = *reinterpret_cast<__m128*>(frame + 0x4b4);
  auto& v466 = *reinterpret_cast<__m128*>(frame + 0x4c4);
  auto& v467 = *reinterpret_cast<__m128 (*)[2]>(frame + 0x4d4);
  auto& v468 = *reinterpret_cast<int*>(frame + 0x4f4);
  auto& v469 = *reinterpret_cast<__m128*>(frame + 0x504);
  auto& v470 = *reinterpret_cast<float*>(frame + 0x514);
  auto& v471 = *reinterpret_cast<__m128*>(frame + 0x524);
  auto& v472 = *reinterpret_cast<__m128 (*)[2]>(frame + 0x534);
  auto& v473 = *reinterpret_cast<__m128*>(frame + 0x554);
  auto& v474 = *reinterpret_cast<int32_t*>(frame + 0x564);
  auto& v475 = *reinterpret_cast<__m128*>(frame + 0x574);
  auto& v476 = *reinterpret_cast<int32_t*>(frame + 0x584);
  auto& v477 = *reinterpret_cast<__m128 (*)[2]>(frame + 0x594);
  auto& v478 = *reinterpret_cast<__m128*>(frame + 0x5b4);
  auto& v479 = *reinterpret_cast<__m128*>(frame + 0x5c4);
  auto& v480 = *reinterpret_cast<int32_t*>(frame + 0x5d4);
  auto& v481 = *reinterpret_cast<__m128 (*)[2]>(frame + 0x5e4);
  auto& v482 = *reinterpret_cast<float*>(frame + 0x604);
  auto& v483 = *reinterpret_cast<__m128*>(frame + 0x614);
  auto& v484 = *reinterpret_cast<int32_t*>(frame + 0x624);
  auto& v485 = *reinterpret_cast<__m128*>(frame + 0x634);
  auto& v486 = *reinterpret_cast<__m128*>(frame + 0x644);
  auto& v487 = *reinterpret_cast<int32_t*>(frame + 0x654);
  auto& v488 = *reinterpret_cast<__m128*>(frame + 0x664);
  auto& v489 = *reinterpret_cast<__m128*>(frame + 0x674);
  auto& v490 = *reinterpret_cast<__m128*>(frame + 0x684);
  auto& v491 = *reinterpret_cast<__m128*>(frame + 0x694);
  auto& v492 = *reinterpret_cast<__m128*>(frame + 0x6a4);
  auto& v493 = *reinterpret_cast<unsigned char (*)[48]>(frame + 0x6b4);
  auto& v494 = *reinterpret_cast<__m128*>(frame + 0x6e4);
  auto& v495 = *reinterpret_cast<__m128*>(frame + 0x6f4);
  auto& v496 = *reinterpret_cast<__m128*>(frame + 0x704);
  auto& v497 = *reinterpret_cast<__m128*>(frame + 0x714);
  auto& v498 = *reinterpret_cast<__m128*>(frame + 0x724);
  auto& v499 = *reinterpret_cast<__m128*>(frame + 0x734);
  auto& v500 = *reinterpret_cast<__m128*>(frame + 0x744);
  auto& v501 = *reinterpret_cast<__m128*>(frame + 0x754);
  auto& v502 = *reinterpret_cast<__m128*>(frame + 0x764);
  auto& v503 = *reinterpret_cast<__m128*>(frame + 0x774);
  auto& v504 = *reinterpret_cast<__m128*>(frame + 0x784);
  auto& v505 = *reinterpret_cast<__m128*>(frame + 0x794);
  auto& v506 = *reinterpret_cast<unsigned char (*)[48]>(frame + 0x7a4);
  auto& v507 = *reinterpret_cast<__m128*>(frame + 0x7d4);
  auto& v508 = *reinterpret_cast<unsigned char (*)[48]>(frame + 0x7e4);
  auto& v509 = *reinterpret_cast<__m128*>(frame + 0x814);
  auto& v510 = *reinterpret_cast<unsigned char (*)[48]>(frame + 0x824);
  auto& v511 = *reinterpret_cast<__m128*>(frame + 0x854);
  auto& v512 = *reinterpret_cast<__m128*>(frame + 0x864);
  auto& v513 = *reinterpret_cast<__m128*>(frame + 0x874);
  auto& v514 = *reinterpret_cast<__m128*>(frame + 0x884);
  auto& v515 = *reinterpret_cast<__m128*>(frame + 0x894);
  auto& v516 = *reinterpret_cast<char (*)[16]>(frame + 0x8a4);
  auto& v517 = *reinterpret_cast<__m128*>(frame + 0x8b4);
  auto& v518 = *reinterpret_cast<char (*)[48]>(frame + 0x8d4);
  auto& v519 = *reinterpret_cast<char (*)[64]>(frame + 0x904);
  auto& v520 = *reinterpret_cast<char (*)[64]>(frame + 0x944);
  auto& v521 = *reinterpret_cast<char (*)[64]>(frame + 0x984);
  CallGameMethod<void,0x00577260>(v493, state);
  sub_54EEF0(v493, v506);
  sub_555080(a3, v506, v446);
  result = (int32_t *)(reinterpret_cast<uintptr_t>(state->rig->indices));
  v372 = result;
  if ( *result == -1 )
    return result;
  v5 = *(int32_t *)(reinterpret_cast<uintptr_t>(state->rig->indices+7)) == -1;
  v386 = (int32_t *)(reinterpret_cast<uintptr_t>(state->rig->indices+7));
  if ( v5 )
    return result;
  if ( result[8] == -1 )
    return result;
  if ( result[9] == -1 )
    return result;
  if ( result[10] == -1 )
    return result;
  v374 = result + 2;
  if ( result[2] == -1 || result[3] == -1 )
    return result;
  v6 = sub_5749B0(reinterpret_cast<uintptr_t>(pose));
  sub_574990(reinterpret_cast<uintptr_t>(pose));
  v7 = reinterpret_cast<uintptr_t>(state->rig->bindWorld);
  v8 = v7 + 48 * *v372;
  v9 = v7 + 48 * *v374;
  v385.m128_u64[1] = 1065353216;
  v385.m128_u64[0] = 0;
  v276 = v9;
  v396 = v385;
  v467[0] = v385;
  sub_555110(v467, v8, v448);
  v478 = v385;
  sub_555110(&v478, v276, &v457);
  v411 = _mm_sub_ps(
           _mm_mul_ps(_mm_shuffle_ps(v457, v457, 18), _mm_shuffle_ps(v448[0], v448[0], 9)),
           _mm_mul_ps(_mm_shuffle_ps(v457, v457, 9), _mm_shuffle_ps(v448[0], v448[0], 18)));
  v10 = _mm_mul_ps(v448[0], v448[0]);
  v11 = _mm_add_ps(_mm_add_ps(_mm_shuffle_ps(v10, v10, 2), _mm_shuffle_ps(v10, v10, 1)), v10);
  v12 = _mm_shuffle_ps(v11, v11, 0);
  v13 = _mm_rsqrt_ps(v12);
  v403 = _mm_shuffle_ps(VectorBits(0x3F800000u), VectorBits(0x3F800000u), 0);
  v385 = _mm_shuffle_ps(VectorBits(0x3F000000u), VectorBits(0x3F000000u), 0);
  v14 = _mm_mul_ps(
          _mm_add_ps(_mm_mul_ps(_mm_sub_ps(v403, _mm_mul_ps(_mm_mul_ps(v13, v13), v12)), _mm_mul_ps(v385, v13)), v13),
          v448[0]);
  v15 = _mm_mul_ps(v457, v457);
  v16 = _mm_add_ps(_mm_add_ps(_mm_shuffle_ps(v15, v15, 2), _mm_shuffle_ps(v15, v15, 1)), v15);
  v17 = _mm_shuffle_ps(v16, v16, 0);
  v18 = _mm_rsqrt_ps(v17);
  v355 = _mm_mul_ps(
           _mm_add_ps(_mm_mul_ps(_mm_sub_ps(v403, _mm_mul_ps(_mm_mul_ps(v18, v18), v17)), _mm_mul_ps(v18, v385)), v18),
           v457);
  v19 = _mm_mul_ps(v355, v14);
  v20 = _mm_add_ps(_mm_add_ps(_mm_shuffle_ps(v19, v19, 2), _mm_shuffle_ps(v19, v19, 1)), v19);
  v365 = _mm_shuffle_ps(VectorBits(0xBF800000), VectorBits(0xBF800000), 0).m128_f32[0];
  *(float *)&v277 = Acos(Minimum(Maximum(_mm_shuffle_ps(v20, v20, 0).m128_f32[0], v365), v403.m128_f32[0]));
  v21 = _mm_mul_ps(
          _mm_sub_ps(
            _mm_mul_ps(_mm_shuffle_ps(v355, v355, 18), _mm_shuffle_ps(v14, v14, 9)),
            _mm_mul_ps(_mm_shuffle_ps(v355, v355, 9), _mm_shuffle_ps(v14, v14, 18))),
          v411);
  v22 = _mm_add_ps(_mm_add_ps(_mm_shuffle_ps(v21, v21, 2), _mm_shuffle_ps(v21, v21, 1)), v21);
  v413 = VectorBits(0);
  v468 = 0;
  v470 = _mm_shuffle_ps(v22, v22, 0).m128_f32[0];
  v23 = _mm_shuffle_ps(VectorBits(v277), VectorBits(v277), 0);
  if ( v470 < 0.0f )
    v23 = _mm_sub_ps(_mm_shuffle_ps(VectorBits(0x40C90FDBu), VectorBits(0x40C90FDBu), 0), v23);
  v24 = _mm_shuffle_ps(VectorBits(0x40000000u), VectorBits(0x40000000u), 0);
  v450[1] = v23;
  v341 = _mm_shuffle_ps(VectorBits(v23.m128_u32[0]), VectorBits(v23.m128_u32[0]), 0).m128_f32[0] / v24.m128_f32[0];
  v387 = v24;
  *(float *)&v278 = Sin(v341);
  v25 = _mm_shuffle_ps(VectorBits(v278), VectorBits(v278), 0);
  *(float *)&v279 = Cos(v341);
  v420 = &v411;
  v441 = &v411;
  v443 = &v411;
  v494 = _mm_mul_ps(_mm_shuffle_ps(v411, v411, 170), v25);
  v504 = _mm_mul_ps(_mm_shuffle_ps(v411, v411, 0), v25);
  v502 = _mm_mul_ps(_mm_shuffle_ps(v411, v411, 85), v25);
  v334 = (__m128 *)(v6 + 48 * *v372);
  v371 = (__m128 *)(v6 + 48 * *v374);
  v381 = v372 + 5;
  v375 = v372 + 6;
  v342 = _mm_shuffle_ps(
           _mm_shuffle_ps(v504, v502, 16),
           _mm_shuffle_ps(v494, _mm_shuffle_ps(VectorBits(v279), VectorBits(v279), 0), 50),
           136);
  v454 = _mm_mul_ps(_mm_add_ps(*(__m128 *)(48 * v372[6] + v6), *(__m128 *)(v6 + 48 * v372[5])), v385);
  sub_555080(&v454, v493, &v445);
  v26 = _mm_sub_ps(*a3, v445);
  v27 = _mm_mul_ps(v26, v26);
  v28 = _mm_add_ps(_mm_add_ps(_mm_shuffle_ps(v27, v27, 2), _mm_shuffle_ps(v27, v27, 1)), v27);
  v460 = _mm_shuffle_ps(v28, v28, 0).m128_f32[0];
  v29 = _mm_shuffle_ps(VectorBits(0), VectorBits(0), 0);
  v487 = v29.m128_i32[0];
  v301 = v29;
  v444 = v26;
  if ( v460 > v29.m128_f32[0] )
  {
    v30 = _mm_add_ps(_mm_add_ps(_mm_shuffle_ps(v27, v27, 2), _mm_shuffle_ps(v27, v27, 1)), v27);
    v31 = _mm_shuffle_ps(v30, v30, 0);
    v32 = _mm_rsqrt_ps(v31);
    v33 = _mm_add_ps(_mm_mul_ps(_mm_sub_ps(v403, _mm_mul_ps(_mm_mul_ps(v32, v32), v31)), _mm_mul_ps(v32, v385)), v32);
    v444 = _mm_mul_ps(
             _mm_add_ps(_mm_mul_ps(_mm_sub_ps(v403, _mm_mul_ps(_mm_mul_ps(v33, v33), v31)), _mm_mul_ps(v33, v385)), v33),
             v26);
  }
  sub_555110(&v444, v506, v472);
  sub_54EEF0(v371, v516);
  v34 = _mm_shuffle_ps(v517, v517, 18);
  v35 = _mm_shuffle_ps(v517, v517, 9);
  v36 = _mm_add_ps(
          _mm_sub_ps(
            _mm_mul_ps(_mm_shuffle_ps(v472[0], v472[0], 18), v35),
            _mm_mul_ps(_mm_shuffle_ps(v472[0], v472[0], 9), v34)),
          _mm_mul_ps(_mm_shuffle_ps(v517, v517, 255), v472[0]));
  v475 = _mm_add_ps(
           _mm_mul_ps(
             _mm_sub_ps(_mm_mul_ps(_mm_shuffle_ps(v36, v36, 18), v35), _mm_mul_ps(_mm_shuffle_ps(v36, v36, 9), v34)),
             v387),
           v472[0]);
  v450[0] = v396;
  sub_555110(v450, v334, &v451);
  v473 = v396;
  sub_555110(&v473, v371, &v455);
  v37 = _mm_shuffle_ps(v342, v342, 18);
  v38 = _mm_shuffle_ps(v342, v342, 9);
  v39 = _mm_add_ps(
          _mm_sub_ps(_mm_mul_ps(_mm_shuffle_ps(v451, v451, 18), v38), _mm_mul_ps(_mm_shuffle_ps(v451, v451, 9), v37)),
          _mm_mul_ps(_mm_shuffle_ps(v342, v342, 255), v451));
  v354 = v371[1];
  v40 = _mm_add_ps(
          _mm_mul_ps(
            _mm_sub_ps(_mm_mul_ps(_mm_shuffle_ps(v39, v39, 18), v38), _mm_mul_ps(_mm_shuffle_ps(v39, v39, 9), v37)),
            v387),
          v451);
  v41 = VectorBits(v354.m128_u32[0]);
  v382 = v354;
  v326 = v354.m128_f32[2] * v354.m128_f32[2];
  v408 = v40;
  v261 = v354.m128_f32[1] * v354.m128_f32[1];
  v42 = v41;
  v42.m128_f32[0] = v41.m128_f32[0] * v41.m128_f32[0];
  v43 = VectorBits(v354.m128_u32[3]);
  v43.m128_f32[0] = (float)(v354.m128_f32[3] * v354.m128_f32[2]) + (float)(v354.m128_f32[1] * v354.m128_f32[0]);
  v44 = (float)((float)((float)(v354.m128_f32[2] * v354.m128_f32[2]) + (float)(v354.m128_f32[1] * v354.m128_f32[1]))
              + (float)(v41.m128_f32[0] * v41.m128_f32[0]))
      + (float)(v354.m128_f32[3] * v354.m128_f32[3]);
  if ( v43.m128_f32[0] > (float)(v44 * 0.49900001f) )
  {
    *(float *)&v280 = Atan2(
                        _mm_shuffle_ps(VectorBits(v354.m128_u32[0]), VectorBits(v354.m128_u32[0]), 0).m128_f32[0],
                        _mm_shuffle_ps(VectorBits(v354.m128_u32[3]), VectorBits(v354.m128_u32[3]), 0).m128_f32[0]);
    v45 = v380;
    v46 = _mm_shuffle_ps(v380, _mm_mul_ps(_mm_shuffle_ps(VectorBits(v280), VectorBits(v280), 0), v387), 0);
    v47 = VectorBits(0x3FC90FDBu);
LABEL_14:
    v48 = _mm_shuffle_ps(v46, v45, 232);
    v49 = _mm_shuffle_ps(v48, _mm_shuffle_ps(v48, _mm_shuffle_ps(v47, v47, 0), 15), 36);
    v50 = _mm_shuffle_ps(_mm_shuffle_ps(v49, v301, 5), v49, 226);
    goto LABEL_18;
  }
  if ( (float)(v44 * -0.49900001f) > v43.m128_f32[0] )
  {
    *(float *)&v281 = Atan2(
                        _mm_shuffle_ps(VectorBits(v354.m128_u32[0]), VectorBits(v354.m128_u32[0]), 0).m128_f32[0],
                        _mm_shuffle_ps(VectorBits(v354.m128_u32[3]), VectorBits(v354.m128_u32[3]), 0).m128_f32[0]);
    v45 = v380;
    v46 = _mm_shuffle_ps(
            v380,
            _mm_mul_ps(
              _mm_shuffle_ps(VectorBits(v281), VectorBits(v281), 0),
              _mm_shuffle_ps(VectorBits(0xC0000000), VectorBits(0xC0000000), 0)),
            0);
    v47 = VectorBits(0xBFC90FDB);
    goto LABEL_14;
  }
  v42.m128_f32[0] = (float)((float)(v42.m128_f32[0] - v261) - v326) + (float)(v354.m128_f32[3] * v354.m128_f32[3]);
  v356 = _mm_shuffle_ps(v42, v42, 0).m128_f32[0];
  v51 = VectorBits(v354.m128_u32[3]);
  v51.m128_f32[0] = (float)((float)(v354.m128_f32[3] * v354.m128_f32[1]) * 2.0f)
                  - (float)((float)(v354.m128_f32[2] * v354.m128_f32[0]) * 2.0f);
  v43.m128_f32[0] = (float)(v43.m128_f32[0] / v44) * 2.0f;
  *(float *)&v282 = Atan2(_mm_shuffle_ps(v51, v51, 0).m128_f32[0], v356);
  v52 = _mm_shuffle_ps(_mm_shuffle_ps(v380, _mm_shuffle_ps(VectorBits(v282), VectorBits(v282), 0), 0), v380, 232);
  *(float *)&v283 = Asin(_mm_shuffle_ps(v43, v43, 0).m128_f32[0]);
  v53 = VectorBits(LODWORD(v261));
  v53.m128_f32[0] = (float)((float)(v261 - (float)(v354.m128_f32[0] * v354.m128_f32[0])) - v326)
                  + (float)(v354.m128_f32[3] * v354.m128_f32[3]);
  v357 = _mm_shuffle_ps(v53, v53, 0).m128_f32[0];
  v54 = VectorBits(v382.m128_u32[3]);
  v380 = _mm_shuffle_ps(v52, _mm_shuffle_ps(v52, _mm_shuffle_ps(VectorBits(v283), VectorBits(v283), 0), 15), 36);
  v54.m128_f32[0] = (float)((float)(v382.m128_f32[3] * v354.m128_f32[0]) * 2.0f)
                  - (float)((float)(v354.m128_f32[2] * v354.m128_f32[1]) * 2.0f);
  *(float *)&v284 = Atan2(_mm_shuffle_ps(v54, v54, 0).m128_f32[0], v357);
  v50 = _mm_shuffle_ps(_mm_shuffle_ps(v380, _mm_shuffle_ps(VectorBits(v284), VectorBits(v284), 0), 5), v380, 226);
LABEL_18:
  v380 = v50;
  v346 = v334[1];
  v382 = v346;
  v335 = v346.m128_f32[2] * v346.m128_f32[2];
  v327 = v346.m128_f32[0] * v346.m128_f32[0];
  v262 = v346.m128_f32[1] * v346.m128_f32[1];
  v55 = VectorBits(v346.m128_u32[3]);
  v55.m128_f32[0] = (float)(v346.m128_f32[3] * v346.m128_f32[2]) + (float)(v346.m128_f32[1] * v346.m128_f32[0]);
  v56 = (float)((float)((float)(v346.m128_f32[2] * v346.m128_f32[2]) + (float)(v346.m128_f32[1] * v346.m128_f32[1]))
              + (float)(v346.m128_f32[0] * v346.m128_f32[0]))
      + (float)(v346.m128_f32[3] * v346.m128_f32[3]);
  if ( v55.m128_f32[0] > (float)(v56 * 0.49900001f) )
  {
    *(float *)&v285 = Atan2(
                        _mm_shuffle_ps(VectorBits(v346.m128_u32[0]), VectorBits(v346.m128_u32[0]), 0).m128_f32[0],
                        _mm_shuffle_ps(VectorBits(v346.m128_u32[3]), VectorBits(v346.m128_u32[3]), 0).m128_f32[0]);
    v57 = v384;
    v58 = _mm_shuffle_ps(v384, _mm_mul_ps(_mm_shuffle_ps(VectorBits(v285), VectorBits(v285), 0), v387), 0);
    v59 = VectorBits(0x3FC90FDBu);
LABEL_20:
    v60 = _mm_shuffle_ps(v58, v57, 232);
    v61 = _mm_shuffle_ps(v60, _mm_shuffle_ps(v60, _mm_shuffle_ps(v59, v59, 0), 15), 36);
    v62 = _mm_shuffle_ps(_mm_shuffle_ps(v61, v301, 5), v61, 226);
    goto LABEL_24;
  }
  if ( (float)(v56 * -0.49900001f) > v55.m128_f32[0] )
  {
    *(float *)&v286 = Atan2(
                        _mm_shuffle_ps(VectorBits(v346.m128_u32[0]), VectorBits(v346.m128_u32[0]), 0).m128_f32[0],
                        _mm_shuffle_ps(VectorBits(v346.m128_u32[3]), VectorBits(v346.m128_u32[3]), 0).m128_f32[0]);
    v57 = v384;
    v58 = _mm_shuffle_ps(
            v384,
            _mm_mul_ps(
              _mm_shuffle_ps(VectorBits(v286), VectorBits(v286), 0),
              _mm_shuffle_ps(VectorBits(0xC0000000), VectorBits(0xC0000000), 0)),
            0);
    v59 = VectorBits(0xBFC90FDB);
    goto LABEL_20;
  }
  v63 = VectorBits(LODWORD(v327));
  v63.m128_f32[0] = (float)((float)(v327 - v262) - v335) + (float)(v346.m128_f32[3] * v346.m128_f32[3]);
  v55.m128_f32[0] = (float)(v55.m128_f32[0] / v56) * 2.0f;
  v358 = _mm_shuffle_ps(v63, v63, 0).m128_f32[0];
  v64 = VectorBits(v346.m128_u32[3]);
  v64.m128_f32[0] = (float)((float)(v346.m128_f32[3] * v346.m128_f32[1]) * 2.0f)
                  - (float)((float)(v346.m128_f32[2] * v346.m128_f32[0]) * 2.0f);
  *(float *)&v287 = Atan2(_mm_shuffle_ps(v64, v64, 0).m128_f32[0], v358);
  v384 = _mm_shuffle_ps(_mm_shuffle_ps(v384, _mm_shuffle_ps(VectorBits(v287), VectorBits(v287), 0), 0), v384, 232);
  *(float *)&v288 = Asin(_mm_shuffle_ps(v55, v55, 0).m128_f32[0]);
  v65 = VectorBits(LODWORD(v262));
  v65.m128_f32[0] = (float)((float)(v262 - v327) - v335) + (float)(v346.m128_f32[3] * v346.m128_f32[3]);
  v359 = _mm_shuffle_ps(v65, v65, 0).m128_f32[0];
  v66 = VectorBits(v346.m128_u32[3]);
  v384 = _mm_shuffle_ps(v384, _mm_shuffle_ps(v384, _mm_shuffle_ps(VectorBits(v288), VectorBits(v288), 0), 15), 36);
  v66.m128_f32[0] = (float)((float)(v346.m128_f32[3] * v382.m128_f32[0]) * 2.0f)
                  - (float)((float)(v346.m128_f32[2] * v346.m128_f32[1]) * 2.0f);
  *(float *)&v289 = Atan2(_mm_shuffle_ps(v66, v66, 0).m128_f32[0], v359);
  v62 = _mm_shuffle_ps(_mm_shuffle_ps(v384, _mm_shuffle_ps(VectorBits(v289), VectorBits(v289), 0), 5), v384, 226);
LABEL_24:
  v384 = v62;
  v379 = v413;
  v388 = VectorBits(0x3F800000uLL);
  v453 = VectorBits(0x3F800000u);
  sub_7D29D0(v477, &v453, &v475);
  v382 = v477[0];
  v67 = VectorBits(LODWORD(v477[0]));
  v263 = *((float *)v477 + 2) * *((float *)v477 + 2);
  v69 = VectorBits(HIDWORD(v477[0]));
  v68 = v67;
  v68.m128_f32[0] = v67.m128_f32[0] * v67.m128_f32[0];
  v69.m128_f32[0] = (float)(*((float *)v477 + 3) * *((float *)v477 + 2))
                  + (float)(*((float *)v477 + 1) * *(float *)v477);
  v70 = (float)((float)((float)(*((float *)v477 + 2) * *((float *)v477 + 2))
                      + (float)(*((float *)v477 + 1) * *((float *)v477 + 1)))
              + (float)(v67.m128_f32[0] * v67.m128_f32[0]))
      + (float)(*((float *)v477 + 3) * *((float *)v477 + 3));
  v338 = *((float *)v477 + 1) * *((float *)v477 + 1);
  if ( !(v69.m128_f32[0] > (float)(v70 * 0.49900001f)) )
  {
    if ( !((float)(v70 * -0.49900001f) > v69.m128_f32[0]) )
    {
      v68.m128_f32[0] = (float)((float)(v68.m128_f32[0] - (float)(*((float *)v477 + 1) * *((float *)v477 + 1))) - v263)
                      + (float)(*((float *)v477 + 3) * *((float *)v477 + 3));
      v69.m128_f32[0] = (float)(v69.m128_f32[0] / v70) * 2.0f;
      v360 = _mm_shuffle_ps(v68, v68, 0).m128_f32[0];
      v77 = VectorBits(HIDWORD(v477[0]));
      v77.m128_f32[0] = (float)((float)(*((float *)v477 + 3) * *((float *)v477 + 1)) * 2.0f)
                      - (float)((float)(*((float *)v477 + 2) * *(float *)v477) * 2.0f);
      *(float *)&v292 = Atan2(_mm_shuffle_ps(v77, v77, 0).m128_f32[0], v360);
      v379 = _mm_shuffle_ps(_mm_shuffle_ps(v379, _mm_shuffle_ps(VectorBits(v292), VectorBits(v292), 0), 0), v379, 232);
      *(float *)&v293 = Asin(_mm_shuffle_ps(v69, v69, 0).m128_f32[0]);
      v78 = VectorBits(LODWORD(v338));
      v78.m128_f32[0] = (float)((float)(v338 - (float)(*(float *)v477 * *(float *)v477)) - v263)
                      + (float)(*((float *)v477 + 3) * *((float *)v477 + 3));
      v361 = _mm_shuffle_ps(v78, v78, 0).m128_f32[0];
      v79 = VectorBits(HIDWORD(v477[0]));
      v379 = _mm_shuffle_ps(v379, _mm_shuffle_ps(v379, _mm_shuffle_ps(VectorBits(v293), VectorBits(v293), 0), 15), 36);
      v79.m128_f32[0] = (float)((float)(*((float *)v477 + 3) * v382.m128_f32[0]) * 2.0f)
                      - (float)((float)(*((float *)v477 + 2) * *((float *)v477 + 1)) * 2.0f);
      *(float *)&v294 = Atan2(_mm_shuffle_ps(v79, v79, 0).m128_f32[0], v361);
      v76 = _mm_shuffle_ps(_mm_shuffle_ps(v379, _mm_shuffle_ps(VectorBits(v294), VectorBits(v294), 0), 5), v379, 226);
      goto LABEL_30;
    }
    *(float *)&v291 = Atan2(
                        _mm_shuffle_ps(VectorBits(LODWORD(v477[0])), VectorBits(LODWORD(v477[0])), 0).m128_f32[0],
                        _mm_shuffle_ps(VectorBits(HIDWORD(v477[0])), VectorBits(HIDWORD(v477[0])), 0).m128_f32[0]);
    v71 = v379;
    v72 = _mm_shuffle_ps(
            v379,
            _mm_mul_ps(
              _mm_shuffle_ps(VectorBits(v291), VectorBits(v291), 0),
              _mm_shuffle_ps(VectorBits(0xC0000000), VectorBits(0xC0000000), 0)),
            0);
    v73 = VectorBits(0xBFC90FDB);
  }
  else
  {
    *(float *)&v290 = Atan2(
                        _mm_shuffle_ps(VectorBits(LODWORD(v477[0])), VectorBits(LODWORD(v477[0])), 0).m128_f32[0],
                        _mm_shuffle_ps(VectorBits(HIDWORD(v477[0])), VectorBits(HIDWORD(v477[0])), 0).m128_f32[0]);
    v71 = v379;
    v72 = _mm_shuffle_ps(v379, _mm_mul_ps(_mm_shuffle_ps(VectorBits(v290), VectorBits(v290), 0), v387), 0);
    v73 = VectorBits(0x3FC90FDBu);
  }
  v74 = _mm_shuffle_ps(v72, v71, 232);
  v75 = _mm_shuffle_ps(v74, _mm_shuffle_ps(v74, _mm_shuffle_ps(v73, v73, 0), 15), 36);
  v76 = _mm_shuffle_ps(_mm_shuffle_ps(v75, v301, 5), v75, 226);
LABEL_30:
  v80 = v408;
  v379 = v76;
  v339 = GlobalFloat<0x0114F2C4>() * 0.017453292f;
  v364 = GlobalFloat<0x0114F2C8>() * 0.017453292f;
  v373 = GlobalFloat<0x0114F2CC>() * 0.017453292f;
  v295 = GlobalFloat<0x0114F2D0>() * 0.017453292f;
  v362 = _mm_sub_ps(
           _mm_mul_ps(_mm_shuffle_ps(v455, v455, 18), _mm_shuffle_ps(v408, v408, 9)),
           _mm_mul_ps(_mm_shuffle_ps(v455, v455, 9), _mm_shuffle_ps(v408, v408, 18)));
  v81 = _mm_mul_ps(v362, v362);
  v82 = _mm_add_ps(_mm_add_ps(_mm_shuffle_ps(v81, v81, 2), _mm_shuffle_ps(v81, v81, 1)), v81);
  v83 = _mm_shuffle_ps(v82, v82, 0);
  v84 = _mm_rsqrt_ps(v83);
  v343 = v83;
  v408 = _mm_add_ps(_mm_mul_ps(_mm_sub_ps(v403, _mm_mul_ps(_mm_mul_ps(v84, v84), v83)), _mm_mul_ps(v84, v385)), v84);
  v85 = _mm_mul_ps(v80, v80);
  v86 = _mm_add_ps(_mm_add_ps(_mm_shuffle_ps(v85, v85, 2), _mm_shuffle_ps(v85, v85, 1)), v85);
  v87 = _mm_shuffle_ps(v86, v86, 0);
  v88 = _mm_rsqrt_ps(v87);
  v89 = _mm_mul_ps(
          _mm_add_ps(_mm_mul_ps(_mm_sub_ps(v403, _mm_mul_ps(_mm_mul_ps(v88, v88), v87)), _mm_mul_ps(v385, v88)), v88),
          v80);
  v90 = _mm_mul_ps(v455, v455);
  v91 = _mm_add_ps(_mm_add_ps(_mm_shuffle_ps(v90, v90, 2), _mm_shuffle_ps(v90, v90, 1)), v90);
  v92 = _mm_shuffle_ps(v91, v91, 0);
  v382 = v89;
  v93 = _mm_rsqrt_ps(v92);
  v376 = _mm_mul_ps(
           _mm_add_ps(_mm_mul_ps(_mm_sub_ps(v403, _mm_mul_ps(_mm_mul_ps(v93, v93), v92)), _mm_mul_ps(v385, v93)), v93),
           v455);
  v94 = _mm_mul_ps(v376, v89);
  v95 = _mm_add_ps(_mm_add_ps(_mm_shuffle_ps(v94, v94, 2), _mm_shuffle_ps(v94, v94, 1)), v94);
  *(float *)&v264 = Acos(Minimum(Maximum(_mm_shuffle_ps(v95, v95, 0).m128_f32[0], v365), v403.m128_f32[0]));
  v96 = _mm_mul_ps(
          _mm_mul_ps(
            _mm_add_ps(
              _mm_mul_ps(_mm_sub_ps(v403, _mm_mul_ps(_mm_mul_ps(v408, v408), v343)), _mm_mul_ps(v408, v385)),
              v408),
            v362),
          _mm_sub_ps(
            _mm_mul_ps(_mm_shuffle_ps(v376, v376, 18), _mm_shuffle_ps(v382, v382, 9)),
            _mm_mul_ps(_mm_shuffle_ps(v376, v376, 9), _mm_shuffle_ps(v382, v382, 18))));
  v97 = _mm_add_ps(_mm_add_ps(_mm_shuffle_ps(v96, v96, 2), _mm_shuffle_ps(v96, v96, 1)), v96);
  v480 = v413.m128_i32[0];
  v459 = _mm_shuffle_ps(v97, v97, 0).m128_f32[0];
  v98 = _mm_shuffle_ps(VectorBits(v264), VectorBits(v264), 0);
  if ( !(v413.m128_f32[0] > v459) )
    v99 = v98;
  else
    v99 = _mm_sub_ps(_mm_shuffle_ps(VectorBits(0x40C90FDBu), VectorBits(0x40C90FDBu), 0), v98);
  X = double(v99.m128_f32[0]) * double(0.15915494f);
  v481[1] = v99;
  v265 = X;
  v101 = v265 - floor(X);
  v266 = v101;
  v102 = v266;
  if ( v101 > 0.5f )
    v102 = v266 - 1.0f;
  v103 = v102 * 6.2831855f;
  v332 = v103;
  if ( v103 > 3.1415927f )
    v332 = v103 - 6.2831855f;
  v347 = _mm_shuffle_ps(v379, v379, 170).m128_f32[0];
  v104 = v347;
  v421 = &v379;
  if ( !(v347 >= 0.0f) )
    v105 = Fmod(double(v104) - double(3.1415927f), 6.283185482025146f) + 3.1415927f;
  else
    v105 = Fmod(double(v104) + double(3.1415927f), 6.283185482025146f) - 3.1415927f;
  *(float *)&v267 = v105;
  v461 = _mm_shuffle_ps(VectorBits(v267), VectorBits(v267), 0);
  v336 = v461.m128_f32[0];
  if ( v461.m128_f32[0] > 3.1415927f )
    v336 = v461.m128_f32[0] - 6.2831855f;
  v348 = _mm_shuffle_ps(v380, v380, 170).m128_f32[0];
  v106 = v348;
  v432 = &v380;
  if ( !(v348 >= 0.0f) )
    v107 = Fmod(double(v106) - double(3.1415927f), 6.283185482025146f) + 3.1415927f;
  else
    v107 = Fmod(double(v106) + double(3.1415927f), 6.283185482025146f) - 3.1415927f;
  *(float *)&v268 = v107;
  v366 = _mm_shuffle_ps(VectorBits(v268), VectorBits(v268), 0);
  v108 = v366.m128_f32[0];
  v483 = v366;
  if ( v366.m128_f32[0] > 3.1415927f )
    v108 = v366.m128_f32[0] - 6.2831855f;
  v391 = 0;
  v401 = (float)(-0.0f - v373) - v108;
  v109 = (float *)&v391;
  if ( !(v401 > 0.0f) )
    v109 = &v401;
  v110 = *v109;
  v392 = 0;
  v402 = v364 - v108;
  v111 = (float *)&v392;
  if ( !(0.0f > (float)(v364 - v108)) )
    v111 = &v402;
  v112 = *v111;
  if ( v110 > *v111 )
  {
    v113 = v110;
    v110 = *v111;
    v112 = v113;
  }
  v269 = (COERCE_INT(v110 - v336) >> 31) & LODWORD(v336) | LODWORD(v110) & ~(COERCE_INT(v110 - v336) >> 31);
  v349 = _mm_shuffle_ps(v379, v379, 85).m128_f32[0];
  v114 = v349;
  v337 = (COERCE_INT(v112 - *(float *)&v269) >> 31) & LODWORD(v112) | v269 & ~(COERCE_INT(v112 - *(float *)&v269) >> 31);
  v422 = &v379;
  if ( !(v349 >= 0.0f) )
    v115 = Fmod(double(v114) - double(3.1415927f), 6.283185482025146f) + 3.1415927f;
  else
    v115 = Fmod(double(v114) + double(3.1415927f), 6.283185482025146f) - 3.1415927f;
  *(float *)&v270 = v115;
  v463 = _mm_shuffle_ps(VectorBits(v270), VectorBits(v270), 0);
  v328 = v463.m128_f32[0];
  if ( v463.m128_f32[0] > 3.1415927f )
    v328 = v463.m128_f32[0] - 6.2831855f;
  v350 = _mm_sub_ps(_mm_shuffle_ps(v380, v380, 85), _mm_shuffle_ps(v384, v384, 85));
  v116 = v350.m128_f32[0];
  v512 = v350;
  v438 = &v384;
  v424 = &v380;
  if ( !(v350.m128_f32[0] >= 0.0f) )
    v117 = Fmod(double(v116) - double(3.1415927f), 6.283185482025146f) + 3.1415927f;
  else
    v117 = Fmod(double(v116) + double(3.1415927f), 6.283185482025146f) - 3.1415927f;
  *(float *)&v271 = v117;
  v367 = _mm_shuffle_ps(VectorBits(v271), VectorBits(v271), 0);
  v118 = v367.m128_f32[0];
  v485 = v367;
  if ( v367.m128_f32[0] > 3.1415927f )
    v118 = v367.m128_f32[0] - 6.2831855f;
  v394 = 0;
  v406 = (float)(-0.0f - v339) - v118;
  v119 = (float *)&v394;
  if ( !(v406 > 0.0f) )
    v119 = &v406;
  v120 = *v119;
  v398 = 0;
  v400 = v339 - v118;
  v121 = (float *)&v398;
  if ( !(0.0f > (float)(v339 - v118)) )
    v121 = &v400;
  v122 = *v121;
  if ( v120 > *v121 )
  {
    v123 = v120;
    v120 = *v121;
    v122 = v123;
  }
  v297 = (COERCE_INT(v120 - v328) >> 31) & LODWORD(v328) | LODWORD(v120) & ~(COERCE_INT(v120 - v328) >> 31);
  v351 = _mm_shuffle_ps(v379, v379, 0).m128_f32[0];
  v124 = v351;
  v329 = (COERCE_INT(v122 - *(float *)&v297) >> 31) & LODWORD(v122) | v297 & ~(COERCE_INT(v122 - *(float *)&v297) >> 31);
  v434 = &v379;
  if ( !(v351 >= 0.0f) )
    v125 = Fmod(double(v124) - double(3.1415927f), 6.283185482025146f) + 3.1415927f;
  else
    v125 = Fmod(double(v124) + double(3.1415927f), 6.283185482025146f) - 3.1415927f;
  *(float *)&v298 = v125;
  v465 = _mm_shuffle_ps(VectorBits(v298), VectorBits(v298), 0);
  v272 = v465.m128_f32[0];
  if ( v465.m128_f32[0] > 3.1415927f )
    v272 = v465.m128_f32[0] - 6.2831855f;
  v352 = _mm_shuffle_ps(v380, v380, 0).m128_f32[0];
  v126 = v352;
  v426 = &v380;
  if ( !(v352 >= 0.0f) )
    v127 = Fmod(double(v126) - double(3.1415927f), 6.283185482025146f) + 3.1415927f;
  else
    v127 = Fmod(double(v126) + double(3.1415927f), 6.283185482025146f) - 3.1415927f;
  *(float *)&v299 = v127;
  v368 = _mm_shuffle_ps(VectorBits(v299), VectorBits(v299), 0);
  v128 = v368.m128_f32[0];
  v488 = v368;
  if ( v368.m128_f32[0] > 3.1415927f )
    v128 = v368.m128_f32[0] - 6.2831855f;
  v393 = 0;
  v395 = (float)(-0.0f - v295) - v128;
  v129 = (float *)&v393;
  if ( !(v395 > 0.0f) )
    v129 = &v395;
  v130 = *v129;
  v397 = 0;
  v399 = v295 - v128;
  v131 = (float *)&v397;
  if ( !(0.0f > (float)(v295 - v128)) )
    v131 = &v399;
  v132 = *v131;
  if ( v130 > *v131 )
  {
    v133 = v130;
    v130 = *v131;
    v132 = v133;
  }
  v134 = (COERCE_INT(v130 - v272) >> 31) & LODWORD(v272) | LODWORD(v130) & ~(COERCE_INT(v130 - v272) >> 31);
  v273 = (COERCE_INT(v132 - *(float *)&v134) >> 31) & LODWORD(v132) | v134 & ~(COERCE_INT(v132 - *(float *)&v134) >> 31);
  v135 = Fabs(v332 * 0.63661975f);
  v136 = (COERCE_INT(1.0f - COERCE_FLOAT((COERCE_INT(-0.0f - v135) >> 31) & LODWORD(v135))) >> 31) & 0x3F800000
       | (COERCE_INT(-0.0f - v135) >> 31)
       & LODWORD(v135)
       & ~(COERCE_INT(1.0f - COERCE_FLOAT((COERCE_INT(-0.0f - v135) >> 31) & LODWORD(v135))) >> 31);
  state->tail[352-72] = 1;
  LODWORD(v137) = (COERCE_INT(1.0f - COERCE_FLOAT((COERCE_INT(-0.0f - *(float *)&v136) >> 31) & v136)) >> 31) & 0x3F800000
                | (COERCE_INT(-0.0f - *(float *)&v136) >> 31)
                & v136
                & ~(COERCE_INT(1.0f - COERCE_FLOAT((COERCE_INT(-0.0f - *(float *)&v136) >> 31) & v136)) >> 31);
  StateScalar(state, 244) = v137;
  v138 = StateScalar(state, 124);
  v274 = (float)((float)(-0.0f - *(float *)&v273) * (float)(v137 * GlobalFloat<0x0114F2D8>())) + *(float *)&v273;
  v405 = v373 + v364;
  v333 = v137 * GlobalFloat<0x0114F2D8>();
  v330 = (float)((float)(-0.0f - *(float *)&v329) * (float)(v137 * GlobalFloat<0x0114F2D8>())) + *(float *)&v329;
  v258 = v138;
  v302 = sub_55DE20(
           v258,
           (float)((float)(-0.0f - *(float *)&v337) * (float)(v137 * GlobalFloat<0x0114F2D8>())) + *(float *)&v337,
           StateScalar(state, 80),
           GlobalFloat<0x0114f2b4>(),
           &v405);
  state->tail[352-72] = 1;
  StateScalar(state, 252) = v302;
  v139 = StateScalar(state, 120);
  v389 = v339 * 2.0f;
  v259 = v139;
  v300 = sub_55DE20(v259, v330, StateScalar(state, 80), GlobalFloat<0x0114f2b4>(), &v389);
  state->tail[352-72] = 1;
  StateScalar(state, 248) = v300;
  v140 = StateScalar(state, 128);
  v404 = v295 * 2.0f;
  v260 = v140;
  v141 = sub_55DE20(v260, v274, StateScalar(state, 80), GlobalFloat<0x0114f2b4>(), &v404);
  state->tail[352-72] = 1;
  v340 = v141;
  StateScalar(state, 256) = v141;
  v390 = 1.0f - v333;
  v142 = &a4;
  if ( !((float)(1.0f - v333) > a4) )
    v142 = &v390;
  a4 = *v142;
  v414 = v396;
  v143 = VectorBits(LODWORD(v302));
  v143.m128_f32[0] = v302 * 0.2f;
  v369 = _mm_shuffle_ps(v143, v143, 0).m128_f32[0] / v387.m128_f32[0];
  *(float *)&v303 = Sin(v369);
  v144 = _mm_shuffle_ps(VectorBits(v303), VectorBits(v303), 0);
  *(float *)&v304 = Cos(v369);
  v442 = &v414;
  v145 = _mm_mul_ps(_mm_shuffle_ps(v396, v396, 85), v144);
  v499 = _mm_mul_ps(_mm_shuffle_ps(v396, v396, 0), v144);
  v497 = _mm_mul_ps(_mm_shuffle_ps(v396, v396, 170), v144);
  v146 = _mm_shuffle_ps(
           _mm_shuffle_ps(v499, v145, 16),
           _mm_shuffle_ps(v497, _mm_shuffle_ps(VectorBits(v304), VectorBits(v304), 0), 50),
           136);
  v428 = &v414;
  v509 = v145;
  v418 = &v414;
  v382 = v146;
  if ( DebugGlobal() && *(unsigned char *)(DebugGlobal() + 48) )
  {
    auto* debugContext=static_cast<unsigned char*>(sub_6E8330())+0x328;
    v305 = (int64_t)(double(a4) * 255.0);
    v147 = reinterpret_cast<uintptr_t>(state->debugOwner);
    if ( v147 )
      v331 = uint32_t(CallGameMethod<uint64_t,0x0054dcb0>(reinterpret_cast<void*>(v147))) % 6u;
    else
      v331 = 0;
    sub_54F260(&v445, a3, ((uint32_t)v305 << 24) | DebugColour(v331));
    sub_6C87F0(a3, 0.1f, ((uint32_t)v305 << 24) | DebugColour(v331));
    sub_54C8D0(v371, v493, v447);
    sub_54EEF0(v447, v518);
    v148 = (__m128 *)CallGameMethod<void*,0x00577240>(v447,v519);
    v377 = _mm_shuffle_ps(VectorBits(0x3E800000u), VectorBits(0x3E800000u), 0);
    v149 = _mm_mul_ps(v377, v388);
    v490 = _mm_add_ps(
             _mm_add_ps(
               _mm_add_ps(_mm_mul_ps(_mm_shuffle_ps(v149, v149, 0), *v148), v148[3]),
               _mm_mul_ps(_mm_shuffle_ps(v149, v149, 85), v148[1])),
             _mm_mul_ps(_mm_shuffle_ps(v149, v149, 170), v148[2]));
    v150 = (__m128 *)CallGameMethod<void*,0x00577240>(v447,v520);
    v151 = _mm_mul_ps(VectorBits(0x3F80000000000000uLL), v377);
    v492 = _mm_add_ps(
             _mm_add_ps(
               _mm_add_ps(_mm_mul_ps(_mm_shuffle_ps(v151, v151, 0), *v150), v150[3]),
               _mm_mul_ps(_mm_shuffle_ps(v151, v151, 85), v150[1])),
             _mm_mul_ps(_mm_shuffle_ps(v151, v151, 170), v150[2]));
    v152 = (__m128 *)CallGameMethod<void*,0x00577240>(v447,v521);
    v153 = _mm_mul_ps(v377, v396);
    v469 = _mm_add_ps(
             _mm_add_ps(
               _mm_add_ps(_mm_mul_ps(_mm_shuffle_ps(v153, v153, 0), *v152), v152[3]),
               _mm_mul_ps(_mm_shuffle_ps(v153, v153, 85), v152[1])),
             _mm_mul_ps(_mm_shuffle_ps(v153, v153, 170), v152[2]));
    sub_54F260(v447, &v490, 0x00ff0000u | ((uint32_t)v305 << 24));
    sub_54F260(v447, &v492, ((uint32_t)v305 << 24) | 0xFF00);
    sub_54F260(v447, &v469, ((uint32_t)v305 << 24) | 0xFF);
    v146 = v382;
  }
  v154 = VectorBits(LODWORD(v300));
  v154.m128_f32[0] = v300 * 0.2f;
  v275 = 0;
  v383 = _mm_div_ps(_mm_shuffle_ps(v154, v154, 0), v387);
  *(float *)&v306 = Sin(v383.m128_f32[0]);
  v370 = _mm_shuffle_ps(VectorBits(v306), VectorBits(v306), 0);
  *(float *)&v307 = Cos(v383.m128_f32[0]);
  v408 = _mm_shuffle_ps(VectorBits(v307), VectorBits(v307), 0);
  v410 = _mm_shuffle_ps(v146, v146, 255);
  v471 = v410;
  v344 = _mm_shuffle_ps(v146, v146, 18);
  v363 = _mm_shuffle_ps(v146, v146, 9);
  v388 = _mm_shuffle_ps(VectorBits(0x3DB2B8C3u), VectorBits(0x3DB2B8C3u), 0);
  v407 = v388;
  *(float *)&v308 = Cos(v388.m128_f32[0]);
  v491 = _mm_shuffle_ps(VectorBits(v308), VectorBits(v308), 0);
  do
  {
    v296 = &v372[v275 + 7];
    sub_54EEF0(v6 + 48 * *v296, v508);
    v155 = 48 * *v296 + sub_574190(reinterpret_cast<uintptr_t>(pose));
    v481[0] = VectorBits(0x3F80000000000000uLL);
    sub_555110(v481, v508, &v415);
    v511 = _mm_mul_ps(_mm_shuffle_ps(v415, v415, 0), v370);
    v513 = _mm_mul_ps(_mm_shuffle_ps(v415, v415, 170), v370);
    v501 = _mm_mul_ps(_mm_shuffle_ps(v415, v415, 85), v370);
    v156 = _mm_shuffle_ps(_mm_shuffle_ps(v511, v501, 16), _mm_shuffle_ps(v513, v408, 50), 136);
    v436 = &v415;
    v430 = &v415;
    v440 = &v415;
    v157 = *(__m128 *)(v155 + 16);
    v158 = _mm_shuffle_ps(v157, v157, 255);
    v159 = _mm_add_ps(
             _mm_sub_ps(
               _mm_mul_ps(_mm_shuffle_ps(v157, v157, 18), v363),
               _mm_mul_ps(_mm_shuffle_ps(v157, v157, 9), v344)),
             _mm_mul_ps(v471, v157));
    v160 = _mm_mul_ps(v157, v382);
    v161 = _mm_add_ps(v159, _mm_mul_ps(v158, v382));
    v162 = _mm_add_ps(_mm_add_ps(_mm_shuffle_ps(v160, v160, 2), _mm_shuffle_ps(v160, v160, 1)), v160);
    v163 = _mm_shuffle_ps(
             v161,
             _mm_shuffle_ps(v161, _mm_sub_ps(_mm_mul_ps(v471, v158), _mm_shuffle_ps(v162, v162, 0)), 10),
             132);
    v164 = _mm_shuffle_ps(v156, v156, 255);
    v165 = _mm_shuffle_ps(v163, v163, 255);
    v166 = _mm_add_ps(
             _mm_sub_ps(
               _mm_mul_ps(_mm_shuffle_ps(v163, v163, 18), _mm_shuffle_ps(v156, v156, 9)),
               _mm_mul_ps(_mm_shuffle_ps(v163, v163, 9), _mm_shuffle_ps(v156, v156, 18))),
             _mm_mul_ps(v164, v163));
    v167 = _mm_mul_ps(v163, v156);
    v168 = _mm_add_ps(v166, _mm_mul_ps(v165, v156));
    v169 = _mm_add_ps(_mm_add_ps(_mm_shuffle_ps(v167, v167, 2), _mm_shuffle_ps(v167, v167, 1)), v167);
    v170 = _mm_shuffle_ps(
             v168,
             _mm_shuffle_ps(v168, _mm_sub_ps(_mm_mul_ps(v165, v164), _mm_shuffle_ps(v169, v169, 0)), 10),
             132);
    v353 = v170;
    if ( v275 == 3 )
    {
      v486 = v396;
      sub_555110(&v486, v508, &v417);
      v171 = VectorBits(LODWORD(v340));
      v171.m128_f32[0] = v340 * 0.5f;
      v388 = _mm_div_ps(_mm_shuffle_ps(v171, v171, 0), v387);
      v472[1] = v388;
      *(float *)&v309 = Sin(v388.m128_f32[0]);
      v172 = _mm_shuffle_ps(VectorBits(v309), VectorBits(v309), 0);
      *(float *)&v310 = Cos(v388.m128_f32[0]);
      v173 = _mm_mul_ps(_mm_shuffle_ps(v417, v417, 85), v172);
      v503 = _mm_mul_ps(_mm_shuffle_ps(v417, v417, 170), v172);
      v505 = _mm_mul_ps(_mm_shuffle_ps(v417, v417, 0), v172);
      v174 = _mm_shuffle_ps(
               _mm_shuffle_ps(v505, v173, 16),
               _mm_shuffle_ps(v503, _mm_shuffle_ps(VectorBits(v310), VectorBits(v310), 0), 50),
               136);
      v175 = _mm_shuffle_ps(v174, v174, 255);
      v515 = v173;
      v176 = _mm_shuffle_ps(v353, v353, 255);
      v177 = _mm_add_ps(
               _mm_sub_ps(
                 _mm_mul_ps(_mm_shuffle_ps(v174, v174, 9), _mm_shuffle_ps(v353, v353, 18)),
                 _mm_mul_ps(_mm_shuffle_ps(v174, v174, 18), _mm_shuffle_ps(v353, v353, 9))),
               _mm_mul_ps(v175, v353));
      v178 = _mm_mul_ps(v176, v174);
      v179 = _mm_mul_ps(v174, v353);
      v180 = _mm_add_ps(v177, v178);
      v181 = _mm_add_ps(_mm_add_ps(_mm_shuffle_ps(v179, v179, 2), _mm_shuffle_ps(v179, v179, 1)), v179);
      v419 = &v417;
      v423 = &v417;
      v425 = &v417;
      v170 = _mm_shuffle_ps(
               v180,
               _mm_shuffle_ps(v180, _mm_sub_ps(_mm_mul_ps(v176, v175), _mm_shuffle_ps(v181, v181, 0)), 10),
               132);
      v353 = v170;
    }
    v182 = *(__m128 *)(v155 + 16);
    v183 = _mm_mul_ps(v182, v170);
    v184 = _mm_add_ps(_mm_shuffle_ps(v183, v183, 14), v183);
    v185 = _mm_add_ps(_mm_shuffle_ps(v184, v184, 1), v184);
    v186 = _mm_shuffle_ps(v185, v185, 0);
    v482 = v413.m128_f32[0];
    v474 = v186.m128_i32[0];
    v187 = _mm_shuffle_ps(VectorBits(LODWORD(a4)), VectorBits(LODWORD(a4)), 0);
    v489 = v187;
    v378 = v182;
    if ( v413.m128_f32[0] > v186.m128_f32[0] )
    {
      v182 = _mm_sub_ps(v413, v182);
      v378 = v182;
      v186 = _mm_sub_ps(v413, v186);
    }
    v484 = v491.m128_i32[0];
    v476 = v186.m128_i32[0];
    if ( !(v186.m128_f32[0] > v491.m128_f32[0]) )
    {
      v477[1] = v186;
      *(float *)&v311 = Acos(v186.m128_f32[0]);
      v200 = _mm_shuffle_ps(VectorBits(v311), VectorBits(v311), 0);
      v479 = _mm_mul_ps(v489, v200);
      v467[1] = v200;
      v437 = _mm_mul_ps(_mm_sub_ps(v403, v489), v200);
      v409 = v200;
      *(float *)&v312 = Sin(v479.m128_f32[0]);
      v201 = _mm_shuffle_ps(VectorBits(v312), VectorBits(v312), 0);
      *(float *)&v313 = Sin(v200.m128_f32[0]);
      v202 = _mm_mul_ps(_mm_div_ps(v201, _mm_shuffle_ps(VectorBits(v313), VectorBits(v313), 0)), v353);
      *(float *)&v314 = Sin(v437.m128_f32[0]);
      v203 = Sin(v200.m128_f32[0]);
      v204 = _mm_shuffle_ps(VectorBits(v314), VectorBits(v314), 0);
      *(float *)&v315 = v203;
      v199 = _mm_add_ps(v202, _mm_mul_ps(_mm_div_ps(v204, _mm_shuffle_ps(VectorBits(v315), VectorBits(v315), 0)), v378));
    }
    else
    {
      v188 = _mm_mul_ps(v182, v170);
      v189 = _mm_add_ps(_mm_shuffle_ps(v188, v188, 14), v188);
      v190 = _mm_add_ps(_mm_shuffle_ps(v189, v189, 1), v189);
      v191 = _mm_cmplt_ps(v413, _mm_shuffle_ps(v190, v190, 0));
      v192 = _mm_or_ps(
               _mm_andnot_ps(v191, _mm_sub_ps(v182, _mm_mul_ps(_mm_add_ps(v182, v170), v187))),
               _mm_and_ps(_mm_add_ps(_mm_mul_ps(_mm_sub_ps(v170, v182), v187), v182), v191));
      v193 = _mm_mul_ps(v192, v192);
      v194 = _mm_add_ps(_mm_shuffle_ps(v193, v193, 14), v193);
      v195 = _mm_add_ps(_mm_shuffle_ps(v194, v194, 1), v194);
      v196 = _mm_shuffle_ps(v195, v195, 0);
      v197 = _mm_rsqrt_ps(v196);
      v198 = _mm_add_ps(
               _mm_mul_ps(_mm_sub_ps(v403, _mm_mul_ps(_mm_mul_ps(v197, v197), v196)), _mm_mul_ps(v385, v197)),
               v197);
      v199 = _mm_mul_ps(
               _mm_add_ps(
                 _mm_mul_ps(_mm_sub_ps(v403, _mm_mul_ps(_mm_mul_ps(v198, v198), v196)), _mm_mul_ps(v385, v198)),
                 v198),
               v192);
    }
    v448[1] = v199;
    *(__m128 *)(v155 + 16) = v199;
    if ( v275 < 3 )
    {
      pose->flags &= ~1u;
      CallGameMethod<void,0x00574500>(pose, *v296, v372[v275 + 8] - *v296 + 1);
    }
    ++v275;
  }
  while ( v275 < 4 );
  pose->flags &= ~1u;
  CallGameMethod<void,0x00574500>(pose, *v386, *v374 - *v386 + 1);
  sub_54EEF0(v6 + 48 * *v374, v510);
  v449 = VectorBits(0x3F80000000000000uLL);
  sub_555110(&v449, v510, &v412);
  *(float *)&v316 = Sin(v383.m128_f32[0]);
  v205 = _mm_shuffle_ps(VectorBits(v316), VectorBits(v316), 0);
  *(float *)&v317 = Cos(v383.m128_f32[0]);
  v427 = &v412;
  v429 = &v412;
  v514 = _mm_mul_ps(_mm_shuffle_ps(v412, v412, 170), v205);
  v495 = _mm_mul_ps(_mm_shuffle_ps(v412, v412, 0), v205);
  v431 = &v412;
  v507 = _mm_mul_ps(_mm_shuffle_ps(v412, v412, 85), v205);
  v409 = _mm_shuffle_ps(
           _mm_shuffle_ps(v495, v507, 16),
           _mm_shuffle_ps(v514, _mm_shuffle_ps(VectorBits(v317), VectorBits(v317), 0), 50),
           136);
  v452 = v396;
  sub_555110(&v452, v510, &v416);
  v206 = VectorBits(LODWORD(v340));
  v206.m128_f32[0] = v340 * 0.5f;
  v206.m128_f32[0] = _mm_shuffle_ps(v206, v206, 0).m128_f32[0];
  *(float *)&v318 = Sin((float)(v206.m128_f32[0] / v387.m128_f32[0]));
  v207 = _mm_shuffle_ps(VectorBits(v318), VectorBits(v318), 0);
  *(float *)&v319 = Cos((float)(v206.m128_f32[0] / v387.m128_f32[0]));
  v500 = _mm_mul_ps(_mm_shuffle_ps(v416, v416, 0), v207);
  v496 = _mm_mul_ps(_mm_shuffle_ps(v416, v416, 170), v207);
  v439 = &v416;
  v433 = &v416;
  v435 = &v416;
  v498 = _mm_mul_ps(_mm_shuffle_ps(v416, v416, 85), v207);
  v383 = _mm_shuffle_ps(
           _mm_shuffle_ps(v500, v498, 16),
           _mm_shuffle_ps(v496, _mm_shuffle_ps(VectorBits(v319), VectorBits(v319), 0), 50),
           136);
  v208 = sub_574190(reinterpret_cast<uintptr_t>(pose));
  v209 = 48 * *v374;
  v210 = *(__m128 *)(v208 + v209 + 16);
  v211 = (__m128 *)(v209 + v208);
  v212 = _mm_mul_ps(v382, v210);
  v213 = _mm_shuffle_ps(v210, v210, 255);
  v214 = _mm_add_ps(
           _mm_add_ps(
             _mm_sub_ps(
               _mm_mul_ps(_mm_shuffle_ps(v210, v210, 18), v363),
               _mm_mul_ps(_mm_shuffle_ps(v210, v210, 9), v344)),
             _mm_mul_ps(v410, v210)),
           _mm_mul_ps(v382, v213));
  v215 = _mm_add_ps(_mm_add_ps(_mm_shuffle_ps(v212, v212, 2), _mm_shuffle_ps(v212, v212, 1)), v212);
  v216 = _mm_shuffle_ps(
           v214,
           _mm_shuffle_ps(v214, _mm_sub_ps(_mm_mul_ps(v213, v410), _mm_shuffle_ps(v215, v215, 0)), 10),
           132);
  v217 = _mm_shuffle_ps(v409, v409, 255);
  v218 = _mm_shuffle_ps(v216, v216, 255);
  v219 = _mm_mul_ps(v409, v216);
  v220 = _mm_add_ps(
           _mm_add_ps(
             _mm_sub_ps(
               _mm_mul_ps(_mm_shuffle_ps(v216, v216, 18), _mm_shuffle_ps(v409, v409, 9)),
               _mm_mul_ps(_mm_shuffle_ps(v216, v216, 9), _mm_shuffle_ps(v409, v409, 18))),
             _mm_mul_ps(v217, v216)),
           _mm_mul_ps(v409, v218));
  v221 = _mm_add_ps(_mm_add_ps(_mm_shuffle_ps(v219, v219, 2), _mm_shuffle_ps(v219, v219, 1)), v219);
  v222 = _mm_shuffle_ps(
           v220,
           _mm_shuffle_ps(v220, _mm_sub_ps(_mm_mul_ps(v218, v217), _mm_shuffle_ps(v221, v221, 0)), 10),
           132);
  v223 = _mm_shuffle_ps(v383, v383, 255);
  v224 = _mm_shuffle_ps(v222, v222, 255);
  v225 = _mm_mul_ps(v383, v222);
  v226 = _mm_add_ps(
           _mm_add_ps(
             _mm_sub_ps(
               _mm_mul_ps(_mm_shuffle_ps(v383, v383, 9), _mm_shuffle_ps(v222, v222, 18)),
               _mm_mul_ps(_mm_shuffle_ps(v383, v383, 18), _mm_shuffle_ps(v222, v222, 9))),
             _mm_mul_ps(v223, v222)),
           _mm_mul_ps(v383, v224));
  v227 = _mm_add_ps(_mm_add_ps(_mm_shuffle_ps(v225, v225, 2), _mm_shuffle_ps(v225, v225, 1)), v225);
  v228 = _mm_shuffle_ps(
           v226,
           _mm_shuffle_ps(v226, _mm_sub_ps(_mm_mul_ps(v224, v223), _mm_shuffle_ps(v227, v227, 0)), 10),
           132);
  v229 = _mm_mul_ps(v210, v228);
  v230 = _mm_add_ps(_mm_shuffle_ps(v229, v229, 14), v229);
  v231 = _mm_add_ps(_mm_shuffle_ps(v230, v230, 1), v230);
  v232 = _mm_shuffle_ps(v231, v231, 0).m128_f32[0];
  v456 = v413.m128_f32[0];
  v458 = v232;
  v233 = _mm_shuffle_ps(VectorBits(LODWORD(a4)), VectorBits(LODWORD(a4)), 0);
  v437 = v228;
  v410 = v233;
  v345 = v210;
  if ( v413.m128_f32[0] > v232 )
  {
    v210 = _mm_sub_ps(v413, v210);
    v345 = v210;
    v232 = v413.m128_f32[0] - v232;
  }
  v462 = v232;
  *(float *)&v320 = Cos(v407.m128_f32[0]);
  v464 = _mm_shuffle_ps(VectorBits(v320), VectorBits(v320), 0).m128_f32[0];
  if ( !(v232 > v464) )
  {
    *(float *)&v321 = Acos(v232);
    v246 = _mm_shuffle_ps(VectorBits(v321), VectorBits(v321), 0);
    v407 = _mm_mul_ps(_mm_sub_ps(v403, v410), v246);
    v383 = v246;
    v409 = v246;
    v410 = _mm_mul_ps(v246, v410);
    *(float *)&v322 = Sin(v407.m128_f32[0]);
    v247 = Sin(v246.m128_f32[0]);
    v248 = _mm_shuffle_ps(VectorBits(v322), VectorBits(v322), 0);
    *(float *)&v323 = v247;
    v249 = _mm_mul_ps(_mm_div_ps(v248, _mm_shuffle_ps(VectorBits(v323), VectorBits(v323), 0)), v345);
    *(float *)&v324 = Sin(v410.m128_f32[0]);
    v250 = _mm_shuffle_ps(VectorBits(v324), VectorBits(v324), 0);
    *(float *)&v325 = Sin(v409.m128_f32[0]);
    v245 = _mm_add_ps(v249, _mm_mul_ps(_mm_div_ps(v250, _mm_shuffle_ps(VectorBits(v325), VectorBits(v325), 0)), v437));
  }
  else
  {
    v234 = _mm_mul_ps(v210, v228);
    v235 = _mm_add_ps(_mm_shuffle_ps(v234, v234, 14), v234);
    v236 = _mm_add_ps(_mm_shuffle_ps(v235, v235, 1), v235);
    v237 = _mm_cmplt_ps(v413, _mm_shuffle_ps(v236, v236, 0));
    v238 = _mm_or_ps(
             _mm_andnot_ps(v237, _mm_sub_ps(v210, _mm_mul_ps(_mm_add_ps(v210, v228), v233))),
             _mm_and_ps(_mm_add_ps(_mm_mul_ps(_mm_sub_ps(v228, v210), v233), v210), v237));
    v239 = _mm_mul_ps(v238, v238);
    v240 = _mm_add_ps(_mm_shuffle_ps(v239, v239, 14), v239);
    v241 = _mm_add_ps(_mm_shuffle_ps(v240, v240, 1), v240);
    v242 = _mm_shuffle_ps(v241, v241, 0);
    v243 = _mm_rsqrt_ps(v242);
    v244 = _mm_add_ps(
             _mm_mul_ps(_mm_sub_ps(v403, _mm_mul_ps(_mm_mul_ps(v243, v243), v242)), _mm_mul_ps(v243, v385)),
             v243);
    v245 = _mm_mul_ps(
             _mm_add_ps(
               _mm_mul_ps(_mm_sub_ps(v403, _mm_mul_ps(_mm_mul_ps(v244, v244), v242)), _mm_mul_ps(v244, v385)),
               v244),
             v238);
  }
  v466 = v245;
  v211[1] = v245;
  result = (int32_t *)sub_574990(reinterpret_cast<uintptr_t>(pose));
  v251 = v381;
  if ( *v381 >= 0 && *v375 >= 0 )
  {
    pose->flags &= ~1u;
    v252 = v251;
    v253 = (int32_t *)sub_555060(v251, v375);
    CallGameMethod<void,0x00574500>(pose, *v386, *v253 - *v386 + 1);
    v254 = v6 + 48 * *v252;
    v255 = sub_574190(reinterpret_cast<uintptr_t>(pose));
    sub_555F50(v446, v254, 48 * *v381 + v255, a4);
    v256 = v6 + 48 * *v375;
    v257 = sub_574190(reinterpret_cast<uintptr_t>(pose));
    return (int32_t *)sub_555F50(v446, v256, 48 * *v375 + v257, a4);
  }
  return result;
}

}
