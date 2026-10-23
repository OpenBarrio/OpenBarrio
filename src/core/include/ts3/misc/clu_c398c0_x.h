// Cluster unit clu_c398c0_x (subsystem misc): the fourteen TS3W.exe functions behind the
// route manager's hash tables (bucketed chains of 16-byte nodes keyed by a 6-byte route
// key) and its compiled route-unit buffer. The hash is
// 100*(b0 + b3 + 200*b2) + b4 + b1 over sign-extended key bytes, modulo bucketCount.
#pragma once

#include <cstdint>

#include "ts3/layout.h"

namespace ts3::misc {

// One hash-chain node. The route key starts at +0x00 and FindRouteByKey compares bytes
// 0..5 (in the original's order 2,0,1,5,3,4); +0x08 is a refcounted RouteSpan (released
// by ReleaseRouteChains/UnlinkRouteNode through its first vtable slot) or the third key
// dword of a node inserted by 0x00C4E680; +0x0C is the chain link. A freed node's key
// bytes are overwritten by the free-list link.
struct RouteHashNode {
    union {
        uint8_t key[8];
        RouteHashNode* freeNext;
    };
    void* value;     // +0x08 RouteSpan* (or third key dword of an inserted node)
    RouteHashNode* next;  // +0x0C
};
TS3_X86_OFFSET(RouteHashNode, key, 0x00);
TS3_X86_OFFSET(RouteHashNode, value, 0x08);
TS3_X86_OFFSET(RouteHashNode, next, 0x0C);
TS3_X86_SIZE(RouteHashNode, 0x10);

// The bucket table shared by 0x00C3A120/0x00C3A9D0/0x00C3AAC0/0x00C3AC20/0x00C3B1B0/
// 0x00C50370. buckets[0..bucketCount-1] are chain heads and buckets[bucketCount] holds
// the end-marker node pointer (non-null, never visited by the walkers). +0x1C points at
// the free-list sentinel node whose first dword is the head of the free list.
struct RouteTable {
    uint32_t unknown00;      // +0x00
    RouteHashNode** buckets;      // +0x04
    uint32_t bucketCount;    // +0x08
    uint32_t count;          // +0x0C live node count
    uint32_t unknown10;      // +0x10
    uint32_t unknown14;      // +0x14
    uint32_t unknown18;      // +0x18
    RouteHashNode* freeSentinel;  // +0x1C sentinel; sentinel->freeNext is the free head
};
TS3_X86_OFFSET(RouteTable, buckets, 0x04);
TS3_X86_OFFSET(RouteTable, bucketCount, 0x08);
TS3_X86_OFFSET(RouteTable, count, 0x0C);
TS3_X86_OFFSET(RouteTable, freeSentinel, 0x1C);

// The refcounted route payload: vtable at +0x00, refcount at +0x04, two vectors - 44-byte
// records at +0x08/+0x0C and u16 units at +0x28/+0x2C.
struct RouteSpan {
    void** vtable;         // +0x00 release slot 0 is called as (object, 1)
    uint32_t refCount;     // +0x04
    uint8_t* recordsBegin; // +0x08
    uint8_t* recordsEnd;   // +0x0C
    uint8_t gap10[0x18];   // +0x10..+0x27 untouched by this unit
    uint16_t* unitsBegin;  // +0x28
    uint16_t* unitsEnd;    // +0x2C
};
TS3_X86_OFFSET(RouteSpan, refCount, 0x04);
TS3_X86_OFFSET(RouteSpan, recordsBegin, 0x08);
TS3_X86_OFFSET(RouteSpan, unitsBegin, 0x28);
TS3_X86_OFFSET(RouteSpan, unitsEnd, 0x2C);

// The node variant whose route link is at +0x0C and whose chain link is at +0x10
// (0x00C399E0 and 0x00C3A940 walk this shape).
struct RouteNodeReverse {
    uint32_t unknown00;    // +0x00
    uint32_t unknown04;    // +0x04
    uint32_t unknown08;    // +0x08
    RouteSpan* route;      // +0x0C
    RouteNodeReverse* next;// +0x10
};
TS3_X86_OFFSET(RouteNodeReverse, route, 0x0C);
TS3_X86_OFFSET(RouteNodeReverse, next, 0x10);

// The view of a bucket table read by 0x00C398C0/0x00C399E0 and written (accumulators) by
// 0x00C3A8B0/0x00C3A940. For the reverse pair the bucket entries are RouteNodeReverse*.
struct RouteTableView {
    uint32_t records44;  // +0x00 accumulated 44-byte record count
    uint32_t units2;     // +0x04 accumulated u16 unit count
    uint32_t unknown08;  // +0x08
    void** buckets;      // +0x0C
    uint32_t bucketCount;// +0x10
};
TS3_X86_OFFSET(RouteTableView, records44, 0x00);
TS3_X86_OFFSET(RouteTableView, units2, 0x04);
TS3_X86_OFFSET(RouteTableView, buckets, 0x0C);
TS3_X86_OFFSET(RouteTableView, bucketCount, 0x10);

// Out pair used by 0x00C3AAC0 (found node / bucket slot) and 0x00C3B1B0 (next node /
// bucket slot after the removal).
struct NodePair {
    RouteHashNode* node;   // +0x00
    RouteHashNode** slot;  // +0x04
};
TS3_X86_SIZE(NodePair, 0x08);

// One compiled route entry appended by 0x00C3BE90 (20 bytes).
struct RouteEntry {
    uint32_t a3;       // +0x00 first argument
    uint32_t a4Div3;   // +0x04 a4/3 (unsigned)
    uint32_t counter6; // +0x08 value of *a6 before the update
    uint32_t counter7; // +0x0C value of *a7 before the update
    uint32_t a2;       // +0x10 second argument
};
TS3_X86_OFFSET(RouteEntry, a3, 0x00);
TS3_X86_OFFSET(RouteEntry, a4Div3, 0x04);
TS3_X86_OFFSET(RouteEntry, counter7, 0x0C);
TS3_X86_OFFSET(RouteEntry, a2, 0x10);
TS3_X86_SIZE(RouteEntry, 0x14);

// The route manager fields this unit touches: the std::map-like tree at +0x3BC
// (0x66F5A0 inserts a {a2, *a5} key there) and the route-entry vector at +0x3D8, laid out
// as a plain begin/end/capacity/allocator quad (the original compares +0x3DC < +0x3E0 and
// inserts at +0x3DC through 0x839220).
struct RouteManagerView {
    uint8_t gap000[0x3BC];
    uint8_t tree[0x1C];        // +0x3BC map header (opaque here)
    RouteEntry* entryBegin;    // +0x3D8 vector begin
    RouteEntry* entryEnd;      // +0x3DC vector end (insert position)
    RouteEntry* entryCapacity; // +0x3E0 vector capacity end
    void* entryAllocator;      // +0x3E4 0x839220's allocator argument
};
TS3_X86_OFFSET(RouteManagerView, tree, 0x3BC);
TS3_X86_OFFSET(RouteManagerView, entryBegin, 0x3D8);
TS3_X86_OFFSET(RouteManagerView, entryEnd, 0x3DC);
TS3_X86_OFFSET(RouteManagerView, entryCapacity, 0x3E0);
TS3_X86_OFFSET(RouteManagerView, entryAllocator, 0x3E4);

// TS3W.exe 0x00C3A0D0: __stdcall, walks a chain of 16-byte nodes and returns the first
// whose key matches (bytes compared in the order 2,0,1,5,3,4), else null. Read-only; the
// third argument (the hash) is unused. Returns the node pointer in EAX.
void* __stdcall FindRouteByKey(void* head, const uint8_t* key, uint32_t hash);

// TS3W.exe 0x00C3A120: __thiscall, pushes every node of each chain in `heads` onto the
// table's free list and zeroes the head slots. The next pointer is read before the node's
// key bytes are overwritten by the free-list link.
void FreeRouteChains(RouteTable* self, void** heads, uint32_t count);

// TS3W.exe 0x00C3A8B0: __stdcall, walks the whole bucket array (route link at node+8,
// next at node+0xC), accumulates the 44-byte record count into +0x00 and the u16 unit
// count into +0x04, and returns the bucket count (EAX is reloaded from +0x10 for the
// end-marker compare on every step). Pointer differences divide by 44 with the compiler's
// signed magic.
uintptr_t __stdcall CountRouteTotalsForward(RouteTableView* view);

// TS3W.exe 0x00C3A940: __stdcall, identical to CountRouteTotalsForward for nodes whose
// route link is at +0x0C and next at +0x10; also returns the bucket count.
uintptr_t __stdcall CountRouteTotalsReverse(RouteTableView* view);

// TS3W.exe 0x00C3A9D0: __thiscall, removes every node with the key from its bucket chain
// (all consecutive matches from the first match on), free-lists them and returns how many
// nodes the live count dropped by.
uint32_t RemoveRoutesByKey(RouteTable* self, const uint8_t* key);

// TS3W.exe 0x00C3AAC0: __thiscall, looks the key up; on a hit out = {node, bucket slot},
// on a miss out = {end-marker node, one past the buckets}. Returns out.
NodePair* FindRouteOrEnd(RouteTable* self, NodePair* out, const uint8_t* key);

// TS3W.exe 0x00C3ABA0: __stdcall, walks a chain (next at +0x10) and returns the first
// node whose dword at +8 is equal and whose two floats at +0/+4 are within 0.001f of the
// query (NaN compares unequal; the third argument is unused).
void* __stdcall FindNodeByFloatKey(void* head, const void* key, uint32_t unused);

// TS3W.exe 0x00C3AC20: __thiscall, releases and free-lists every node of each chain in
// `heads` (value at node+8: refcount at +4, released through vtable slot 0 with argument
// 1 when it drops to 0) and zeroes the head slots. Returns the free-list sentinel pointer
// on the last push (undefined in the original when no node was pushed).
uintptr_t ReleaseRouteChains(RouteTable* self, void** heads, uint32_t count);

// TS3W.exe 0x00C3B1B0: __thiscall, unlinks `node` from the chain headed at *slot, releases
// its value like ReleaseRouteChains, free-lists the node and decrements the live count;
// out gets the node's chain link (advanced to the next non-empty bucket when null) and the
// slot. Returns out.
NodePair* UnlinkRouteNode(RouteTable* self, NodePair* out, RouteHashNode* node, RouteHashNode** slot);

// TS3W.exe 0x00C3B370: __thiscall, erases [first, end) by forward-copying whole 44-byte
// records down to dest; end moves by 44*((first-dest)/-44). Returns dest.
void* EraseRouteSpan(void* vec, void* dest, void* first);

// TS3W.exe 0x00C3BE90: __thiscall, inserts {a2, *a5} into the tree at this+0x3BC through
// 0x0066F5A0, appends {a3, a4/3, *a6, *a7, a2} to the entry array at this+0x3D8 (inline
// when cur < end, else through 0x00839220), then adds a3 to *a6, a4 to *a7 and 1 to *a5.
// Returns a7.
uint32_t* AppendRouteEntry(RouteManagerView* self, uint32_t a2, uint32_t a3, uint32_t a4,
                           uint32_t* a5, uint32_t* a6, uint32_t* a7);

// TS3W.exe 0x00C50370: __thiscall, finds the key in the table and returns node+8, or
// inserts a new node through 0x00C4E680 (which copies a 12-byte key: the 6 key bytes, two
// uninitialised stack padding bytes and a zero dword) and returns the new node+8.
void* GetOrInsertRoutePayload(RouteTable* self, const uint8_t* key);

// TS3W.exe 0x00C398C0: __thiscall, copies the u16 units of every route in the table
// (route link at node+8, next at node+0xC) into `out` starting at the entry's +0x0C
// field, adding the running 44-byte record count as a packed 0x00010001 base to each
// copied dword (the 32-bit add can carry into the high u16) and as a 16-bit add to an odd
// trailing unit. Returns the last record count (the end-marker pointer when the table is
// empty).
uintptr_t CopyRouteUnitsForward(RouteManagerView* self, RouteTableView* view, uint16_t* out,
                                uint32_t entryIndex);

// TS3W.exe 0x00C399E0: __thiscall, identical to CopyRouteUnitsForward for nodes whose
// route link is at +0x0C and next at +0x10.
uintptr_t CopyRouteUnitsReverse(RouteManagerView* self, RouteTableView* view, uint16_t* out,
                                uint32_t entryIndex);

}  // namespace ts3::misc