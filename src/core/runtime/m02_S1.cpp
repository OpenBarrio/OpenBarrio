#include "ts3/runtime/m02_S1.h"
#define NOMINMAX
#include <windows.h>
#include <intrin.h>
namespace ts3::runtime::m02::s1 {
namespace {
thread_local PlatformDispatch* clock=nullptr;
template<class T> T Global(uint32_t a) {
#if defined(_M_IX86)
    return *reinterpret_cast<T*>(a);
#else
    (void)a; NotRewritten("M02 S1 calibration global");
#endif
}
uint64_t Cycles() { return clock?clock->rdtsc(clock->context):__rdtsc(); }
uint32_t Counter(uint64_t& v) {
    if(clock) return clock->counter(clock->context,reinterpret_cast<int64_t*>(&v));
    return QueryPerformanceCounter(reinterpret_cast<LARGE_INTEGER*>(&v));
}
uint64_t Ticks(StopWatch* p) {
    uint64_t v=p->accumulated;
    if(p->start) {
        uint64_t now; if(p->mode==1) now=Cycles(); else Counter(now);
        auto delta=now-p->start; auto overhead=Global<uint64_t>(0x011CB910);
        v+=delta>overhead?delta-overhead:1;
    }
    return v;
}
StopWatch* Lookup(uint32_t h) { return CallGameVirtual<StopWatch*,51>(CallGameFunction<void*,0x007692B0>(),h); }
uint64_t Scale(uint64_t v,float factor,bool round) {
#if defined(_M_IX86)
    uint64_t lo=v&0x7fffffffffffffffull, sign=v&0x8000000000000000ull, result;
    float half=round?Global<float>(0x00F98960):0.f;
    __asm {
        fild lo
        fild sign
        fchs
        faddp st(1), st(0)
        fmul factor
        fadd half
        mov eax, 0x00F7EC76
        call eax
        mov dword ptr result, eax
        mov dword ptr result[4], edx
    }
    return result;
#else
    return static_cast<uint64_t>(static_cast<long double>(v)*factor+(round?static_cast<long double>(Global<float>(0x00F98960)):0.L));
#endif
}
}
PlatformDispatch* SetClock(PlatformDispatch* d) { auto old=clock; clock=d; return old; }
uint32_t __fastcall Restart(StopWatch* p,void*) {
    p->start=0; p->accumulated=0;
    if(p->mode==1) { p->start=Cycles(); return uint32_t(p->start); }
    return Counter(p->start);
}
float __cdecl UnitFactor(uint32_t mode) { return mode>=2 && mode<=6?Global<float>(0x011CB904-4*(mode-2)):1.f; }
uint32_t __fastcall SetMode(StopWatch* p,void*,uint32_t mode) {
    p->mode=mode; p->factor=Global<float>(0x0107A538);
    if(mode==1) {
        auto a=Global<uint64_t>(0x0114C9F0), b=Global<uint64_t>(0x0114C9F8);
        if(a==b) return uint32_t(a>>32);
#if defined(_M_IX86)
        uint64_t alo=a&0x7fffffffffffffffull, ahi=a&0x8000000000000000ull;
        uint64_t blo=b&0x7fffffffffffffffull, bhi=b&0x8000000000000000ull;
        float factor;
        __asm {
            fild alo
            fild ahi
            fchs
            faddp st(1), st(0)
            fild blo
            fild bhi
            fchs
            faddp st(1), st(0)
            fdivp st(1), st(0)
            fstp factor
        }
        p->factor=factor;
#else
        p->factor=float(static_cast<long double>(a)/static_cast<long double>(b));
#endif
        return uint32_t(b>>32)&0x7fffffff;
    }
    if(mode>=2 && mode<=6) p->factor=UnitFactor(mode);
    return mode;
}
uint64_t __fastcall ElapsedUInt(StopWatch* p,void*) { return Scale(Ticks(p),p->factor,true); }
float __fastcall ElapsedFloat(StopWatch* p,void*) {
    auto v=Ticks(p);
#if defined(_M_IX86)
    uint64_t lo=v&0x7fffffffffffffffull,sign=v&0x8000000000000000ull;
    float factor=p->factor,result;
    __asm {
        fild lo
        fild sign
        fchs
        faddp st(1), st(0)
        fmul factor
        fstp result
    }
    return result;
#else
    return float(static_cast<long double>(v)*p->factor);
#endif
}
uint32_t __fastcall SetDeadline(DeadlineTimer* p,void*,uint32_t duration,uint32_t start) {
    uint64_t now; Counter(now);
    uint64_t delta;
#if defined(_M_IX86)
    float factor=p->watch.factor;
    int64_t positive=duration;
    __asm {
        fild positive
        fdiv factor
        mov eax, 0x00F7EC76
        call eax
        mov dword ptr delta, eax
        mov dword ptr delta[4], edx
    }
#else
    delta=uint64_t(static_cast<long double>(duration)/p->watch.factor);
#endif
    p->deadline=now+delta;
    if((start&255) && !p->watch.start) {
        if(p->watch.mode==1) { p->watch.start=Cycles(); return uint32_t(p->watch.start); }
        return Counter(p->watch.start);
    }
    return uint32_t(p->deadline);
}
StopWatch* __fastcall Construct(StopWatch* p,void*,uint32_t mode,uint32_t start) {
    p->start=p->accumulated=0; p->mode=0; p->factor=Global<float>(0x0107A538);
    SetMode(p,nullptr,mode);
    if(start&255) { if(p->mode==1) p->start=Cycles(); else Counter(p->start); }
    return p;
}
float __cdecl ManagedFloat(uint32_t h) { if((h&0xff000000)!=0x10000000) return 0; auto p=Lookup(h); return p?ElapsedFloat(p,nullptr):0; }
void* __fastcall DeleteRegistration(void* p,void*,uint32_t flags) {
#if defined(_M_IX86)
    *reinterpret_cast<void**>(0x011D6834)=nullptr;
    *static_cast<void**>(p)=reinterpret_cast<void*>(0x00FDE658);
#else
    NotRewritten("M02 S1 registration global");
#endif
    if(flags&1) CallGameFunction<void,0x004E3F70>(p);
    return p;
}
uintptr_t __cdecl Reset(uint32_t h) {
    auto tag=h&0xff000000; if(tag!=0x10000000) return tag;
    auto p=Lookup(h); if(p) p->start=p->accumulated=0;
    return reinterpret_cast<uintptr_t>(p);
}
uint32_t __cdecl IsRunning(uint32_t h) {
    auto tag=h&0xff000000; if(tag!=0x10000000) return tag;
    auto p=Lookup(h); return p && p->start?1:0;
}
uintptr_t __cdecl Destroy(uint32_t h) {
    auto tag=h&0xff000000; if(tag!=0x10000000) return tag;
    auto p=CallGameVirtual<void*,54>(CallGameFunction<void*,0x007692B0>(),h);
    return p?CallGameFunction<uintptr_t,0x004E3F70>(p):0;
}
}
