#pragma once
#include "ts3/layout.h"
namespace ts3::ui::b11 {
// Sparse receiver view; most entries receive the window's secondary interface.
struct View {
 void** vtable; unsigned char gap04[0x28-4];
 int32_t refs; View* manager; View* target;
 unsigned char gap34[0x4c-0x34]; float rect[4];
 unsigned char gap5c[0x74-0x5c]; uint32_t value;
 unsigned char gap78[0x1f8-0x78]; uint32_t order; uint32_t unused1fc;
 uint32_t active; void* resource; void* data;
};
TS3_X86_OFFSET(View,refs,0x28); TS3_X86_OFFSET(View,rect,0x4c);
TS3_X86_OFFSET(View,data,0x208);
// TS3W.exe 0x00513700: Walk argument ancestors, excluding the argument itself.
uint8_t __fastcall Ancestor(View*,void*,View*);
// TS3W.exe 0x00513750: Compare manager focus slot 54 against this interface.
uint32_t __fastcall Focus(View*,void*,uint32_t);
// TS3W.exe 0x00513790: Compare manager focus slot 60 against this interface.
uint32_t __fastcall Capture(View*,void*,uint32_t);
// TS3W.exe 0x005137D0: Tail forward to target slot 90.
uintptr_t __fastcall Forward(View*,void*);
// TS3W.exe 0x005137E0: Check ancestor flags; nonzero low byte selects mask 1, otherwise 3.
uint8_t __fastcall AncestorFlags(View*,void*,uint32_t);
// TS3W.exe 0x00513950: Atomic release with temporary shutdown reference and final destruction.
uint32_t __fastcall Release(View*,void*);
// TS3W.exe 0x005139A0: Return the borrowed data pointer.
void* __fastcall Data(View*,void*);
// TS3W.exe 0x005139B0: Update value and dispatch message 12 before optional invalidation.
void __fastcall SetValue(View*,void*,uint32_t);
// TS3W.exe 0x00513A10: Update ordering and remove/reinsert in the manager.
void __fastcall SetOrder(View*,void*,uint32_t);
// TS3W.exe 0x00513A50: Retain new resource before publishing it and releasing old resource.
void __fastcall SetResource(View*,void*,void*);
// TS3W.exe 0x00513A90: Set rectangle and invalidate on unequal or unordered components.
void __fastcall SetRect(View*,void*,const float*);
// TS3W.exe 0x00513B00: Move rectangle, preserving width and height with SSE arithmetic.
void __fastcall Move(View*,void*,float,float);
// TS3W.exe 0x00513B60: Resize rectangle from its existing origin.
void __fastcall Resize(View*,void*,float,float);
}
