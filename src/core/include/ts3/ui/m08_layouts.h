#pragma once
#include "ts3/layout.h"
#include "ts3/ui/cls_4a4230_UI.h"
#include "ts3/ui/clu_44da00_UI.h"
#include "ts3/scratch/clu_408e20_Scratch.h"
#include <cstddef>
#include <cstdint>
namespace ts3::ui::m08 {
// Image addresses are tokens; no implicit native pointer conversion.
template<class T> struct WirePointer { uint32_t address; };
template<class T> using NativePointer=T*;
using ts3::ui::MatrixRowList;
using ts3::ui::GridMessage;
using ts3::ui::Utf16Vector;
using ts3::ui::UiGlyphContext;
using ts3::ui::UiGlyphRun;
using ts3::scratch::WideString;
struct WideStringWire { WirePointer<void> begin,end,cap,allocName; };
template<template<class> class P> struct ListHeadLayout { P<void> next,previous; };
template<template<class> class P,class S> struct WindowLayout {
    P<void> primary,secondary; // +00, +04 adjusted interface; not a second object
    uint8_t opaque008[0x20];
    uint32_t word028,zero02C[5];
    float values040[12];
    uint32_t flags070;
    uint8_t opaque074[8];
    S text07C;
    uint8_t opaque08C[4];
    S text090;
    uint8_t opaque0A0[0x158];
    uint8_t flag1F8,flag1F9;
    uint8_t opaque1FA[2];
    uint32_t index1FC;
    uint8_t opaque200[0x10];
    ListHeadLayout<P> list210,list218;
    P<void> interface220;
    uint8_t opaque224[0x1C];
    P<void> interface240;
};
using Window=WindowLayout<NativePointer,WideString>;
using WindowWire=WindowLayout<WirePointer,WideStringWire>;
template<template<class> class P> struct ManagerQueueLayout {
    P<void> primary;
    uint32_t word004;
    ListHeadLayout<P> list008,list010,list018;
    P<void> current020;
    uint8_t flag024,opaque025[3];
    uint32_t zero028,zero02C;
};
using ManagerQueue=ManagerQueueLayout<NativePointer>;
using ManagerQueueWire=ManagerQueueLayout<WirePointer>;
template<template<class> class P> struct LayoutInstanceLayout {
    P<void> primary;
    uint8_t opaque004[4];
    uint32_t keys008[4];
    uint8_t opaque018[0x88];
    P<void> interface0A0;
    uint32_t refs0A4,word0A8,word0AC,word0B0;
    P<void> acquired0B4;
    uint8_t byte0B8;
};
using LayoutInstance=LayoutInstanceLayout<NativePointer>;
using LayoutInstanceWire=LayoutInstanceLayout<WirePointer>;
// Only the two observed map words are exposed; delegate values stay opaque.
template<template<class> class P,class W> struct CustomProxyLayout {
    W window;
    uint8_t opaque244[0x14];
    uint8_t delegateMap258[4]; // map starts here; first word's role is unproved
    P<void> storage25C;
};
using CustomProxy=CustomProxyLayout<NativePointer,Window>;
using CustomProxyWire=CustomProxyLayout<WirePointer,WindowWire>;
struct alignas(16) Float4 { float lane[4]; };
struct MapTagView { uint8_t opaque000[0x460];Float4 value460; };
template<class W> struct StartupWindowLayout {
    W window;
    uint8_t opaque244[0xC];
    float value250,value254,value258;
    uint32_t word25C;
    float value260;
    uint32_t word264,word268,word26C;
};
using StartupWindow=StartupWindowLayout<Window>;
using StartupWindowWire=StartupWindowLayout<WindowWire>;
// Observed ends are not class/allocation sizes. No native blanket packing.
TS3_X86_OFFSET(Window,primary,0x0);
static_assert(offsetof(WindowWire,primary)==0x0);
TS3_X86_OFFSET(Window,secondary,0x4);
static_assert(offsetof(WindowWire,secondary)==0x4);
TS3_X86_OFFSET(Window,word028,0x28);
static_assert(offsetof(WindowWire,word028)==0x28);
TS3_X86_OFFSET(Window,zero02C,0x2C);
static_assert(offsetof(WindowWire,zero02C)==0x2C);
TS3_X86_OFFSET(Window,values040,0x40);
static_assert(offsetof(WindowWire,values040)==0x40);
TS3_X86_OFFSET(Window,flags070,0x70);
static_assert(offsetof(WindowWire,flags070)==0x70);
TS3_X86_OFFSET(Window,text07C,0x7C);
static_assert(offsetof(WindowWire,text07C)==0x7C);
TS3_X86_OFFSET(Window,text090,0x90);
static_assert(offsetof(WindowWire,text090)==0x90);
TS3_X86_OFFSET(Window,flag1F8,0x1F8);
static_assert(offsetof(WindowWire,flag1F8)==0x1F8);
TS3_X86_OFFSET(Window,flag1F9,0x1F9);
static_assert(offsetof(WindowWire,flag1F9)==0x1F9);
TS3_X86_OFFSET(Window,index1FC,0x1FC);
static_assert(offsetof(WindowWire,index1FC)==0x1FC);
TS3_X86_OFFSET(Window,list210,0x210);
static_assert(offsetof(WindowWire,list210)==0x210);
TS3_X86_OFFSET(Window,list218,0x218);
static_assert(offsetof(WindowWire,list218)==0x218);
TS3_X86_OFFSET(Window,interface220,0x220);
static_assert(offsetof(WindowWire,interface220)==0x220);
TS3_X86_OFFSET(Window,interface240,0x240);
static_assert(offsetof(WindowWire,interface240)==0x240);
TS3_X86_OFFSET(ManagerQueue,primary,0x0);
static_assert(offsetof(ManagerQueueWire,primary)==0x0);
TS3_X86_OFFSET(ManagerQueue,word004,0x4);
static_assert(offsetof(ManagerQueueWire,word004)==0x4);
TS3_X86_OFFSET(ManagerQueue,list008,0x8);
static_assert(offsetof(ManagerQueueWire,list008)==0x8);
TS3_X86_OFFSET(ManagerQueue,list010,0x10);
static_assert(offsetof(ManagerQueueWire,list010)==0x10);
TS3_X86_OFFSET(ManagerQueue,list018,0x18);
static_assert(offsetof(ManagerQueueWire,list018)==0x18);
TS3_X86_OFFSET(ManagerQueue,current020,0x20);
static_assert(offsetof(ManagerQueueWire,current020)==0x20);
TS3_X86_OFFSET(ManagerQueue,flag024,0x24);
static_assert(offsetof(ManagerQueueWire,flag024)==0x24);
TS3_X86_OFFSET(ManagerQueue,zero028,0x28);
static_assert(offsetof(ManagerQueueWire,zero028)==0x28);
TS3_X86_OFFSET(ManagerQueue,zero02C,0x2C);
static_assert(offsetof(ManagerQueueWire,zero02C)==0x2C);
TS3_X86_OFFSET(LayoutInstance,primary,0x0);
static_assert(offsetof(LayoutInstanceWire,primary)==0x0);
TS3_X86_OFFSET(LayoutInstance,keys008,0x8);
static_assert(offsetof(LayoutInstanceWire,keys008)==0x8);
TS3_X86_OFFSET(LayoutInstance,interface0A0,0xA0);
static_assert(offsetof(LayoutInstanceWire,interface0A0)==0xA0);
TS3_X86_OFFSET(LayoutInstance,refs0A4,0xA4);
static_assert(offsetof(LayoutInstanceWire,refs0A4)==0xA4);
TS3_X86_OFFSET(LayoutInstance,word0A8,0xA8);
static_assert(offsetof(LayoutInstanceWire,word0A8)==0xA8);
TS3_X86_OFFSET(LayoutInstance,word0AC,0xAC);
static_assert(offsetof(LayoutInstanceWire,word0AC)==0xAC);
TS3_X86_OFFSET(LayoutInstance,word0B0,0xB0);
static_assert(offsetof(LayoutInstanceWire,word0B0)==0xB0);
TS3_X86_OFFSET(LayoutInstance,acquired0B4,0xB4);
static_assert(offsetof(LayoutInstanceWire,acquired0B4)==0xB4);
TS3_X86_OFFSET(LayoutInstance,byte0B8,0xB8);
static_assert(offsetof(LayoutInstanceWire,byte0B8)==0xB8);
TS3_X86_OFFSET(CustomProxy,delegateMap258,0x258);
static_assert(offsetof(CustomProxyWire,delegateMap258)==0x258);
TS3_X86_OFFSET(CustomProxy,storage25C,0x25C);
static_assert(offsetof(CustomProxyWire,storage25C)==0x25C);
TS3_X86_OFFSET(StartupWindow,value250,0x250);
static_assert(offsetof(StartupWindowWire,value250)==0x250);
TS3_X86_OFFSET(StartupWindow,value254,0x254);
static_assert(offsetof(StartupWindowWire,value254)==0x254);
TS3_X86_OFFSET(StartupWindow,value258,0x258);
static_assert(offsetof(StartupWindowWire,value258)==0x258);
TS3_X86_OFFSET(StartupWindow,word25C,0x25C);
static_assert(offsetof(StartupWindowWire,word25C)==0x25C);
TS3_X86_OFFSET(StartupWindow,value260,0x260);
static_assert(offsetof(StartupWindowWire,value260)==0x260);
TS3_X86_OFFSET(StartupWindow,word264,0x264);
static_assert(offsetof(StartupWindowWire,word264)==0x264);
TS3_X86_OFFSET(StartupWindow,word268,0x268);
static_assert(offsetof(StartupWindowWire,word268)==0x268);
TS3_X86_OFFSET(StartupWindow,word26C,0x26C);
static_assert(offsetof(StartupWindowWire,word26C)==0x26C);
TS3_X86_OFFSET(MapTagView,value460,0x460);
static_assert(alignof(Float4)==16);
inline constexpr uint32_t ManagerSingletonAddress=0x011CD080;
inline constexpr uint32_t ResourceVectorAddress=0x011658CC;
inline constexpr uint32_t CustomControlInterfaceId=0x056BB768;
inline constexpr uint32_t UiResourceType=0x2F7D0008;
constexpr bool IsUiHandle(uint32_t h) { return (h&0xFF000000u)==0x30000000u; }
struct OpaqueDelegate { uintptr_t value[2]; }; // managed values, NEVER callable function pointers
struct Vector2 { float x,y; };
}
