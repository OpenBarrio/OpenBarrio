#pragma once
#include "ts3/render/m07_layouts.h"
#include "ts3/render/m07_abi.h"
namespace ts3::render::m07::b14 {
// TS3W.exe 0x00619BC0: Construct the GPU texture, leaving uninitialized holes untouched.
GpuTexture* __fastcall Construct(GpuTexture*,void*);
// TS3W.exe 0x00619C00: Delete through the secondary interface (this minus eight).
GpuTexture* __fastcall DeleteInterface(void*,void*,uint32_t);
// TS3W.exe 0x00619C10: Unlink, release GPU state, and notify registered texture observers.
uintptr_t __fastcall Destruct(GpuTexture*,void*);
// TS3W.exe 0x00619DB0: Return wrapper plus conditional backing size with uint32 wrapping.
uint32_t __fastcall TotalSize(void*,void*);
// TS3W.exe 0x00619DD0: Return zero for flag 100, otherwise the backing size.
uint32_t __fastcall BackingSize(void*,void*);
// TS3W.exe 0x00619DF0: Destruct and optionally free, returning the former object address.
GpuTexture* __fastcall Delete(GpuTexture*,void*,uint32_t);
// TS3W.exe 0x00619E70: Return vertex-format storage size with wrapping multiplication.
uint32_t __fastcall FormatSize(void*,void*);
// TS3W.exe 0x00619F00: Return vertex-stream storage size with wrapping addition.
uint32_t __fastcall StreamSize(void*,void*);
// TS3W.exe 0x00722C20: Allocate and construct a named texture resource, or return null.
void* __fastcall Create(void*,void*);
// TS3W.exe 0x00722C80: Retain an existing texture stream or construct an owning stream adapter.
void __fastcall Convert(void*,void*,void*,void**);
// TS3W.exe 0x009E6950: Allocate the InGameAd-named texture resource, or return null.
void* __fastcall CreateAd(void*,void*);
// TS3W.exe 0x009E6980: Decode an ad image and upload it, falling back on any failure; result is AL-only.
uint8_t __fastcall LoadAd(void*,void*,void*,TextureResource*);
}
