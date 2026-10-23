#pragma once
#include "ts3/render/m07_abi.h"
#include "ts3/renderer/clu_58aad0_Renderer.h"
namespace ts3::render::m07::b39 {
struct Lookup { void* buckets; uint32_t capacity,count; float low,high; void* data; const char* tag; uint32_t untouched; };
struct MaterialLookup { void* buckets; uint32_t capacity,count; float low,high; void* data; const char* tag; };
struct Resources { void** table; uint32_t untouched; Lookup maps[3]; MaterialLookup fourth; void* first; void* last; void* limit; const char* tag; };
struct Vector { void* first; void* last; void* limit; const char* tag; };
struct Swarm {
    void** table; Resources resources; void** shader; void** bindings; void** lifetime; uint32_t refs;
    uint32_t ids[7],untouchedC4; void* handles[4]; Vector params,textures;
    uint32_t count; uint8_t particles[0x3100-0xfc];
    uint32_t capacity,state,index,cursor; Vector textureRecords,otherRecords;
    void* handle3130; Vector matrices,records; uint32_t state3154; uint8_t gap3158[8];
    ts3::renderer::Camera camera;
    void* handle3890; uint8_t enabled,padding3895[3]; void* small[5]; uint32_t untouched38ac;
    void* manager; void* handle38b4; uint8_t flag,padding38b9[3];
    uint32_t word38bc; void* handle38c0; void* handle38c4; uint32_t untouched38c8;
    Vector bindingsVector; void* name; void* nameEnd; const char* nameTag; uint32_t nameSize; Vector tail;
};
struct Manager { void** table; Resources resources; void** lifetime; uint32_t refs; uint32_t untouchedA0; MaterialLookup materials; Vector first,second; };
struct Dependencies {
    Thiscall<void*,Resources> base=nullptr;
    Thiscall<ts3::renderer::Camera*,ts3::renderer::Camera> camera=nullptr;
    Thiscall<void*,Swarm,uint32_t> destroySwarm=nullptr;
    Thiscall<void*,Manager,uint32_t> destroyManager=nullptr;
    void* (*resolve)(uint32_t)=nullptr;
};
extern Dependencies dependencies;
// TS3W.exe 0x00720620: Initialize swarm resources without clearing untouched storage.
Swarm* __fastcall ConstructSwarm(Swarm*,void*);
// TS3W.exe 0x00720850: Return the first kind 1 or 64, or the zero terminator.
uint32_t __fastcall SelectKind(void*,void*,const uint32_t*);
// TS3W.exe 0x00720880: Adjust secondary receiver and dispatch deleting destructor.
void* __fastcall DestroySecondary(Resources*,void*,uint32_t);
// TS3W.exe 0x00720890: Adjust lifetime receiver and dispatch deleting destructor.
void* __fastcall DestroyLifetime(void***,void*,uint32_t);
// TS3W.exe 0x007208A0: Increment the non-atomic wrapping reference count.
uint32_t __fastcall AddRef(Swarm*,void*);
// TS3W.exe 0x007208B0: Decrement refs; pin to one before last-ref virtual destruction.
uint32_t __fastcall Release(Swarm*,void*);
// TS3W.exe 0x007208E0: Release through the secondary receiver.
uint32_t __fastcall ReleaseSecondary(Resources*,void*);
// TS3W.exe 0x007208F0: Release through the lifetime receiver.
uint32_t __fastcall ReleaseLifetime(void***,void*);
// TS3W.exe 0x00720900: AddRef through the secondary receiver.
uint32_t __fastcall AddRefSecondary(Resources*,void*);
// TS3W.exe 0x00720910: AddRef through the lifetime receiver.
uint32_t __fastcall AddRefLifetime(void***,void*);
// TS3W.exe 0x00720A60: Initialize material lookup and resource interfaces.
Manager* __fastcall ConstructManager(Manager*,void*);
// TS3W.exe 0x00720B30: Adjust lifetime receiver and dispatch manager deleting destructor.
void* __fastcall DestroyManagerLifetime(void***,void*,uint32_t);
// TS3W.exe 0x00720B40: Adjust secondary receiver and dispatch manager deleting destructor.
void* __fastcall DestroyManagerSecondary(Resources*,void*,uint32_t);
// TS3W.exe 0x00720B50: Increment the wrapping manager reference count.
uint32_t __fastcall ManagerAddRef(Manager*,void*);
TS3_X86_SIZE(Resources,0x94);
TS3_X86_OFFSET(Swarm,refs,0xa4);
TS3_X86_OFFSET(Swarm,camera,0x3160);
TS3_X86_OFFSET(Swarm,tail,0x38ec);
TS3_X86_SIZE(Swarm,0x38fc);
TS3_X86_OFFSET(Manager,refs,0x9c);
TS3_X86_OFFSET(Manager,materials,0xa4);
TS3_X86_SIZE(Manager,0xe0);
}
