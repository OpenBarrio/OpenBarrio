#pragma once
#include "ts3/layout.h"
namespace ts3::anim::m78 {
struct Id { uint32_t low, high; };
struct Target { uint8_t opaque[8]; Id id; };
struct Record {
    uint8_t opaque[224];
    Id current, saved;
    uint32_t channel;
    uint8_t unknown[12];
    int32_t index;
    uint32_t kind, flags, tail;
};
struct Row { uint8_t opaque[8]; Record* begin; Record* end; uint8_t tail[2728]; };
struct State {
    uint8_t opaque[144];
    Row *begin, *end, *capacity;
    uint32_t descriptor;
    void *gateBegin, *gateEnd, *gateCapacity;
    uint32_t gateDescriptor;
    void** lookup;
    uint8_t unknown[12];
    Id saved;
    Target* target;
    uint32_t flags;
    void* key;
};
struct Writer { uint8_t opaque[8]; void* stream; uint32_t unknown, mode; uint8_t status; };
struct Bytes { uint8_t *begin, *end, *capacity; uint32_t descriptor; };
TS3_X86_SIZE(Record,272);
TS3_X86_SIZE(Row,2744);
TS3_X86_OFFSET(State,begin,0x90);
TS3_X86_OFFSET(State,saved,0xc0);
TS3_X86_OFFSET(State,target,0xc8);
TS3_X86_OFFSET(State,flags,0xcc);
TS3_X86_OFFSET(State,key,0xd0);
TS3_X86_OFFSET(Writer,status,20);
// TS3W.exe 0x005775E0: Copy the target identifier or clear it, returning its low word.
uint32_t AssignId(Id* self, const Target* target);
// TS3W.exe 0x005777A0: Rebind matching targets and preserve saved IDs on null replacement; returns the final iterator.
void* Rebind(State* self, void* key, Target* target);
// TS3W.exe 0x00577890: Serialize flags, rows and resolved target IDs in original call order.
uint8_t Save(State* self, Writer* writer, uint32_t unused);
// TS3W.exe 0x00577A80: Resolve target IDs and latch completion, including failed root lookup.
uint8_t Resolve(State* self);
// TS3W.exe 0x00577C20: Assign bytes; shrinking moves the old end byte, not a synthesized zero.
Bytes* AssignBytes(Bytes* self, const char* begin, const char* end);
// TS3W.exe 0x00577D30: Assign a range of 272-byte records through the original member dependency.
Record* CopyRecords(const Record* begin, const Record* end, Record* destination);
}
#if defined(_M_IX86)
namespace ts3::anim::m78::hooks {
uint32_t __fastcall AssignId(Id*,void*,const Target*);
void* __fastcall Rebind(State*,void*,void*,Target*);
uint8_t __fastcall Save(State*,void*,Writer*,uint32_t);
uint8_t __fastcall Resolve(State*,void*);
Bytes* __fastcall AssignBytes(Bytes*,void*,const char*,const char*);
}
#endif
