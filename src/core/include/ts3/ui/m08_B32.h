#pragma once
#include <cstddef>
#include <cstdint>
namespace ts3::ui::b32 {
template<class R,class... A> using BridgeCdecl=R (__cdecl*)(A...);
template<class R,class T,class... A> using BridgeThiscall=R (__fastcall*)(T*,void*,A...);
struct Explicit { void* primary; void* secondary; uint32_t words08[2]; void* interface10; uint32_t words14[2]; void* interface1c; void* target; };
struct Fade { void* primary; void* secondary; uint32_t words08[2]; void* interface10; uint32_t words14[23]; float start,end; };
struct Binding { void* pointer; uint32_t value; };
struct Navigator { void* primary; void* secondary; uint32_t word08; Binding bindings[8]; };
struct Descriptor { void* table; void* object; uint32_t count,type; };
struct Dependencies {
    BridgeCdecl<void*,uint32_t,uint32_t,const char*,uint32_t> allocate=nullptr;
    BridgeThiscall<void*,void> explicitCtor=nullptr,fadeCtor=nullptr;
    BridgeThiscall<void*,void,uint32_t> destroyExplicit1c=nullptr,destroyExplicit4=nullptr,destroyFade=nullptr,queryBase=nullptr;
};
extern thread_local Dependencies dependencies;
// TS3W.exe 0x00522430: Adjust the ExplicitNavigator interface receiver and forward destruction.
void* __fastcall DestroyExplicit1c(void*,void*,uint32_t);
// TS3W.exe 0x00522440: Adjust the secondary receiver and forward destruction.
void* __fastcall DestroyExplicit4(void*,void*,uint32_t);
// TS3W.exe 0x00522450: Allocate and initialize an ExplicitNavigator, returning null on allocation failure.
void* __stdcall CreateExplicit(uint32_t,uint32_t);
// TS3W.exe 0x00522510: Interpolate fade alpha and forward packed color; subtractive alpha packing is retained.
uintptr_t __fastcall ApplyFade(Fade*,void*,void*,float);
// TS3W.exe 0x00522590: Return the FadeEffect type ID.
uint32_t __fastcall FadeType(void*,void*);
// TS3W.exe 0x005225A0: Adjust the +10 interface and forward FadeEffect destruction.
void* __fastcall DestroyFade10(void*,void*,uint32_t);
// TS3W.exe 0x005225B0: Adjust the +4 interface and forward FadeEffect destruction.
void* __fastcall DestroyFade4(void*,void*,uint32_t);
// TS3W.exe 0x005225F0: Allocate and initialize a FadeEffect, returning null on allocation failure.
void* __stdcall CreateFade(uint32_t,uint32_t);
// TS3W.exe 0x00522690: Write the FadeEffect descriptor and return the queried type.
uint32_t __fastcall DescribeFade(void*,void*,Descriptor*);
// TS3W.exe 0x005226E0: Query primary or secondary navigation interfaces, falling back to the base query.
void* __fastcall QueryNavigator(void*,void*,uint32_t);
// TS3W.exe 0x00522710: Return the navigation type ID.
uint32_t __fastcall NavigatorType(void*,void*);
// TS3W.exe 0x00522720: Select a populated binding using EA's fallback order.
Binding* __fastcall SelectBinding(Navigator*,void*,uint32_t);
// TS3W.exe 0x00522800: Forward four rectangle floats to virtual slot 14 and return its EAX.
uintptr_t __fastcall ForwardRectangle(void*,void*,const float*);
// TS3W.exe 0x00522860: Write the navigation descriptor and return the queried type.
uint32_t __fastcall DescribeNavigator(void*,void*,Descriptor*);
}
