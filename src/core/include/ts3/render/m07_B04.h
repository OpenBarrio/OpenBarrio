#pragma once
#include "ts3/render/m07_layouts.h"
#include "ts3/render/m07_abi.h"
namespace ts3::render::m07::b04 {
// Local refinement of the scaffold's opaque host fields. All handles stay native pointers.
struct Host {
 uint8_t prefix[8]; void* d3d9; void* device;
 uint8_t gap10[0x14]; uint8_t windowed; uint8_t gap25[0x1b];
 uint32_t adapter; uint32_t word44; float scalar;
 uint8_t gap4c[0x34]; IndexBuffer* indexBuffer;
 uint8_t gap84[0x10]; void* handle94;
 uint8_t gap98[0x3c]; uint8_t instancing; uint8_t gapd5[0xb];
 DevicePrivate* privateData; uint8_t gape4[0x10]; uint32_t* stack;
 uint32_t index,capacity; uint8_t gap100[0x20]; void* textures[16];
 uint32_t desired[203]; uint32_t applied[203];
 uint32_t dirty[7],unused[7],invalid[7],samplerDirty;
 struct Sampler { uint32_t desired[13],applied[13],dirty,unused,invalid; } samplers[16];
};
TS3_X86_OFFSET(Host,scalar,0x48);
TS3_X86_OFFSET(Host,privateData,0xe0);
TS3_X86_OFFSET(Host,desired,0x160);
TS3_X86_OFFSET(Host,applied,0x48c);
TS3_X86_OFFSET(Host,dirty,0x7b8);
TS3_X86_OFFSET(Host,invalid,0x7f0);
TS3_X86_OFFSET(Host,samplers,0x810);
struct Saved { void* target; void* depth; int32_t x,y,width,height; float minZ,maxZ; int32_t scissor[4]; };
// TS3W.exe 0x0060E410: Restore saved surfaces, viewport and scissor; release consumed references.
void Restore(Host*, Saved*);
// TS3W.exe 0x0060E590: Check multisample support; only AL is defined.
bool Check(Host*, uint32_t);
// TS3W.exe 0x0060E5F0: Search downwards for a supported sample mode; zero is not queried.
uint32_t Find(Host*, uint32_t);
// TS3W.exe 0x0060E6F0: Read scalar +48 (adapted verified getter twin).
float Scalar(Host*);
// TS3W.exe 0x0060E740: Probe INST support; preserve exact-zero HRESULT check.
uintptr_t Instancing(Host*);
// TS3W.exe 0x0060E860: Cache render state and save its first prior value, even when stack full.
Host* RenderState(Host*, uint32_t, uint32_t);
// TS3W.exe 0x0060E900: Cache sampler state and save its first prior value.
Host* SamplerState(Host*, uint32_t, uint32_t, uint32_t);
// TS3W.exe 0x0060EAD0: Submit dirty non-invalid states; retain dirty masks when all bits invalid.
void* Flush(Host*);
// TS3W.exe 0x0060EC50: Push invalid masks; require 25 free words but consume only 24.
Host* Push(Host*);
// TS3W.exe 0x0060ED00: Reset stack and invalidate masks; return end-of-sampler cursor.
void* Invalidate(Host*);
// TS3W.exe 0x0060EDA0: Unbind all matching texture aliases without releasing them.
void Unbind(Host*, void*);
// TS3W.exe 0x0060EE30: Forward index-buffer format query through the original dependency.
uint32_t IndexFormat(Host*);
// TS3W.exe 0x0060EE40: Read handle +94 (adapted verified getter twin).
void* Handle(Host*);
// TS3W.exe 0x0060EE50: Copy rectangle then dispatch device stdcall slot +12C.
int32_t Scissor(Host*, const int32_t*);

// ABI-only thiscall adapters (ECX self, unused EDX, callee pops stack args).
inline void __fastcall HookRestore(Host* h, void*, Saved* s) { return Restore(h, s); }
inline bool __fastcall HookCheck(Host* h, void*, uint32_t n) { return Check(h, n); }
inline uint32_t __fastcall HookFind(Host* h, void*, uint32_t n) { return Find(h, n); }
inline float __fastcall HookScalar(Host* h, void*) { return Scalar(h); }
inline uintptr_t __fastcall HookInstancing(Host* h, void*) { return Instancing(h); }
inline Host* __fastcall HookRenderState(Host* h, void*, uint32_t n, uint32_t v) { return RenderState(h, n,v); }
inline Host* __fastcall HookSamplerState(Host* h, void*, uint32_t i, uint32_t n, uint32_t v) { return SamplerState(h, i,n,v); }
inline void* __fastcall HookFlush(Host* h, void*) { return Flush(h); }
inline Host* __fastcall HookPush(Host* h, void*) { return Push(h); }
inline void* __fastcall HookInvalidate(Host* h, void*) { return Invalidate(h); }
inline void __fastcall HookUnbind(Host* h, void*, void* t) { return Unbind(h, t); }
inline uint32_t __fastcall HookIndexFormat(Host* h, void*) { return IndexFormat(h); }
inline void* __fastcall HookHandle(Host* h, void*) { return Handle(h); }
inline int32_t __fastcall HookScissor(Host* h, void*, const int32_t* r) { return Scissor(h, r); }
}
