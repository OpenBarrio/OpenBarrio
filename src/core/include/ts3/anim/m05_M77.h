#pragma once
#include "ts3/layout.h"
namespace ts3::anim {
struct M77Link { M77Link* next; M77Link* previous; };
struct M77Record {
    void* vtable;
    M77Link listeners;
    uint32_t dispatchDepth;
    void* resource;
    uint8_t pendingFirst, pendingSecond;
    uint8_t padding[2];
};
struct M77Node { void* key; M77Record record; M77Node* next; };
struct M77Iterator { M77Node* node; M77Node** bucket; };
struct M77InsertResult { M77Iterator iterator; uint8_t inserted; };
struct M77Map {
    void* vtable;
    M77Node** buckets;
    uint32_t bucketCount, size;
    uint32_t policy[4];
};
struct M77RigView { unsigned char opaque[0xc8]; void* value; };
TS3_X86_OFFSET(M77Node,record,4);
TS3_X86_OFFSET(M77Node,next,0x1c);
TS3_X86_OFFSET(M77Record,resource,0x10);
TS3_X86_OFFSET(M77Record,pendingFirst,0x14);
TS3_X86_OFFSET(M77Map,size,0xc);
TS3_X86_OFFSET(M77RigView,value,0xc8);
// TS3W.exe 0x00576F40: Erase a chain node and return the supplied next-iterator buffer; scan to sentinel without bounds checks.
M77Iterator* __fastcall EraseM77(M77Map*, void*, M77Iterator*, M77Iterator);
// TS3W.exe 0x00576FB0: Dispatch flagged records then erase them; no scalar return contract (EAX can be a dead stack iterator).
void __fastcall SweepM77(M77Map*, void*);
// TS3W.exe 0x00577060: Sweep the global scratch registry; same void tail-call contract.
void SweepGlobalM77();
// TS3W.exe 0x00577070: Find or insert a record and return its value; insertion forwards the low-byte-cleared key pointer.
M77Record* __fastcall GetOrInsertM77(M77Map*, void*, void* const*);
// TS3W.exe 0x00577120: For nonnull listener/resource, set the resource and append the listener; null paths leave EAX undefined.
void RegisterM77(void* listener, void* resource);
// TS3W.exe 0x005775D0: Return the pointer at +C8 (verified pointer-getter twin, displacement adapted).
void* __fastcall RigValueM77(const M77RigView*, void*);
}
