#pragma once
#include "ts3/math/m04_abi.h"
#include "ts3/ui/m08_layouts.h"
#include <stdexcept>
namespace ts3::ui::m08 {
using RawReturn=ts3::math::m04::RawReturn;
using ReturnMask=ts3::math::m04::ReturnMask;
using ts3::math::m04::SameReturn;
constexpr uint64_t PairReturn(uint32_t eax,uint32_t edx) { return uint64_t(eax)|(uint64_t(edx)<<32); }
#if defined(_M_IX86)
// ECX receiver, dummy EDX, callee RET N. AL-only methods must use uint8_t.
template<class R,class T,class... A> using Thiscall=R (__fastcall*)(T*,void*,A...);
template<class R,class... A> using Stdcall=R (__stdcall*)(A...);
template<class R,class... A> using Cdecl=R (__cdecl*)(A...);
using ts3::math::m04::Capture; // full EAX/EDX, optional raw ten-byte ST0, measured RET N
#else
template<class R,class T,class... A> using Thiscall=R (*)(T*,void*,A...);
template<class R,class... A> using Stdcall=R (*)(A...);
template<class R,class... A> using Cdecl=R (*)(A...);
inline RawReturn Capture(void*,uint32_t,uint32_t,const uint32_t*,size_t,bool) {
    throw std::runtime_error("M08 x86 register capture unavailable on native host");
}
#endif
template<class R,class T,class... A> R CallThis(Thiscall<R,T,A...> fn,T* self,A... a) { return fn(self,nullptr,a...); }
template<class Fn> Fn Original(uint32_t address,Fn injected=nullptr) {
    if(injected) return injected;
#if defined(_M_IX86)
    return reinterpret_cast<Fn>(static_cast<uintptr_t>(address));
#else
    (void)address;throw std::runtime_error("M08 original x86 bridge unavailable: inject native interface");
#endif
}
// Wire/native conversion is explicit and delegated to the fixture/image resolver.
struct PointerResolver {
    virtual ~PointerResolver()=default;
    virtual void* Resolve(uint32_t)=0;
    virtual uint32_t Encode(const void*)=0;
};
template<class T> T* ToNative(WirePointer<T> p,PointerResolver& resolver) { return p.address?static_cast<T*>(resolver.Resolve(p.address)):nullptr; }
template<class T> WirePointer<T> ToWire(T* p,PointerResolver& resolver) { return {p?resolver.Encode(p):0}; }
// 007F2C50's EAX receiver and 007E7B80's -A0 adjustor belong to their G tasks.
// Adjust native views by member address, never by adding/subtracting x86 offsets.
inline void* WindowSecondary(Window* p) { return &p->secondary; }
inline void* LayoutSecondary(LayoutInstance* p) { return &p->interface0A0; }
}
