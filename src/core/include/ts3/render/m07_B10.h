#pragma once
#include "ts3/render/m07_interfaces.h"
namespace ts3::render::b10 {
using Index = m07::IndexBuffer;
// These are the distinct FE9468/FE9480 views, not the smaller FE9590 stream.
struct SurfaceBuffer {
    void* primary; uint32_t refs; void* secondary; void* link; void* owner;
    uint32_t flags,width,height,tag; void* gpu;
};
struct AdjustedSurface { void* secondary; void* link; void* owner; uint32_t flags,width,height,tag; void* gpu; };
TS3_X86_OFFSET(SurfaceBuffer,gpu,0x24);
TS3_X86_OFFSET(AdjustedSurface,gpu,0x1c);
// TS3W.exe 0x00616BB0: Lock bytes; bit 4 overrides the other lock flags.
void* __fastcall LockIndex(Index*,void*,uint32_t,uint32_t,uint32_t);
// TS3W.exe 0x00616C10: Unlock the nonnull COM buffer and return its HRESULT.
int32_t __fastcall UnlockIndex(Index*,void*);
// TS3W.exe 0x00616C40: Create the COM index buffer; only AL is a defined boolean.
bool __fastcall CreateIndex(Index*,void*);
// TS3W.exe 0x00616D30: Unbind and release an existing index buffer.
void __fastcall ReleaseIndex(Index*,void*);
// TS3W.exe 0x00616DF0: Store creation parameters and register on success.
bool __fastcall InitializeIndex(Index*,void*,uint32_t,uint32_t,uint32_t);
// TS3W.exe 0x00616E30: Unregister then release an index buffer.
void __fastcall DetachIndex(Index*,void*);
// TS3W.exe 0x00616E50: Copy bytes under a COM lock; null output does not unlock.
bool __fastcall UploadIndex(Index*,void*,const void*,uint32_t);
// TS3W.exe 0x00616EE0: Construct the index wrapper, leaving flags/format untouched.
Index* __fastcall ConstructIndex(Index*,void*);
// TS3W.exe 0x00616F30: Destroy the index wrapper without freeing its allocation.
void __fastcall DestroyIndex(Index*,void*);
// TS3W.exe 0x00617960: Detach and release the FE9468 surface buffer.
void __fastcall DetachSurface(SurfaceBuffer*,void*);
// TS3W.exe 0x00617A20: Query the description and test its word at offset 0x10.
bool __fastcall SurfaceAvailable(SurfaceBuffer*,void*);
// TS3W.exe 0x00617A70: Adopt a surface and derive flags from its description.
void __fastcall AdoptSurface(SurfaceBuffer*,void*,void*,uint32_t);
// TS3W.exe 0x00617B00: Release a surface through the already-adjusted interface.
void __fastcall ReleaseSurface(AdjustedSurface*,void*);
// TS3W.exe 0x00617BB0: Store surface creation state and register on success.
bool __fastcall InitializeSurface(SurfaceBuffer*,void*,uint32_t,uint32_t,uint32_t,uint32_t);
}
