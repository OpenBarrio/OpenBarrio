#pragma once
#include "ts3/render/m07_abi.h"
namespace ts3::render::m07::b40 {
using Resolver=void*(*)(uint32_t);
extern Resolver resolve;
struct Swarm {
    void* primary; void* secondary; uint8_t opaque[0x90];
    void** lifetime; uint32_t refs;
    uint32_t vectorUnknown; void* items; uint32_t capacity,size;
    uint8_t gapB0[16]; void* bufferC0; uint8_t gapC4[12]; void* bufferD0;
};
TS3_X86_OFFSET(Swarm,lifetime,0x98); TS3_X86_OFFSET(Swarm,items,0xa4);
TS3_X86_OFFSET(Swarm,bufferD0,0xd0);
struct Key { uint32_t words[4]; };
struct Fixup { void** vtable; uint32_t ref; Key diffuse,specular; uint8_t flags; };
struct Binding {uint32_t type,key,unused[2]; Key texture; void* value; uint32_t count;};
struct Substitution {
    Binding* bindings; uint32_t count; void* context; uint32_t id; uint8_t alpha,option; uint8_t pad[2];
    void* unknown; void* buckets; uint32_t capacity,size; float load1,load2; void* storage; const char* tag;
};
TS3_X86_OFFSET(Substitution,buckets,24); TS3_X86_SIZE(Binding,40);
struct Material {void** vtable; uint8_t opaque[0x6e]; uint8_t editable;};
TS3_X86_OFFSET(Material,editable,0x72);
struct String {char* begin;char* end;char* capacity;const char* tag;};
struct Vector {void* begin;void* end;void* capacity;const char* tag;};
struct Tree {
    Vector lists[10]; uint8_t opaque[528]; uint8_t helper[48];
    void* owner; void* argument; uint32_t zero; uint8_t gap[12];
    void* empty[11]; uint32_t atomic; uint8_t untouched[4]; uint8_t flags[2];
};
TS3_X86_OFFSET(Tree,helper,688); TS3_X86_OFFSET(Tree,atomic,804);
struct ReflectionMap {
    void* untouched0; void* buckets; uint32_t capacity,size; float load1,load2;
    void* storage;const char* tag;void* next;uint8_t flag;uint8_t padding[3];
    uint32_t state,untouched44;void* entries[3];
};
TS3_X86_OFFSET(ReflectionMap,flag,36);
struct Visualizer {alignas(16) float color[4];uint8_t gap[8];void* shader;uint8_t gap1[8];uint32_t zero;uint8_t gap2[15];uint8_t enabled;};
TS3_X86_OFFSET(Visualizer,enabled,55);
// TS3W.exe 0x00720B60: Release the swarm lifetime interface, restoring one before deleting on last ref.
uint32_t __fastcall Release(Swarm*,void*);
// TS3W.exe 0x00720B90: Adjust secondary receiver to the swarm release implementation.
uint32_t __fastcall ReleaseSecondary(void*,void*);
// TS3W.exe 0x00720BA0: Adjust lifetime receiver to the original swarm AddRef.
uint32_t __fastcall AddLifetime(void*,void*);
// TS3W.exe 0x00720BB0: Adjust secondary receiver to the original swarm AddRef.
uint32_t __fastcall AddSecondary(void*,void*);
// TS3W.exe 0x00720BC0: Adjust lifetime receiver to swarm release.
uint32_t __fastcall ReleaseLifetime(void*,void*);
// TS3W.exe 0x00720BD0: Adjust secondary receiver and forward the property conversion, preserving EAX.
uintptr_t __fastcall Convert(void*,void*,uint32_t,void*,void*);
// TS3W.exe 0x00720BE0: Destroy owned buffers and vector before replacing the lifetime vtable and destroying the base.
uintptr_t __fastcall Destroy(Swarm*,void*);
// TS3W.exe 0x007227E0: Return this for either recognized texture-fixup interface ID, otherwise null.
void* __fastcall Query(Fixup*,void*,uint32_t);
// TS3W.exe 0x00722810: Assemble texture/alpha substitutions and retain or clone a material; returns AL with cleanup EAX high bits.
uintptr_t __fastcall Apply(Fixup*,void*,Material*,Material**,void*,void*);
// TS3W.exe 0x00722AD0: Format texture fixup keys, freeing temporary strings in diffuse/specular order.
uintptr_t __fastcall Describe(Fixup*,void*,void*);
// TS3W.exe 0x00725370: Bind colorVisualizer and map count per projected area to color, preserving x87/SSE evaluation.
void* __cdecl Visualize(void*,uint32_t,Visualizer*);
// TS3W.exe 0x00725C10: Initialize the reflection marker map, leaving its holes untouched.
ReflectionMap* __fastcall InitReflection(ReflectionMap*,void*);
// TS3W.exe 0x00712CD0: Register the SpeedTree service only when the registry result's low byte is zero.
uintptr_t __cdecl RegisterTree();
// TS3W.exe 0x00716E60: Initialize ten tagged vectors and the sparse SpeedTree renderer state.
Tree* __fastcall InitTree(Tree*,void*,void*,void*);
}
