#include "ts3/anim/m05_M104.h"
#include <immintrin.h>
#include <cstring>
namespace ts3::anim::m104 {
template<class T> static T Read(const void* p,size_t off) { T v; std::memcpy(&v,static_cast<const unsigned char*>(p)+off,sizeof v); return v; }
static float Fraction(float interval,float time,uint32_t& frame) {
 float reciprocal=_mm_cvtss_f32(_mm_div_ss(_mm_set_ss(1),_mm_set_ss(interval)));
 float f;
#if defined(_M_IX86)
 int64_t index; uint16_t cw,truncate; double unsignedIndex;
 __asm { fnstcw cw }
 truncate=cw|0xc00;
 __asm {
  fld reciprocal
  fmul time
  fldcw truncate
  fistp index
  fldcw cw
 }
 frame=static_cast<uint32_t>(index); unsignedIndex=static_cast<double>(frame);
 __asm {
  fld unsignedIndex
  fmul interval
  fsubr time
  fmul reciprocal
  fstp f
 }
#else
 frame=static_cast<uint32_t>(static_cast<int64_t>(double(reciprocal)*time));
 f=static_cast<float>((double(time)-double(frame)*interval)*reciprocal);
#endif
 // EA's bitwise clamp preserves signed zero and uses the sign of -f.
 uint32_t bits=Read<uint32_t>(&f,0), neg=Read<uint32_t>(&f,0)^0x80000000u;
 bits&=static_cast<uint32_t>(static_cast<int32_t>(neg)>>31);
 float lower; std::memcpy(&lower,&bits,4); float delta=1.0f-lower;
 if(Read<uint32_t>(&delta,0)&0x80000000u) return 1.0f;
 return lower;
}
static __m128 Sum(__m128 x) { x=_mm_add_ps(_mm_shuffle_ps(x,x,14),x); x=_mm_add_ps(_mm_shuffle_ps(x,x,1),x); return _mm_shuffle_ps(x,x,0); }
static __m128 Rotation(__m128 a,__m128 b,__m128 t) {
 auto zero=_mm_setzero_ps(),one=_mm_set1_ps(1),half=_mm_set1_ps(.5f),minus=_mm_set1_ps(-1),eps=_mm_set1_ps(1.0f/65536);
 auto near=[&](__m128 x) { x=_mm_sub_ps(x,zero); return _mm_movemask_ps(_mm_cmple_ps(_mm_max_ps(_mm_mul_ps(minus,x),x),eps))==15; };
 if(near(a)&&near(b)) { a=_mm_shuffle_ps(a,a,_MM_SHUFFLE(0,2,1,0)); b=_mm_shuffle_ps(b,b,_MM_SHUFFLE(0,2,1,0)); alignas(16) float aa[4],bb[4]; _mm_store_ps(aa,a);_mm_store_ps(bb,b);aa[3]=bb[3]=1; a=_mm_load_ps(aa);b=_mm_load_ps(bb); }
 auto mask=_mm_cmplt_ps(zero,Sum(_mm_mul_ps(b,a)));
 auto v=_mm_or_ps(_mm_andnot_ps(mask,_mm_sub_ps(a,_mm_mul_ps(_mm_add_ps(b,a),t))),_mm_and_ps(_mm_add_ps(_mm_mul_ps(_mm_sub_ps(b,a),t),a),mask));
 auto len=Sum(_mm_mul_ps(v,v)),r=_mm_rsqrt_ps(len);
 for(int i=0;i<2;++i) r=_mm_add_ps(_mm_mul_ps(_mm_sub_ps(one,_mm_mul_ps(_mm_mul_ps(r,r),len)),_mm_mul_ps(half,r)),r);
 return _mm_mul_ps(r,v);
}
template<unsigned Stride,unsigned Decoder> static bool Sample(const void* clip,b20::Pose* pose,float time,const int32_t* binding,uint8_t finish) {
 auto* output=b20::Local(pose); uint32_t frame;
 float fraction=Fraction(Read<float>(clip,16),time,frame); auto t=_mm_set1_ps(fraction);
 auto* base=static_cast<const unsigned char*>(clip);
 int32_t channelOffset=Read<int32_t>(clip,32),valueOffset=Read<int32_t>(clip,36);
 auto* channels=channelOffset==-1?nullptr:base+channelOffset;
 for(uint32_t i=0;i<Read<uint32_t>(clip,24);++i) {
  auto* channel=channels+Stride*i; uint32_t key=Read<uint32_t>(channel,4); if(key==0x2af0a8ae) continue;
  uint8_t kind=channel[Stride-1];
  int32_t bone=0;
  if(kind!=7) { bone=binding?binding[i]:CallGameMethod<int32_t,0x575b00>(reinterpret_cast<unsigned char*>(pose->rig)+0x28,key); if(bone<0) continue; }
  alignas(16) float a[4],b[4]; const void* values=valueOffset==-1?nullptr:base+valueOffset;
  CallGameFunction<void,Decoder>(clip,channel,frame,frame+1,a,b,&values);
  if(kind==7) { CallGameMethod<void,0x574e20>(pose,key,(b[0]-a[0])*fraction+a[0]); continue; }
  if(kind>=1&&kind<=3) {
   __m128 av=_mm_load_ps(a),bv=_mm_load_ps(b),result;
   if(kind==2) result=Rotation(av,bv,t);
   else { av=_mm_shuffle_ps(av,av,_MM_SHUFFLE(0,2,1,0)); bv=_mm_shuffle_ps(bv,bv,_MM_SHUFFLE(0,2,1,0)); result=_mm_add_ps(_mm_mul_ps(_mm_sub_ps(bv,av),t),av); }
   _mm_store_ps(reinterpret_cast<float*>(output+bone)+4*(kind-1),result);
  }
 }
 if(finish) CallGameFunction<void,0x584010>(pose);
 return true;
}
bool Sample12(const void* c,b20::Pose* p,float t,const int32_t* b,uint8_t f) { return Sample<12,0x5855f0>(c,p,t,b,f); }
bool Sample20(const void* c,b20::Pose* p,float t,const int32_t* b,uint8_t f) { return Sample<20,0x585990>(c,p,t,b,f); }
}
