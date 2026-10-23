#pragma once
#include "ts3/render/m07_layouts.h"
#include "ts3/render/m07_abi.h"
#include "ts3/renderer/clu_798e30_Renderer.h"
namespace ts3::render::m07::b24 {
// Expanded native views of the opaque material scaffold regions. Pointer slots
// expand on x64; wire offsets are asserted below, never used as native offsets.
struct Resource { void** table; volatile long refs; void* fields[8]; };
struct Entry { uint32_t key; uint8_t gap04[16]; void* value; uint8_t gap18[6]; uint8_t type,pad; };
struct Items {
    Resource base; uint32_t helper[3]; void* state34; void* state38;
    void* state3c; void* state40; void* state44; uint32_t depth;
    void* next; void* prev; void* words54[4]; Entry* begin; Entry* end; Entry* capacity;
    const char* tag; uint32_t selector;
};
struct Definition { Resource base; void* word28; void* word2c; void* parameters; void* word34; Items* cached; uint8_t flags[2]; uint8_t tail[2]; };
struct Env { uint8_t untouched[8]; void* begin; void* end; void* capacity; const char* tag; ts3::renderer::ItemList list; };
struct Binding { void** table; volatile long refs; uint8_t untouched[48]; Env env; uint32_t tail; };
// Resolver permits relocated-image differential tests and native injected fakes.
// Null uses the typed original x86 bridge, and fails explicitly on x64.
extern void* (*resolve)(uint32_t);
struct BindingTable { uint8_t opaque[4]; void* table; uint32_t one,count; float loadFactor,maxLoadFactor; void* storage; const char* tag; uint8_t mutex[24]; };
// TS3W.exe 0x006C9970: Replace a material texture entry and preserve release return.
uintptr_t __fastcall SetTexture(Items*,void*,Entry*,void*);
// TS3W.exe 0x006C9E20: Resolve deferred textures; the type-4 sentinel path does not advance.
uintptr_t __fastcall ResolveTextures(Items*,void*,void*);
// TS3W.exe 0x006C9F70: Construct unpacked material items, leaving padding untouched.
Items* __fastcall InitItems(Items*,void*);
// TS3W.exe 0x006CA080: Clear items and overwrite only AL of the dependency return.
uintptr_t __fastcall UnloadItems(Items*,void*);
// TS3W.exe 0x006CA0F0: Destroy items and optionally free, returning the receiver.
Items* __fastcall DeleteItems(Items*,void*,uint32_t);
// TS3W.exe 0x006CA7D0: Select cached shader binding or construct a new one.
void* __fastcall SelectBinding(Items*,void*,void*,void*,Binding**);
// TS3W.exe 0x006CAC30: Lazily create the cached material and AddRef its output.
uint8_t __fastcall GetMaterial(Definition*,void*,Items**,void*);
// TS3W.exe 0x006CADC0: Release definition state; AL is one, empty path retains incoming EAX.
uintptr_t __fastcall ClearDefinition(Definition*,void*);
// TS3W.exe 0x006CAF20: Release definition state and destroy the resource base.
uintptr_t __fastcall DestroyDefinition(Definition*,void*);
// TS3W.exe 0x006CAFD0: Destroy and optionally free a substituted material definition.
Definition* __fastcall DeleteDefinition(Definition*,void*,uint32_t);
// TS3W.exe 0x006CB1E0: Compose a substituted material definition from parameter overrides.
uint8_t __fastcall Compose(Definition*,void*,const void*,uint32_t,uint8_t,uint8_t,Definition**);
// TS3W.exe 0x006CB680: Initialize a binding environment, preserving its first eight bytes.
Env* __fastcall InitEnv(Env*,void*);
// TS3W.exe 0x006CB6B0: Initialize a reference-counted binding with an embedded environment.
Binding* __fastcall InitBinding(Binding*,void*);
// TS3W.exe 0x006CBD00: Initialize the material binding table and recursive lock.
BindingTable* __fastcall InitBindings(BindingTable*,void*);
#if defined(_M_IX86)
static_assert(sizeof(Resource)==0x28 && sizeof(Items)==0x78 && sizeof(Definition)==0x40);
static_assert(sizeof(Entry)==32 && offsetof(Binding,env)==0x38 && sizeof(Binding)==156);
static_assert(offsetof(Items,begin)==0x64 && offsetof(Env,list)==24);
#endif
}
