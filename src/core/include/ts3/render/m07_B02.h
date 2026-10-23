#pragma once
#include "ts3/render/m07_interfaces.h"
namespace ts3::render::m07::b02 {
struct Guid { uint8_t bytes[16]; };
// Fake-device prefix: retained parent at +04; not the refcounted D3D9 prefix.
struct DevicePrefix { void* const* vtable; void* parent; };
TS3_X86_OFFSET(DevicePrefix,parent,4);
// TS3W.exe 0x006119D0: Returns success without touching the seven stack arguments.
uint32_t __stdcall Zero7(uintptr_t, uintptr_t, uintptr_t, uintptr_t, uintptr_t, uintptr_t, uintptr_t);
// TS3W.exe 0x00611A00: Returns one without inspecting self.
uint32_t __stdcall One(uintptr_t);
// TS3W.exe 0x00611A10: Returns positive zero in ST0, not EAX.
float __stdcall FloatZero(uintptr_t);
// TS3W.exe 0x00611A20: Returns success without touching the nine stack arguments.
uint32_t __stdcall Zero9(uintptr_t, uintptr_t, uintptr_t, uintptr_t, uintptr_t, uintptr_t, uintptr_t, uintptr_t, uintptr_t);
// TS3W.exe 0x00611A30: Returns E_NOTIMPL without clearing output arguments.
uint32_t __stdcall NotImplemented(uintptr_t, uintptr_t, uintptr_t, uintptr_t);
// TS3W.exe 0x00611A40: Returns success without touching the four stack arguments.
uint32_t __stdcall Zero4(uintptr_t, uintptr_t, uintptr_t, uintptr_t);
// TS3W.exe 0x00611A50: Returns the original 8876086A status without writes.
uint32_t __stdcall InvalidCall(uintptr_t, uintptr_t, uintptr_t);
// TS3W.exe 0x00611A60: Queries pixel shader or IUnknown and AddRefs before writing output.
uint32_t __stdcall QueryPixel(FakeShader*, const Guid*, void**);
// TS3W.exe 0x00611AC0: Queries vertex shader or IUnknown and AddRefs before writing output.
uint32_t __stdcall QueryVertex(FakeShader*, const Guid*, void**);
// TS3W.exe 0x00611B20: Writes retained device before invoking its COM AddRef.
uint32_t __stdcall GetDevice(FakeShader*, void**);
// TS3W.exe 0x00611B40: Installs base vtable, optionally frees, and returns self.
void* __fastcall DeleteD3D9(void*, void*, uint32_t);
// TS3W.exe 0x00611B60: Atomically increments the wrapping reference count.
uint32_t __stdcall AddRef(ResourceHeader*);
// TS3W.exe 0x00611B80: Atomically decrements; calls thiscall deleting slot 17 on zero.
uint32_t __stdcall Release(ResourceHeader*);
// TS3W.exe 0x00611BB0: Installs device table, releases parent, optionally frees, and returns self.
void* __fastcall DeleteDevice(DevicePrefix*, void*, uint32_t);
}
