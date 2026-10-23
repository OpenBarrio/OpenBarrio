#pragma once
#include "ts3/layout.h"
namespace ts3::audio::b46 {
// Native pointers expand independently of the observed x86 views.
struct Variant {
    void** table;
    union { void* pointer; uint32_t word; } data;
    uint16_t flags, type;
    uint8_t tail[20];
};
struct Entry { uint32_t key, padding; Variant value; };
struct Parent { uint8_t prefix[40]; void** properties; };
struct PropertySet {
    void** table; uint32_t refs; uint8_t prefix[8];
    uint8_t localOnly, pad11[3];
    Entry *begin,*end,*capacity; const char* label;
    uint8_t sorted, pad25[3]; Parent* parent; uint8_t pad2c[12];
    Variant groups, map;
    // TS3W.exe 0x008A8160: find a property, including built-ins, then optionally delegate; AL is the result.
    uint8_t Get(uint32_t,void**);
    uint8_t Get(uint32_t k,void* out) { return Get(k,static_cast<void**>(out)); }
    // TS3W.exe 0x008A8220: optionally clear and then enumerate keys, including the inherited set.
    uintptr_t Enumerate(void*,uint32_t);
    // TS3W.exe 0x008A9240: test the local table then the parent, ignoring localOnly for parent lookup.
    uint8_t Contains(uint32_t);
    // TS3W.exe 0x008A92A0: find a mutable property, with the same built-in handling as Get.
    uint8_t GetMutable(uint32_t,void**);
    uint8_t GetMutable(uint32_t k,void* out) { return GetMutable(k,static_cast<void**>(out)); }
};
struct EntryVector {
    Entry *begin,*end,*capacity; const char* label;
    // TS3W.exe 0x008A90D0: insert with alias adjustment or doubled reallocation; return assignment result or capacity.
    uintptr_t InsertSlow(Entry*,Entry*);
    uintptr_t InsertSlow(void* at,void* source) { return InsertSlow(static_cast<Entry*>(at),static_cast<Entry*>(source)); }
    // TS3W.exe 0x008A9490: append or insert and return the relocated insertion position; publish end before construction.
    Entry* Insert(Entry*,Entry*);
    Entry* Insert(void* at,void* source) { return Insert(static_cast<Entry*>(at),static_cast<Entry*>(source)); }
};
struct StringVector {
    void *begin,*end,*capacity; const char* label;
    // TS3W.exe 0x008A9070: allocate and initialize count 16-byte strings using the first range helper.
    StringVector* InitA(uint32_t,void*);
    // TS3W.exe 0x008A9430: allocate and initialize count 16-byte strings using the second range helper.
    StringVector* InitB(uint32_t,void*);
};
struct Loader {
    void** table; uint32_t refs; alignas(void*) uint8_t mutex[24]; uint8_t tail[8]; void** properties;
    // TS3W.exe 0x008A7FA0: lock, allocate and retain a configuration, call the loader, then publish or close the input.
    uint8_t Create(void*,void**,uint32_t,uint32_t);
    uint8_t Create(void* in,void* out,uint32_t a,uint32_t b) { return Create(in,static_cast<void**>(out),a,b); }
    // TS3W.exe 0x008A9360: read properties, continuing setter callbacks with stale values after a read failure.
    uint8_t ReadProperties(void*);
};
// TS3W.exe 0x008A8080: unsigned sorted multiset difference, supporting in-place output.
uint32_t* Difference(uint32_t*,uint32_t*,uint32_t*,uint32_t*,uint32_t*);
// TS3W.exe 0x008A80E0: unsigned sorted multiset union, retaining the maximum duplicate multiplicity.
uint32_t* Union(uint32_t*,uint32_t*,uint32_t*,uint32_t*,uint32_t*);
TS3_X86_OFFSET(Loader,mutex,8);
TS3_X86_OFFSET(Loader,properties,40);
TS3_X86_SIZE(Variant,32);
TS3_X86_SIZE(Entry,40);
TS3_X86_OFFSET(PropertySet,localOnly,0x10);
TS3_X86_OFFSET(PropertySet,begin,0x14);
TS3_X86_OFFSET(PropertySet,sorted,0x24);
TS3_X86_OFFSET(PropertySet,parent,0x28);
TS3_X86_OFFSET(PropertySet,groups,0x38);
TS3_X86_OFFSET(PropertySet,map,0x58);
}
