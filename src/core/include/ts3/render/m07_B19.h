#pragma once
#include "ts3/render/m07_layouts.h"
namespace ts3::render::m07::b19 {
struct Drawable {
    void* root; void* p1; void* p2; uint32_t opaque0C[2];
    void* cleared14[9]; uint8_t opaque38[0x18];
    void* primitives; uint32_t primitiveCount;
    uint32_t* skin; uint32_t* rig; uint32_t jointCount;
    void* skeleton; uint32_t joint; uint32_t mappedJoint;
    void* matrices; void* p74; void* p78; uint16_t sort; uint8_t opaque7E[50];
    float sphere[4]; uint8_t flags[3]; uint8_t opaqueC3; void* tailC4; void* tailC8;
};
TS3_X86_OFFSET(Drawable,primitives,0x50);
TS3_X86_OFFSET(Drawable,matrices,0x70);
struct PooledMesh {
    void* table; uint32_t refs;
    void* vertices; void* indices;
    uint32_t vertexCount,indexCount,slot,unknown1,unknown2;
    void* backing;
};
TS3_X86_OFFSET(PooledMesh,backing,0x24);
struct PoolView {
    void* table; uint32_t unknown[2]; void* format;
    void* vb; void* ib; uint32_t vertexCount,stride,indexCount;
    uint32_t state[4]; uint32_t boneCount,binding;
};
TS3_X86_OFFSET(PoolView,vertexCount,0x18);
struct FormatView { void* table; uint8_t opaque04[20]; uint8_t* elements; uint32_t count; uint32_t* stride; };
TS3_X86_OFFSET(FormatView,elements,0x18);
struct LodRecord { void* drawable; uint32_t untouched; uint32_t words[8]; };
struct ModelView { void* table; uint8_t opaque04[0x24]; LodRecord* lods; uint32_t lodCount; uint8_t flag; uint8_t gap31[11]; uint32_t token; uint8_t tail40[8]; };
TS3_X86_OFFSET(ModelView,token,0x3C);
struct Sequence { void* begin; void* end; void* capacity; const char* tag; };
struct Builder {
    void* table; uint8_t opaque04[12]; uint8_t flags10[4];
    Sequence vb,ib; uint32_t opaque34; Sequence meshes,positionIndices,normalIndices,tangentIndices,colorIndices[2],uvIndices[8];
    Sequence vertices,current,elements; uint8_t opaque148[60];
    Sequence positions,normals,tangents,colors[2],uvs[8],lods; uint32_t opaque264;
    Sequence ranges,materials,formats; uint8_t opaque298[40]; uint8_t flags2C0[3],opaque2C3; Sequence normalArray;
};
TS3_X86_OFFSET(Builder,normalArray,708);
// TS3W.exe 0x006D3E30: initialize model-builder vectors and flags without touching unknown fields.
Builder* __fastcall ConstructBuilder(Builder*,void*,uint8_t);
struct IndexBuilder { uint8_t prefix[0x13]; uint8_t perElement; uint8_t gap[0x114]; uint32_t* current; };
TS3_X86_OFFSET(IndexBuilder,current,0x128);
struct PointerVector { void** begin; void** end; void** capacity; };
struct ColorRecord { void* begin; void* end; void* capacity; const char* tag; uint32_t unknown; uint32_t maximum; };
struct ColorVector { ColorRecord* begin; ColorRecord* end; ColorRecord* capacity; };
// TS3W.exe 0x0068E9C0: resolve drawable joints, retaining the raw final lookup return even for a missing joint.
uint32_t __fastcall BindRig(Drawable*,void*,void*);
// TS3W.exe 0x0068FA50: initialize a drawable and lazily intern its shader bindings, preserving padding.
Drawable* __fastcall ConstructDrawable(Drawable*,void*);
// TS3W.exe 0x006CF470: validate format, create pool buffers, and initialize pool allocation state.
bool __fastcall InitializePool(PoolView*,void*,FormatView*,uint32_t,uint32_t);
// TS3W.exe 0x006D0490: test model readiness or schedule unloading; only AL is a return value.
bool __fastcall Ready(ModelView*,void*);
// TS3W.exe 0x006D0650: clone model LOD payloads and retain/release the clone in EA order.
bool __fastcall Clone(ModelView*,void*,ModelView**,uint32_t);
// TS3W.exe 0x0068EBA0: replace primitive-state storage, retaining stale count when old storage is null.
bool __fastcall SetPrimitiveCount(Drawable*,void*,uint32_t);
// TS3W.exe 0x0068ED50: replace skin map and matrices; failed second allocation retains the first.
bool __fastcall SetSkin(Drawable*,void*,const void*,uint32_t);
// TS3W.exe 0x006CF8E0: free pooled backing and optionally the receiver, returning the receiver.
PooledMesh* __fastcall DeletePooled(PooledMesh*,void*,uint32_t);
// TS3W.exe 0x006CFBD0: copy vertex and index payloads into a pooled mesh.
PooledMesh* __fastcall MakePooled(PoolView*,void*,const void*,uint32_t,const void*,uint32_t,uint32_t,uint32_t);
// TS3W.exe 0x006D01E0: return the receiver for four supported interface IDs.
void* __fastcall Query(void*,void*,uint32_t);
// TS3W.exe 0x006D03C0: clear model and its variant fields, returning zero.
uint32_t __fastcall Clear(void*,void*);
// TS3W.exe 0x006D04F0: allocate and construct LODs, leaving old count unchanged on failure.
bool __fastcall AllocateLods(ModelView*,void*,uint32_t);
// TS3W.exe 0x006D53E0: copy current indices or append an independently allocated per-element copy.
void* __fastcall CopyIndices(IndexBuilder*,void*,const void*,int32_t,uint32_t,PointerVector*);
// TS3W.exe 0x006D64A0: erase or append color records; the erase path does not update end itself.
uintptr_t __fastcall ResizeColors(ColorVector*,void*,uint32_t);
}
