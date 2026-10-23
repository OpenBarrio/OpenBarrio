#pragma once
#include "ts3/render/m07_abi.h"
#include "ts3/render/m07_layouts.h"
namespace ts3::render::m07::b36 {
using Resolver=void*(*)(uint32_t);
extern Resolver resolve;
struct Binding { uint32_t revision; uint16_t stamp; uint16_t pad; uint32_t unused; void* data; };
struct Parameters { void* table; Binding* bindings; uint32_t count; uint8_t pad[6]; uint16_t stamp; };
struct Globals { void* host; Parameters* parameters; void* shaders; };
extern Globals globals;
struct alignas(16) Vector { uint32_t word[4]; };
struct Positions { Vector* begin; Vector* end; Vector* capacity; const char* tag; };
struct Node { Node* left; Node* parent; Node* right; uint32_t key; void* value; };
struct ShadowManager { void** table; uint32_t pad04; Node* headLeft; Node* headRight; uint8_t pad10[16]; Positions positions; };
struct Genie { void** table; uint32_t refs; uint8_t enabled; uint8_t pad09[3]; void* cached; };
struct Gamma { uint8_t ready; uint8_t pad01[3]; uint32_t enable; void* shader; uint32_t blit; float data[4]; };
struct Instancing { void** table; uint32_t pad04; uint32_t pad08; uint32_t size; uint32_t capacity; uint8_t* first; uint8_t* second; };
struct Shadow { void** table; uint8_t pad04[0x20c]; void* shader; void* format; void* buffer; uint8_t pad21c[0x44]; void* pool; uint8_t pad264[4]; uint8_t texture[0x18]; uint32_t textureId; uint32_t pad284; uint32_t locations; };
TS3_X86_OFFSET(Shadow,buffer,0x218);
TS3_X86_OFFSET(Shadow,pool,0x260);
TS3_X86_OFFSET(Shadow,textureId,0x280);
TS3_X86_OFFSET(ShadowManager,positions,0x20);
TS3_X86_OFFSET(Genie,cached,0xc);
TS3_X86_OFFSET(Instancing,first,0x14);
// Refined constructor-only view of the scaffold's opaque RenderSystem regions.
struct TaggedVector { void* begin; void* end; void* capacity; const char* tag; };
struct System {
 void* primary; uint8_t pad04[8]; void* secondary; int32_t refs; uint8_t pad14[0xdd]; uint8_t ready; uint8_t padf2[2];
 void* shadowPool; void* shadowFormat; void* dropPool; void* dropFormat; void* instanceVB;
 uint32_t zero108; uint32_t invalid; void* zero110[13]; uint32_t untouched144;
 uint8_t blob148[24]; uint8_t blob160[24]; uint8_t flags178[12]; void* zero184[2]; uint32_t untouched18c;
 uint8_t mutex190[0x30]; void* jobs[10]; const char* jobsTag; uint32_t untouched1ec; uint8_t mutex1f0[0x30];
 void* condemnedNext; void* condemnedPrev; uint32_t condemnedCount; void* condemnedStorage; uint32_t condemnedCapacity; const char* condemnedTag;
 TaggedVector settings; uint32_t zero248; uint8_t tail[0x100];
};
TS3_X86_OFFSET(System,shadowPool,0xf4); TS3_X86_OFFSET(System,blob148,0x148);
TS3_X86_OFFSET(System,mutex190,0x190); TS3_X86_OFFSET(System,condemnedNext,0x220);
TS3_X86_OFFSET(System,tail,0x24c);
// TS3W.exe 0x006E1D70: Create cube-strip targets and register them, retaining original cleanup return.
uintptr_t __fastcall CreateTargets(void*,void*,void*);
// TS3W.exe 0x006E3250: Construct the render system, leaving uninitialized holes untouched.
System* __fastcall ConstructSystem(System*,void*);
// TS3W.exe 0x00692480: Lazily allocate and fill interior shadow vertices; lock failure still succeeds.
uint8_t __fastcall CreateVertices(Shadow*,void*);
// TS3W.exe 0x006925E0: Initialize exterior sim drop shadow shader and texture bindings.
uint8_t __fastcall InitializeShadow(Shadow*,void*);
// TS3W.exe 0x00692750: Copy two aligned bounds vectors in load/store order and return destination.
void* __fastcall CopyBounds(void*,void*,void*);
// TS3W.exe 0x00692790: Construct the exterior blob shadow manager and its positions vector.
ShadowManager* __fastcall ConstructManager(ShadowManager*,void*);
// TS3W.exe 0x00692B40: Destroy positions and base manager, optionally freeing the receiver.
ShadowManager* __fastcall DeleteManager(ShadowManager*,void*,uint32_t);
// TS3W.exe 0x00692D20: Collect at most 100 accepted shadow positions, returning the last iterator.
uintptr_t __fastcall CollectPositions(ShadowManager*,void*);
// TS3W.exe 0x006940A0: Lazily bind gamma-blit parameters and shader.
void __fastcall InitializeGamma(Gamma*,void*);
// TS3W.exe 0x006946D0: Query the two supported GenieFixup interfaces without refcounting.
void* __fastcall QueryGenie(Genie*,void*,uint32_t);
// TS3W.exe 0x00694730: Release the cached LOD and optionally free the GenieFixup receiver.
Genie* __fastcall DeleteGenie(Genie*,void*,uint32_t);
// TS3W.exe 0x00694770: Apply GenieFixup to a LOD; preserve incidental EAX high bytes.
uintptr_t __fastcall FixupGenie(Genie*,void*,void*,void**,void*,void*);
// TS3W.exe 0x006948E0: Format the GenieFixup enabled byte, not the whole flags word.
uintptr_t __fastcall DescribeGenie(Genie*,void*,void*);
// TS3W.exe 0x00696B80: Resize paired instance arrays; preserve wrapped doubling and unreachable zero-free branch.
void __fastcall ResizeInstances(Instancing*,void*,uint32_t);
}
