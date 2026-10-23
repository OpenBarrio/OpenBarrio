#pragma once
#include "ts3/layout.h"
namespace ts3::anim::m76 {
struct Link { Link* next; Link* previous; };
struct Record {
    uint32_t key; void* vtable; Link listeners; uint32_t value, extra;
    uint8_t enabled, dirty, padding[2]; Record* next;
};
struct Registry {
    void* vtable; Record** buckets; uint32_t bucketCount, count;
    uint32_t policy[3]; void* allocator;
};
struct Iterator { Record* record; Record** bucket; };
struct InsertResult { Record* record; Record** bucket; bool inserted; };
TS3_X86_OFFSET(Record, next, 0x1c);
TS3_X86_OFFSET(Registry, allocator, 0x1c);
// TS3W.exe 0x00576C20: Broadcast to the global registry through the original traversal.
uintptr_t Broadcast(void* listener);
// TS3W.exe 0x00576CB0: Set the matching record's enabled byte, returning AL only.
bool Enable(uint32_t key);
// TS3W.exe 0x00576D00: Set the dirty byte and preserve the returned record pointer.
Record* Dirty(uint32_t key);
// TS3W.exe 0x00576D40: Append a listener if both arguments and the lookup are valid; keep incidental EAX.
uintptr_t Attach(void* listener, uint32_t key);
// TS3W.exe 0x00576D90: Allocate a record, copy payload but not listeners, and clear its chain link; null allocation still faults.
Record* Clone(Registry* self, const Record* source);
// TS3W.exe 0x00576DF0: Find or insert a key with policy-directed rehash, returning the output iterator.
InsertResult* Insert(Registry* self, InsertResult* output, const Record* source, uint32_t unused);
#if defined(_M_IX86)
namespace hooks {
Record* __fastcall Clone(Registry*, void*, const Record*);
InsertResult* __fastcall Insert(Registry*, void*, InsertResult*, const Record*, uint32_t);
}
#endif
}
