#pragma once
#include "ts3/render/m07_abi.h"
#include <cstddef>
#include <cstdint>
namespace ts3::render::m07::b30 {
struct Object { void** table; };
struct Fixup { void** table; int32_t refs; uint32_t words[8]; uint8_t flags; uint8_t tail[7]; };
struct Model { void** table; uint8_t gap[0x210-4]; uint8_t dirty[4]; };
struct DebugInfo { char* begin; char* end; char* capacity; const char* tag; };
struct Floor { uint8_t prefix[0x94]; Model* models[2]; uint8_t gap[4]; uint32_t params[8]; uint8_t gap2[0x128-0xc0]; uint32_t alternate[4]; };
struct Roof { uint8_t prefix[0x30]; Model* model; };
struct TerrainItem { uint32_t unused; float height; uint8_t gap[0x28-8]; uint32_t params[8]; uint8_t gap2[16]; Model* model; };
struct alignas(16) TerrainState { uint8_t gap[0x220]; float vector[4]; };
struct Terrain { uint8_t prefix[0x20]; TerrainState* state; };
struct Node { void* header[2]; uint8_t value[48]; }; // Links use a pointer at value+32; cached values use all 48 bytes.
struct Cursor { Node* node; Node** bucket; Node* sentinel; };
struct Map { uint32_t unknown; Node** buckets; uint32_t index; uint8_t tail[20]; };
struct Bindings { uint8_t prefix[0x40]; Map objects,values,links; };
struct Dependencies {
    void* (__cdecl *allocate)(uint32_t,uint32_t,uint32_t,const char*,uint32_t,uint32_t,uint32_t,uint32_t)=nullptr;
    Thiscall<Object*,void,void*,void*> constructors[4]{};
    Thiscall<uint32_t,void,Object*> attach=nullptr;
    Thiscall<void,Object,int32_t> lod=nullptr;
    void (__cdecl *assign)(DebugInfo*,const char*)=nullptr;
    Thiscall<uintptr_t,void> dirty=nullptr, deferred=nullptr;
    Thiscall<uint32_t,void,const void*> equal=nullptr;
    Thiscall<Object*,void,const void*,const void*> texture=nullptr;
    Thiscall<Object*,void,float> distant=nullptr;
    Thiscall<uintptr_t,Map,Cursor*,void**> findValue=nullptr,findLinks=nullptr;
    Thiscall<uintptr_t,Map,Cursor*,const void*> findObject=nullptr;
    void* (*data)(uint32_t)=nullptr;
};
extern Dependencies dependencies;
// TS3W.exe 0x00700E50: Allocate, initialize and attach a scene model with LOD -2.
uint8_t __fastcall CreateModel(void*,void*,void*,void*,Object**);
// TS3W.exe 0x00700EE0: Allocate, initialize and attach a fur model with LOD -2.
uint8_t __fastcall CreateFur(void*,void*,void*,void*,Object**);
// TS3W.exe 0x00700F70: Create an instance, transferring its reference only when output exists.
uint8_t __fastcall CreateInstance(void*,void*,void*,void*,Object**);
// TS3W.exe 0x00701000: Create a model array, transferring its reference only when output exists.
uint8_t __fastcall CreateArray(void*,void*,void*,void*,Object**);
// TS3W.exe 0x00A51770: Initialize and return Clouds debug information.
DebugInfo* __fastcall DebugClouds(void*,void*,DebugInfo*);
// TS3W.exe 0x00A9DC70: Install floor edge highlight and optionally remove the keyed fixup.
void __fastcall FloorHighlight(Floor*,void*,uint8_t);
// TS3W.exe 0x00A9E030: Refresh both floor texture fixups; allocation failure retains EA's null dereference.
void __fastcall FloorTexture(Floor*,void*);
// TS3W.exe 0x00B27070: Update cached texture parameters and each linked scene model.
uintptr_t __fastcall UpdateBindings(Bindings*,void*,void*,const uint32_t*);
// TS3W.exe 0x00BA0610: Install roof highlight and optionally remove the keyed fixup.
void __fastcall RoofHighlight(Roof*,void*,uint8_t);
// TS3W.exe 0x00C0F030: Initialize and return Sea debug information.
DebugInfo* __fastcall DebugSea(void*,void*,DebugInfo*);
// TS3W.exe 0x00C16340: Initialize and return Sky debug information.
DebugInfo* __fastcall DebugSky(void*,void*,DebugInfo*);
// TS3W.exe 0x00C206E0: Install distant terrain height fixup using two single precision operations.
uint8_t __fastcall DistantTerrain(Terrain*,void*,TerrainItem*);
// TS3W.exe 0x00C208D0: Install or remove terrain texture fixup, preserving immediate/deferred dispatch.
void __stdcall TerrainTexture(TerrainItem*,uint8_t);
#if defined(_M_IX86)
static_assert(offsetof(Floor,models)==0x94 && offsetof(Floor,params)==0xa0 && offsetof(Floor,alternate)==0x128);
static_assert(offsetof(TerrainItem,model)==0x58 && offsetof(Model,dirty)==0x210);
static_assert(offsetof(Node,value)==8 && sizeof(Map)==0x20);
static_assert(offsetof(Fixup,flags)==0x28);
#endif
}
