#pragma once
#include "ts3/layout.h"
#include "ts3/math/m04_abi.h"
#include <stdexcept>
namespace ts3::render::m07 {
using RawReturn = ts3::math::m04::RawReturn;
using ReturnMask = ts3::math::m04::ReturnMask;
using ts3::math::m04::SameReturn;
constexpr uint64_t PairReturn(uint32_t eax,uint32_t edx) { return uint64_t(eax)|(uint64_t(edx)<<32); }
#if defined(_M_IX86)
// Fastcall adapter supplies ECX self and unused EDX; callee performs RET N.
template<class R,class T,class... A> using Thiscall = R (__fastcall*)(T*,void*,A...);
template<class R,class... A> using Stdcall = R (__stdcall*)(A...);
template<class R,class T,class... A> R CallThis(Thiscall<R,T,A...> fn,T* self,A... args) { return fn(self,nullptr,args...); }
using ts3::math::m04::Capture; // full EAX/EDX, optional raw 80-bit ST0, observed RET N
#else
template<class R,class T,class... A> using Thiscall = R (*)(T*,void*,A...);
template<class R,class... A> using Stdcall = R (*)(A...);
template<class R,class T,class... A> R CallThis(Thiscall<R,T,A...> fn,T* self,A... args) { return fn(self,nullptr,args...); }
#endif
// Explicit COM self is a stack argument on x86, NOT an ECX engine receiver.
template<class R,class T,class... A> using ComMethod = Stdcall<R,T*,A...>;
template<class Fn> Fn Original(uint32_t address,Fn injected=nullptr) {
    if(injected) return injected;
#if defined(_M_IX86)
    return reinterpret_cast<Fn>(static_cast<uintptr_t>(address));
#else
    (void)address;
    throw std::runtime_error("M07 original x86 bridge unavailable: inject a native fake");
#endif
}
// No bridge for 0070B1E0 (ESI/EDI), 006D2780 or 00611080: owned by G jobs.
}
