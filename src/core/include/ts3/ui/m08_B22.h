#pragma once
#include "ts3/layout.h"
#include <cstdint>
namespace ts3::ui::b22 {
struct Circular { uint8_t *begin,*end,*read,*write; };
// Sparse native views: padding is opaque; pointer fields grow on native hosts.
struct Window {
    void** primary; void** secondary; uint8_t opaque008[0x248];
    void** control; void** focus; uint8_t opaque258[8]; uint32_t selection;
    uint8_t opaque264[0x24]; uint32_t flags; uint8_t opaque28C[5]; uint8_t state;
    uint8_t opaque292[0x6E]; void* active;
};
struct Listener { uint8_t opaque[16]; Window* owner; };
struct Payload { uint32_t kind,unused,value; };
struct Message { uint32_t unused[2],kind; uint8_t opaque00C[12]; Payload* payload; };
TS3_X86_OFFSET(Window,control,0x250); TS3_X86_OFFSET(Window,focus,0x254);
TS3_X86_OFFSET(Window,selection,0x260); TS3_X86_OFFSET(Window,flags,0x288);
TS3_X86_OFFSET(Window,state,0x291); TS3_X86_OFFSET(Window,active,0x300);
// TS3W.exe 0x0051AD90: allocate and construct a cascade effect; null allocation stays null.
void* __stdcall CreateEffect(uint32_t ignored,uint32_t tag);
// TS3W.exe 0x0051ADE0: resize circular storage, retaining storage when capacity is unchanged.
uintptr_t __fastcall Resize(Circular*,void*,uint32_t,const char*);
// TS3W.exe 0x0051AE60: compute aligned free capacity, preserving EA's non-power-of-two mask arithmetic.
uintptr_t __fastcall Available(Circular*,void*,uint32_t);
// TS3W.exe 0x0051AEB0: initialize circular storage and return the receiver.
Circular* __fastcall Init(Circular*,void*,uint32_t);
// TS3W.exe 0x0051AF40: conditionally transfer focus; return AL indicates a different focused window.
uint8_t __fastcall FocusChanged(Listener*,void*,void*,uint32_t,uint32_t,uint32_t,uint32_t);
// TS3W.exe 0x0051AFB0: process combo focus transitions, always returning zero in AL.
uint8_t __fastcall Transition(Listener*,void*,void*,uint32_t,void*);
// TS3W.exe 0x0051B0E0: activate the combo's focus target when necessary and return one in AL.
uint8_t __fastcall Activate(Listener*,void*,void*,uint32_t,uint32_t,uint32_t);
// TS3W.exe 0x0051B140: process a selection message before forwarding to the base listener.
uintptr_t __fastcall MessageReceived(Listener*,void*,void*,Message*);
// TS3W.exe 0x0051B200: update via virtual slot A0 then present the layout (AL result).
uint8_t __fastcall Present(Window*,void*);
// TS3W.exe 0x0051B220: query the combo interface, forwarding unknown IDs to the base window.
void* __fastcall Query(Window*,void*,uint32_t);
// TS3W.exe 0x0051B250: return the combo type identifier.
uint32_t __fastcall Type(void*,void*);
// TS3W.exe 0x0051B260: store interface flags then refresh the primary receiver.
uintptr_t __fastcall SetFlags(void*,void*,uint32_t);
// TS3W.exe 0x0051B280: modify flags using only the low byte of the set argument.
uintptr_t __fastcall ModifyFlags(void*,void*,uint32_t,uint32_t);
// TS3W.exe 0x0051B2B0: forward selection to the adjusted secondary then refresh the primary.
uintptr_t __fastcall Select(void*,void*,uint32_t);
// TS3W.exe 0x0051B2F0: return the raw state byte, not a normalized bool.
uint8_t __fastcall State(void*,void*);
}
