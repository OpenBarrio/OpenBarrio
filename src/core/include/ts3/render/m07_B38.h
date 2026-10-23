#pragma once
#include "ts3/layout.h"
namespace ts3::render::m07::b38 {
using Resolver=void*(*)(uint32_t);
extern Resolver resolve;
struct Vector { void* begin; void* end; void* capacity; uint32_t tag; };
struct Shadow { void** table; uint8_t pad04[0xc]; void* backing; uint8_t pad14[0xc]; void* allocation; };
struct Skin { uint8_t prefix[0x28]; uint32_t count; uint32_t* hashes; void* bindPose; };
struct Entry { uint32_t key[2]; void* definition; void* material; uint32_t users; };
struct Table { uint32_t opaque; Entry** buckets; uint32_t sentinelIndex; };
struct Resources { uint8_t prefix[0xa0]; Table table; };
struct Particle { uintptr_t owner; void* object; void* resource; uint8_t pad0c[0x14]; uint32_t key[2]; void* material; uint8_t pad2c[0x18]; uint8_t enabled; uint8_t tail[0x33]; };
struct Instance { uintptr_t owner; void* model; uint32_t key[2]; void* material; void* binding; uint32_t state; uint32_t pad1c; uint32_t extra[2]; uint8_t pad28[0x24]; uint16_t flags; uint8_t enabled,mode; };
struct Emitter { uint8_t prefix[0x50]; void* model; float value; uint8_t enabled; uint8_t pad59[7]; uint32_t key[2]; void* material; uint32_t tail; };
struct Swarm { uint8_t prefix[0xd8]; Vector particles; Vector freeParticles; uint8_t gap[0x3110-0xf8]; Vector instances; Vector freeInstances; uint32_t gap3130; Vector emitters; Vector freeEmitters; uint8_t gap3154[0x3894-0x3154]; uint8_t fallbackEnabled; uint8_t gap3895[0x1b]; void* fallback; void* manager; };
struct Materials { uint8_t prefix[0xbc]; void** begin; void** end; void** capacity; uint32_t tag; Vector free; };
struct Param { uint32_t key,unused; union { uint32_t words[2]; void* pointer; } data; int16_t count; int8_t type; uint8_t tail[5]; };
struct MaterialInput { uint8_t prefix[0x10]; uint32_t shader; uint32_t pad14; Param* begin; Param* end; };
struct MaterialOwner { uint8_t prefix[0x9c]; uint8_t table[4]; };
struct Metadata { uint8_t prefix[0x94]; uint16_t index,count; uint32_t* data; };
struct InstanceInput { uint32_t modelKey[2]; uint8_t gap08[0xa]; uint16_t flags; uint32_t gap14; uint32_t materialKey[2]; uint32_t gap20; Metadata* metadata; };
TS3_X86_SIZE(Particle,0x78); TS3_X86_SIZE(Instance,0x50); TS3_X86_SIZE(Emitter,0x70); TS3_X86_SIZE(Param,0x18);
TS3_X86_OFFSET(Swarm,instances,0x3110); TS3_X86_OFFSET(Swarm,emitters,0x3134); TS3_X86_OFFSET(Swarm,manager,0x38b4);
TS3_X86_OFFSET(Materials,free,0xcc);
// TS3W.exe 0x006999B0: destroy shadow storage and optionally free the receiver.
Shadow* __fastcall DeleteShadow(Shadow*,void*,uint32_t);
// TS3W.exe 0x00699E80: rebuild then submit shadow constants.
uintptr_t __fastcall UpdateShadow(void*,void*);
// TS3W.exe 0x0070D0B0: allocate joint hashes and aligned bind poses with overflow guards.
bool __fastcall AllocateSkin(Skin*,void*,uint32_t);
// TS3W.exe 0x0071E930: acquire a cached material; only AL denotes success.
uintptr_t __fastcall Acquire(Resources*,void*,uint32_t,uint32_t,void**);
// TS3W.exe 0x0071E9C0: decrement cache users, releasing only on the transition to zero.
uintptr_t __fastcall Release(Resources*,void*,uint32_t,uint32_t);
// TS3W.exe 0x0071F220: construct and replace a swarm material definition; AL-only result.
uintptr_t __fastcall SetMaterial(MaterialOwner*,void*,uint32_t,uint32_t,MaterialInput*);
// TS3W.exe 0x0071FC40: recycle a particle index and release its adjusted interfaces.
uintptr_t __fastcall RemoveParticle(Swarm*,void*,int32_t);
// TS3W.exe 0x0071FD20: allocate/reuse a model instance; preserve partial record initialization.
uintptr_t __fastcall AddInstance(Swarm*,void*,uintptr_t,InstanceInput*,int32_t*);
// TS3W.exe 0x0071FF10: recycle a model instance and notify external owners before clearing it.
uintptr_t __fastcall RemoveInstance(Swarm*,void*,int32_t);
// TS3W.exe 0x00720010: allocate/reinitialize an emitter and set its model and float value.
uintptr_t __fastcall AddEmitter(Swarm*,void*,uint32_t,uint32_t,int32_t*,float);
// TS3W.exe 0x007201A0: recycle an emitter and release its nonfallback model.
uintptr_t __fastcall RemoveEmitter(Swarm*,void*,int32_t);
// TS3W.exe 0x00720280: recycle a material index without clearing the stored pointer (EA quirk).
uintptr_t __fastcall RemoveMaterial(Materials*,void*,int32_t);
// TS3W.exe 0x00720510: return the unsupported-operation sentinel, ignoring all four arguments.
int32_t __fastcall Unsupported(void*,void*,uintptr_t,uintptr_t,uintptr_t,uintptr_t);
// TS3W.exe 0x00720520: report a fixed 64-byte requirement, ignoring all three arguments.
uint32_t __fastcall RequiredSize(void*,void*,uintptr_t,uintptr_t,uintptr_t);
}
