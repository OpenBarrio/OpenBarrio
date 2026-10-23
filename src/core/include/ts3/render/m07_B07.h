#pragma once
#include "ts3/render/m07_interfaces.h"
namespace ts3::render::m07::b07 {
// TS3W.exe 0x0060F7B0: Set cached depth-write state; preserve low-byte truncation, not boolean normalization.
DeviceHost* SetDepthWrite(DeviceHost* self, uint32_t value);
// TS3W.exe 0x0060F7C0: Adapt engine state arguments to cached D3D render state.
DeviceHost* SetDepthCompare(DeviceHost* self, uint32_t value);
// TS3W.exe 0x0060F7E0: Set cached stencil-enable state; preserve low-byte truncation, not boolean normalization.
DeviceHost* SetStencilEnable(DeviceHost* self, uint32_t value);
// TS3W.exe 0x0060F7F0: Adapt engine state arguments to cached D3D render state.
DeviceHost* SetStencilCompare(DeviceHost* self, uint32_t value, uint32_t reference, uint32_t mask);
// TS3W.exe 0x0060F830: Read the cached stencil reference scalar.
uint32_t GetStencilReference(DeviceHost* self);
// TS3W.exe 0x0060F840: Adapt engine state arguments to cached D3D render state.
DeviceHost* SetStencilWriteMask(DeviceHost* self, uint32_t value);
// TS3W.exe 0x0060F850: Adapt engine state arguments to cached D3D render state.
DeviceHost* SetStencilOperations(DeviceHost* self, uint32_t fail, uint32_t depthFail, uint32_t pass);
// TS3W.exe 0x0060F8A0: Set cached alpha-test-enable state; preserve low-byte truncation, not boolean normalization.
DeviceHost* SetAlphaTestEnable(DeviceHost* self, uint32_t value);
// TS3W.exe 0x0060F8B0: Set alpha comparison and rounded reference; keep EA wrapping saturation. 
DeviceHost* SetAlphaCompare(DeviceHost* self, uint32_t value, float reference);
// TS3W.exe 0x0060F920: Adapt engine state arguments to cached D3D render state.
DeviceHost* SetBlendOperation(DeviceHost* self, uint32_t value);
// TS3W.exe 0x0060F940: Set cached blend-enable state; preserve low-byte truncation, not boolean normalization.
DeviceHost* SetBlendEnable(DeviceHost* self, uint32_t value);
// TS3W.exe 0x0060F950: Adapt engine state arguments to cached D3D render state.
DeviceHost* SetBlendFactors(DeviceHost* self, uint32_t source, uint32_t destination);
// TS3W.exe 0x0060F990: Adapt engine state arguments to cached D3D render state.
DeviceHost* SetFillMode(DeviceHost* self, uint32_t value);

#if defined(_M_IX86)
// TS3W.exe 0x0060F7B0: Fastcall adapter preserving the engine thiscall stack.
DeviceHost* __fastcall HookSetDepthWrite(DeviceHost* self, void*, uint32_t value);
// TS3W.exe 0x0060F7C0: Fastcall adapter preserving the engine thiscall stack.
DeviceHost* __fastcall HookSetDepthCompare(DeviceHost* self, void*, uint32_t value);
// TS3W.exe 0x0060F7E0: Fastcall adapter preserving the engine thiscall stack.
DeviceHost* __fastcall HookSetStencilEnable(DeviceHost* self, void*, uint32_t value);
// TS3W.exe 0x0060F7F0: Fastcall adapter preserving the engine thiscall stack.
DeviceHost* __fastcall HookSetStencilCompare(DeviceHost* self, void*, uint32_t value, uint32_t reference, uint32_t mask);
// TS3W.exe 0x0060F830: Fastcall adapter preserving the engine thiscall stack.
uint32_t __fastcall HookGetStencilReference(DeviceHost* self, void*);
// TS3W.exe 0x0060F840: Fastcall adapter preserving the engine thiscall stack.
DeviceHost* __fastcall HookSetStencilWriteMask(DeviceHost* self, void*, uint32_t value);
// TS3W.exe 0x0060F850: Fastcall adapter preserving the engine thiscall stack.
DeviceHost* __fastcall HookSetStencilOperations(DeviceHost* self, void*, uint32_t fail, uint32_t depthFail, uint32_t pass);
// TS3W.exe 0x0060F8A0: Fastcall adapter preserving the engine thiscall stack.
DeviceHost* __fastcall HookSetAlphaTestEnable(DeviceHost* self, void*, uint32_t value);
// TS3W.exe 0x0060F8B0: Fastcall adapter preserving the engine thiscall stack.
DeviceHost* __fastcall HookSetAlphaCompare(DeviceHost* self, void*, uint32_t value, float reference);
// TS3W.exe 0x0060F920: Fastcall adapter preserving the engine thiscall stack.
DeviceHost* __fastcall HookSetBlendOperation(DeviceHost* self, void*, uint32_t value);
// TS3W.exe 0x0060F940: Fastcall adapter preserving the engine thiscall stack.
DeviceHost* __fastcall HookSetBlendEnable(DeviceHost* self, void*, uint32_t value);
// TS3W.exe 0x0060F950: Fastcall adapter preserving the engine thiscall stack.
DeviceHost* __fastcall HookSetBlendFactors(DeviceHost* self, void*, uint32_t source, uint32_t destination);
// TS3W.exe 0x0060F990: Fastcall adapter preserving the engine thiscall stack.
DeviceHost* __fastcall HookSetFillMode(DeviceHost* self, void*, uint32_t value);
#endif
}
