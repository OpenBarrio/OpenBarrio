#pragma once
#include "ts3/layout.h"
namespace ts3::render::m07::b37 {
using Resolver=void*(*)(uint32_t);
extern Resolver resolve;
struct Parameter { uint32_t revision; uint16_t stamp,type; uint32_t count; const void* data; };
struct Registry { void* table; Parameter* entries; uint32_t count; uint8_t pad[6]; uint16_t stamp; };
struct Batcher;
struct Library { uint32_t bytes; Batcher** begin; Batcher** end; Batcher** capacity; const char* tag; };
struct Globals { Library** library=nullptr; Registry* registry=nullptr; void* shaders=nullptr; uint8_t* options=nullptr; void* resources=nullptr; uint32_t* dimensionsId=nullptr; uint32_t* dimensionsFlags=nullptr; };
extern Globals globals;
struct Batcher { uint32_t refs; void* vb; void* ib; uint32_t opaque[2]; uint32_t vertices,triangles,indices,instances; uint8_t disabled; uint8_t pad[3]; uint32_t bytes; uint32_t mesh[6]; uint8_t tail[24]; void* source; void* retained; };
struct Instance { void* key; uint32_t count; Batcher** batches; uint32_t tail[4]; };
struct SceneList { void** table; uintptr_t untouched; void* left; void* right; void* root; uint32_t flags,count; const char* tag; };
struct RenderData { uint32_t handle; uint8_t active,dirty,pad[46]; void* object; uint8_t tail[8]; };
struct SceneObjects : SceneList { RenderData* begin; RenderData* end; RenderData* capacity; const char* renderTag; void* shader[2]; uint32_t deferred,immediate,depthId; };
struct ShadowPass { void** table; uint8_t prefix[0x20c]; void* shader; void* format; void* buffer; uint8_t middle[0x84]; void* geometry; uintptr_t unknown; uintptr_t texture[6]; uint32_t textureId; uint8_t tail[8]; uint32_t vertices; };
TS3_X86_OFFSET(Batcher,source,0x5c); TS3_X86_SIZE(Batcher,100);
TS3_X86_OFFSET(SceneObjects,begin,0x20); TS3_X86_SIZE(RenderData,64);
TS3_X86_OFFSET(ShadowPass,format,0x214); TS3_X86_OFFSET(ShadowPass,geometry,0x2a0); TS3_X86_OFFSET(ShadowPass,vertices,0x2cc);
// TS3W.exe 0x00696D90: Build repeated vertex/index data for an instancing batcher.
uintptr_t __fastcall SetSource(Batcher*,void*,void*);
// TS3W.exe 0x006972B0: Initialize the batcher library and default shader parameters.
Library* __fastcall InitLibrary(Library*,void*);
// TS3W.exe 0x006973B0: Find or create and retain an instancing batcher.
bool __fastcall FindBatcher(Library*,void*,void*,Batcher**);
// TS3W.exe 0x00697480: Initialize instance state and lazily allocate its shared library.
Instance* __fastcall InitInstance(Instance*,void*);
// TS3W.exe 0x006974F0: Rebind the instance batch array; unchanged key returns its low byte.
uint8_t __fastcall BindInstance(Instance*,void*,void*,void*);
// TS3W.exe 0x00697580: Return bit 21 of the supplied object's descriptor.
uint32_t __fastcall ShadowFlag(void*,void*,void*);
// TS3W.exe 0x00697680: Lazily create the interior shadow vertex format (AL return).
bool __fastcall EnsureFormat(ShadowPass*,void*);
// TS3W.exe 0x00697740: Lazily create the shadow VB; missing geometry leaves the allocation installed.
bool __fastcall EnsureBuffer(ShadowPass*,void*);
// TS3W.exe 0x00697E70: Initialize drop-shadow resources in original failure order.
bool __fastcall InitShadow(ShadowPass*,void*);
// TS3W.exe 0x00699210: Reset the list vtable and destroy tree nodes.
uintptr_t __fastcall DestroyList(SceneList*,void*);
// TS3W.exe 0x00699300: Render immediate scratch shadows and mark deferred records.
uintptr_t __fastcall RenderShadows(SceneObjects*,void*);
// TS3W.exe 0x00699850: Initialize the scene-object tree, preserving the unused word.
SceneList* __fastcall InitList(SceneList*,void*);
// TS3W.exe 0x00699880: Destroy and optionally free the scene-object list, returning self.
SceneList* __fastcall DeleteList(SceneList*,void*,uint32_t);
// TS3W.exe 0x006998B0: Construct the interior-shadow scene-object manager.
SceneObjects* __fastcall InitObjects(SceneObjects*,void*);
}
