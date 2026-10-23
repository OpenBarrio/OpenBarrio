#include "ts3/anim/m05_B25.h"
#include "ts3/layout.h"
#include <emmintrin.h>
#include <cstring>
#include <cmath>
namespace ts3::anim::b25 {
namespace {
void* Table(uintptr_t a) {
#if defined(_M_IX86)
 return reinterpret_cast<void*>(a);
#else
 (void)a; static uintptr_t native[8]{}; return native;
#endif
}
uint32_t Word(const void* p) { uint32_t v; std::memcpy(&v,p,4); return v; }
void Swap(uint8_t* p,unsigned n,uint32_t swap) {
 if(uint8_t(swap)) CallGameFunction<void,0x00739270>(p,n);
}
bool Near(float a,float b) { return !(std::fabs(double(a)-double(b))>double(0.001f)); }
uint32_t Result(uint32_t value,bool ok) { return (value&0xffffff00u)|uint32_t(ok); }
}
Accumulator* Normalize(Accumulator* a) {
 auto* p=reinterpret_cast<float*>(a);
 if(!(p[18]>0.0f)) {
  _mm_store_ps(p+4,_mm_setzero_ps()); _mm_store_ps(p+8,_mm_set_ps(1,0,0,0));
 } else {
  __m128 reciprocal=_mm_set1_ps(1.0f/p[18]);
  __m128 q=_mm_mul_ps(reciprocal,_mm_load_ps(p+8));
  __m128 square=_mm_mul_ps(q,q);
  __m128 sum=_mm_add_ps(_mm_shuffle_ps(square,square,14),square);
  sum=_mm_add_ps(_mm_shuffle_ps(sum,sum,1),sum);
  __m128 norm=_mm_shuffle_ps(sum,sum,0), r=_mm_rsqrt_ps(norm);
  const __m128 one=_mm_set1_ps(1), half=_mm_set1_ps(0.5f);
  for(unsigned i=0;i<2;++i) r=_mm_add_ps(_mm_mul_ps(_mm_sub_ps(one,_mm_mul_ps(_mm_mul_ps(r,r),norm)),_mm_mul_ps(half,r)),r);
  _mm_store_ps(p+4,_mm_mul_ps(reciprocal,_mm_load_ps(p+4)));
  _mm_store_ps(p+8,_mm_mul_ps(r,q));
 }
 p[18]=0; return a;
}
uint8_t __fastcall Accept(Event*,void*,void*,uint8_t**,uint32_t*,uint32_t) { return 1; }
uint8_t __fastcall ReadScalar(ScalarEvent* s,void*,void*,uint8_t** p,uint32_t* n,uint32_t swap) {
 if(*n<4) return 0;
 uint8_t* data=*p; Swap(data,1,swap); float v;std::memcpy(&v,data,4);
 if(v>=0) s->value=v;
 *p+=4; --*n; return 1;
}
uint8_t __fastcall ReadWord(DataEvent* s,void*,void*,uint8_t** p,uint32_t* n,uint32_t swap) {
 if(*n<4) return 0;
 uint8_t* data=*p; Swap(data,1,swap); s->data[0]=Word(data); *p+=4; --*n; return 1;
}
uint8_t __fastcall ReadSix(DataEvent* s,void*,void*,uint8_t** p,uint32_t* n,uint32_t swap) {
 if(*n<24) return 0;
 uint8_t* data=*p; Swap(data,6,swap);
 s->data[4]=Word(data+12);s->data[5]=Word(data+16);s->data[6]=Word(data+8);
 s->data[2]=Word(data);s->data[3]=Word(data+4); *p+=24; *n-=6; return 1;
}
uint32_t __fastcall Identifier(DataEvent* s,void*) { return s->data[0]?s->data[0]:s->base.fallback; }
uint8_t __fastcall ReadPair(DataEvent* s,void*,void*,uint8_t** p,uint32_t* n,uint32_t swap) {
 if(*n<8) return 0;
 uint8_t* data=*p;Swap(data,2,swap);s->data[0]=Word(data);s->data[1]=Word(data+4);*p+=8;*n-=2;return 1;
}
Event* __fastcall Construct(Event* s,void*,uint32_t owner) {
 s->vtable=Table(0x00FCB4D8);s->owner=owner;s->type=0xc1e40004;s->subtype=0;
 s->time=0;uint32_t t=1007192201,d=1023969417;std::memcpy(&s->tolerance,&t,4);std::memcpy(&s->duration,&d,4);
 s->flags=6;s->unused=0;s->fallback=0;return s;
}
MatrixEvent* __fastcall ConstructMatrix(MatrixEvent* s,void*,uint32_t owner) {
 Construct(&s->base,nullptr,owner);s->base.vtable=Table(0x00FCB4F8);s->base.type=0xc1e40001;s->base.flags=70;
 for(auto& key:s->keys) key=0;
 for(unsigned i=0;i<16;++i) s->matrix[i]=(i%5==0)?1.0f:0.0f;
 return s;
}
uint32_t __fastcall Equal(Event* s,void*,const Event* b) {
 uint32_t r=s->type;if(r!=b->type) return Result(r,false);
 r=s->subtype;if(r!=b->subtype) return Result(r,false);
 r=s->flags;return Result(r,r==b->flags&&Near(s->time,b->time)&&Near(s->tolerance,b->tolerance)&&Near(s->duration,b->duration));
}
uint8_t __fastcall ReadMatrix(MatrixEvent* s,void*,void*,uint8_t** p,uint32_t* n,uint32_t swap) {
 if(*n<80) return 0;
 uint8_t* data=*p;Swap(data,20,swap);
 std::memcpy(s->keys,data,12);std::memcpy(s->matrix,data+16,64);*p+=80;*n-=20;return 1;
}
uint32_t __fastcall EqualMatrix(MatrixEvent* s,void*,const MatrixEvent* b) {
 uint32_t r=Equal(&s->base,nullptr,&b->base);if(!uint8_t(r)) return r;
 r=s->keys[0];if(r!=b->keys[0]||s->keys[1]!=b->keys[1]) return Result(r,false);
 r=s->keys[2];if(r!=b->keys[2]) return Result(r,false);
 __m128 mask=_mm_castsi128_ps(_mm_set1_epi32(-1));
 for(unsigned i=0;i<16;i+=4) {
  __m128 d=_mm_sub_ps(_mm_load_ps(s->matrix+i),_mm_load_ps(b->matrix+i));
  mask=_mm_and_ps(mask,_mm_cmple_ps(_mm_max_ps(_mm_mul_ps(_mm_set1_ps(-1),d),d),_mm_set1_ps(0x1p-16f)));
 }
 return Result(r,_mm_movemask_ps(mask)==15);
}
uint32_t __fastcall EqualScalar(ScalarEvent* s,void*,const ScalarEvent* b) {
 uint32_t r=Equal(&s->base,nullptr,&b->base);return Result(r,uint8_t(r)&&Near(s->value,b->value));
}
Event* __fastcall Destroy(Event* s,void*,uint32_t flags) {
 s->vtable=Table(0x00FCB4D8);if(flags&1) CallGameFunction<void,0x004E3F70>(s);return s;
}
void* __fastcall CollectKeys(MatrixEvent* s,void*,void* set) {
 struct Iterator { void* node;void* bucket;uint32_t inserted; } iterator;
 uint32_t key=s->keys[1];CallGameMethod<void*,0x00B7AAD0>(set,&iterator,&key,uint32_t(0));
 key=s->keys[0];return CallGameMethod<void*,0x00B7AAD0>(set,&iterator,&key,uint32_t(0));
}
}
