#pragma once
#include "ts3/render/m07_abi.h"
#include <cstdint>
namespace ts3::render::m07::b25 {
// Sparse SceneModel view: unknown members are not interpreted as native pointers.
struct Model {
    void** vtable;
    uint8_t p04[12];
    void* owner0; void* owner1;
    uint8_t p18[24];
    void* context;
    uint8_t p34[12];
    uint32_t flags;
    uint32_t flags44;
    uint8_t p48[8];
    uint8_t bounds[32];
    uint8_t p70[16];
    uint8_t matrix[64];
    uint8_t pC0[0x130];
    uint32_t selector;
    uint8_t p1F4[4];
    void* begin;
    void* end;
    uint8_t p200[16];
    uint8_t handle[24];
    void* pending;
    uint8_t p22C[0x3c];
    Model* self;
    uint8_t p26C[0x1c];
    void* parts; void* partsEnd; void* partsCapacity; const char* partsTag;
    void* scratch;
    uint8_t state0, state1, state2, state3;
    uint32_t a0,a4,a8,ac;
    uint16_t b0,b2;
    uint32_t b4,b8;
    uint8_t tail[0x14];
};
TS3_X86_OFFSET(Model,flags,0x40);
TS3_X86_OFFSET(Model,bounds,0x50);
TS3_X86_OFFSET(Model,matrix,0x80);
TS3_X86_OFFSET(Model,selector,0x1f0);
TS3_X86_OFFSET(Model,handle,0x210);
TS3_X86_OFFSET(Model,pending,0x228);
TS3_X86_OFFSET(Model,scratch,0x298);
TS3_X86_OFFSET(Model,state1,0x29d);
struct Binding { uint8_t p00[24]; void* shader; uint8_t p1C[8]; uint32_t error; };
struct Lod { uint8_t p00[56]; uint32_t active; uint8_t p3C[4]; void** routes; };
struct Dependencies {
    Thiscall<uintptr_t,void,Binding*> bind=nullptr;
    Thiscall<void*,void,const char*,uint32_t,uint32_t> shader=nullptr;
    void* shaderManager=nullptr;
    void* (*system)()=nullptr;
    Thiscall<uint32_t,Model> flags=nullptr;
    Thiscall<Lod*,void> lod=nullptr, lodResolved=nullptr;
    Thiscall<uintptr_t,void> route=nullptr;
    Thiscall<uintptr_t,Model,int32_t> fallback=nullptr;
    Thiscall<const void*,Lod> bounds=nullptr;
    Thiscall<uintptr_t,Model,const void*> matrixChanged=nullptr;
    Thiscall<uintptr_t,void,int32_t,void*> inverse=nullptr, transform=nullptr;
    Thiscall<uintptr_t,Model> refresh=nullptr;
    Thiscall<uintptr_t,void> dirty=nullptr;
};
extern Dependencies dependencies;
// TS3W.exe 0x006F4D40: Select the pick/error shader and bind unless flag 0x1000 inhibits it.
uintptr_t __fastcall Bind(Model*,void*,void*,Binding*,const void*);
// TS3W.exe 0x006F4DD0: Change bit 3 and notify slot 0x90; the entry has an AL-only return path.
uint8_t __fastcall SetHidden(Model*,void*,uint8_t);
// TS3W.exe 0x006F4E10: Recompute bit 2 from renderer, LOD and model flags.
uintptr_t __fastcall UpdateFlags(Model*,void*);
// TS3W.exe 0x006F4E90: Resolve a LOD route or use the inherited fallback.
uintptr_t __fastcall Route(Model*,void*,int32_t);
// TS3W.exe 0x006F5000: Replace the thumbnail fixup when the selector changes.
uintptr_t __fastcall SetSelector(Model*,void*,uint32_t,int32_t);
// TS3W.exe 0x006F5130: Copy bounds and matrix, optionally substituting resolved LOD bounds.
uintptr_t __fastcall GetBounds(Model*,void*,void*,void*);
// TS3W.exe 0x006F51C0: Suppress unchanged matrices only when the part range is nonempty.
uintptr_t __fastcall SetMatrix(Model*,void*,const void*);
// TS3W.exe 0x006F5280: Delegate to scratch or write identity; preserve pointer-derived high EAX bits.
uintptr_t __fastcall Inverse(Model*,void*,int32_t,void*);
// TS3W.exe 0x006F5340: Delegate to scratch or write identity; preserve pointer-derived high EAX bits.
uintptr_t __fastcall Transform(Model*,void*,int32_t,void*);
// TS3W.exe 0x006F5B40: Construct the scene-model base, LOD handle and local state, leaving padding alone.
Model* __fastcall Construct(Model*,void*,void*,void*);
// TS3W.exe 0x006F5C40: Destroy scratch, parts, LOD handle and base in that order.
uintptr_t __fastcall Destroy(Model*,void*);
// TS3W.exe 0x006F5D30: Clone a scene model and its LOD resource; the return is AL-only.
uint8_t __fastcall Clone(Model*,void*,Model**);
struct Compositor {
    uint8_t p00[56]; void* texture;
    uint8_t p3C[48]; uint32_t height,width;
    uint8_t p74[16]; uint8_t flags;
};
// TS3W.exe 0x006CC140: Allocate and install a compositor texture, then create and mark its payload.
void* __fastcall CreateTexture(Compositor*,void*);
struct Composition {
    uint8_t p00[8]; void* link;
    uint8_t p0C[12]; void* material;
    uint32_t key0,key1;
    void* texture; void* opaque28; uintptr_t resource;
    void* animated; void* target;
    uint32_t key2,key3;
    uint16_t index; uint8_t flags,p43; volatile long state;
};
TS3_X86_OFFSET(Composition,state,0x44);
// TS3W.exe 0x006CED00: Process a ready material-composition job and preserve branch-dependent raw returns.
uintptr_t __fastcall Compose(Composition*,void*);
}

