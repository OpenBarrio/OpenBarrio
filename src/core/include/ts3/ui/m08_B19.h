#pragma once
#include "ts3/layout.h"
#include <cstdint>
namespace ts3::ui::b19 {
// Sparse native views: only fields consumed by this batch are named.
struct Button {
    void** primary; void** secondary;
    uint8_t gap008[0x28]; void* manager;
    uint8_t gap034[0x21C]; void** controller;
    uint8_t initialized; uint8_t gap255[0x17]; void* index;
};
struct Drawable {
    void** primary; void** secondary; uint32_t refs;
    void** interface00C; void* resource;
};
struct Binding { void** table; void* object; uint8_t flag; };
struct Resource { void** table; uint8_t gap004[0x18]; int32_t width,height; };
struct Handle { void** table; void* object; uint32_t count,type; };
TS3_X86_OFFSET(Button,manager,0x30);
TS3_X86_OFFSET(Button,controller,0x250);
TS3_X86_OFFSET(Button,initialized,0x254);
TS3_X86_OFFSET(Button,index,0x26C);
TS3_X86_OFFSET(Drawable,resource,0x10);
TS3_X86_OFFSET(Resource,width,0x1C);
// TS3W.exe 0x00517F60: update controller flags for focus/capture; SETE preserves pointer high bytes in arguments.
uint8_t Notify(Button*,uint32_t,void*);
// TS3W.exe 0x00518030: invoke primary slot A8 and report success in AL.
uint8_t InvokeAction(Button*);
// TS3W.exe 0x00518040: return the button class identifier (constant-leaf twin).
uint32_t ButtonType(Button*);
// TS3W.exe 0x00518050: lazily initialize through the secondary interface, then bridge base rectangle work.
uintptr_t InitializeSecondary(void*);
// TS3W.exe 0x00518080: query drawable interfaces, falling back to the original base implementation.
void* QueryDrawable(Drawable*,uint32_t);
// TS3W.exe 0x005180C0: replace a retained object, releasing old before retaining new, and store the byte flag.
void SetBinding(Binding*,void*,uint8_t);
// TS3W.exe 0x00518110: return the drawable class identifier (constant-leaf twin).
uint32_t DrawableType(Drawable*);
// TS3W.exe 0x00518120: return quarter-width and height from signed resource dimensions; null leaves output untouched.
uint8_t StandardSize(Drawable*,float*,uint32_t,uint32_t);
// TS3W.exe 0x00518160: return sixth-width and height with single-precision SSE rounding.
uint8_t RadioSize(Drawable*,float*,uint32_t,uint32_t);
// TS3W.exe 0x00518240: initialize a borrowed handle and return the object's class identifier.
uint32_t MakeHandle(Button*,Handle*);
// TS3W.exe 0x00518270: lazily initialize through the secondary interface, then bridge base transform work.
uintptr_t InitializePrimary(Button*);
// TS3W.exe 0x005182D0: return the pointer at button offset 26C (field-leaf twin).
void* Index(Button*);
// TS3W.exe 0x005182E0: adjust secondary receiver to primary and bridge deleting destruction.
void* DeleteSecondary(void*,uint32_t);
// TS3W.exe 0x005182F0: adjust controller receiver to primary and bridge interface query.
void* QueryController(void*,uint32_t);
#if defined(_M_IX86)
uint8_t __fastcall HookNotify(Button*,void*,uint32_t,void*);
uint8_t __fastcall HookInvokeAction(Button*,void*);
uint32_t __fastcall HookButtonType(Button*,void*);
uintptr_t __fastcall HookInitializeSecondary(void*,void*);
void* __fastcall HookQueryDrawable(Drawable*,void*,uint32_t);
void __fastcall HookSetBinding(Binding*,void*,void*,uint8_t);
uint32_t __fastcall HookDrawableType(Drawable*,void*);
uint8_t __fastcall HookStandardSize(Drawable*,void*,float*,uint32_t,uint32_t);
uint8_t __fastcall HookRadioSize(Drawable*,void*,float*,uint32_t,uint32_t);
uint32_t __fastcall HookMakeHandle(Button*,void*,Handle*);
uintptr_t __fastcall HookInitializePrimary(Button*,void*);
void* __fastcall HookIndex(Button*,void*);
void* __fastcall HookDeleteSecondary(void*,void*,uint32_t);
void* __fastcall HookQueryController(void*,void*,uint32_t);
#endif
}
