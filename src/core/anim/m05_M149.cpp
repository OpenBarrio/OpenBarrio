#include "ts3/anim/m05_M149.h"
#include <cmath>
#include <intrin.h>
#include "ts3/debug/clu_6c8680_Debug.h"
#include <cstring>
namespace ts3::anim::m149 {
namespace {
void* GamePointer(uintptr_t address) {
#if defined(_M_IX86)
    return reinterpret_cast<void*>(address);
#else
    (void)address; return nullptr;
#endif
}
}
void InitBase(void* self) {
    *static_cast<void**>(self) = GamePointer(0x01026ff8);
}
Event* InitEvent(Event* self) {
    self->vtable = GamePointer(0x0102780c);
    self->value = _mm_setzero_ps();
    return self;
}
Vehicle* Delete(Vehicle* self, uint32_t flags) {
    CallGameMethod<void, 0x00859890>(self);
    if (flags & 1)
        CallGameFunction<void, 0x004e3f70>(self);
    return self;
}
Vehicle* Construct(Vehicle* self, void* host, uint32_t argument) {
    CallGameMethod<void, 0x008397e0>(self, host);
    self->interface18 = GamePointer(0x01027770);
    self->vtable = GamePointer(0x010277d8);
    self->interface4 = GamePointer(0x010277d4);
    self->interface18 = GamePointer(0x010277c4);
    self->velocity = _mm_setzero_ps();
    self->rotation = _mm_set_ps(1,0,0,0);
    InitEvent(&self->cruise);
    CallGameMethod<void, 0x0085a900>(&self->decelerate);
    self->cruiseName = {GamePointer(0x011c7e54),GamePointer(0x011c7e54),GamePointer(0x011c7e56),static_cast<const char*>(GamePointer(0x010277a4))};
    self->decelerateName = {GamePointer(0x011c7e54),GamePointer(0x011c7e54),GamePointer(0x011c7e56),static_cast<const char*>(GamePointer(0x01027780))};
    self->maxSpeed = 18.0f;
    self->speed = 0;
    self->acceleration = 5;
    self->valueB4 = 4;
    self->argument = argument;
    self->distance = 2;
    self->valueBC = 2;
    self->valueC0 = 1;
    self->mode = 2;
    self->valueC8 = 0;
    self->flags = 0;
    CallGameMethod<void, 0x008399e0>(self, &self->cruise);
    CallGameMethod<void, 0x008399e0>(self, &self->decelerate);
    return self;
}
namespace {
uint32_t Bits(float f) { uint32_t b; std::memcpy(&b,&f,4); return b; }
int32_t SignedBits(float f) { return static_cast<int32_t>(Bits(f)); }
float FloatBits(uint32_t b) { float f; std::memcpy(&f,&b,4); return f; }
__m128 Low(uint32_t b) { return _mm_castsi128_ps(_mm_cvtsi32_si128(b)); }
__m128 YAxis() { return _mm_set_ps(0,0,1,0); }
uint32_t& AngleFlags() {
#if defined(_M_IX86)
 return *reinterpret_cast<uint32_t*>(0x011d93e8);
#else
 static uint32_t v=0; return v;
#endif
}
uint32_t& AngleBits() {
#if defined(_M_IX86)
 return *reinterpret_cast<uint32_t*>(0x011d93e4);
#else
 static uint32_t v=0; return v;
#endif
}
}
uintptr_t Aim(Vehicle* self, const __m128* target)
{
  uintptr_t result; // eax
  __m128 v4; // xmm1
  __m128 v5; // xmm0
  __m128 v6; // xmm4
  __m128 v7; // xmm2
  __m128 v8; // xmm0
  __m128 v9; // xmm3
  __m128 v10; // xmm1
  __m128 v11; // xmm6
  __m128 v12; // xmm5
  __m128 v13; // xmm2
  __m128 v14; // xmm1
  __m128 v15; // xmm1
  __m128 v16; // xmm1
  __m128 v17; // xmm1
  __m128 v18; // xmm4
  __m128 v19; // xmm2
  __m128 v20; // xmm2
  __m128 v21; // xmm1
  __m128 v22; // xmm1
  __m128 v23; // xmm4
  __m128 v24; // xmm2
  __m128 v25; // xmm1
  __m128 v26; // xmm4
  __m128 v27; // xmm2
  __m128 v28; // xmm2
  __m128 v29; // xmm4
  __m128 v30; // xmm5
  __m128 v31; // xmm4
  __m128 v32; // xmm2
  __m128 v33; // xmm2
  __m128 v34; // xmm4
  __m128 v35; // xmm2
  __m128 v36; // xmm1
  __m128 v37; // xmm2
  __m128 v38; // xmm0
  __m128 v39; // xmm1
  __m128 v40; // xmm0
  __m128 v41; // xmm2
  __m128 v42; // xmm0
  float v43; // xmm2_4
  float v44; // xmm1_4
  float v45; // xmm0_4
  __m128 v46; // xmm3
  __m128 v47; // xmm1
  __m128 v48; // xmm1
  __m128 v49; // xmm4
  __m128 v50; // xmm5
  __m128 v51; // xmm0
  __m128 v52; // xmm4
  __m128 v53; // xmm1
  __m128 v54; // xmm0
  __m128 v55; // xmm0
  __m128 v56; // xmm2
  __m128 v57; // xmm5
  __m128 v58; // xmm1
  __m128 v59; // xmm0
  __m128 v60; // xmm2
  __m128 v61; // xmm1
  __m128 v62; // xmm0
  __m128 v63; // xmm1
  __m128 v64; // xmm0
  __m128 v65; // xmm0
  __m128 v66; // xmm1
  void* v67; // ecx
  int v68; // [esp+8h] [ebp-1C8h]
  unsigned int v69; // [esp+8h] [ebp-1C8h]
  unsigned int v70; // [esp+8h] [ebp-1C8h]
  unsigned int v71; // [esp+Ch] [ebp-1C4h]
  float v72; // [esp+20h] [ebp-1B0h]
  float f_AC; // [esp+3Ch] [ebp-194h]
  __m128 v74; // [esp+40h] [ebp-190h] BYREF
  __m128 v75; // [esp+50h] [ebp-180h]
  __m128 v76; // [esp+60h] [ebp-170h]
  __m128 v77; // [esp+70h] [ebp-160h]
  __m128 v78; // [esp+80h] [ebp-150h]
  __m128 v79; // [esp+90h] [ebp-140h]
  __m128 v80; // [esp+A0h] [ebp-130h]
  __m128 v81; // [esp+B0h] [ebp-120h]
  __m128 v82; // [esp+C0h] [ebp-110h] BYREF
  __m128 v83; // [esp+D0h] [ebp-100h]
  __m128 v84; // [esp+E0h] [ebp-F0h] BYREF
  __m128 v85; // [esp+F0h] [ebp-E0h]
  __m128 v86; // [esp+100h] [ebp-D0h]
  __m128 v87; // [esp+110h] [ebp-C0h]
  __m128 v88; // [esp+120h] [ebp-B0h] BYREF
  __m128 v89; // [esp+130h] [ebp-A0h]
  unsigned char v90[16]; // [esp+140h] [ebp-90h] BYREF
  float v91; // [esp+150h] [ebp-80h]
  __m128 v92; // [esp+160h] [ebp-70h] BYREF
  __m128 v93; // [esp+170h] [ebp-60h]
  float v94; // [esp+180h] [ebp-50h]
  int32_t v95; // [esp+190h] [ebp-40h]
  __m128 v96; // [esp+1A0h] [ebp-30h]
  __m128 v97; // [esp+1B0h] [ebp-20h]
  __m128 v98; // [esp+1C0h] [ebp-10h]

  CallGameVirtual<void, 7>(self->host, &v84);
  CallGameVirtual<void, 2>(&self->interface18, &v82);
  result = reinterpret_cast<uintptr_t>(target);
  v83 = _mm_sub_ps(v82, v84);
  v83.m128_i32[1] = 0;
  v4 = _mm_mul_ps(v83, v83);
  v5 = _mm_add_ps(_mm_add_ps(_mm_shuffle_ps(v4, v4, 2), _mm_shuffle_ps(v4, v4, 1)), v4);
  v6 = _mm_shuffle_ps(v5, v5, 0);
  v7 = _mm_rsqrt_ps(v6);
  v8 = _mm_shuffle_ps(Low(0x3F800000u), Low(0x3F800000u), 0);
  v9 = _mm_shuffle_ps(Low(0x3F000000u), Low(0x3F000000u), 0);
  v10 = _mm_add_ps(_mm_mul_ps(_mm_sub_ps(v8, _mm_mul_ps(_mm_mul_ps(v7, v7), v6)), _mm_mul_ps(v9, v7)), v7);
  v11 = _mm_mul_ps(
          _mm_add_ps(_mm_mul_ps(_mm_sub_ps(v8, _mm_mul_ps(_mm_mul_ps(v10, v10), v6)), _mm_mul_ps(v9, v10)), v10),
          v83);
  v12 = _mm_sub_ps(*target, v82);
  v13 = _mm_mul_ps(v12, v12);
  v14 = _mm_add_ps(_mm_add_ps(_mm_shuffle_ps(v13, v13, 2), _mm_shuffle_ps(v13, v13, 1)), v13);
  v94 = _mm_shuffle_ps(v14, v14, 0).m128_f32[0];
  v15 = _mm_shuffle_ps(Low(0), Low(0), 0);
  v95 = v15.m128_i32[0];
  v79 = v15;
  v81 = v8;
  v80 = v9;
  v93 = v11;
  v77 = v12;
  if ( v94 > v15.m128_f32[0] )
  {
    v16 = _mm_add_ps(_mm_add_ps(_mm_shuffle_ps(v13, v13, 2), _mm_shuffle_ps(v13, v13, 1)), v13);
    v17 = _mm_shuffle_ps(v16, v16, 0);
    v18 = _mm_rsqrt_ps(v17);
    v19 = _mm_add_ps(_mm_mul_ps(_mm_sub_ps(v8, _mm_mul_ps(_mm_mul_ps(v18, v18), v17)), _mm_mul_ps(v9, v18)), v18);
    v74 = _mm_mul_ps(
            _mm_add_ps(_mm_mul_ps(_mm_sub_ps(v8, _mm_mul_ps(_mm_mul_ps(v19, v19), v17)), _mm_mul_ps(v9, v19)), v19),
            v12);
    v85 = v12;
    v85.m128_i32[1] = 0;
    v20 = _mm_mul_ps(v85, v85);
    v21 = _mm_add_ps(_mm_add_ps(_mm_shuffle_ps(v20, v20, 2), _mm_shuffle_ps(v20, v20, 1)), v20);
    v22 = _mm_shuffle_ps(v21, v21, 0);
    v23 = _mm_rsqrt_ps(v22);
    v24 = _mm_add_ps(_mm_mul_ps(_mm_sub_ps(v8, _mm_mul_ps(_mm_mul_ps(v23, v23), v22)), _mm_mul_ps(v9, v23)), v23);
    f_AC = self->speed;
    v25 = _mm_mul_ps(
            _mm_add_ps(_mm_mul_ps(_mm_sub_ps(v8, _mm_mul_ps(_mm_mul_ps(v24, v24), v22)), _mm_mul_ps(v9, v24)), v24),
            v85);
    v26 = _mm_mul_ps(v11, v11);
    v27 = _mm_add_ps(_mm_add_ps(_mm_shuffle_ps(v26, v26, 2), _mm_shuffle_ps(v26, v26, 1)), v26);
    v28 = _mm_shuffle_ps(v27, v27, 0);
    v29 = _mm_rsqrt_ps(v28);
    v89 = v25;
    v30 = _mm_mul_ps(
            _mm_add_ps(_mm_mul_ps(_mm_sub_ps(v8, _mm_mul_ps(_mm_mul_ps(v29, v29), v28)), _mm_mul_ps(v9, v29)), v29),
            v11);
    v31 = _mm_mul_ps(v25, v25);
    v32 = _mm_add_ps(_mm_add_ps(_mm_shuffle_ps(v31, v31, 2), _mm_shuffle_ps(v31, v31, 1)), v31);
    v33 = _mm_shuffle_ps(v32, v32, 0);
    v34 = _mm_rsqrt_ps(v33);
    v35 = _mm_mul_ps(
            _mm_add_ps(_mm_mul_ps(_mm_sub_ps(v8, _mm_mul_ps(_mm_mul_ps(v34, v34), v33)), _mm_mul_ps(v9, v34)), v34),
            v25);
    v36 = _mm_mul_ps(v35, v30);
    v76 = v35;
    v37 = _mm_add_ps(_mm_add_ps(_mm_shuffle_ps(v36, v36, 2), _mm_shuffle_ps(v36, v36, 1)), v36);
    *(float *)&v71 = static_cast<float>(std::acos(static_cast<double>(_mm_cvtss_f32(_mm_min_ps(_mm_max_ps(_mm_shuffle_ps(v37,v37,0), _mm_set1_ps(-1)), v8)))));
    v38 = _mm_mul_ps(
            _mm_sub_ps(
              _mm_mul_ps(_mm_shuffle_ps(v76, v76, 18), _mm_shuffle_ps(v30, v30, 9)),
              _mm_mul_ps(_mm_shuffle_ps(v76, v76, 9), _mm_shuffle_ps(v30, v30, 18))),
            YAxis());
    v39 = _mm_add_ps(_mm_add_ps(_mm_shuffle_ps(v38, v38, 2), _mm_shuffle_ps(v38, v38, 1)), v38);
    v91 = _mm_shuffle_ps(v39, v39, 0).m128_f32[0];
    v76 = _mm_setzero_ps();
    v40 = _mm_setzero_ps();
    v41 = _mm_shuffle_ps(Low(v71), Low(v71), 0);
    // COMISS 0,cross; JBE takes the positive-angle arm for unordered too.
    if ( !(v91 < 0.0f) )
    {
      v42 = v41;
    }
    else
    {
      v40.m128_f32[0] = 6.2831855f;
      v42 = _mm_sub_ps(_mm_shuffle_ps(v40, v40, 0), v41);
    }
    v43 = v42.m128_f32[0];
    v86 = v42;
    if ( v42.m128_f32[0] > 3.1415927f )
      v43 = v42.m128_f32[0] - 6.2831855f;
    if ( (AngleFlags() & 1) != 0 )
    {
      v44 = FloatBits(AngleBits());
    }
    else
    {
      v44 = 1.2217305f;
      AngleFlags() |= 1u;
      AngleBits() = 1067213226;
    }
    *reinterpret_cast<uint32_t*>(&v45) = (SignedBits((float)(-0.0f - v44) - v43) >> 31) & Bits(v43)
                 | Bits(-0.0f - v44) & ~(SignedBits((float)(-0.0f - v44) - v43) >> 31);
    v87 = YAxis();
    v68 = (SignedBits(v44 - v45) >> 31) & Bits(v44) | Bits(v45) & ~(SignedBits(v44 - v45) >> 31);
    v46 = _mm_shuffle_ps(Low(0x40000000u), Low(0x40000000u), 0);
    v72 = _mm_shuffle_ps(Low(v68), Low(v68), 0).m128_f32[0] / v46.m128_f32[0];
    *(float *)&v69 = static_cast<float>(std::sin(static_cast<double>(v72)));
    v47 = _mm_shuffle_ps(Low(v69), Low(v69), 0);
    v98 = _mm_mul_ps(_mm_shuffle_ps(v87, v87, 0), v47);
    v96 = _mm_mul_ps(_mm_shuffle_ps(v87, v87, 170), v47);
    v97 = _mm_mul_ps(_mm_shuffle_ps(v87, v87, 85), v47);
    *(float *)&v70 = static_cast<float>(std::cos(static_cast<double>(v72)));
    v48 = _mm_shuffle_ps(
            _mm_shuffle_ps(v98, v97, 16),
            _mm_shuffle_ps(v96, _mm_shuffle_ps(Low(v70), Low(v70), 0), 50),
            136);
    v49 = _mm_shuffle_ps(v48, v48, 9);
    v50 = _mm_shuffle_ps(v48, v48, 18);
    v51 = _mm_add_ps(
            _mm_sub_ps(_mm_mul_ps(_mm_shuffle_ps(v93, v93, 18), v49), _mm_mul_ps(_mm_shuffle_ps(v93, v93, 9), v50)),
            _mm_mul_ps(_mm_shuffle_ps(v48, v48, 255), v93));
    v89 = _mm_add_ps(
            _mm_mul_ps(
              _mm_sub_ps(_mm_mul_ps(_mm_shuffle_ps(v51, v51, 18), v49), _mm_mul_ps(_mm_shuffle_ps(v51, v51, 9), v50)),
              v46),
            v93);
    v74 = _mm_shuffle_ps(_mm_shuffle_ps(v89, v74, 16), v89, 40);
    result = CallGameFunction<uintptr_t, 0x00860460>(&v74, &self->rotation);
    v52 = v81;
    v53 = _mm_mul_ps(v74, v74);
    v54 = _mm_add_ps(_mm_add_ps(_mm_shuffle_ps(v53, v53, 2), _mm_shuffle_ps(v53, v53, 1)), v53);
    v55 = _mm_shuffle_ps(v54, v54, 0);
    v56 = _mm_rsqrt_ps(v55);
    v57 = v80;
    v58 = _mm_add_ps(_mm_mul_ps(_mm_sub_ps(v81, _mm_mul_ps(_mm_mul_ps(v56, v56), v55)), _mm_mul_ps(v80, v56)), v56);
    v74 = _mm_mul_ps(
            _mm_add_ps(_mm_mul_ps(_mm_sub_ps(v81, _mm_mul_ps(_mm_mul_ps(v58, v58), v55)), _mm_mul_ps(v58, v80)), v58),
            v74);
    v59 = v77;
    self->velocity = _mm_mul_ps(
                                     _mm_shuffle_ps(Low(Bits(f_AC)), Low(Bits(f_AC)), 0),
                                     v74);
    v77 = _mm_add_ps(v59, v82);
    v60 = _mm_sub_ps(v77, v84);
    v61 = _mm_mul_ps(v60, v60);
    v62 = _mm_add_ps(_mm_add_ps(_mm_shuffle_ps(v61, v61, 2), _mm_shuffle_ps(v61, v61, 1)), v61);
    v78.m128_f32[0] = _mm_shuffle_ps(v62, v62, 0).m128_f32[0];
    v75.m128_i32[0] = v79.m128_i32[0];
    v88 = v60;
    if ( v78.m128_f32[0] > v79.m128_f32[0] )
    {
      v79 = _mm_sub_ps(v76, v60);
      v63 = _mm_mul_ps(v79, v79);
      v64 = _mm_add_ps(_mm_add_ps(_mm_shuffle_ps(v63, v63, 2), _mm_shuffle_ps(v63, v63, 1)), v63);
      v65 = _mm_shuffle_ps(v64, v64, 0);
      v66 = _mm_rsqrt_ps(v65);
      v78 = v65;
      v75 = _mm_add_ps(_mm_mul_ps(_mm_sub_ps(v52, _mm_mul_ps(_mm_mul_ps(v66, v66), v65)), _mm_mul_ps(v66, v57)), v66);
      CallGameFunction<void, 0x00860460>(&v88, v90);
      v67 = self->host;
      v92 = _mm_add_ps(
              _mm_mul_ps(
                _mm_mul_ps(
                  _mm_add_ps(
                    _mm_mul_ps(_mm_sub_ps(v81, _mm_mul_ps(_mm_mul_ps(v75, v75), v78)), _mm_mul_ps(v75, v80)),
                    v75),
                  v79),
                _mm_shuffle_ps(Low(Bits(self->distance)), Low(Bits(self->distance)), 0)),
              v77);
      return CallGameVirtual<uintptr_t, 12>(v67, &v92, v90);
    }
  }
  return result;
}


namespace {
struct RouteRef { void* vtable; volatile long refs; };
uintptr_t Release(RouteRef* r) {
    auto result = reinterpret_cast<uintptr_t>(&r->refs);
    if (_InterlockedDecrement(&r->refs) == 0) {
        _InterlockedExchange(&r->refs, 1);
        return CallGameVirtual<uintptr_t, 0>(r, 1);
    }
    return result;
}
float SignMin(float a, float b) {
    uint32_t mask = static_cast<uint32_t>(SignedBits(a-b) >> 31);
    return FloatBits((Bits(a)&mask) | (Bits(b)&~mask));
}
float Positive(float a) {
    uint32_t mask = static_cast<uint32_t>(SignedBits(-0.0f-a) >> 31);
    return FloatBits(Bits(a)&mask);
}
__m128 Sum3(__m128 a) {
    return _mm_shuffle_ps(_mm_add_ps(_mm_add_ps(_mm_shuffle_ps(a,a,2),_mm_shuffle_ps(a,a,1)),a),
                          _mm_add_ps(_mm_add_ps(_mm_shuffle_ps(a,a,2),_mm_shuffle_ps(a,a,1)),a),0);
}
}
uintptr_t Update(Vehicle* self, float elapsed) {
    auto* route = CallGameMethod<RouteRef*, 0x00c93970>(self->host);
    if (route) {
        _InterlockedIncrement(&route->refs);
        if (!(self->flags & 1)) {
            CallGameVirtual<void, 9>(self);
            __m128 pos;
            CallGameVirtual<void, 2>(&self->interface18, &pos);
            auto* second = CallGameMethod<RouteRef*, 0x00c93970>(self->host);
            if (second) {
                _InterlockedIncrement(&second->refs);
                float nearest = static_cast<float>(CallGameVirtual<double, 70>(second, &pos));
                uint64_t id = CallGameVirtual<uint64_t, 4>(self->host);
                if (nearest > CallGameVirtual<double, 41>(second, uint32_t(id), uint32_t(id>>32))) {
                    id = CallGameVirtual<uint64_t, 4>(self->host);
                    CallGameVirtual<void, 42>(second, nearest, uint32_t(id), uint32_t(id>>32));
                }
                Release(second);
            }
            self->flags |= 1;
        }
        if (!(self->flags & 8) && uint8_t(CallGameMethod<uintptr_t, 0x0085a8f0>(&self->decelerate)) &&
            (reinterpret_cast<unsigned char*>(&self->eventState)[1] != 0)) {
            self->flags |= 8;
            CallGameVirtual<void, 11>(self);
            return CallGameMethod<uintptr_t, 0x007460f0>(&route);
        }
        uint64_t id = CallGameVirtual<uint64_t, 4>(self->host);
        float current = static_cast<float>(CallGameVirtual<double, 41>(route, uint32_t(id), uint32_t(id>>32)));
        double length = CallGameVirtual<double, 75>(route);
        __m128 pos;
        // FCOMPI 0,difference; unordered follows the cruise arm.
        if (!(length - current <= 0.0)) {
            if (!(self->flags & 2)) {
                CallGameMethod<void, 0x00859720>(self, &self->cruiseName, 1);
                self->flags |= 2;
            }
            self->speed = SignMin(self->maxSpeed, Positive(self->acceleration * elapsed + self->speed));
            float step = self->speed * elapsed;
            float next = SignMin(step + current, static_cast<float>(length));
            id = CallGameVirtual<uint64_t, 4>(self->host);
            CallGameVirtual<void, 42>(route, next, uint32_t(id), uint32_t(id>>32));
            CallGameVirtual<void, 71>(route, &pos, next, 0, 0);
            if (CallGameVirtual<int, 6>(self->host) == 4) {
                auto* component = CallGameVirtual<void*, 1>(self->host, 0x14cfd2f8);
                if (component) {
                    auto* brain = CallGameMethod<void*, 0x00955ef0>(component);
                    auto* state = CallGameVirtual<void*, 3>(route);
                    bool active = uint8_t(CallGameMethod<uintptr_t, 0x0081c880>(state, 0, 0x200000, 0)) != 0;
                    if (brain && !uint8_t(CallGameMethod<uintptr_t, 0x0085af40>(brain)) &&
                        *reinterpret_cast<uint32_t*>(static_cast<unsigned char*>(brain)+0xa8) >= 2) {
                        id = CallGameVirtual<uint64_t, 4>(self->host);
                        if (CallGameMethod<double, 0x0055d9c0>(brain, uint32_t(id), uint32_t(id>>32)) > 0.0 && !active) {
                            alignas(16) __m128 temp;
                            pos = *CallGameMethod<__m128*, 0x0085d890>(brain, &temp, step);
                        }
                    }
                }
            }
            CallGameVirtual<void, 12>(self, &pos);
        } else {
            if (!(self->flags & 4)) {
                CallGameMethod<void, 0x00859720>(self, &self->decelerateName, 0);
                self->flags |= 4;
            }
            id = CallGameVirtual<uint64_t, 4>(self->host);
            CallGameVirtual<void, 42>(route, static_cast<float>(length), uint32_t(id), uint32_t(id>>32));
            __m128 end;
            CallGameVirtual<void, 77>(route, &end);
            CallGameVirtual<void, 7>(self->host, &pos);
            __m128 delta = _mm_sub_ps(end,pos);
            if (_mm_cvtss_f32(_mm_sqrt_ps(Sum3(_mm_mul_ps(delta,delta)))) <= 0.001f) {
                CallGameVirtual<void, 11>(self);
                reinterpret_cast<ts3::debug::DebugShapeDrawer*>(self)->DrawShapes();
                return Release(route);
            }
            pos = _mm_add_ps(_mm_mul_ps(_mm_set1_ps(0.5f),delta),pos);
            CallGameVirtual<void, 8>(self->host, &pos);
            self->speed *= 0.5f;
        }
        CallGameVirtual<void, 10>(self);
        CallGameVirtual<void, 19>(self->host);
    } else {
        CallGameVirtual<void, 11>(self);
    }
    uintptr_t result = reinterpret_cast<uintptr_t>(reinterpret_cast<ts3::debug::DebugShapeDrawer*>(self)->DrawShapes());
    return route ? Release(route) : result;
}

}
#if defined(_M_IX86)
namespace ts3::anim::m149::hooks {
uintptr_t __fastcall Aim(Vehicle* s,void*,const __m128* p) {return m149::Aim(s,p);}
Vehicle* __fastcall Construct(Vehicle* s,void*,void* p,uint32_t a) {return m149::Construct(s,p,a);}
Vehicle* __fastcall Delete(Vehicle* s,void*,uint32_t a) {return m149::Delete(s,a);}
uintptr_t __fastcall Update(Vehicle* s,void*,float a) {return m149::Update(s,a);}
void __fastcall InitBase(void* s,void*) {m149::InitBase(s);}
Event* __fastcall InitEvent(Event* s,void*) {return m149::InitEvent(s);}
}
#endif
