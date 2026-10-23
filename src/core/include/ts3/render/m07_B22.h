#pragma once
#include "ts3/render/m07_abi.h"
#include "ts3/render/m07_layouts.h"
namespace ts3::render::b22 {
// TS3W.exe 0x006D9340: Allocate and initialize the renderer resource.
void* __cdecl CreateAsyncModel();
// TS3W.exe 0x006D93C0: Allocate and initialize the renderer resource.
void* __cdecl CreateModel();
// TS3W.exe 0x006D9460: Allocate and initialize the renderer resource.
void* __cdecl CreateBuffer();
// TS3W.exe 0x006D94C0: Run cleanup and set AL to one, preserving upper EAX.
uintptr_t __fastcall Unload(void* self, void*);
// TS3W.exe 0x006D94D0: Allocate and initialize the renderer resource.
void* __cdecl CreateSkin();
// TS3W.exe 0x006D9530: Destroy then conditionally free, returning the receiver.
void* __fastcall DeleteSkin(void* self, void*, uint32_t flags);
// TS3W.exe 0x006D9560: Allocate and initialize the renderer resource.
void* __cdecl CreateComposite();
// TS3W.exe 0x006D95D0: Destroy then conditionally free, returning the receiver.
void* __fastcall DeleteLod(void* self, void*, uint32_t flags);
// TS3W.exe 0x006D9600: Allocate and initialize the renderer resource.
void* __cdecl CreateLod();
// TS3W.exe 0x006D9660: Allocate and initialize the renderer resource.
void* __cdecl CreateProxyA();
// TS3W.exe 0x006D9690: Allocate and initialize the renderer resource.
void* __cdecl CreateProxyB();
// TS3W.exe 0x006D96C0: Allocate and initialize the renderer resource.
void* __cdecl CreateResourceA();
// TS3W.exe 0x006D9710: Allocate and initialize the renderer resource.
void* __cdecl CreateResourceB();
// TS3W.exe 0x006D9760: Allocate and initialize the renderer resource.
void* __cdecl CreateResourceC();
// TS3W.exe 0x006D9800: Register twelve model factories in original order, returning the final registration result.
uintptr_t __cdecl RegisterFactories();
}
