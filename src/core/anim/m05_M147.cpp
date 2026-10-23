#include "ts3/anim/m05_M147.h"
#include "ts3/routemanager/cls_857220_RouteManager.h"
#include <cstring>
#include "ts3/math/m04_B12.h"
#include <cmath>
#include <emmintrin.h>
namespace ts3::anim::m147 {
namespace {
uint32_t Bits(float x) { uint32_t r; std::memcpy(&r,&x,4); return r; }
int32_t SignedBits(float x) { return static_cast<int32_t>(Bits(x)); }
float Float(uint32_t x) { float r; std::memcpy(&r,&x,4); return r; }
__m128 VectorBits(uint32_t x) { return _mm_castsi128_ps(_mm_cvtsi32_si128(x)); }
__m128 Vector64(uint64_t x) { return _mm_castsi128_ps(_mm_set_epi64x(0,x)); }
void* Address(uint32_t x) {
#if defined(_M_IX86)
 return reinterpret_cast<void*>(uintptr_t(x));
#else
 (void)x; AbortNotRewritten("m147 game global"); return nullptr;
#endif
}
uint32_t& AngleFlags() { return *static_cast<uint32_t*>(Address(0x011d93c4)); }
uint32_t& AngleBits() { return *static_cast<uint32_t*>(Address(0x011d93c0)); }
// Keep the x87-ABI call out of the SIMD caller so the compiler sees a full register-clobber boundary.
__declspec(noinline) float ArcCos(float x) {
#if defined(_M_IX86)
 float r;
 __asm { fld x
         mov eax, 0x00f7edde
         call eax
         fstp r }
 return r;
#else
 return static_cast<float>(std::acos(static_cast<double>(x)));
#endif
}
__declspec(noinline) float Sine(float x) {
#if defined(_M_IX86)
 float r;
 __asm { fld x
         fsin
         fstp r }
 return r;
#else
 return static_cast<float>(std::sin(static_cast<double>(x)));
#endif
}
__declspec(noinline) float Cosine(float x) {
#if defined(_M_IX86)
 float r;
 __asm { fld x
         fcos
         fstp r }
 return r;
#else
 return static_cast<float>(std::cos(static_cast<double>(x)));
#endif
}
}
void* Query(Brain* self,uint32_t key) {
 if(key==0x0754f112 || key==0x0f612d91) return self;
 return CallGameMethod<void*,0x00855800>(self,key);
}
void* QueryPose(void* self,uint32_t key) {
 // Verified secondary-interface twin pattern, with this adjustment 0x18 rather than 4.
 return Query(reinterpret_cast<Brain*>(static_cast<unsigned char*>(self)-offsetof(Brain,poseTable)),key);
}
uintptr_t Destroy(Vehicle* self) {
 self->vtable=static_cast<void**>(Address(0x0102770c)); self->secondary=Address(0x01027708);
 CallGameMethod<void,0x00839a10>(self,&self->firstTable);
 CallGameMethod<void,0x00839a10>(self,&self->secondTable);
 CallGameMethod<void,0x00839a10>(self,&self->thirdTable);
 self->thirdTable=Address(0x01026ff8);
 CallGameMethod<void,0x0085a7d0>(&self->secondTable);
 CallGameMethod<void,0x0085a7d0>(&self->firstTable);
 return CallGameMethod<uintptr_t,0x00839750>(self);
}
void* DeleteSecondary(void* secondary,uint32_t flags) {
 auto* self=reinterpret_cast<Vehicle*>(static_cast<unsigned char*>(secondary)-offsetof(Vehicle,secondary));
 Destroy(self);
 if(flags&1) CallGameFunction<void,0x004e3f70>(self);
 return self;
}
uintptr_t Update(Brain* self,float dt) {
 // Explicit scalar sequencing and SSE tail retain the original float rounding points.
 __m128 v87=self->position;
 if(!(self->flags&1)) { CallGameVirtual<void,9>(self); self->flags|=1; }
 void* manager=*static_cast<void**>(Address(0x011d9140));
 auto* oldPath=self->path;
 if(!(self->flags&6)) { CallGameMethod<void,0x00859720>(self,self->startEvent,1); self->flags|=2; }
 auto endTime=[&]() { auto count=(self->path->end-self->path->begin)>>5; return *reinterpret_cast<float*>(oldPath->begin+32*count-44); };
 float remaining=endTime()-self->time;
 uint32_t nearBits=Bits(remaining);
 if(remaining>0.0f && remaining<1.0f && !(self->flags&4)) {
#if defined(_M_IX86)
 reinterpret_cast<routemanager::RouteManagerObject*>(self)->ReleaseHandle();
#else
 routemanager::RouteManagerObject proxy{}; proxy.request14=self->host; proxy.handleC8=self->handle;
 proxy.ReleaseHandle(); self->handle=proxy.handleC8;
#endif
 CallGameMethod<void,0x00859720>(self,self->endEvent,0); self->flags|=4;
 }
 nearBits=(nearBits&0xffffff00u)|((self->time<0.30000001f || remaining<0.5f)?1u:0u);
 if(self->time>endTime() || self->time<0.0f) {
 self->flags&=0xfffffff9; self->time=0;
 if(self->currentIndex==self->stopIndex) return CallGameVirtual<uintptr_t,11>(self);
 self->currentIndex+=self->reverse?-1:1;
 int count=CallGameMethod<int,0x0089f6c0>(manager);
 if(self->currentIndex<0) self->currentIndex=count-1;
 else if(self->currentIndex>=count) self->currentIndex=0;
 int index=self->currentIndex;
 if(*static_cast<void**>(Address(0x011d9140))) self->path=CallGameMethod<Path*,0x0083ce90>(*static_cast<void**>(Address(0x011d9140)),index);
 self->time=0; self->currentIndex=index;
 }
 self->time+=dt;
 auto* begin=self->path->begin;
 int count=static_cast<int>((self->path->end-begin)>>5);
 uint8_t segment=static_cast<uint8_t>(count-4);
 float v15=1.0f;
 if(self->reverse) {
 segment=3;
 uint8_t i=static_cast<uint8_t>(count-3);
 while(i) {
 float hi=*reinterpret_cast<float*>(begin+32*i+24), lo=*reinterpret_cast<float*>(begin+32*i+56);
 if(hi>=self->time) { segment=static_cast<uint8_t>(i+2); v15=(self->time-lo)/(hi-lo); break; }
 --i;
 }
 } else {
 uint8_t i=2; uint32_t limit=static_cast<uint32_t>(count-1);
 while(i<limit) {
 float hi=*reinterpret_cast<float*>(begin+32*i+20),lo=*reinterpret_cast<float*>(begin+32*i-12);
 if(hi>=self->time) { segment=static_cast<uint8_t>(i-2); v15=(self->time-lo)/(hi-lo); break; }
 ++i;
 }
 }
 int v96=32*int(segment),v31=32*(int(segment)+(self->reverse?-1:1));
 int v30=32*(int(segment)+(self->reverse?-2:2)),v94=32*(int(segment)+(self->reverse?-3:3));
 float v85=v15,v93=(v15*v15)*v15;
 __m128 v92,v99,v100;
 auto* v32=CallGameFunction<__m128*,0x00969040>(&v92,v85,v15*v15,v93,begin+v96,begin+v31,begin+v30,begin+v94);
 __m128 v35,v36,v37,v38,v39,v40,v41,v42,v43,v45,v46,v47,v48,v49,v50,v51,v52,v53,v54,v55,v56,v57,v58,v59,v60,v61,v62,v63,v64,v65,v66,v67,v68,v69,v70,v71,v72,v73,v74,v75,v76,v77,v78,v79,v80,v81,v82,v83,v88,v89,v90,v91;
 __m128* v44;
 int v34;
  auto* samples = self->path->begin;
  v34 = v94;
  v35 = VectorBits(Bits(v93));
  v36 = VectorBits(Bits(v15*v15));
  self->position = *v32;
  v82.m128_u64[1] = *(unsigned int *)(v34 + samples + 16);
  v82.m128_u64[0] = 0;
  v37 = v82;
  v82.m128_u64[1] = *(unsigned int *)(v30 + samples + 16);
  v82.m128_u64[0] = 0;
  v38 = v82;
  v82.m128_u64[1] = *(unsigned int *)(v31 + samples + 16);
  v88.m128_u64[1] = *(unsigned int *)(v96 + samples + 16);
  v39 = v35;
  v39.m128_f32[0] = (float)(v35.m128_f32[0] - (float)(v15 * v15)) + (float)(v85 * 0.0f);
  v82.m128_u64[0] = 0;
  v99 = _mm_mul_ps(_mm_shuffle_ps(v39, v39, 0), v37);
  v40 = v35;
  v41 = v36;
  v88.m128_u64[0] = 0;
  v41.m128_f32[0] = (float)((float)((float)(v15 * v15) * 4.0f) - (float)(v35.m128_f32[0] * 3.0f)) + v85;
  v40.m128_f32[0] = (float)((float)((float)(v35.m128_f32[0] * 3.0f) - (float)((float)(v15 * v15) * 5.0f))
                          + (float)(v85 * 0.0f))
                  + 2.0f;
  v36.m128_f32[0] = (float)((float)((float)(v15 * v15) * 2.0f) - v35.m128_f32[0]) - v85;
  v42 = _mm_add_ps(
          _mm_add_ps(
            _mm_add_ps(_mm_mul_ps(_mm_shuffle_ps(v36, v36, 0), v88), _mm_mul_ps(_mm_shuffle_ps(v40, v40, 0), v82)),
            _mm_mul_ps(_mm_shuffle_ps(v41, v41, 0), v38)),
          v99);
  v43 = _mm_shuffle_ps(VectorBits(0x3F000000u), VectorBits(0x3F000000u), 0);
  v44 = &self->tangent;
  self->tangent = _mm_mul_ps(v43, v42);
  if ( self->reverse == 1 )
    *v44 = _mm_mul_ps(*v44, _mm_shuffle_ps(VectorBits(0xBF800000u), VectorBits(0xBF800000u), 0));
  v100 = self->position;
  v89 = _mm_sub_ps(v100, v87);
  v45 = _mm_mul_ps(v89, v89);
  v46 = _mm_shuffle_ps(v45, v45, 1);
  v47 = _mm_add_ps(_mm_add_ps(_mm_shuffle_ps(v45, v45, 2), v46), v45);
  v46.m128_f32[0] = 0.0f;
  v48 = _mm_shuffle_ps(v46, v46, 0);
  v99.m128_i32[0] = v48.m128_i32[0];
  v92 = v48;
  if ( _mm_shuffle_ps(v47, v47, 0).m128_f32[0] > v48.m128_f32[0] )
  {
    v49 = _mm_shuffle_ps(v45, v45, 1);
    v50 = _mm_add_ps(_mm_add_ps(_mm_shuffle_ps(v45, v45, 2), v49), v45);
    v51 = _mm_shuffle_ps(v50, v50, 0);
    v52 = _mm_rsqrt_ps(v51);
    v49.m128_f32[0] = 1.0f;
    v53 = _mm_shuffle_ps(v49, v49, 0);
    v54 = _mm_add_ps(_mm_mul_ps(_mm_mul_ps(v52, v43), _mm_sub_ps(v53, _mm_mul_ps(_mm_mul_ps(v52, v52), v51))), v52);
    v55 = _mm_mul_ps(
            _mm_add_ps(_mm_mul_ps(_mm_mul_ps(v54, v43), _mm_sub_ps(v53, _mm_mul_ps(_mm_mul_ps(v54, v54), v51))), v54),
            v89);
    v90.m128_u64[1] = 0;
    v90.m128_u64[0] = 0x3F80000000000000LL;
    v56 = _mm_shuffle_ps(v55, v55, 9);
    v57 = _mm_shuffle_ps(v55, v55, 18);
    v99 = _mm_sub_ps(_mm_mul_ps(v57, _mm_shuffle_ps(v90, v90, 9)), _mm_mul_ps(v56, _mm_shuffle_ps(v90, v90, 18)));
    v58 = _mm_mul_ps(v99, v99);
    v59 = _mm_add_ps(_mm_add_ps(_mm_shuffle_ps(v58, v58, 2), _mm_shuffle_ps(v58, v58, 1)), v58);
    v60 = _mm_shuffle_ps(v59, v59, 0);
    v61 = _mm_rsqrt_ps(v60);
    v62 = _mm_add_ps(_mm_mul_ps(_mm_mul_ps(v61, v43), _mm_sub_ps(v53, _mm_mul_ps(_mm_mul_ps(v61, v61), v60))), v61);
    v63 = _mm_mul_ps(
            _mm_add_ps(_mm_mul_ps(_mm_mul_ps(v62, v43), _mm_sub_ps(v53, _mm_mul_ps(_mm_mul_ps(v62, v62), v60))), v62),
            v99);
    v64 = _mm_sub_ps(_mm_mul_ps(_mm_shuffle_ps(v63, v63, 18), v56), _mm_mul_ps(_mm_shuffle_ps(v63, v63, 9), v57));
    v65 = _mm_mul_ps(v64, v64);
    v66 = _mm_add_ps(_mm_add_ps(_mm_shuffle_ps(v65, v65, 2), _mm_shuffle_ps(v65, v65, 1)), v65);
    v67 = _mm_shuffle_ps(v66, v66, 0);
    v68 = _mm_rsqrt_ps(v67);
    v69 = _mm_add_ps(_mm_mul_ps(_mm_mul_ps(v68, v43), _mm_sub_ps(v53, _mm_mul_ps(_mm_mul_ps(v68, v68), v67))), v68);
    v70 = _mm_mul_ps(
            _mm_add_ps(_mm_mul_ps(_mm_mul_ps(v69, v43), _mm_sub_ps(v53, _mm_mul_ps(_mm_mul_ps(v69, v69), v67))), v69),
            v64);
    v83 = _mm_shuffle_ps(v70, v70, 85);
    v71 = _mm_shuffle_ps(v63, v63, 0);
    v91 = _mm_shuffle_ps(v55, v55, 170);
    v72 = _mm_add_ps(v91, _mm_add_ps(v83, v71));
    if ( v72.m128_f32[0] <= v92.m128_f32[0] )
    {
      v92.m128_i32[0] = v71.m128_i32[0];
      if ( v71.m128_f32[0] <= v83.m128_f32[0] || (v92.m128_i32[0] = v71.m128_i32[0], v71.m128_f32[0] <= v91.m128_f32[0]) )
      {
        v92.m128_i32[0] = v83.m128_i32[0];
        if ( v83.m128_f32[0] <= v91.m128_f32[0] )
        {
          v80 = _mm_sqrt_ps(_mm_add_ps(_mm_sub_ps(v91, _mm_add_ps(v83, v71)), v53));
          v81 = _mm_div_ps(v43, v80);
          v75 = _mm_shuffle_ps(
                  _mm_shuffle_ps(
                    _mm_mul_ps(_mm_add_ps(_mm_shuffle_ps(v55, v55, 0), _mm_shuffle_ps(v63, v63, 170)), v81),
                    _mm_mul_ps(_mm_add_ps(_mm_shuffle_ps(v70, v70, 170), _mm_shuffle_ps(v55, v55, 85)), v81),
                    16),
                  _mm_shuffle_ps(
                    _mm_mul_ps(v43, v80),
                    _mm_mul_ps(_mm_sub_ps(_mm_shuffle_ps(v63, v63, 85), _mm_shuffle_ps(v70, v70, 0)), v81),
                    50),
                  136);
        }
        else
        {
          v78 = _mm_sqrt_ps(_mm_add_ps(_mm_sub_ps(v83, _mm_add_ps(v71, v91)), v53));
          v79 = _mm_div_ps(v43, v78);
          v75 = _mm_shuffle_ps(
                  _mm_shuffle_ps(
                    _mm_mul_ps(_mm_add_ps(_mm_shuffle_ps(v63, v63, 85), _mm_shuffle_ps(v70, v70, 0)), v79),
                    _mm_mul_ps(v43, v78),
                    16),
                  _mm_shuffle_ps(
                    _mm_mul_ps(_mm_add_ps(_mm_shuffle_ps(v70, v70, 170), _mm_shuffle_ps(v55, v55, 85)), v79),
                    _mm_mul_ps(_mm_sub_ps(_mm_shuffle_ps(v55, v55, 0), _mm_shuffle_ps(v63, v63, 170)), v79),
                    50),
                  136);
        }
      }
      else
      {
        v76 = _mm_sqrt_ps(_mm_add_ps(_mm_sub_ps(v71, _mm_add_ps(v91, v83)), v53));
        v77 = _mm_div_ps(v43, v76);
        v75 = _mm_shuffle_ps(
                _mm_shuffle_ps(
                  _mm_mul_ps(v76, v43),
                  _mm_mul_ps(v77, _mm_add_ps(_mm_shuffle_ps(v63, v63, 85), _mm_shuffle_ps(v70, v70, 0))),
                  16),
                _mm_shuffle_ps(
                  _mm_mul_ps(v77, _mm_add_ps(_mm_shuffle_ps(v55, v55, 0), _mm_shuffle_ps(v63, v63, 170))),
                  _mm_mul_ps(v77, _mm_sub_ps(_mm_shuffle_ps(v70, v70, 170), _mm_shuffle_ps(v55, v55, 85))),
                  50),
                136);
      }
    }
    else
    {
      v73 = _mm_sqrt_ps(_mm_add_ps(v53, v72));
      v74 = _mm_div_ps(v43, v73);
      v75 = _mm_shuffle_ps(
              _mm_shuffle_ps(
                _mm_mul_ps(_mm_sub_ps(_mm_shuffle_ps(v70, v70, 170), _mm_shuffle_ps(v55, v55, 85)), v74),
                _mm_mul_ps(_mm_sub_ps(_mm_shuffle_ps(v55, v55, 0), _mm_shuffle_ps(v63, v63, 170)), v74),
                16),
              _mm_shuffle_ps(
                _mm_mul_ps(_mm_sub_ps(_mm_shuffle_ps(v63, v63, 85), _mm_shuffle_ps(v70, v70, 0)), v74),
                _mm_mul_ps(v73, v43),
                50),
              136);
    }
    v92 = v75;
    CallGameMethod<void,0x00857170>(self, &v100, &v92);
  }

 CallGameVirtual<void,13>(self,&v100,&self->tangent,nearBits);
 void* queried=CallGameVirtual<void*,1>(self->host,0x14cfd2f8u);
 if(queried) return (reinterpret_cast<uintptr_t>(queried)&~uintptr_t(0xffff)) | reinterpret_cast<routemanager::RouteManagerObject*>(queried)->SetWordFloat(static_cast<uint8_t>(self->currentIndex),self->time);
 return 0;
}
uintptr_t Steer(Brain* self,const __m128* a0,const __m128* a1,uint32_t a2) {

  uintptr_t result;
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
  __m128 v35;
  __m128 v36;
  __m128 v37;
  __m128 v38;
  __m128 v39;
  __m128 v40;
  __m128 v41;
  __m128 v42;
  float v43;
  float v44;
  float v45;
  uint32_t v46;
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
  __m128 v69;
  __m128 v70;
  unsigned int v72;
  unsigned int v73;
  __m128 v74;
  unsigned int v75;
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
  int v88;
  float v89;
  __m128 v90;
  __m128 v91;
  __m128 v92;
  float v93;
  float v94;
  __int32 v95;
  __m128 v96;
  __m128 v97[2];
  __m128 v98;

  CallGameVirtual<void,7>(self->host, &v84);
  CallGameVirtual<void,2>(&self->poseTable, &v79);
  v82 = _mm_sub_ps(v79, v84);
  result = reinterpret_cast<uintptr_t>(a0);
  v82.m128_i32[1] = 0;
  v6 = _mm_mul_ps(v82, v82);
  v7 = _mm_add_ps(_mm_add_ps(_mm_shuffle_ps(v6, v6, 2), _mm_shuffle_ps(v6, v6, 1)), v6);
  v8 = _mm_shuffle_ps(v7, v7, 0);
  v9 = _mm_rsqrt_ps(v8);
  v10 = _mm_shuffle_ps(VectorBits(0x3F800000u), VectorBits(0x3F800000u), 0);
  v11 = _mm_shuffle_ps(VectorBits(0x3F000000u), VectorBits(0x3F000000u), 0);
  v12 = _mm_sub_ps(*(__m128 *)a0, v79);
  v13 = _mm_add_ps(_mm_mul_ps(_mm_mul_ps(v9, v11), _mm_sub_ps(v10, _mm_mul_ps(_mm_mul_ps(v9, v9), v8))), v9);
  v85 = _mm_mul_ps(
          _mm_add_ps(_mm_mul_ps(_mm_mul_ps(v13, v11), _mm_sub_ps(v10, _mm_mul_ps(_mm_mul_ps(v13, v13), v8))), v13),
          v82);
  v14 = _mm_mul_ps(v12, v12);
  v15 = _mm_shuffle_ps(v14, v14, 1);
  v16 = _mm_add_ps(_mm_add_ps(_mm_shuffle_ps(v14, v14, 2), v15), v14);
  v15.m128_f32[0] = 0.0f;
  v17 = _mm_shuffle_ps(v15, v15, 0);
  v89 = _mm_shuffle_ps(v16, v16, 0).m128_f32[0];
  v95 = v17.m128_i32[0];
  v96 = v10;
  v87 = v11;
  v81 = v12;
  v91 = v17;
  if ( v89 > v17.m128_f32[0] )
  {
    v18 = _mm_add_ps(_mm_add_ps(_mm_shuffle_ps(v14, v14, 2), _mm_shuffle_ps(v14, v14, 1)), v14);
    v19 = _mm_shuffle_ps(v18, v18, 0);
    v20 = _mm_rsqrt_ps(v19);
    v21 = _mm_add_ps(_mm_mul_ps(_mm_mul_ps(v20, v11), _mm_sub_ps(v10, _mm_mul_ps(_mm_mul_ps(v20, v20), v19))), v20);
    v77 = _mm_mul_ps(
            _mm_add_ps(_mm_mul_ps(_mm_mul_ps(v21, v11), _mm_sub_ps(v10, _mm_mul_ps(_mm_mul_ps(v21, v21), v19))), v21),
            v12);
    v83 = v12;
    v83.m128_i32[1] = 0;
    v22 = _mm_mul_ps(v83, v83);
    v23 = _mm_add_ps(_mm_add_ps(_mm_shuffle_ps(v22, v22, 2), _mm_shuffle_ps(v22, v22, 1)), v22);
    v24 = _mm_shuffle_ps(v23, v23, 0);
    v25 = _mm_rsqrt_ps(v24);
    v26 = _mm_add_ps(_mm_mul_ps(_mm_mul_ps(v25, v11), _mm_sub_ps(v10, _mm_mul_ps(_mm_mul_ps(v25, v25), v24))), v25);
    v75 = Bits(self->speed);
    v27 = _mm_mul_ps(v85, v85);
    v92 = _mm_mul_ps(
            _mm_add_ps(_mm_mul_ps(_mm_mul_ps(v26, v11), _mm_sub_ps(v10, _mm_mul_ps(_mm_mul_ps(v26, v26), v24))), v26),
            v83);
    v28 = _mm_add_ps(_mm_add_ps(_mm_shuffle_ps(v27, v27, 2), _mm_shuffle_ps(v27, v27, 1)), v27);
    v29 = _mm_shuffle_ps(v28, v28, 0);
    v30 = _mm_rsqrt_ps(v29);
    v31 = _mm_mul_ps(
            _mm_add_ps(_mm_mul_ps(_mm_mul_ps(v30, v11), _mm_sub_ps(v10, _mm_mul_ps(_mm_mul_ps(v30, v30), v29))), v30),
            v85);
    v32 = _mm_mul_ps(v92, v92);
    v33 = _mm_add_ps(_mm_add_ps(_mm_shuffle_ps(v32, v32, 2), _mm_shuffle_ps(v32, v32, 1)), v32);
    v34 = _mm_shuffle_ps(v33, v33, 0);
    v35 = _mm_rsqrt_ps(v34);
    v78 = _mm_mul_ps(
            _mm_add_ps(_mm_mul_ps(_mm_mul_ps(v35, v11), _mm_sub_ps(v10, _mm_mul_ps(_mm_mul_ps(v35, v35), v34))), v35),
            v92);
    v36 = _mm_mul_ps(v78, v31);
    v37 = _mm_add_ps(_mm_add_ps(_mm_shuffle_ps(v36, v36, 2), _mm_shuffle_ps(v36, v36, 1)), v36);
    v74 = _mm_min_ps(
            _mm_max_ps(_mm_shuffle_ps(v37, v37, 0), _mm_shuffle_ps(VectorBits(0xBF800000u), VectorBits(0xBF800000u), 0)),
            v10);
    v90 = v31;
    *(float *)&v73 = ArcCos(v74.m128_f32[0]);
    v38 = _mm_mul_ps(
            _mm_sub_ps(
              _mm_mul_ps(_mm_shuffle_ps(v78, v78, 18), _mm_shuffle_ps(v31, v31, 9)),
              _mm_mul_ps(_mm_shuffle_ps(v78, v78, 9), _mm_shuffle_ps(v31, v31, 18))),
            Vector64(0x3F80000000000000ull));
    v39 = _mm_shuffle_ps(VectorBits(v73), VectorBits(v73), 0);
    v40 = _mm_add_ps(_mm_add_ps(_mm_shuffle_ps(v38, v38, 2), _mm_shuffle_ps(v38, v38, 1)), v38);
    v94 = _mm_shuffle_ps(v40, v40, 0).m128_f32[0];
    v88 = 0;
    v41 = _mm_setzero_ps();
    v78 = _mm_setzero_ps();
    if ( v94 >= 0.0f )
    {
      v42 = v39;
    }
    else
    {
      v41.m128_f32[0] = 6.2831855f;
      v42 = _mm_sub_ps(_mm_shuffle_ps(v41, v41, 0), v39);
    }
    v76 = v42;
    v43 = v42.m128_f32[0];
    v86 = v42;
    if ( v42.m128_f32[0] > 3.1415927f )
      v43 = v42.m128_f32[0] - 6.2831855f;
    if ( (AngleFlags() & 1) != 0 )
    {
      v44 = *(float *)&AngleBits();
    }
    else
    {
      v44 = 1.2217305f;
      AngleFlags() |= 1u;
      AngleBits() = 1067213226;
    }
    v45 = Float( (SignedBits((float)(-0.0f - v44) - v43) >> 31) & Bits(v43)
                 | Bits(-0.0f - v44) & ~(SignedBits((float)(-0.0f - v44) - v43) >> 31));
    v46 = (SignedBits(v44 - v45) >> 31) & Bits(v44) | Bits(v45) & ~(SignedBits(v44 - v45) >> 31);
    v47 = _mm_shuffle_ps(VectorBits(0x40000000u), VectorBits(0x40000000u), 0);
    v97[1] = Vector64(0x3F80000000000000ull);
    v76 = _mm_div_ps(_mm_shuffle_ps(VectorBits(v46), VectorBits(v46), 0), v47);
    *(float *)&v72 = Sine(v76.m128_f32[0]);
    v48 = _mm_shuffle_ps(VectorBits(v72), VectorBits(v72), 0);
    *(float *)&v72 = Cosine(v76.m128_f32[0]);
    v49 = _mm_shuffle_ps(
            _mm_shuffle_ps(
              _mm_mul_ps(_mm_shuffle_ps(Vector64(0x3F80000000000000ull), Vector64(0x3F80000000000000ull), 0), v48),
              _mm_mul_ps(_mm_shuffle_ps(Vector64(0x3F80000000000000ull), Vector64(0x3F80000000000000ull), 85), v48),
              16),
            _mm_shuffle_ps(
              _mm_mul_ps(v48, _mm_shuffle_ps(Vector64(0x3F80000000000000ull), Vector64(0x3F80000000000000ull), 170)),
              _mm_shuffle_ps(VectorBits(v72), VectorBits(v72), 0),
              50),
            136);
    v50 = _mm_shuffle_ps(v49, v49, 255);
    v51 = _mm_shuffle_ps(v49, v49, 9);
    v52 = _mm_shuffle_ps(v49, v49, 18);
    v53 = _mm_add_ps(
            _mm_mul_ps(v50, v85),
            _mm_sub_ps(_mm_mul_ps(_mm_shuffle_ps(v85, v85, 18), v51), _mm_mul_ps(_mm_shuffle_ps(v85, v85, 9), v52)));
    v92 = _mm_add_ps(
            _mm_mul_ps(
              _mm_sub_ps(_mm_mul_ps(_mm_shuffle_ps(v53, v53, 18), v51), _mm_mul_ps(_mm_shuffle_ps(v53, v53, 9), v52)),
              v47),
            v85);
    v77 = _mm_shuffle_ps(_mm_shuffle_ps(v92, v77, 16), v92, 40);
    result = (reinterpret_cast<uintptr_t>(&v77)&~uintptr_t(255)) | uint8_t(ts3::math::b12::Direction(&v77, &self->facing));
    v54 = v96;
    v55 = v87;
    v56 = _mm_mul_ps(v77, v77);
    v57 = _mm_add_ps(_mm_add_ps(_mm_shuffle_ps(v56, v56, 2), _mm_shuffle_ps(v56, v56, 1)), v56);
    v58 = _mm_shuffle_ps(v57, v57, 0);
    v59 = _mm_rsqrt_ps(v58);
    v60 = _mm_add_ps(_mm_mul_ps(_mm_mul_ps(v59, v87), _mm_sub_ps(v96, _mm_mul_ps(_mm_mul_ps(v59, v59), v58))), v59);
    v61 = _mm_mul_ps(
            _mm_add_ps(_mm_mul_ps(_mm_mul_ps(v60, v87), _mm_sub_ps(v96, _mm_mul_ps(_mm_mul_ps(v60, v60), v58))), v60),
            v77);
    self->velocity = _mm_mul_ps(_mm_shuffle_ps(VectorBits(v75), VectorBits(v75), 0), v61);
    v77 = v61;
    v81 = _mm_add_ps(v81, v79);
    v62 = _mm_sub_ps(v81, v84);
    v63 = _mm_mul_ps(v62, v62);
    v64 = _mm_add_ps(_mm_add_ps(_mm_shuffle_ps(v63, v63, 2), _mm_shuffle_ps(v63, v63, 1)), v63);
    v93 = _mm_shuffle_ps(v64, v64, 0).m128_f32[0];
    v80.m128_i32[0] = v91.m128_i32[0];
    v97[0] = v62;
    if ( v93 > v91.m128_f32[0] )
    {
      v65 = _mm_sub_ps(v78, v62);
      v66 = _mm_mul_ps(v65, v65);
      v67 = _mm_add_ps(_mm_add_ps(_mm_shuffle_ps(v66, v66, 2), _mm_shuffle_ps(v66, v66, 1)), v66);
      v68 = _mm_shuffle_ps(v67, v67, 0);
      v69 = _mm_rsqrt_ps(v68);
      v70 = _mm_add_ps(_mm_mul_ps(_mm_mul_ps(v69, v55), _mm_sub_ps(v54, _mm_mul_ps(_mm_mul_ps(v69, v69), v68))), v69);
      v80 = _mm_mul_ps(
              _mm_add_ps(_mm_mul_ps(_mm_mul_ps(v70, v55), _mm_sub_ps(v54, _mm_mul_ps(_mm_mul_ps(v70, v70), v68))), v70),
              v65);
      if ( static_cast<uint8_t>(a2) )
        CallGameVirtual<void,10>(self->host, &v74);
      else
        routemanager::QuaternionFromAxisAngle(v97, a1, &v74);

      v98 = _mm_add_ps(
              _mm_mul_ps(
                _mm_shuffle_ps(
                  VectorBits(Bits(self->distance)),
                  VectorBits(Bits(self->distance)),
                  0),
                v80),
              v81);
      return CallGameVirtual<uintptr_t,12>(self->host, &v98, &v74);
    }
  }
  return result;

}
#if defined(_M_IX86)
void* __fastcall HookQuery(Brain* s,void*,uint32_t k) { return Query(s,k); }
void* __fastcall HookQueryPose(void* s,void*,uint32_t k) { return QueryPose(s,k); }
uintptr_t __fastcall HookUpdate(Brain* s,void*,float d) { return Update(s,d); }
uintptr_t __fastcall HookSteer(Brain* s,void*,const __m128* a,const __m128* b,uint32_t f) { return Steer(s,a,b,f); }
uintptr_t __fastcall HookDestroy(Vehicle* s,void*) { return Destroy(s); }
void* __fastcall HookDeleteSecondary(void* s,void*,uint32_t f) { return DeleteSecondary(s,f); }
#endif
}
