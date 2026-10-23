#pragma once
#include "ts3/layout.h"
#include <cstdint>
namespace ts3::ui {
// ABI adapter scope: the generated thiscall hooks invoke the member overloads.
struct b23 {
struct Window {
    void** primary; void** secondary;
    uint8_t opaque008[0x30-8]; void* manager;
    uint8_t opaque034[0x254-0x34]; void** delegate;
    int32_t hover, pressed;
    uint8_t opaque260[0x288-0x260]; uint32_t flags;
    uint8_t opaque28C[4]; uint8_t started, selecting;
    uint8_t opaque292[0x300-0x292]; void* popup;
};
struct Style {
    uint8_t opaque000[0x18]; uint32_t colors[6];
    uint8_t opaque030[12]; uint32_t value; uint8_t dirty;
    uint8_t opaque041[0x8c-0x41]; float rectangle[4]; float scale;
};
TS3_X86_OFFSET(Window,manager,0x30);
TS3_X86_OFFSET(Window,delegate,0x254);
TS3_X86_OFFSET(Window,hover,0x258);
TS3_X86_OFFSET(Window,flags,0x288);
TS3_X86_OFFSET(Window,started,0x290);
TS3_X86_OFFSET(Window,popup,0x300);
TS3_X86_OFFSET(Style,rectangle,0x8c);
struct Message { uint32_t opaque[2],kind,argument; };
// TS3W.exe 0x0051B300: Select the pressed item and acquire channel-one focus.
static uint8_t Press(Window*, float, float, uint32_t, uint32_t);
// TS3W.exe 0x0051B390: Release focus and update hover or pressed selection.
static uint8_t Release(Window*, float, float, uint32_t, uint32_t);
// TS3W.exe 0x0051B440: Update hover and invalidate only on change.
static uint8_t Move(Window*, float, float, uint32_t);
// TS3W.exe 0x0051B480: Handle channel-one focus changes and popup dismissal.
static uint8_t FocusChanged(Window*, uint32_t, void*);
// TS3W.exe 0x0051B540: Dismiss popup on message 21 then call the base handler.
static uintptr_t Handle(Window*, const Message*);
// TS3W.exe 0x0051B580: Call activation slot and return true in AL.
static uint8_t Activate(Window*);
// TS3W.exe 0x0051B590: Call the primary reset slot then the secondary base method.
static uintptr_t SecondaryReset(void*);
// TS3W.exe 0x0051B5B0: Enable flag eight once, setting the guard before callback.
static void Start(Window*);
// TS3W.exe 0x0051B5D0: Set one of six unsigned-indexed colors; return the input index.
static uint32_t SetColor(Style*, uint32_t, uint32_t);
// TS3W.exe 0x0051B5F0: Get a color, returning zero for unsigned indices at least six.
static uint32_t Color(const Style*, uint32_t);
// TS3W.exe 0x0051B610: Store the value, mark dirty, and return the input word.
static uint32_t SetValue(Style*, uint32_t);
// TS3W.exe 0x0051B620: Copy rectangle components in order and return the third output pointer.
static float* Rectangle(const Style*, float*, float*, float*, float*);
// TS3W.exe 0x0051B660: Store four raw floats and mark dirty.
static void SetRectangle(Style*, float, float, float, float);
// TS3W.exe 0x0051B6A0: Return the scale via ST0 on x86.
static float Scale(const Style*);
// TS3W.exe 0x0051B6B0: Store scale without changing the dirty byte.
static void SetScale(Style*, float);

uint8_t Press(float x,float y,uint32_t a,uint32_t b) { return Press(reinterpret_cast<Window*>(this),x,y,a,b); }
uint8_t Release(float x,float y,uint32_t a,uint32_t b) { return Release(reinterpret_cast<Window*>(this),x,y,a,b); }
uint8_t Move(float x,float y,uint32_t a) { return Move(reinterpret_cast<Window*>(this),x,y,a); }
uint8_t FocusChanged(uint32_t c,void* p) { return FocusChanged(reinterpret_cast<Window*>(this),c,p); }
uintptr_t Handle(void* p) { return Handle(reinterpret_cast<Window*>(this),static_cast<const Message*>(p)); }
uint8_t Activate() { return Activate(reinterpret_cast<Window*>(this)); }
uintptr_t SecondaryReset() { return SecondaryReset(static_cast<void*>(this)); }
void Start() { Start(reinterpret_cast<Window*>(this)); }
uint32_t SetColor(uint32_t i,uint32_t v) { return SetColor(reinterpret_cast<Style*>(this),i,v); }
uint32_t Color(uint32_t i) { return Color(reinterpret_cast<Style*>(this),i); }
uint32_t SetValue(uint32_t v) { return SetValue(reinterpret_cast<Style*>(this),v); }
void* Rectangle(void* a,void* b,void* c,void* d) { return Rectangle(reinterpret_cast<Style*>(this),static_cast<float*>(a),static_cast<float*>(b),static_cast<float*>(c),static_cast<float*>(d)); }
void SetRectangle(float a,float b,float c,float d) { SetRectangle(reinterpret_cast<Style*>(this),a,b,c,d); }
float Scale() { return Scale(reinterpret_cast<Style*>(this)); }
void SetScale(float v) { SetScale(reinterpret_cast<Style*>(this),v); }
};
}
