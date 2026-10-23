// CAS keyed lookup: TS3W.exe 0x00617200 (bucket-chain slot search) and 0x00617540
// (value-slot find), the unit clu_617200_CAS. Both are read-only walks over a small
// open-chained hash table embedded at +0x20 of various CAS objects (28+ game callers);
// neither allocates, writes through embedded pointers, or touches globals.
//
// Table shape (x86, reconstructed from the two walks):
//   +0x00  dword, never read by this unit's functions
//   +0x04  buckets: pointer to `count` + 1 chain heads. buckets[i] heads the chain of
//          nodes with (key % count) == i; buckets[count] is a SENTINEL node the insert
//          side keeps for "slot not found" (the walk never scans it).
//   +0x08  count: the chain count, used as the key-hash divisor (0 raises #DE in the
//          original - the game never builds an empty table; the rewrite divides too).
// Node shape: +0x00 key (u32), +0x04 value slot, +0x08..+0x27 opaque, +0x28 next.
#pragma once

#include "ts3/layout.h"

namespace ts3::cas {

// One hash-table node. Only +0x00 and +0x28 are read here; +0x04's ADDRESS is the
// 0x00617540 "found" result (the caller dereferences the value itself).
struct CasLookupNode {
    uint32_t key;           // +0x00 compared bit-exactly against the lookup key
    uint32_t value04;       // +0x04 opaque to this unit
    uint8_t body08[0x20];   // +0x08..+0x27 opaque
    CasLookupNode* next;    // +0x28 next node of the chain, null-terminated
};
TS3_X86_OFFSET(CasLookupNode, next, 0x28);
TS3_X86_SIZE(CasLookupNode, 0x2C);

// The {node, bucket} pair 0x00617200 writes: the matching node and the address of its
// chain head, or the sentinel node and &buckets[count] when the key is absent (the
// insert side uses the pair to splice a new node in at the chain's tail).
struct CasSlotPair {
    CasLookupNode* node;
    CasLookupNode** bucket;
};

// The table sub-object at owner+0x20.
struct CasLookupTable {
    // TS3W.exe 0x00617200, thiscall ret 8 (two stack args), returns `out`. Finds the
    // node whose +0x00 equals *key in buckets[*key % count]; on a miss both out fields
    // name the sentinel slot. Reads *key exactly once.
    CasSlotPair* FindSlot(CasSlotPair* out, const uint32_t* key);

    uint32_t field00;           // +0x00 never read by this unit
    CasLookupNode** buckets;    // +0x04 count + 1 chain heads (see the file comment)
    uint32_t count;             // +0x08 chain count = hash divisor
};
TS3_X86_OFFSET(CasLookupTable, buckets, 0x04);
TS3_X86_OFFSET(CasLookupTable, count, 0x08);
TS3_X86_SIZE(CasLookupTable, 0x0C);

// Partial view of the owning object (4476-byte CAS class of unit cls_5ca300_x et al.;
// only the embedded table matters to this unit).
struct CasLookupOwner {
    // TS3W.exe 0x00617540, thiscall ret 4 (one stack arg). Looks the key up in the
    // +0x20 table and returns the address of the found node's value slot (+4), or null
    // when the lookup came back sentinel (not found). Side-effect-free.
    void* FindValueSlot(uint32_t key);

    uint8_t head[0x20];     // +0x00..+0x1F opaque
    CasLookupTable table;   // +0x20
};
TS3_X86_OFFSET(CasLookupOwner, table, 0x20);

}  // namespace ts3::cas
