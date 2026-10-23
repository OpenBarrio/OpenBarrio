#pragma once
#include "ts3/render/m07_layouts.h"
#include "ts3/render/m07_abi.h"
namespace ts3::render::m07::b20 {
struct Entry { uint32_t words[6]; };
struct Proxy {
    void* primary; void* resource; uint32_t bounds[4];
    Entry* begin; Entry* end;
    uint8_t opaque20[0x20]; void* secondary;
};
TS3_X86_OFFSET(Proxy,secondary,0x40);
struct ReleaseList { uint8_t opaque[0x80]; void** begin; void** end; };
struct ModelState { Model base; uint8_t flags[4]; float distance; uint32_t selector; uint32_t opaque3c; void* boxes; uint32_t boxCount; };
TS3_X86_OFFSET(ModelState,boxes,0x40);
struct StreamData { void* table; void* opaque; uint32_t* data; };
struct Stream { void** table; StreamData* storage; };
// TS3W.exe 0x006D7880: Query the model proxy's primary or adjusted interface.
void* __fastcall QueryModel(Proxy*,void*,uint32_t);
// TS3W.exe 0x006D78D0: Copy four bounds words in forward order and return the destination.
uint32_t* __fastcall Bounds(Proxy*,void*,uint32_t*);
// TS3W.exe 0x006D7E00: Install the scratch vtable, destroy the base, and optionally free.
void* __fastcall DestroyScratch(void*,void*,uint32_t);
// TS3W.exe 0x006D7F40: Adjust the secondary receiver before deleting the software proxy.
void* __fastcall DestroyAdjusted(void*,void*,uint32_t);
// TS3W.exe 0x006D7F50: Query the software proxy's primary or adjusted interface.
void* __fastcall QuerySoftware(Proxy*,void*,uint32_t);
// TS3W.exe 0x006D7FF0: Retain and return the resource; preserve the incidental upper EAX bits.
uintptr_t __fastcall GetResource(Proxy*,void*,void**);
// TS3W.exe 0x006D8020: Return the signed, truncating 24-byte vector count.
int32_t __fastcall Count(Proxy*,void*);
// TS3W.exe 0x006D8040: Read an unchecked entry, ID first, then four words sequentially.
uint32_t __fastcall GetEntry(Proxy*,void*,uint32_t,uint32_t*,uint32_t*);
// TS3W.exe 0x006D8280: Pop and release resources in reverse order, then invoke base cleanup.
uintptr_t __fastcall Clear(ReleaseList*,void*);
// TS3W.exe 0x006D82D0: Load versioned model data, retaining the original size checks and LOD rules.
uint8_t __fastcall LoadModel(ModelState*,void*,Stream*,void*,uint8_t);
// TS3W.exe 0x006D88E0: Adjust the secondary receiver before querying the software proxy.
void* __fastcall QueryAdjusted(void*,void*,uint32_t);
// TS3W.exe 0x006D88F0: Convert, allocate and replace software vertex data before accounting.
uint8_t __fastcall LoadVertices(SoftwareVB*,void*,uint32_t,const void*,uint32_t);
// TS3W.exe 0x006D8990: Convert, allocate and replace software index data before accounting.
uint8_t __fastcall LoadIndices(SoftwareIB*,void*,uint32_t,const void*,uint8_t);
// TS3W.exe 0x006D8A40: Install software proxy vtables, destroy its base and optionally free.
void* __fastcall DestroySoftware(Proxy*,void*,uint32_t);
// TS3W.exe 0x006D8A80: Test whether any 24-byte vector entry has the supplied ID.
int32_t __fastcall Contains(Proxy*,void*,uint32_t);
}
