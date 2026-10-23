#pragma once
#include "ts3/anim/m05_layouts.h"
namespace ts3::anim::m59 {
// TS3W.exe 0x00570F80: query the secondary interface, returning the owner only for the exact identifier.
void* Query(void* self, uint32_t id);
// TS3W.exe 0x00570FA0: destroy the controller and optionally free it; returns this even after free.
Controller* Destroy(Controller* self, uint32_t flags);
// TS3W.exe 0x00570FE0: allocate and construct the 0x28-byte controller.
Controller* CreateBase(void* host);
// TS3W.exe 0x00571030: allocate the 0x48-byte controller and forward its constructor result.
void* Create48(void* host);
// TS3W.exe 0x00571060: allocate the 0x3C-byte controller and forward its constructor result.
void* Create3C(void* host);
// TS3W.exe 0x00571090: allocate the 0x5C-byte controller and forward its constructor result.
void* Create5C(void* host);
#if defined(_M_IX86)
void* __fastcall HookQuery(void* self, void*, uint32_t id);
Controller* __fastcall HookDestroy(Controller* self, void*, uint32_t flags);
Controller* __fastcall HookCreateBase(void*, void*, void* host);
void* __fastcall HookCreate48(void*, void*, void* host);
void* __fastcall HookCreate3C(void*, void*, void* host);
void* __fastcall HookCreate5C(void*, void*, void* host);
#endif
}
