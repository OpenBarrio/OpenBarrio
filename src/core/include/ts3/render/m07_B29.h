#pragma once
#include "ts3/layout.h"
namespace ts3::render::m07::b29 {
struct Owner { void* prefix; void* buckets[1033]; void* sentinel; uint8_t tail[141]; uint8_t flag; };
TS3_X86_OFFSET(Owner,sentinel,0x1028);
TS3_X86_OFFSET(Owner,flag,0x10b9);
struct String { char* begin; char* end; char* capacity; const char* tag; };
struct DebugObject { void** table; uint8_t pad04[12]; uint64_t id; uint8_t pad18[24]; void* state; uint8_t pad34[60]; float position[4]; };
TS3_X86_OFFSET(DebugObject,id,0x10);
TS3_X86_OFFSET(DebugObject,state,0x30);
TS3_X86_OFFSET(DebugObject,position,0x70);
// TS3W.exe 0x006FD9D0: Delete renderer operation; original ordering and sentinel behavior retained.
void* __fastcall Delete(void* self,void*,uint32_t flags);
// TS3W.exe 0x006FDA70: Debug renderer operation; original ordering and sentinel behavior retained.
String* __fastcall Debug(DebugObject* self,void*,String* out);
// TS3W.exe 0x006FE5F0: Cutout renderer operation; original ordering and sentinel behavior retained.
uint8_t __fastcall Cutout(Owner* self,void*,uint32_t lo,uint32_t hi,void* data,uint32_t value);
// TS3W.exe 0x006FE6D0: Video renderer operation; original ordering and sentinel behavior retained.
uint8_t __fastcall Video(Owner* self,void*,uint32_t lo,uint32_t hi,void* data);
// TS3W.exe 0x006FE7B0: Painting renderer operation; original ordering and sentinel behavior retained.
uint8_t __fastcall Painting(Owner* self,void*,uint32_t lo,uint32_t hi,void* data,uint32_t value);
// TS3W.exe 0x006FEC70: Ghost renderer operation; original ordering and sentinel behavior retained.
uint8_t __fastcall Ghost(Owner* self,void*,uint32_t lo,uint32_t hi,void* data,uint32_t value);
// TS3W.exe 0x006FEDD0: Sculpture renderer operation; original ordering and sentinel behavior retained.
uint8_t __fastcall Sculpture(Owner* self,void*,uint32_t lo,uint32_t hi,uint32_t value,void* data);
// TS3W.exe 0x006FEF30: Burninate renderer operation; original ordering and sentinel behavior retained.
uintptr_t __fastcall Burninate(Owner* self,void*,uint32_t lo,uint32_t hi);
// TS3W.exe 0x006FF050: Scratch renderer operation; original ordering and sentinel behavior retained.
uintptr_t __fastcall Scratch(Owner* self,void*,uint32_t lo,uint32_t hi,uint8_t enabled);
// TS3W.exe 0x006FF870: PoolWindow renderer operation; original ordering and sentinel behavior retained.
uint8_t __fastcall PoolWindow(Owner* self,void*,uint32_t lo,uint32_t hi,uint8_t enabled);
// TS3W.exe 0x006FF960: DistantTerrain renderer operation; original ordering and sentinel behavior retained.
uint8_t __fastcall DistantTerrain(Owner* self,void*,uint32_t lo,uint32_t hi);
// TS3W.exe 0x006FFA30: FrostedGlass renderer operation; original ordering and sentinel behavior retained.
uintptr_t __fastcall FrostedGlass(Owner* self,void*,uint8_t enabled);
// TS3W.exe 0x00700470: TexComp renderer operation; original ordering and sentinel behavior retained.
uint8_t __fastcall TexComp(Owner* self,void*,uint32_t lo,uint32_t hi,void* first,void* second);
}
