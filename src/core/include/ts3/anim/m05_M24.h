#pragma once
#include "ts3/anim/m05_layouts.h"
namespace ts3::anim::m24 {
struct Entry { Controller* controller; uint32_t handle, flags; float time, weight; };
struct Sequence {
    Controller base;
    void* secondary;
    Controller* transition;
    ListLink nodes;
    void* descriptor;
    Entry first, second;
    uint32_t value64, mode, flags;
};
struct Stream { void* vtable; uint32_t value4; void* context; uint32_t valueC, options; uint8_t good; };
struct Node { ListLink links; Controller* controller; };
struct Registration { ListLink links; void* delegate; int32_t mask; };
TS3_X86_SIZE(Entry,0x14);
TS3_X86_OFFSET(Sequence,first,0x3c);
TS3_X86_OFFSET(Sequence,second,0x50);
TS3_X86_OFFSET(Sequence,mode,0x68);
TS3_X86_OFFSET(Stream,good,0x14);
// TS3W.exe 0x00566070: Select the active controller and forward slot 3, publishing the mode first.
uintptr_t Forward(Sequence*,void*);
// TS3W.exe 0x005660C0: Serialize an entry, stopping on failure; preserve raw AL status and x87 NaN quieting.
uint8_t Save(Sequence*,Entry*,Stream*);
// TS3W.exe 0x00566290: Register a delegate on the entries and every non-null list controller.
void Register(Sequence*,void*);
// TS3W.exe 0x00566420: Load an entry and conditionally bind its handle; preserve raw AL stream status.
uint8_t Load(Sequence*,Entry*,Stream*);
// TS3W.exe 0x00566650: Clear an entry after handle cleanup, optionally notifying its retained controller.
void Clear(Entry*,Sequence*,bool);
// TS3W.exe 0x00566710: Extract the preferred controller and reset both entries; output is zeroed without release.
Controller** Extract(Sequence*,Controller**);
#if defined(_M_IX86)
uintptr_t __fastcall HookForward(Sequence*,void*,void*);
uint8_t __fastcall HookSave(Sequence*,void*,Entry*,Stream*);
void __fastcall HookRegister(Sequence*,void*,void*);
uint8_t __fastcall HookLoad(Sequence*,void*,Entry*,Stream*);
void __fastcall HookClear(Entry*,void*,Sequence*,bool);
Controller** __fastcall HookExtract(Sequence*,void*,Controller**);
#endif
}
