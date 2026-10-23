#pragma once
#include <cstddef>
#include <cstdint>
namespace ts3::math::m04 {
enum class ReturnMask : uint32_t { AL=0xff, EAX=0xffffffffu };
constexpr bool SameReturn(uint32_t a,uint32_t b,ReturnMask mask) {
    return ((a^b)&static_cast<uint32_t>(mask))==0;
}
struct RawReturn {
    uint32_t eax=0, ecx=0, edx=0;
    unsigned char st0[10]{}; // exact x87 extended representation, popped iff requested
    int stackBytesPopped=0; // distinguishes plain RET from RET N
};
#if defined(_M_IX86)
template<class R,class... A> using Cdecl = R (__cdecl*)(A...);
// Caller supplies the dummy EDX argument; useful only for recovered ECX methods.
template<class R,class T,class... A> using Thiscall = R (__fastcall*)(T*,void*,A...);
// Recovered stack words only; not an adapter for XMM-input or variadic kernels.
// Callee must obey normal nonvolatile-register rules; capture preserves full EAX.
RawReturn Capture(void* target,uint32_t ecxValue,uint32_t edxValue,
                  const uint32_t* words,size_t count,bool popSt0);
#endif
}
