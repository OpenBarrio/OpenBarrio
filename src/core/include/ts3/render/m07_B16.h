#pragma once
#include "ts3/render/m07_abi.h"
#include "ts3/render/m07_layouts.h"
namespace ts3::render::m07::b16 {
struct Shader { void* vtable; uint16_t kind,flags; void* code; uint32_t bytes; void* handle; };
struct Storage { void* begin; void* end; void* capacity; const char* tag; void* begin2; void* end2; void* capacity2; const char* tag2; };
struct Texture { uint32_t tag; void* resource; uint8_t gap08[0x20]; float bounds[6]; uint8_t tail[0x10]; };
struct Owner { uint8_t prefix[0xbc]; void** entries; uint8_t gapC0[0x3064-0xc0]; int32_t count; uint8_t gap3068[0xc]; Texture* active; uint8_t gap3078[0x1c]; void* context; uint8_t gap3098[0x78]; Texture* begin; Texture* end; uint8_t gap3118[0x714]; void* state; uint8_t gap3830[0xb8]; uint32_t index; };
// TS3W.exe 0x0071BDD0: Return signed minimum and expose records at adjusted receiver +64.
int32_t __fastcall Records(Owner*,void*,int32_t,void**,uint32_t*);
// TS3W.exe 0x0071C120: Resolve a resource, falling back to image loading; false can still write output.
uint8_t __fastcall Resolve(void*,void*,uint32_t,uint32_t,uint32_t,void**);
// TS3W.exe 0x0071C210: Invoke selected record slot zero and return one, ignoring its result.
uint32_t __fastcall Forward(Owner*,void*,uint32_t,uint32_t,void**,uint32_t*);
// TS3W.exe 0x0071CD40: Expand texture bounds, including resource scale when unbound.
void __fastcall Bounds(Owner*,void*,int32_t,const float*,float);
// TS3W.exe 0x0071CEB0: Submit requested records and return zero, preserving early-exit state.
uint32_t __fastcall Submit(Owner*,void*,uint32_t);
// TS3W.exe 0x00799300: Initialize two tagged storage vectors.
Storage* __fastcall InitStorage(Storage*,void*);
// TS3W.exe 0x00799780: Create vertex shader; only HRESULT zero succeeds, and buffered code is always freed.
uint8_t __fastcall CreateVertex(Shader*,void*,const void*);
// TS3W.exe 0x00799810: Create pixel shader; only HRESULT zero succeeds, and buffered code is always freed.
uint8_t __fastcall CreatePixel(Shader*,void*,const void*);
// TS3W.exe 0x00799870: Destroy base shader storage, leaving byte count unchanged.
Shader* __fastcall DestroyBase(Shader*,void*,uint32_t);
// TS3W.exe 0x00799A10: Release vertex handle before destroying base storage.
Shader* __fastcall DestroyVertex(Shader*,void*,uint32_t);
// TS3W.exe 0x00799A60: Clone vertex wrapper and AddRef nonnull COM handle.
uint32_t __fastcall CloneVertex(Shader*,void*,Shader**);
// TS3W.exe 0x00799AD0: Release pixel handle before destroying base storage.
Shader* __fastcall DestroyPixel(Shader*,void*,uint32_t);
// TS3W.exe 0x00799B20: Clone pixel wrapper and AddRef nonnull COM handle.
uint32_t __fastcall ClonePixel(Shader*,void*,Shader**);
#if defined(_M_IX86)
static_assert(sizeof(Shader)==0x14 && sizeof(Texture)==0x50);
static_assert(offsetof(Owner,count)==0x3064 && offsetof(Owner,begin)==0x3110 && offsetof(Owner,index)==0x38e8);
#endif
}
