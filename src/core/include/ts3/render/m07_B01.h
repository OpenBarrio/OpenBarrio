#pragma once
#include "ts3/render/m07_interfaces.h"
namespace ts3::render::m07::b01 {
// Sparse fake-device prefix; the second word is a retained parent COM pointer.
struct DevicePrefix { ComHeader com; ComHeader* parent; };
struct DisplayMode { uint32_t width, height, refresh, format; };
// TS3W.exe 0x00611810: QueryInterface retains self for its own IID or IUnknown.
uint32_t __stdcall QueryD3D9(ComHeader* self,const void* iid,void** output);
// TS3W.exe 0x006118A0: QueryInterface retains self for its own IID or IUnknown.
uint32_t __stdcall QueryDevice(ComHeader* self,const void* iid,void** output);
// TS3W.exe 0x00611870: Constant-return COM stub; leaves all argument storage untouched.
uint32_t __stdcall Stub00611870(uintptr_t,uintptr_t,uintptr_t);
// TS3W.exe 0x00611880: Constant-return COM stub; leaves all argument storage untouched.
uint32_t __stdcall Stub00611880(uintptr_t,uintptr_t,uintptr_t,uintptr_t,uintptr_t);
// TS3W.exe 0x00611890: Constant-return COM stub; leaves all argument storage untouched.
uint32_t __stdcall Stub00611890(uintptr_t,uintptr_t,uintptr_t,uintptr_t,uintptr_t,uintptr_t);
// TS3W.exe 0x00611900: Constant-return COM stub; leaves all argument storage untouched.
uint32_t __stdcall Stub00611900(uintptr_t);
// TS3W.exe 0x00611970: Constant-return COM stub; leaves all argument storage untouched.
uint32_t __stdcall Stub00611970(uintptr_t,uintptr_t,uintptr_t,uintptr_t,uintptr_t,uintptr_t,uintptr_t,uintptr_t,uintptr_t);
// TS3W.exe 0x00611980: Constant-return COM stub; leaves all argument storage untouched.
uint32_t __stdcall Stub00611980(uintptr_t,uintptr_t,uintptr_t,uintptr_t,uintptr_t,uintptr_t,uintptr_t,uintptr_t,uintptr_t,uintptr_t);
// TS3W.exe 0x00611990: Constant-return COM stub; leaves all argument storage untouched.
uint32_t __stdcall Stub00611990(uintptr_t,uintptr_t,uintptr_t,uintptr_t,uintptr_t,uintptr_t,uintptr_t,uintptr_t);
// TS3W.exe 0x006119A0: Constant-return COM stub; leaves all argument storage untouched.
uint32_t __stdcall Stub006119A0(uintptr_t,uintptr_t,uintptr_t,uintptr_t,uintptr_t,uintptr_t,uintptr_t);
// TS3W.exe 0x006119B0: Constant-return COM stub; leaves all argument storage untouched.
uint32_t __stdcall Stub006119B0(uintptr_t,uintptr_t,uintptr_t);
// TS3W.exe 0x006119C0: Constant-return COM stub; leaves all argument storage untouched.
uint32_t __stdcall Stub006119C0(uintptr_t,uintptr_t);
// TS3W.exe 0x00611910: Returns and AddRefs parent, or E_POINTER for null output.
uint32_t __stdcall GetParent(DevicePrefix* self,ComHeader** output);
// TS3W.exe 0x00611940: Writes the fixed display mode regardless of swap-chain index.
uint32_t __stdcall GetDisplayMode(ComHeader*,uintptr_t,DisplayMode* output);
}
