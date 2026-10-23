#pragma once
#include "ts3/render/m07_abi.h"
#include <cstddef>
#include <cstdint>
namespace ts3::render::m07::m02 {
// Optional native dispatch for dependencies and sparse globals. No x86 pointer truncation.
struct Dependencies { void* (*function)(uint32_t)=nullptr; void* (*global)(uint32_t)=nullptr; };
extern Dependencies dependencies;
struct Joint { uint32_t words[5]; };
struct alignas(16) Drawable {
    void* attached;
    void* streams[2]; uint32_t streamWords[2];
    void* vertexFormat; void* indices; uint32_t word1C;
    void* resource; void* atomicResource; void* ref28; void* ref2C; void* ref30; void* ref34;
    uint32_t words38[6]; Joint* joints; uint32_t jointCount;
    void* map; void* rig; uint32_t mapCount; void* ref64;
    uint32_t words68[3]; uint32_t word74,word78; uint16_t word7C,pad7E;
    alignas(16) uint8_t matrices[64]; uint8_t flags[3],padC3; uint32_t wordC4,wordC8;
};
struct Node { Node* next; Node* prev; };
struct Mediator {
    void* primary; void** table; int32_t refs; uint8_t opaque0C[44];
    void* texture; void* tagged; void* builder; uint8_t opaque44[36];
    uint32_t state,word6C,word70; int32_t pending; uint32_t key0,key1; int16_t selector; uint16_t pad82;
};
struct Entry { Entry* next; void* unknown; uint32_t key[4]; Node node; Mediator* mediator; uint32_t release; uint8_t enabled; uint8_t pad[3]; };
struct Compositor {
    uint8_t opaque00[60]; Node active,parked; uint32_t pending;
    uint8_t opaque50[596]; Entry* buckets[521]; Entry* sentinel;
};
struct RefArray { Mediator** begin; Mediator** end; Mediator** capacity; const char* tag; };
struct ParamEntry { uint32_t refs; uint16_t revision,pad; uint32_t unknown; void* value; };
struct ParamManager { void* table; ParamEntry* entries; uint32_t count; uint8_t opaque0C[6]; uint16_t revision; };
#if defined(_M_IX86)
static_assert(offsetof(Drawable,flags)==192 && sizeof(Drawable)==208);
static_assert(offsetof(Mediator,state)==104 && offsetof(Mediator,selector)==128);
static_assert(offsetof(Entry,node)==24 && offsetof(Entry,enabled)==40);
static_assert(offsetof(Compositor,buckets)==676 && offsetof(Compositor,sentinel)==2760);
static_assert(sizeof(RefArray)==16 && sizeof(ParamEntry)==16 && offsetof(ParamManager,revision)==18);
#endif
// TS3W.exe 0x006909E0: assign drawable state, preserving ref order and the wrong-new-ref30 detach quirk.
uint32_t __fastcall AssignDrawable(Drawable*,void*,const Drawable*);
// TS3W.exe 0x006B4F70: initialize probe tables and resources on the first signed reference count.
uintptr_t InitializeProbes();
// TS3W.exe 0x006CE0D0: drain compositor queues, schedule builds and release expired textures.
Mediator** __fastcall UpdateCompositor(Compositor*,void*);
}
