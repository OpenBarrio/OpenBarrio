#pragma once
#include "ts3/layout.h"
namespace ts3::anim {
struct m45 {
struct Handles { void** begin; void** end; void** capacity; void* descriptor; };
struct Record { uint32_t key[3]; void* handle; };
struct RandomController {
    unsigned char base[0x2c];
    void* child;
    unsigned char reserved30[8];
    Handles handles;
    Record* records; Record* recordsEnd; Record* recordsCapacity; void* recordDescriptor;
    uint32_t* weights; uint32_t* weightsEnd; uint32_t* weightsCapacity; void* weightDescriptor;
    uint32_t* indices; uint32_t* indicesEnd; uint32_t* indicesCapacity; void* indexDescriptor;
    unsigned char reserved78[8]; uint32_t state; uint32_t seed;
};
struct Reader { void* vtable; uint32_t unknown; void* stream; uint32_t unknownC; uint32_t flags; uint8_t good; };
TS3_X86_OFFSET(RandomController,handles,0x38);
TS3_X86_OFFSET(RandomController,records,0x48);
TS3_X86_OFFSET(RandomController,state,0x80);
TS3_X86_OFFSET(Reader,good,0x14);
// TS3W.exe 0x0056D780: destroy then optionally free, returning the original address.
static RandomController* Destroy(RandomController*, uint32_t);
// TS3W.exe 0x0056D7A0: fill pending clips then advance the child (AL alone controls the loop).
static uintptr_t Update(RandomController*,float);
// TS3W.exe 0x0056D9E0: insert retained handles with EA growth and temporary-handle ordering.
static uintptr_t Insert(Handles*,void**,uint32_t,void**);
// TS3W.exe 0x0056DC40: resize the retained-handle vector using a null fill handle.
static uintptr_t Resize(Handles*,uint32_t);
// TS3W.exe 0x0056DCF0: synchronize each record into its matching retained-handle slot.
static uintptr_t Synchronize(RandomController*);
// TS3W.exe 0x0056DD70: load random-controller records and scalar arrays, preserving partial failures.
static uintptr_t Load(RandomController*,Reader*);
uintptr_t Update(float dt) { return Update(reinterpret_cast<RandomController*>(this),dt); }
uintptr_t Synchronize() { return Synchronize(reinterpret_cast<RandomController*>(this)); }
uintptr_t Load(void* reader) { return Load(reinterpret_cast<RandomController*>(this),static_cast<Reader*>(reader)); }
};
}
