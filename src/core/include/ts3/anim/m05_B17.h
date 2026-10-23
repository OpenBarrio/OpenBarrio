#pragma once
#include "ts3/anim/m05_layouts.h"
#include <cstdint>
namespace ts3::anim::b17 {
struct Wrapper { Controller base; void* secondary28; void* child; float scale; float value; };
TS3_X86_OFFSET(Wrapper,child,0x2c);
TS3_X86_OFFSET(Wrapper,scale,0x30);
TS3_X86_OFFSET(Wrapper,value,0x34);
// TS3W.exe 0x0056F640: Controller wrapper ScaleValue; retains original forwarding and lifetime semantics.
float __fastcall ScaleValue(Wrapper* self, void* edx);
// TS3W.exe 0x0056F650: Controller wrapper SetScaled; retains original forwarding and lifetime semantics.
uintptr_t __fastcall SetScaled(Wrapper* self, void* edx, float value);
// TS3W.exe 0x0056F690: Controller wrapper Scaled5; retains original forwarding and lifetime semantics.
double __fastcall Scaled5(Wrapper* self, void* edx);
// TS3W.exe 0x0056F6B0: Controller wrapper Scaled6; retains original forwarding and lifetime semantics.
double __fastcall Scaled6(Wrapper* self, void* edx);
// TS3W.exe 0x0056F6D0: Controller wrapper Scaled7; retains original forwarding and lifetime semantics.
double __fastcall Scaled7(Wrapper* self, void* edx);
// TS3W.exe 0x0056F6F0: Controller wrapper Forward3; retains original forwarding and lifetime semantics.
uintptr_t __fastcall Forward3(Wrapper* self, void* edx, void* argument);
// TS3W.exe 0x0056F7F0: Controller wrapper TypeA; retains original forwarding and lifetime semantics.
uint32_t __fastcall TypeA(Wrapper* self, void* edx);
// TS3W.exe 0x0056F800: Controller wrapper QueryA; retains original forwarding and lifetime semantics.
void* __fastcall QueryA(Wrapper* self, void* edx, uint32_t id);
// TS3W.exe 0x0056F8C0: Controller wrapper UpdateScaled; retains original forwarding and lifetime semantics.
uintptr_t __fastcall UpdateScaled(Wrapper* self, void* edx, float value);
// TS3W.exe 0x0056FB10: Controller wrapper SetDirect; retains original forwarding and lifetime semantics.
uintptr_t __fastcall SetDirect(Wrapper* self, void* edx, float value);
// TS3W.exe 0x0056FB30: Controller wrapper Direct6; retains original forwarding and lifetime semantics.
double __fastcall Direct6(Wrapper* self, void* edx);
// TS3W.exe 0x0056FB40: Controller wrapper NotifyChild; retains original forwarding and lifetime semantics.
void __fastcall NotifyChild(Wrapper* self, void* edx);
// TS3W.exe 0x0056FBE0: Controller wrapper TypeB; retains original forwarding and lifetime semantics.
uint32_t __fastcall TypeB(Wrapper* self, void* edx);
// TS3W.exe 0x0056FBF0: Controller wrapper QueryB; retains original forwarding and lifetime semantics.
void* __fastcall QueryB(Wrapper* self, void* edx, uint32_t id);
// TS3W.exe 0x0056FC70: Controller wrapper UpdateDirect; retains original forwarding and lifetime semantics.
uintptr_t __fastcall UpdateDirect(Wrapper* self, void* edx, float value);
}
