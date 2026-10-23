#pragma once
#include "ts3/anim/m05_layouts.h"
namespace ts3::anim::m86 {
struct Reader { void* owner; void* view; void* stream; uint32_t size; uint32_t endian; uint8_t good, cached; uint8_t pad[2]; };
struct TrackMask { ResourcePrefix base; float* begin; float* end; float* capacity; void* descriptor; };
TS3_X86_SIZE(Reader,0x18);
TS3_X86_SIZE(TrackMask,0x38);
// TS3W.exe 0x0057C180: Load a rig, retaining legacy lookup and zero-joint failure behavior.
uint8_t Load(RigResource*, Reader*);
// TS3W.exe 0x0057C720: Adapt a resource stream to the rig reader.
uint8_t LoadStream(void* stream, RigResource*);
// TS3W.exe 0x0057C770: Return the signed weight range count.
int32_t Count(const TrackMask*);
// TS3W.exe 0x0057C780: Read a weight, returning -1 for an unsigned out-of-range index.
float Weight(const TrackMask*, uint32_t);
// TS3W.exe 0x0057C7B0: Query the track-mask resource and its three base interfaces.
void* Query(TrackMask*,uint32_t);
// TS3W.exe 0x0057C7F0: Construct a track-mask resource and empty weight vector.
TrackMask* Construct(TrackMask*);
uint8_t __stdcall HookLoadStream(void*,RigResource*);
int32_t __fastcall HookCount(const TrackMask*,void*);
float __fastcall HookWeight(const TrackMask*,void*,uint32_t);
void* __fastcall HookQuery(TrackMask*,void*,uint32_t);
TrackMask* __fastcall HookConstruct(TrackMask*,void*);
}
