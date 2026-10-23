#pragma once
#include "ts3/render/m07_abi.h"
#include <cstdint>
namespace ts3::render::m07::b41 {
using Resolver=void*(*)(uint32_t);
extern Resolver resolve;
struct Vector { void** begin; void** end; void** capacity; const char* tag; };
struct Owner { uint8_t prefix[8]; void* scene; };
struct Scene { uint8_t prefix[0x2c]; void* tree; };
struct Object { uint8_t prefix[0x40]; uint32_t flags,mask; };
struct Model { void** table; uint8_t prefix[0x1ec]; uint32_t shader; uint8_t gap[0x1c]; uint8_t variant[0x8d]; uint8_t state; };
TS3_X86_OFFSET(Scene,tree,0x2c);
TS3_X86_OFFSET(Model,shader,0x1f0);
TS3_X86_OFFSET(Model,variant,0x210);
TS3_X86_OFFSET(Model,state,0x29d);
// Sparse native views: real pointers grow on x64; no blanket packing.
struct Manager {
    void** table; uint8_t p04[4]; void* scene; uint8_t p0c[4]; void* display;
    uint8_t p14[0x10]; void* camera; uint8_t p28[0x198]; uint32_t cullFlags; uint8_t p1c4[0x9e];
    uint8_t frustumEnabled,p263[4],filterEnabled,p268[8],render,p271[0x1d];
    uint8_t temporary1,p28f[0xd],temporary2,state,p29e[4],mode,p2a3;
    uint32_t enabled; uint8_t p2a8[0x18]; uint32_t sortFlags,mask; uint8_t p2c8[0xc];
    void* first; void* second; void* names1; void* names2; uint8_t p2e4[0x44];
    uint8_t mutex1[0x1c],mutex2[0x80]; void* scratch; uint32_t capacity; uint8_t p3cc[0x108];
    uint8_t mutex3[0x1c]; void* callback; void* hash; uint8_t pending[12]; void* extraStorage;
    uint8_t p508[0xc],isolate,debug,p516[0x1a]; void* extra;
};
TS3_X86_OFFSET(Manager,frustumEnabled,0x262);
TS3_X86_OFFSET(Manager,temporary1,0x28e);
TS3_X86_OFFSET(Manager,state,0x29d);
TS3_X86_OFFSET(Manager,first,0x2d4);
TS3_X86_OFFSET(Manager,scratch,0x3c4);
TS3_X86_OFFSET(Manager,callback,0x4f0);
TS3_X86_OFFSET(Manager,extra,0x530);
struct Entry { void* object; uint32_t key; };
struct Entries { Entry* begin; Entry* end; Entry* capacity; const char* tag; };
struct SortObject { void** table; uint8_t p04[8]; uint32_t id; alignas(16) float position[4]; };
TS3_X86_OFFSET(SortObject,position,0x10);
struct Hash { uint32_t opaque; void* data; uint32_t count,used; };
struct ConstantRecord { uint32_t revision; uint16_t serial; uint8_t pad[6]; void* data; };
struct Constants { uint32_t opaque; ConstantRecord* records; uint32_t count; uint8_t p0c[6]; uint16_t serial; };
struct DisplayList { uint8_t prefix[0x110]; Entries first,second; uint8_t tail[0xf0]; };
TS3_X86_SIZE(DisplayList,0x220);
// TS3W.exe 0x006E4770: Replace the scene quadtree; a null allocation is still passed to initialization.
bool __fastcall ReplaceTree(Scene*,void*,void*,uint32_t);
// TS3W.exe 0x006E6660: Attach the in-game Sim shader LOD fixup, preserving cleanup-dependent EAX.
uintptr_t __cdecl ForceSimShaders(Model*);
// TS3W.exe 0x006E6710: Attach the isolated-head LOD fixup and set its scale/callback.
void __cdecl IsolateHead(Model*);
// TS3W.exe 0x006E7A70: Set the scheduler time and update all nonnull scheduled objects.
uintptr_t __fastcall Schedule(float*,void*,const float*);
// TS3W.exe 0x006EB770: Force matching or nearby objects to high LOD using SSE distance ordering.
uintptr_t __fastcall ForceHighLOD(Owner*,void*,uint32_t,uint32_t,const float*,int32_t,uintptr_t);
// TS3W.exe 0x006EB8C0: Notify every scene-list entry of a deleted lighting solver, including null entries.
uintptr_t __fastcall LightingDeleted(Owner*,void*,void*);
// TS3W.exe 0x006EDBB0: Query matching scene objects for either flag 02 or 40; static vector storage is reused.
bool __fastcall HasActive(void*,void*,void*,int32_t);
// TS3W.exe 0x006EEF40: Render a scene with temporary wall/floor specular override and restore state.
uintptr_t __fastcall RenderOverride(void*,void*,void*,uintptr_t,uintptr_t,uintptr_t,uintptr_t,uintptr_t,uint8_t,uintptr_t,uintptr_t,uint8_t);
// TS3W.exe 0x006F16B0: Generate render-set keys and invoke the original stable/id or distance sort.
uintptr_t __fastcall SortEntries(void*,void*,void*,uint8_t);
// TS3W.exe 0x006F1800: Destroy the scene manager, unregistering singleton callbacks before releasing storage.
uintptr_t __fastcall DestroyManager(void*,void*);
// TS3W.exe 0x006F1B00: Cull a scene, sort both render lists, then optionally notify the tool.
uintptr_t __fastcall Cull(void*,void*);
// TS3W.exe 0x006F3DC0: Find the first object matching a mask and active flags, optionally returning it.
bool __fastcall FindActive(Owner*,void*,int32_t,uint8_t,Object**,uintptr_t);
}
