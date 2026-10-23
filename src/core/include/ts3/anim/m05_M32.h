#pragma once
#include "ts3/anim/m05_B08.h"
namespace ts3::anim::m32 {
using Stream = b08::StreamState;
struct Resource { void* vtable; unsigned char opaque04[0x44]; ClipIkInfo* ik; void* view; };
struct Clip {
    Controller base;
    void *begin,*end,*capacity;
    unsigned char opaque34[4],inlineStorage[0x4b8];
    ClipIkInfo ik;
    Resource* resource;
    unsigned char opaque5d4[8];
    float stateDC,stateE0,stateE4,period;
    uint32_t mode,valueF0;
    int32_t valueF4;
    uint8_t flags[7],opaque5ff;
};
struct PairController { Controller base; void* secondary28; Controller *first,*second; float value34,value38; };
TS3_X86_OFFSET(Clip,resource,0x5d0);
TS3_X86_OFFSET(Clip,stateDC,0x5dc);
TS3_X86_OFFSET(Clip,flags,0x5f8);
TS3_X86_OFFSET(Resource,ik,0x48);
TS3_X86_OFFSET(PairController,value38,0x38);
// TS3W.exe 0x00569C50: Retain and assign a clip resource, release the old resource, then refresh IK and duration.
void SetResource(Clip*,Resource*);
// TS3W.exe 0x00569D30: Resolve a resource key and assign it, testing only AL of the lookup result.
uint8_t LoadResource(Clip*,const void*);
// TS3W.exe 0x00569D90: Load versioned clip state, preserving partial writes when a stream operation fails.
uint8_t Load(Clip*,Stream*);
// TS3W.exe 0x0056A230: Save two optional controllers and two floats, stopping on the first failed operation.
uint8_t Save(PairController*,Stream*);
// TS3W.exe 0x0056A2C0: Construct a two-controller wrapper, preserving uninitialized base bytes.
PairController* Construct(PairController*,void*);
// TS3W.exe 0x0056A310: Return the wrapper interface type ID (constant-return twin with a different immediate).
uint32_t Type(void*);
#if defined(_M_IX86)
namespace hooks {
void __fastcall SetResource(Clip*,void*,Resource*);
uint8_t __fastcall LoadResource(Clip*,void*,const void*);
uint8_t __fastcall Load(Clip*,void*,Stream*);
uint8_t __fastcall Save(PairController*,void*,Stream*);
PairController* __fastcall Construct(PairController*,void*,void*);
uint32_t __fastcall Type(void*,void*);
}
#endif
}
