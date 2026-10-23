#include "ts3/anim/m05_M139.h"
#include "ts3/layout.h"
#include "ts3/route/clu_848d20_Route.h"
#include <cstddef>
#define WIN32_LEAN_AND_MEAN
#define NOMINMAX
#include <windows.h>
#include <xmmintrin.h>
#include <cstring>
#include <cmath>
namespace ts3::anim::m139 {
#if defined(_M_IX86)
static_assert(offsetof(Body,controllers)==0x1c);
static_assert(offsetof(Body,factory)==0x40);
static_assert(offsetof(Body,styleIndex)==0x7c);
static_assert(offsetof(Body,styleType)==0x84);
static_assert(offsetof(Body,slaveHost)==0xb0);
static_assert(offsetof(Body,slaves)==0xb4);
static_assert(offsetof(Body,walker)==0xc4);
static_assert(offsetof(Body,slaveCount)==0xc8);
static_assert(offsetof(Body,statef0)==0xf0);
static_assert(offsetof(Body,argument)==0x10c);
static_assert(offsetof(Body,registry)==0x124);
static_assert(offsetof(Body,speed)==0x150);
static_assert(sizeof(RootRequest)==28);
static_assert(offsetof(RootRequest,owned)==24);
static_assert(offsetof(Body,lower)==0x16c);
static_assert(offsetof(Body,dirty)==0x182);
static_assert(offsetof(Body,begin)==0x188);
static_assert(offsetof(Body,count)==0x24c);
static_assert(offsetof(Body,flag253)==0x253);
#endif
namespace {
float Cell(uint32_t address,float native) {
#if defined(_M_IX86)
    return *reinterpret_cast<const float*>(uintptr_t(address));
#else
    (void)address; return native;
#endif
}
float Lane(__m128 x) { return _mm_cvtss_f32(x); }
__m128 Sum3(__m128 x) { return _mm_add_ps(_mm_add_ps(_mm_shuffle_ps(x,x,2),_mm_shuffle_ps(x,x,1)),x); }
__m128 Splat3(__m128 x) { auto sum=Sum3(x); return _mm_shuffle_ps(sum,sum,0); }
__m128 Refine(__m128 n,__m128 r) {
    return _mm_add_ps(_mm_mul_ps(_mm_sub_ps(_mm_set1_ps(1),_mm_mul_ps(_mm_mul_ps(r,r),n)),_mm_mul_ps(r,_mm_set1_ps(.5f))),r);
}
__m128 Normalize(__m128 x,unsigned refinements) {
    auto n=Splat3(_mm_mul_ps(x,x)); auto r=_mm_rsqrt_ps(n);
    for(unsigned i=0;i<refinements;++i) r=Refine(n,r);
    return _mm_mul_ps(r,x);
}
__m128 Cross(__m128 a,__m128 b) {
    return _mm_sub_ps(_mm_mul_ps(_mm_shuffle_ps(a,a,18),_mm_shuffle_ps(b,b,9)),_mm_mul_ps(_mm_shuffle_ps(a,a,9),_mm_shuffle_ps(b,b,18)));
}
__m128 Forward(__m128 q) {
    auto axis=_mm_set_ps(0,1,0,0);
    auto t=_mm_add_ps(Cross(axis,q),_mm_mul_ps(_mm_shuffle_ps(q,q,255),axis));
    auto result=_mm_add_ps(_mm_mul_ps(Cross(t,q),_mm_set1_ps(2)),axis);
    // This is a lane store, not multiplication by a mask (NaNs and signed zeros matter).
    alignas(16) float lanes[4]; _mm_store_ps(lanes,result); lanes[1]=0; return _mm_load_ps(lanes);
}
// Keep the assembly call out of the SIMD caller: _CIacos may clobber volatile
// XMM registers, which an inlined MSVC asm call does not describe to the optimizer.
__declspec(noinline) float Acos(float x) {
#if defined(_M_IX86)
    float result; uintptr_t fn=0x00f7edde;
    __asm { fld x }
    __asm { call fn }
    __asm { fstp result }
    return result;
#else
    return static_cast<float>(std::acos(static_cast<double>(x)));
#endif
}
// These consumers intentionally keep a callee's ST0 value until EA's first
// rounding point instead of inserting a C++ double spill at the bridge boundary.
__declspec(noinline) bool TimeExceeds(void* target,float baseline,float epsilon) {
#if defined(_M_IX86)
    uintptr_t fn=reinterpret_cast<uintptr_t>((*static_cast<void***>(target))[75]);
    uint8_t result;
    __asm {
        mov ecx,target
        call fn
        fsub baseline
        fld epsilon
        fxch st(1)
        fcomip st,st(1)
        fstp st(0)
        seta result
    }
    return result!=0;
#else
    return CallGameVirtual<double,75>(target)-double(baseline)>double(epsilon);
#endif
}
__declspec(noinline) float SampleDelta(void* target,float baseline) {
#if defined(_M_IX86)
    uintptr_t fn=reinterpret_cast<uintptr_t>((*static_cast<void***>(target))[75]);
    float result;
    __asm {
        mov ecx,target
        call fn
        fsub baseline
        fstp result
    }
    return result;
#else
    return static_cast<float>(CallGameVirtual<double,75>(target)-double(baseline));
#endif
}
__declspec(noinline) bool TooFar(Body* self,float distance) {
#if defined(_M_IX86)
    uintptr_t fn=0x00848890; uint8_t result;
    __asm {
        push 1
        mov ecx,self
        call fn
        fld distance
        fcomip st,st(1)
        fstp st(0)
        seta result
    }
    return result!=0;
#else
    return double(distance)>CallGameMethod<double,0x00848890>(self,1);
#endif
}
__declspec(noinline) bool PositiveDuration(void* controller,float* rounded) {
#if defined(_M_IX86)
    uintptr_t fn=0x00849540; uint8_t result;
    __asm {
        push controller
        call fn
        add esp,4
        mov eax,rounded
        fst dword ptr [eax]
        fldz
        fxch st(1)
        fcomip st,st(1)
        fstp st(0)
        seta result
    }
    return result!=0;
#else
    double duration=CallGameFunction<double,0x00849540>(controller);
    *rounded=static_cast<float>(duration); return duration>0;
#endif
}
float Wrapped(float angle) {
    // The x87 multiply is retained for floor, but its separately rounded float is
    // used for subtraction. Do not replace this with fmod or round the product early.
    const double product=double(angle)*double(Cell(0x00fc13c0,0.15915494f));
    volatile float rounded=static_cast<float>(product);
    const double fraction=double(rounded)-std::floor(product);
    float f=static_cast<float>(fraction);
    if(fraction>0.5) f=f-Cell(0x0107a538,1);
    f=f*Cell(0x01026fe0,6.2831855f);
    if(f>Cell(0x01026fdc,3.1415927f)) f=f-Cell(0x01026fe0,6.2831855f);
    float lo=Cell(0x01004650,-3.1415927f),hi=Cell(0x01026fdc,3.1415927f);
    auto bits=[](float v) { uint32_t b; std::memcpy(&b,&v,4); return b; };
    uint32_t mask=uint32_t(int32_t(bits(lo-f))>>31);
    uint32_t b=(mask&bits(f))|(~mask&bits(lo)); std::memcpy(&f,&b,4);
    mask=uint32_t(int32_t(bits(hi-f))>>31);
    b=(mask&bits(hi))|(~mask&bits(f)); std::memcpy(&f,&b,4); return f;
}
float Heading(__m128 direction,__m128 forward) {
    auto dot=Splat3(_mm_mul_ps(direction,forward));
    auto clamped=_mm_min_ps(_mm_max_ps(dot,_mm_set1_ps(-1)),_mm_set1_ps(1));
    float angle=Acos(Lane(clamped));
    float sign=Lane(Splat3(_mm_mul_ps(Cross(direction,forward),_mm_set_ps(0,0,1,0))));
    // COMISS/JBE selects the acos value for unordered as well as nonnegative.
    if(0.f>sign) angle=Cell(0x01026fe0,6.2831855f)-angle;
    return Wrapped(angle);
}
struct TargetPrefix { void** vtable; volatile long refs; };
void ReleaseLocal(void* target) {
    auto* p=static_cast<TargetPrefix*>(target);
    if(InterlockedDecrement(&p->refs)==0) {
        InterlockedExchange(&p->refs,1);
        CallGameVirtual<void,0>(target,1);
    }
}
// 0084B590 consumes ECX=self, EDI=retained target, and seven stack words
// (ret 1C). The EDI argument is NOT part of an ordinary thiscall declaration.
__declspec(noinline) void Install(Body* self,void* target,void* controller,void** slaves,int count) {
#if defined(_M_IX86)
    uintptr_t fn=0x0084b590;
    __asm {
        push edi
        mov edi,target
        push 03f800000h
        push 0
        push 0
        push 7
        push count
        push slaves
        push controller
        mov ecx,self
        call fn
        pop edi
    }
#else
    (void)self; (void)target; (void)controller; (void)slaves; (void)count;
    AbortNotRewritten("0084B590 ECX/EDI original bridge");
#endif
}
}
uint8_t RootMotion(Body* s,uint8_t force,uint8_t snap) {
    if(s->gap110[1] || s->disabled) return 0;
    void* target=CallGameMethod<void*,0x00c93970>(s->canvas);
    if(!target) return 0;
    InterlockedIncrement(&static_cast<TargetPrefix*>(target)->refs);
    alignas(16) __m128 origin,position,rotation;
    CallGameVirtual<void,7>(s->walker,&origin);
    CallGameVirtual<void,77>(target,&position);
    auto* route=CallGameVirtual<void*,3>(target);
    if((CallGameMethod<uint8_t,0x0081c880>(route,0,0x2000,0)&255) && !CallGameVirtual<uint32_t,31>(target)) {
        const float baseline=CallGameFunction<float,0x008484d0>(s->styleIndex,s->styleType);
        if(TimeExceeds(target,baseline,Cell(0x01166268,.0381f)) || snap) {
            float delta=SampleDelta(target,baseline);
            alignas(16) __m128 sampled;
            auto* result=CallGameVirtual<__m128*,71>(target,&sampled,delta,0,0);
            position=_mm_load_ps(reinterpret_cast<const float*>(result));
        }
    }
    auto difference=_mm_sub_ps(position,origin);
    auto horizontal=_mm_shuffle_ps(_mm_shuffle_ps(difference,_mm_setzero_ps(),80),difference,232);
    float distance=Lane(_mm_sqrt_ps(Splat3(_mm_mul_ps(horizontal,horizontal))));
    route=CallGameVirtual<void*,3>(target);
    uint8_t disabled=CallGameMethod<uint8_t,0x0081c880>(route,0,0x20000,0);
    // The original JB rejects unordered distance too (unlike C++ <).
    if(!(distance>=Cell(0x01166268,.0381f)) || disabled) { ReleaseLocal(target); return 0; }
    void* retained;
    CallGameMethod<void,0x008730e0>(&retained,&target);
    auto* service=CallGameMethod<void*,0x0083a280>(s->canvas,retained);
    auto* entry=CallGameVirtual<void*,1>(service,0x14cfd2f8);
    bool tooFar=TooFar(s,distance);
    struct Relation { uint8_t opaque[0xbc]; uint8_t flag; };
    struct Entry { uint8_t opaque[0x10c]; Relation* relation; };
    auto* e=static_cast<Entry*>(entry);
    if(tooFar && (!e || !e->relation || e->relation->flag)) {
        CallGameVirtual<void,24>(s->canvas,0,0,1);
        CallGameMethod<void,0x007460f0>(&target); return 0;
    }
    auto direction=Normalize(horizontal,2);
    CallGameVirtual<void,10>(s->walker,&rotation);
    auto forward=Normalize(Forward(rotation),2);
    if(force && (s->styleType==512 || (s->styleType==1024 && (uint32_t(1)<<(uint32_t(s->styleIndex)&31))!=4) || s->styleType==1536)) {
        route=CallGameVirtual<void*,3>(target);
        if(!CallGameMethod<uint8_t,0x0081c880>(route,0,0x400000,0)) {
            route=CallGameVirtual<void*,3>(target);
            uint32_t a=CallGameMethod<uint32_t,0x0081c8e0>(route);
            route=CallGameVirtual<void*,3>(target);
            uint32_t b=CallGameMethod<uint32_t,0x0081c8d0>(route);
            uint64_t targetId=CallGameVirtual<uint64_t,84>(target);
            uint64_t walkerId=CallGameVirtual<uint64_t,4>(s->walker);
            if(!CallGameFunction<uint8_t,0x007967d0>(walkerId,targetId,&position,&rotation,b,a,1)) {
                s->disabled=1; CallGameMethod<void,0x007460f0>(&target); return 0;
            }
        }
    }
    auto forwardOne=Normalize(forward,1);
    auto directionOne=Normalize(direction,1);
    float angle=Heading(directionOne,forwardOne),heading=angle;
    if(CallGameVirtual<uint8_t,79>(target)) {
        alignas(16) __m128 sampled;
        auto* q=CallGameVirtual<__m128*,80>(target,&sampled);
        auto other=Normalize(Forward(_mm_load_ps(reinterpret_cast<float*>(q))),2);
        auto base=Normalize(forward,1);
        other=Normalize(other,1);
        heading=Heading(other,base);
    }
    // Padding is deliberately not zeroed: EA initializes fields, not the two pad bytes.
    RootRequest request;
    request.registry=s->registry; request.distance=request.angle=request.heading=request.weight=0;
    request.flag0=request.flag1=0; request.owned=nullptr;
    CallGameMethod<void,0x00849bf0>(s,&request);
    request.distance=distance; request.angle=angle; request.heading=heading;
    ControllerResult controller; controller.pointer=nullptr; controller.flag=0;
    if(s->factory) CallGameVirtual<void,3>(s->factory,&request,&controller);
    if(!controller.pointer) {
        CallGameMethod<void,0x0055cfa0>(&controller);
        CallGameMethod<void,0x008499b0>(&request);
        ReleaseLocal(target); return 0;
    }
    SlaveVector slaves{nullptr,nullptr,nullptr,nullptr};
#if defined(_M_IX86)
    slaves.descriptor=reinterpret_cast<const void*>(uintptr_t(0x010272f4));
#else
    slaves.descriptor="Locomotion/AnimatedSimBody/SlaveControllersVector";
#endif
    CallGameMethod<void,0x00aef830>(&slaves,s->slaveCount);
    RootRequest slave;
    slave.registry=nullptr; slave.distance=slave.angle=slave.heading=slave.weight=0;
    slave.flag0=slave.flag1=0; slave.owned=nullptr;
    if(s->slaveMotion) { slave.distance=distance; slave.angle=angle; slave.heading=heading; }
    if(s->gap184) slave.flag1=1;
    bool ready=true;
    if(s->slaveCount>0 && s->slaveHost) {
        RootRequest copy;
        CallGameMethod<void,0x008499e0>(&copy,&slave);
        ready=CallGameMethod<uint8_t,0x00849d40>(s,&slaves,7,copy,0)!=0;
    }
    if(ready) {
        float roundedDuration;
        bool positive=PositiveDuration(controller.pointer,&roundedDuration);
        s->speed=Cell(0x0107a538,1);
        if(positive) s->speed=distance/roundedDuration;
        else {
            auto* variable=CallGameVirtual<void*,1>(s->registry,0xf4c95ed2);
            if(variable) CallGameVirtual<void,2>(variable,1.f);
        }
        if(s->gap184) request.flag1=1;
        auto** begin=slaves.begin;
        Install(s,target,controller.pointer,s->slaveCount<=0?nullptr:begin,s->slaveCount);
        for(int i=0;i<s->slaveCount;++i) {
            if(s->slaves[i]) { CallGameMethod<void,0x00b31c50>(begin[i]); begin[i]=nullptr; }
        }
        s->blocked=1;
        CallGameMethod<void,0x008499b0>(&slave);
        if(begin) CallGameFunction<void,0x004e3f70>(begin);
    } else {
        CallGameMethod<void,0x008499b0>(&slave);
        if(slaves.begin) CallGameFunction<void,0x004e3f70>(slaves.begin);
    }
    CallGameMethod<void,0x0055cfa0>(&controller);
    CallGameMethod<void,0x008499b0>(&request);
    CallGameMethod<void,0x007460f0>(&target);
    return uint8_t(ready);
}
uint32_t Advance(Body* s) {
    if(s->dirty) {
        auto* sentinel=&s->controllers;
        for(auto* n=sentinel->next;n!=sentinel;n=n->next)
            CallGameMethod<void,0x00566590>(static_cast<unsigned char*>(n->controller)+3*sizeof(void*),&s->secondary);
        CallGameMethod<void,0x0081f320>(sentinel);
        sentinel->next=sentinel; sentinel->previous=sentinel;
        s->pending=0;
        // Signed byte distance divided by 28, with truncation toward zero.
        auto distance=reinterpret_cast<intptr_t>(s->end)-reinterpret_cast<intptr_t>(s->begin);
        int mode=3;
        if(distance/28!=0 && s->count==0 && s->limit==0 && s->current>=s->lower && s->upper>=s->current) mode=4;
#if defined(_M_IX86)
        reinterpret_cast<ts3::route::RouteTuner*>(s)->Update84AF90(mode,static_cast<int8_t>(s->force));
#else
        // The verified tuner still uses its own x86 opaque layout; no native bridge exists yet.
        (void)mode; AbortNotRewritten("native RouteTuner layout");
#endif
        s->force=0; s->dirty=0;
    }
    if(s->count>s->limit) return CallGameMethod<uint32_t,0x00850e20>(s);
    if(s->current>=s->lower && s->upper>=s->current && s->count==s->limit) {
        if(!s->flag253 && !(CallGameMethod<uint32_t,0x0085a8f0>(s->statef0)&255))
            return CallGameMethod<uint32_t,0x0084b0c0>(s,0);
        s->count=0;
        uint32_t result=CallGameMethod<uint32_t,0x00848590>(s);
        if(result&255) {
            s->upper=s->lower; s->current=s->lower;
            return CallGameMethod<uint32_t,0x00851280>(s);
        }
        if(s->flag251 || s->flag250 || s->blocked) return result;
        return CallGameMethod<uint32_t,0x008504d0>(s,s->argument);
    }
    uint32_t result=CallGameMethod<uint32_t,0x00848590>(s);
    if(result&255) return CallGameMethod<uint32_t,0x00851280>(s);
    if(s->current<s->lower && s->upper>=s->current) {
        if(s->count==s->limit) return CallGameMethod<uint32_t,0x00850120>(s);
        return s->count;
    }
    return static_cast<uint32_t>(s->current); // Original reloads EAX after testing AL.
}
#if defined(_M_IX86)
uint32_t __fastcall HookAdvance(Body* self,void*) { return Advance(self); }
uint8_t __fastcall HookRootMotion(Body* self,void*,uint8_t a,uint8_t b) { return RootMotion(self,a,b); }
#endif
Body* Delete(Body* s,uint32_t flags) {
    CallGameMethod<void,0x0084f2b0>(s);
    if(flags&1) CallGameFunction<void,0x004e3f70>(s);
    return s;
}
}
