#pragma once
#include "ts3/layout.h"
// WinUser's object-like alias collides with this original factory name.
#ifdef CreateDialog
#undef CreateDialog
#endif
namespace ts3::ui::b31 {
struct DialogState {
 void** vtable;
 uint8_t active; uint8_t pad[3]; uint32_t flags; int32_t selection;
 float zero[2]; void* resources[3];
 float geometry[60]; int32_t index; void* resource;
 char16_t *begin,*end,*capacity; const char* tag;
};
struct Dialog { void** vtable; void** secondary; uint8_t opaque[0x250-8]; DialogState state; };
struct Drawable {
 void** vtable; void** secondary; uint32_t refs; void** tertiary;
 uint32_t color,value; void* resources[10];
};
struct ForwardView { uint8_t opaque[0x20]; void* value; };
struct Record { const void* descriptor; void* owner; uint32_t flags,kind; };
TS3_X86_OFFSET(Dialog,state,0x250);
TS3_X86_OFFSET(DialogState,geometry,0x24);
TS3_X86_OFFSET(DialogState,index,0x114);
TS3_X86_OFFSET(Drawable,resources,0x18);
// TS3W.exe 0x00521BA0: Adjust dialog interface to the deleting destructor receiver.
void* __fastcall DeleteDialogState(DialogState*,void*,uint32_t);
// TS3W.exe 0x00521BB0: Adjust secondary dialog interface to the deleting destructor receiver.
void* __fastcall DeleteDialogSecondary(void*,void*,uint32_t);
// TS3W.exe 0x00521C10: Adjust drawable secondary interface for deleting destruction.
void* __fastcall DeleteDrawableSecondary(void*,void*,uint32_t);
// TS3W.exe 0x00521C20: Adjust drawable tertiary interface for deleting destruction.
void* __fastcall DeleteDrawableTertiary(void*,void*,uint32_t);
// TS3W.exe 0x00521C30: Adjust drawable secondary interface for interface query.
void* __fastcall QueryDrawableSecondary(void*,void*,uint32_t);
// TS3W.exe 0x00521C40: Adjust tertiary interface for the reference-release operation (no stack arguments).
uint32_t __fastcall ForwardTertiary(void*,void*);
// TS3W.exe 0x00521C50: Adjust drawable tertiary interface for interface query.
void* __fastcall QueryDrawableTertiary(void*,void*,uint32_t);
// TS3W.exe 0x00521C60: Initialize dialog state after the original base constructor.
Dialog* __fastcall ConstructDialog(Dialog*,void*);
// TS3W.exe 0x00521DE0: Union child bounds, reposition children and resize the dialog; asymmetrical type exclusions kept.
uintptr_t __fastcall LayoutDialog(DialogState*,void*);
// TS3W.exe 0x00522130: Initialize the dialog drawable and its ten null resources.
Drawable* __fastcall ConstructDrawable(Drawable*,void*);
// TS3W.exe 0x005221B0: Allocate a dialog and return its secondary interface, or null.
void* __stdcall CreateDialog(uint32_t,void*);
// TS3W.exe 0x005221F0: Allocate and initialize a dialog drawable, or null.
Drawable* __stdcall CreateDrawable(uint32_t,void*);
// TS3W.exe 0x00522270: Forward to argument slot 58 when the stored value exists; return AL only.
uint8_t __fastcall ForwardValue(ForwardView*,void*,void*,void*);
// TS3W.exe 0x005222A0: Return the fixed control type ID 4F8772AA.
uint32_t __fastcall ControlType(void*,void*);
// TS3W.exe 0x00522320: Populate the registration record and return the queried interface type.
uint32_t __fastcall RegisterControl(void*,void*,Record*);
}
