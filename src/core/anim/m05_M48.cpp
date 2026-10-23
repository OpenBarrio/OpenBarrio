#include "ts3/anim/m05_M48.h"
#include "ts3/anim/m05_B09.h"
#include <cstring>
#include <cmath>
#include <xmmintrin.h>
#if defined(_M_IX86)
#include <intrin.h>
#endif
#pragma float_control(precise,on,push)
namespace ts3::anim::m48 {
namespace {
uint32_t Bits(float x) { uint32_t u; std::memcpy(&u,&x,4); return u; }
float Float(uint32_t u) { float x; std::memcpy(&x,&u,4); return x; }
template<unsigned A> void* Table() {
#if defined(_M_IX86)
    return reinterpret_cast<void*>(A);
#else
    NotRewritten("M48 game vtable");
#endif
}
void Setup(ScaleController* s) {
    s->base.vtable=Table<0xfc20c0>(); s->base.secondary8=Table<0xfc20b8>();
    s->base.secondaryC=Table<0xfc20b4>(); s->delegate=Table<0xfc1e5c>();
}
void Retain(void* p) {
    if(!p) return;
    auto* r=reinterpret_cast<int32_t*>(static_cast<unsigned char*>(p)+sizeof(void*));
#if defined(_M_IX86)
    _InterlockedIncrement(reinterpret_cast<volatile long*>(r));
#else
    ++*r;
#endif
}
void Release(void* p) {
    if(!p) return;
    auto* r=reinterpret_cast<int32_t*>(static_cast<unsigned char*>(p)+sizeof(void*));
#if defined(_M_IX86)
    const auto n=_InterlockedDecrement(reinterpret_cast<volatile long*>(r));
#else
    const auto n=--*r;
#endif
    if(!n) { *r=1; CallGameVirtual<void,0>(p,1u); }
}
void* Secondary(void* p) { return static_cast<unsigned char*>(p)+2*sizeof(void*); }
__declspec(noinline) float Acos(float x) {
#if defined(_M_IX86)
    float r; const uintptr_t target=0xf7edde;
    __asm { fld x
        call target
        fstp r }
    return r;
#else
    return static_cast<float>(std::acos(double(x)));
#endif
}
}
__declspec(noinline) float __fastcall Sample(EventController* s,void*,uint32_t lo,uint32_t hi,float start,float end,uint32_t count,float parameter) {
    float sum=0.0f, value=0.0f;
    const float step=count>1?static_cast<float>((double(end)-double(start))/double(count-1)):0.0f;
    for(int32_t i=static_cast<int32_t>(count);i>0;--i) {
        const float r=CallGameMethod<float,0x0056e8e0>(s,lo,hi,start,parameter);
        // Original uses sign bits of rounded differences, not ordered min/max.
        value=(Bits(static_cast<float>(-2.5-double(r)))>>31)?r:-2.5f;
        value=(Bits(2.5f-value)>>31)?2.5f:value;
        sum=sum+value; start=start+step;
    }
#if defined(_M_IX86)
    // No float spill: EA returns the division in ST0, and Evaluate multiplies it
    // before rounding. Callers which store a float still see the usual float value.
    const double divisor=count>1?double(count):1.0;
    __asm { fld sum
        fdiv divisor }
#else
    return count>1?static_cast<float>(double(sum)/double(count)):sum;
#endif
}
uint8_t __fastcall Save(ScaleController* s,void*,b08::StreamState* out) {
    CallGameMethod<void,0x007d77c0>(out,CallGameFunction<uint32_t,0x004ddbd0>(out->handle,1u,out->extra));
    if(!out->good) return 0;
    b08::SaveOptional(s->child,out);
    if(!out->good) return 0;
    CallGameMethod<void,0x007d77c0>(out,CallGameFunction<uint32_t,0x004ddff0>(out->handle,s->scale,out->extra));
    if(!out->good) return 0;
    CallGameMethod<void,0x007d77c0>(out,CallGameFunction<uint32_t,0x004ddff0>(out->handle,s->duration,out->extra));
    return out->good;
}
ScaleController* __fastcall Construct(ScaleController* s,void*,void* host) {
    b08::Construct(&s->base,host); Setup(s); s->child=nullptr; s->scale=1.0f; s->duration=0.0f; return s;
}
Controller* __fastcall DeleteSecondary(void* s,void*,uint32_t flags) {
    // Same thunk shape as verified 00409770, but adjustment is 0x0c and target is 0056faf0.
    return CallGameMethod<Controller*,0x0056faf0>(static_cast<unsigned char*>(s)-3*sizeof(void*),flags);
}
ScaleController* __fastcall ConstructChild(ScaleController* s,void*,void* host,Controller* child,float scale) {
    b08::Construct(&s->base,host); Setup(s); s->child=child;
    if(child) child->refs=static_cast<int32_t>(static_cast<uint32_t>(child->refs)+1);
    s->scale=scale;
    s->duration=static_cast<float>(double(CallGameVirtual<float,8>(s->child))/double(s->scale));
    b09impl::AddDelegate(reinterpret_cast<b09impl::Delegates*>(&s->child->secondaryC),&s->delegate,~0u);
    return s;
}

namespace {
// Literal SSE operation order from 0056F1D5..0056F55D. W participates in SIMD but not dot sums.
float Direction(__m128 position,__m128 rotation,__m128 goal) {
    alignas(16) float mem[64]{};
    _mm_store_ps(mem+0x30/4,goal); _mm_store_ps(mem+0xa0/4,position); _mm_store_ps(mem+0xb0/4,rotation);
    __m128 xmm0{},xmm1{},xmm2{},xmm3{},xmm4{},xmm5{},xmm6{},xmm7{};
    xmm1 = _mm_load_ps(mem+0xa0/4); // 0x56f1d5
    xmm0 = _mm_load_ps(mem+0x30/4); // 0x56f1dd
    xmm5 = _mm_xor_ps(xmm5,xmm5); // 0x56f1e2
    xmm0 = _mm_sub_ps(xmm0,xmm1); // 0x56f1e5
    xmm1 = _mm_set_ss(Float(0xbf800000u)); // 0x56f1e8
    _mm_store_ps(mem+0x70/4,xmm0); // 0x56f1f0
    xmm1 = _mm_shuffle_ps(xmm1,xmm1,0); // 0x56f1f5
    _mm_store_ss(mem+0x74/4,xmm5); // 0x56f1f9
    xmm6 = _mm_load_ps(mem+0x70/4); // 0x56f1ff
    _mm_store_ps(mem+0x50/4,xmm1); // 0x56f204
    xmm0 = xmm6; // 0x56f209
    xmm0 = _mm_sub_ps(xmm0,_mm_load_ps(mem+0x80/4)); // 0x56f20c
    xmm1 = _mm_mul_ps(xmm1,xmm0); // 0x56f214
    xmm1 = _mm_max_ps(xmm1,xmm0); // 0x56f217
    xmm0 = _mm_set_ss(Float(0x37800000u)); // 0x56f21a
    xmm0 = _mm_shuffle_ps(xmm0,xmm0,0); // 0x56f222
    xmm1 = _mm_cmple_ps(xmm1,xmm0); // 0x56f226
    if((_mm_movemask_ps(xmm1)&7)==7) return 0.0f;
    xmm1 = xmm6; // 0x56f238
    xmm1 = _mm_mul_ps(xmm1,xmm6); // 0x56f23b
    xmm0 = xmm1; // 0x56f23e
    xmm0 = _mm_shuffle_ps(xmm0,xmm1,2); // 0x56f241
    xmm2 = xmm1; // 0x56f245
    xmm2 = _mm_shuffle_ps(xmm2,xmm1,1); // 0x56f248
    xmm0 = _mm_add_ps(xmm0,xmm2); // 0x56f24c
    xmm2 = _mm_set_ss(Float(0x3f000000u)); // 0x56f24f
    xmm0 = _mm_add_ps(xmm0,xmm1); // 0x56f257
    xmm0 = _mm_shuffle_ps(xmm0,xmm0,0); // 0x56f25a
    xmm4 = xmm0; // 0x56f25e
    xmm0 = _mm_set_ss(Float(0x3f800000u)); // 0x56f261
    xmm3 = _mm_rsqrt_ps(xmm4); // 0x56f269
    xmm7 = xmm3; // 0x56f26c
    xmm7 = _mm_mul_ps(xmm7,xmm3); // 0x56f26f
    xmm7 = _mm_mul_ps(xmm7,xmm4); // 0x56f272
    xmm0 = _mm_shuffle_ps(xmm0,xmm0,0); // 0x56f275
    xmm2 = _mm_shuffle_ps(xmm2,xmm2,0); // 0x56f279
    xmm1 = xmm0; // 0x56f27d
    xmm1 = _mm_sub_ps(xmm1,xmm7); // 0x56f280
    xmm7 = xmm2; // 0x56f283
    xmm7 = _mm_mul_ps(xmm7,xmm3); // 0x56f286
    xmm1 = _mm_mul_ps(xmm1,xmm7); // 0x56f289
    xmm1 = _mm_add_ps(xmm1,xmm3); // 0x56f28c
    xmm7 = xmm1; // 0x56f28f
    xmm7 = _mm_mul_ps(xmm7,xmm1); // 0x56f292
    xmm7 = _mm_mul_ps(xmm7,xmm4); // 0x56f295
    _mm_store_ss(mem+0x3c/4,xmm5); // 0x56f298
    _mm_store_ss(mem+0x34/4,xmm5); // 0x56f29e
    _mm_store_ss(mem+0x30/4,xmm5); // 0x56f2a4
    xmm5 = _mm_load_ps(mem+0xb0/4); // 0x56f2aa
    xmm3 = xmm0; // 0x56f2b2
    xmm3 = _mm_sub_ps(xmm3,xmm7); // 0x56f2b5
    xmm7 = xmm5; // 0x56f2b8
    xmm7 = _mm_shuffle_ps(xmm7,xmm5,9); // 0x56f2bb
    xmm4 = xmm2; // 0x56f2bf
    xmm4 = _mm_mul_ps(xmm4,xmm1); // 0x56f2c2
    xmm3 = _mm_mul_ps(xmm3,xmm4); // 0x56f2c5
    xmm3 = _mm_add_ps(xmm3,xmm1); // 0x56f2c8
    xmm1 = _mm_set_ss(Float(0x3f800000u)); // 0x56f2cb
    _mm_store_ss(mem+0x38/4,xmm1); // 0x56f2d3
    xmm4 = _mm_load_ps(mem+0x30/4); // 0x56f2d9
    _mm_store_ps(mem+0x30/4,xmm7); // 0x56f2de
    xmm1 = xmm4; // 0x56f2e3
    xmm1 = _mm_shuffle_ps(xmm1,xmm4,0x12); // 0x56f2e6
    xmm1 = _mm_mul_ps(xmm1,xmm7); // 0x56f2ea
    xmm3 = _mm_mul_ps(xmm3,xmm6); // 0x56f2ed
    xmm6 = xmm5; // 0x56f2f0
    xmm6 = _mm_shuffle_ps(xmm6,xmm5,0x12); // 0x56f2f3
    xmm7 = xmm4; // 0x56f2f7
    xmm7 = _mm_shuffle_ps(xmm7,xmm4,9); // 0x56f2fa
    xmm7 = _mm_mul_ps(xmm7,xmm6); // 0x56f2fe
    xmm1 = _mm_sub_ps(xmm1,xmm7); // 0x56f301
    xmm7 = xmm5; // 0x56f304
    xmm7 = _mm_shuffle_ps(xmm7,xmm5,0xff); // 0x56f307
    xmm7 = _mm_mul_ps(xmm7,xmm4); // 0x56f30b
    xmm1 = _mm_add_ps(xmm1,xmm7); // 0x56f30e
    xmm5 = xmm1; // 0x56f311
    xmm5 = _mm_shuffle_ps(xmm5,xmm1,0x12); // 0x56f314
    xmm5 = _mm_mul_ps(xmm5,_mm_load_ps(mem+0x30/4)); // 0x56f318
    xmm7 = xmm1; // 0x56f31d
    xmm7 = _mm_shuffle_ps(xmm7,xmm1,9); // 0x56f320
    xmm1 = _mm_set_ss(Float(0x40000000u)); // 0x56f324
    xmm1 = _mm_shuffle_ps(xmm1,xmm1,0); // 0x56f32c
    xmm7 = _mm_mul_ps(xmm7,xmm6); // 0x56f330
    xmm5 = _mm_sub_ps(xmm5,xmm7); // 0x56f333
    xmm5 = _mm_mul_ps(xmm5,xmm1); // 0x56f336
    xmm1 = _mm_xor_ps(xmm1,xmm1); // 0x56f339
    xmm5 = _mm_add_ps(xmm5,xmm4); // 0x56f33c
    _mm_store_ps(mem+0x30/4,xmm5); // 0x56f33f
    _mm_store_ss(mem+0x34/4,xmm1); // 0x56f344
    xmm6 = _mm_load_ps(mem+0x30/4); // 0x56f34a
    xmm4 = xmm6; // 0x56f34f
    xmm4 = _mm_mul_ps(xmm4,xmm6); // 0x56f352
    xmm1 = xmm4; // 0x56f355
    xmm1 = _mm_shuffle_ps(xmm1,xmm4,2); // 0x56f358
    xmm5 = xmm4; // 0x56f35c
    xmm5 = _mm_shuffle_ps(xmm5,xmm4,1); // 0x56f35f
    xmm1 = _mm_add_ps(xmm1,xmm5); // 0x56f363
    xmm1 = _mm_add_ps(xmm1,xmm4); // 0x56f366
    xmm1 = _mm_shuffle_ps(xmm1,xmm1,0); // 0x56f369
    xmm5 = _mm_rsqrt_ps(xmm1); // 0x56f36d
    xmm7 = xmm5; // 0x56f370
    xmm7 = _mm_mul_ps(xmm7,xmm5); // 0x56f373
    xmm7 = _mm_mul_ps(xmm7,xmm1); // 0x56f376
    xmm4 = xmm0; // 0x56f379
    xmm4 = _mm_sub_ps(xmm4,xmm7); // 0x56f37c
    xmm7 = xmm5; // 0x56f37f
    xmm7 = _mm_mul_ps(xmm7,xmm2); // 0x56f382
    xmm4 = _mm_mul_ps(xmm4,xmm7); // 0x56f385
    xmm4 = _mm_add_ps(xmm4,xmm5); // 0x56f388
    xmm5 = xmm4; // 0x56f38b
    xmm5 = _mm_mul_ps(xmm5,xmm4); // 0x56f38e
    xmm5 = _mm_mul_ps(xmm5,xmm1); // 0x56f391
    xmm1 = xmm0; // 0x56f394
    xmm1 = _mm_sub_ps(xmm1,xmm5); // 0x56f397
    xmm5 = xmm4; // 0x56f39a
    xmm5 = _mm_mul_ps(xmm5,xmm2); // 0x56f39d
    xmm1 = _mm_mul_ps(xmm1,xmm5); // 0x56f3a0
    xmm1 = _mm_add_ps(xmm1,xmm4); // 0x56f3a3
    xmm1 = _mm_mul_ps(xmm1,xmm6); // 0x56f3a6
    xmm5 = xmm1; // 0x56f3a9
    xmm5 = _mm_mul_ps(xmm5,xmm1); // 0x56f3ac
    xmm4 = xmm5; // 0x56f3af
    xmm4 = _mm_shuffle_ps(xmm4,xmm5,2); // 0x56f3b2
    xmm6 = xmm5; // 0x56f3b6
    xmm6 = _mm_shuffle_ps(xmm6,xmm5,1); // 0x56f3b9
    xmm4 = _mm_add_ps(xmm4,xmm6); // 0x56f3bd
    xmm4 = _mm_add_ps(xmm4,xmm5); // 0x56f3c0
    xmm4 = _mm_shuffle_ps(xmm4,xmm4,0); // 0x56f3c3
    xmm6 = _mm_rsqrt_ps(xmm4); // 0x56f3c7
    xmm7 = xmm6; // 0x56f3ca
    xmm7 = _mm_mul_ps(xmm7,xmm6); // 0x56f3cd
    xmm7 = _mm_mul_ps(xmm7,xmm4); // 0x56f3d0
    xmm4 = xmm6; // 0x56f3d3
    xmm4 = _mm_mul_ps(xmm4,xmm2); // 0x56f3d6
    xmm5 = xmm0; // 0x56f3d9
    xmm5 = _mm_sub_ps(xmm5,xmm7); // 0x56f3dc
    xmm5 = _mm_mul_ps(xmm5,xmm4); // 0x56f3df
    xmm5 = _mm_add_ps(xmm5,xmm6); // 0x56f3e2
    xmm5 = _mm_mul_ps(xmm5,xmm1); // 0x56f3e5
    xmm4 = xmm3; // 0x56f3e8
    xmm4 = _mm_mul_ps(xmm4,xmm3); // 0x56f3eb
    xmm1 = xmm4; // 0x56f3ee
    xmm1 = _mm_shuffle_ps(xmm1,xmm4,2); // 0x56f3f1
    xmm6 = xmm4; // 0x56f3f5
    xmm6 = _mm_shuffle_ps(xmm6,xmm4,1); // 0x56f3f8
    xmm1 = _mm_add_ps(xmm1,xmm6); // 0x56f3fc
    xmm1 = _mm_add_ps(xmm1,xmm4); // 0x56f3ff
    xmm1 = _mm_shuffle_ps(xmm1,xmm1,0); // 0x56f402
    xmm4 = _mm_rsqrt_ps(xmm1); // 0x56f406
    xmm6 = xmm4; // 0x56f409
    xmm6 = _mm_mul_ps(xmm6,xmm4); // 0x56f40c
    xmm6 = _mm_mul_ps(xmm6,xmm1); // 0x56f40f
    xmm1 = xmm0; // 0x56f412
    xmm1 = _mm_sub_ps(xmm1,xmm6); // 0x56f415
    xmm6 = xmm4; // 0x56f418
    xmm6 = _mm_mul_ps(xmm6,xmm2); // 0x56f41b
    xmm1 = _mm_mul_ps(xmm1,xmm6); // 0x56f41e
    xmm1 = _mm_add_ps(xmm1,xmm4); // 0x56f421
    xmm1 = _mm_mul_ps(xmm1,xmm3); // 0x56f424
    _mm_store_ps(mem+0x90/4,xmm1); // 0x56f427
    xmm1 = _mm_mul_ps(xmm1,xmm5); // 0x56f42f
    xmm2 = xmm1; // 0x56f432
    xmm2 = _mm_shuffle_ps(xmm2,xmm1,2); // 0x56f435
    xmm3 = xmm1; // 0x56f439
    xmm3 = _mm_shuffle_ps(xmm3,xmm1,1); // 0x56f43c
    xmm2 = _mm_add_ps(xmm2,xmm3); // 0x56f440
    xmm2 = _mm_add_ps(xmm2,xmm1); // 0x56f443
    xmm1 = xmm2; // 0x56f446
    xmm1 = _mm_shuffle_ps(xmm1,xmm2,0); // 0x56f449
    xmm1 = _mm_max_ps(xmm1,_mm_load_ps(mem+0x50/4)); // 0x56f44d
    xmm1 = _mm_min_ps(xmm1,xmm0); // 0x56f452
    _mm_store_ps(mem+0x50/4,xmm1); // 0x56f455
    _mm_store_ps(mem+0xc0/4,xmm5);
    mem[0x14/4]=Acos(mem[0x50/4]);
    xmm2 = _mm_load_ss(mem+0x14/4); // 0x56f46f
    xmm0 = _mm_load_ps(mem+0x90/4); // 0x56f475
    xmm3 = xmm0; // 0x56f47d
    xmm2 = _mm_shuffle_ps(xmm2,xmm2,0); // 0x56f480
    xmm1 = _mm_load_ps(mem+0xc0/4); // 0x56f484
    xmm3 = _mm_shuffle_ps(xmm3,xmm0,9); // 0x56f48c
    xmm4 = xmm1; // 0x56f490
    xmm4 = _mm_shuffle_ps(xmm4,xmm1,0x12); // 0x56f493
    xmm1 = _mm_shuffle_ps(xmm1,xmm1,9); // 0x56f497
    xmm0 = _mm_shuffle_ps(xmm0,xmm0,0x12); // 0x56f49b
    xmm0 = _mm_mul_ps(xmm0,xmm1); // 0x56f49f
    xmm1 = _mm_xor_ps(xmm1,xmm1); // 0x56f4a2
    xmm3 = _mm_mul_ps(xmm3,xmm4); // 0x56f4a5
    xmm0 = _mm_sub_ps(xmm0,xmm3); // 0x56f4a8
    xmm3 = _mm_set_ss(Float(0x3f800000u)); // 0x56f4ab
    _mm_store_ss(mem+0x3c/4,xmm1); // 0x56f4b3
    _mm_store_ss(mem+0x38/4,xmm1); // 0x56f4b9
    _mm_store_ss(mem+0x30/4,xmm1); // 0x56f4bf
    _mm_store_ss(mem+0x34/4,xmm3); // 0x56f4c5
    xmm1 = _mm_load_ps(mem+0x30/4); // 0x56f4cb
    xmm0 = _mm_mul_ps(xmm0,xmm1); // 0x56f4d0
    xmm1 = xmm0; // 0x56f4d3
    xmm1 = _mm_shuffle_ps(xmm1,xmm0,2); // 0x56f4d6
    xmm3 = xmm0; // 0x56f4da
    xmm3 = _mm_shuffle_ps(xmm3,xmm0,1); // 0x56f4dd
    xmm1 = _mm_add_ps(xmm1,xmm3); // 0x56f4e1
    xmm1 = _mm_add_ps(xmm1,xmm0); // 0x56f4e4
    xmm0 = _mm_load_ss(mem+0x80/4); // 0x56f4e7
    xmm1 = _mm_shuffle_ps(xmm1,xmm1,0); // 0x56f4f0
    _mm_store_ss(mem+0x50/4,xmm0); // 0x56f4f4
    xmm0 = _mm_load_ss(mem+0x50/4); // 0x56f4fa
    _mm_store_ss(mem+0x60/4,xmm1); // 0x56f500
    const float cross=mem[0x60/4];
    const float angle=mem[0x14/4];
    return 0.0f>cross ? Float(0x40c90fdbu)-angle : angle;
}
}
namespace {
__declspec(noinline) float WeightedSample(EventController* s,uint64_t pair,float lower,float upper,float half,float range) {
#if defined(_M_IX86)
    const auto fn=&Sample;
    const uint32_t lo=uint32_t(pair),hi=uint32_t(pair>>32);
    float result;
    __asm { push half
        push 6
        push upper
        push lower
        push hi
        push lo
        mov ecx,s
        xor edx,edx
        call fn
        fmul range
        fstp result }
    return result;
#else
    return static_cast<float>(double(Sample(s,nullptr,uint32_t(pair),uint32_t(pair>>32),lower,upper,6,half))*double(range));
#endif
}
}
void* __fastcall Evaluate(EventController* s,void*,void* argument) {
    if(!s->factory) return nullptr;
    void* created=CallGameVirtual<void*,3>(s->factory,argument);
    if(!created) return nullptr;
    auto* iface=CallGameMethod<void*,0x0054c770>(argument,0x073bb514u);
    auto* out=static_cast<unsigned char*>(CallGameVirtual<void*,1>(iface,0x073bb514u));
    CallGameVirtual<void,3>(out,created);
    CallGameMethod<void,0x0055dff0>(out,s->source);
    CallGameMethod<void,0x0055e040>(out,s->target);
    auto Put=[&](size_t offset,auto value) { std::memcpy(out+offset,&value,sizeof(value)); };
    Put(0x40,s->weight); Put(0x54,s->option44); Put(0x4c,s->mode); Put(0x50,s->flags); Put(0x55,s->option45);
    void* owner=CallGameVirtual<void*,9>(s->base.hostSlot);
    void* resource=nullptr;
    if(owner) {
        Retain(owner);
#if defined(_M_IX86)
        const uint32_t key=*reinterpret_cast<uint32_t*>(0x011e9138);
#else
        const uint32_t key=0;
#endif
        resource=CallGameMethod<void*,0x00935770>(owner,key);
        if(resource) CallGameMethod<void,0x0055dab0>(out,CallGameVirtual<uint64_t,4>(Secondary(resource)));
    }
    __m128 goal=_mm_setzero_ps();
    float response=0.0f, lower=Float(0xbeaa7efau),upper=Float(0x3d8f5c29u);
    if(s->source) {
        const uint32_t flags=s->flags;
        if(!CallGameFunction<uint8_t,0x0055da30>(flags)) {
            const uint32_t bit=1u<<(s->mode&31);
            if((bit&0x60) && flags!=0x800) {
                void* cas=CallGameFunction<void*,0x005eb7c0>();
                if(cas) {
                    upper=CallGameMethod<float,0x005ecd80>(cas,s->flags|bit);
                    lower=CallGameMethod<float,0x005ecdd0>(cas,s->flags|bit);
                }
                const float range=upper-lower;
                const float half=range*0.5f;
                const uint64_t pair=CallGameVirtual<uint64_t,4>(Secondary(resource));
                response=WeightedSample(s,pair,lower,upper,half,range);
            }
        }
        void* route=s->source;
        float remaining=1.5f;
        Retain(route);
        while(true) {
            const float duration=CallGameVirtual<float,75>(route);
            const uint64_t pair=CallGameVirtual<uint64_t,4>(Secondary(resource));
            const float time=CallGameVirtual<float,41>(route,pair);
            const float next=time+remaining;
            alignas(16) __m128 sampled;
            if(!(next>=duration)) { // COMISS unordered takes the short/sample path.
                goal=*CallGameVirtual<__m128*,71>(route,&sampled,next,0u,0u);
                break;
            }
            goal=*CallGameVirtual<__m128*,71>(route,&sampled,duration,0u,0u);
            remaining=remaining-(duration-time);
            void* successor=CallGameVirtual<void*,31>(route);
            if(successor!=route) { Retain(successor); Release(route); route=successor; }
            if(!route || !(remaining>0.0f)) break;
        }
        Release(route);
    }
    alignas(16) __m128 position,rotation;
    void* secondary=Secondary(resource);
    CallGameVirtual<void*,15>(secondary,&position);
    CallGameVirtual<void*,16>(secondary,&rotation);
    Put(0x38,Direction(position,rotation,goal));
    if(s->source) {
        if(!CallGameFunction<uint8_t,0x0055da30>(s->flags)) { Put(0x3c,response); Put(0x48,upper); Put(0x44,lower); }
        if(s->source && CallGameFunction<uint8_t,0x0055da60>(s->flags)) {
            float ratio=1.0f;
            if(s->option46) {
                const float a=CallGameVirtual<float,9>(s);
                ratio=static_cast<float>(double(a)/double(CallGameVirtual<float,10>(s)));
            }
            Put(0x58,ratio);
        }
    }
    Release(owner);
    return out;
}
}
#pragma float_control(pop)
