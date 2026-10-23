// Cluster unit clu_727290_ResourceSystem (ResourceSystem): the ten TS3W.exe functions at
// 0x00727290, 0x007293E0, 0x0072A520, 0x0072AD90, 0x0072EC00, 0x00731DD0, 0x00732130,
// 0x00732B60, 0x00732E90 and 0x009953C0, rewritten together: they share the resource
// cache (LRU + hash map), the DirectoryManager singleton and the refcount machinery.
// x86 layouts are asserted below; x64 builds get their own layout, as pointer-sized
// fields grow (see ts3/layout.h). Nothing here runs against game memory on x64 - the
// game callees are reached through the layout.h bridge, which aborts on x64.
#pragma once

#include <cstdint>

#include "ts3/layout.h"
#include "ts3/thread/mutex.h"

namespace ts3::resourcesystem {

// The vector view the game uses for collect-and-release lists: {begin, cursor, cap, tag},
// tag being the allocator-name string passed when a push grows the buffer (the originals
// grow through 0x008621C0 or 0x00B7B1E0, both thiscall ret 8 on (cursor, &value)).
struct GameVec {
    void* begin;
    void* cursor;
    void* cap;
    const char* tag;
};
TS3_X86_SIZE(GameVec, 0x10);

// TrimCache's fixed vector has an extra allocator field after the common prefix.
// Both 0x007326B0 (range insert) and 0x00B7B1E0 (push) read +0x10 before freeing
// an old buffer: the inline stack arena must be exempt. A separate local variable
// with the same value does not supply that native layout.
struct InlineGameVec {
    void* begin;
    void* cursor;
    void* cap;
    const char* tag;
    void* inlineBegin;
};
TS3_X86_OFFSET(InlineGameVec, inlineBegin, 0x10);
TS3_X86_SIZE(InlineGameVec, 0x14);

// ---------------------------------------------------------------------------
// 0x007293E0's container: a job-tag hash whose bucket array is inline at +0x80
// (191 slots) with a non-null sentinel link value in the last slot at +0x37C;
// the sentinel doubles as the terminator of the bucket walk.
// ---------------------------------------------------------------------------
struct ResourceDirContainer {
    uint8_t unknown0[0x40];
    thread::Mutex mutex;   // +0x40 (lock tag string 0x00FFD2DC)
    uint8_t gap70[0x10];   // +0x70..+0x7F
    void* buckets[191];    // +0x80 — each entry holds &ResourceDirNode::next (a link
                           //         address, i.e. node+8), not the node itself
    void* sentinel;        // +0x37C — sentinel link value the walk stops on
};
TS3_X86_OFFSET(ResourceDirContainer, mutex, 0x40);
TS3_X86_OFFSET(ResourceDirContainer, buckets, 0x80);
TS3_X86_OFFSET(ResourceDirContainer, sentinel, 0x37C);
TS3_X86_SIZE(ResourceDirContainer, 0x380);

// A node of that hash: buckets chain through +8, the job tag lives at +0x2C and the
// refcounted job pointer at +0xA8. 0x007293E0 walks links as (node+8) addresses.
struct ResourceDirNode {
    uint8_t head[8];         // +0x00 (vtable etc.; not read by this unit)
    ResourceDirNode* next;   // +0x08 — a chain link IS &next of the node
    uint8_t pad2[0x20];      // +0x0C..+0x2B
    uint32_t tag;            // +0x2C — the tag 0x007293E0 releases jobs for
    uint8_t pad3[0x78];      // +0x30..+0xA7
    void* job;               // +0xA8 — refcounted job (AddRef 0x00599460, release pair)
};
TS3_X86_OFFSET(ResourceDirNode, next, 0x08);
TS3_X86_OFFSET(ResourceDirNode, tag, 0x2C);
TS3_X86_OFFSET(ResourceDirNode, job, 0xA8);
TS3_X86_SIZE(ResourceDirNode, 0xAC);

// ---------------------------------------------------------------------------
// 0x0072EC00's DirectoryManager singleton: 20 bytes, vector {begin,end,cap} of
// package names at +4 and the "…/mPackages" tag string at +0x10.
// ---------------------------------------------------------------------------
struct DirectoryManager {
    void* vtable;         // +0x00 = 0x00FFDAE8
    void* packagesBegin;  // +0x04
    void* packagesEnd;    // +0x08
    void* packagesCap;    // +0x0C
    const char* name;     // +0x10 = 0x00FFDAB0
};
TS3_X86_SIZE(DirectoryManager, 0x14);

// ---------------------------------------------------------------------------
// The resource cache: hash map at +0x1C, LRU list through the sentinel node whose
// next/prev link fields sit at +0x3C/+0x40, pending-release vector at +0x48,
// mutex at +0x58 and the byte counters at +0x8C..+0x98. 0x00732130's `this` is
// &ResourceCache::map; link fields store handles = (nodeBase + 0x18).
// ---------------------------------------------------------------------------
struct CacheMap {
    void* allocStorage;         // +0x00 (eastl allocator word; not read by this unit)
    struct CacheHashNode** buckets;  // +0x04 — buckets[bucketCount] is the sentinel slot
    uint32_t bucketCount;       // +0x08
    uint32_t elementCount;      // +0x0C
    uint8_t gap10[0x0C];        // +0x10..+0x1B
    struct CacheHashNode** freeList;  // +0x1C — ADDRESS OF the free-list head slot
};
TS3_X86_SIZE(CacheMap, 0x20);

// A hash node of CacheMap: keys are the four dwords of the 16-byte resource key,
// value points at the CacheEntry, next (+0x18, confirmed by 0x00732130's
// the original instruction sequence) chains the bucket. While the node sits on the map's free
// list, +0x00 holds the next free node instead (0x00732130 writes [node+0] when
// pushing; the key dword is rewritten by the next insert).
struct CacheHashNode {
    union {
        uint32_t key[4];             // +0x00..+0x0F in-bucket
        struct CacheHashNode* freeNext;  // +0x00 while free
    };
    void* value;                     // +0x10 — the CacheEntry (LRU node base)
    uint32_t unknown14;              // +0x14 — not read by this unit
    struct CacheHashNode* next;      // +0x18 — bucket-chain link
};
TS3_X86_OFFSET(CacheHashNode, value, 0x10);
TS3_X86_OFFSET(CacheHashNode, next, 0x18);
TS3_X86_SIZE(CacheHashNode, 0x1C);

// 0x00732130's iteration pair: the erased node's successor and the bucket to keep
// walking from (the original returns `out` in eax).
struct CacheMapIter {
    CacheHashNode* node;
    CacheHashNode** bucket;
};

struct CacheEntry;

// Slots of a cache entry's vtable this unit calls: +8 destroys a fully released
// entry (thiscall ret 4, one int flag), +0x14 answers "may this entry be trimmed?"
// for 0x00731DD0.
struct CacheEntryVTable {
    void* slot0;                                            // +0x00
    void* slot1;                                            // +0x04
    int32_t(__thiscall* destroy)(CacheEntry* self, int32_t arg);  // +0x08
    void* slot3;                                            // +0x0C
    void* slot4;                                            // +0x10
    bool(__thiscall* keep)(CacheEntry* self);               // +0x14
};

// One cache entry: refcount in units of 0x100 at +4, the 16-byte key at +8, the
// LRU links at +0x18/+0x1C (handles = otherBase + 0x18), size at +0x20, flags at
// +0x24 (cleared when the entry leaves the cache).
struct CacheEntry {
    void* vtable;         // +0x00 (CacheEntryVTable view)
    int32_t refCount;     // +0x04 — 0x00731DD0 adjusts it with lock xadd
    uint32_t key[4];      // +0x08..+0x17
    void* listNext;       // +0x18 — handle of the next entry (nextBase + 0x18)
    void* listPrev;       // +0x1C — handle of the previous entry (prevBase + 0x18)
    uint32_t size;        // +0x20 — byte cost charged against the budget
    uint32_t flags;       // +0x24
};
TS3_X86_SIZE(CacheEntry, 0x28);

// The filter objects 0x00732B60 / 0x00732E90 accept: slot 1 (+4) is asked about a
// key (the hash node itself for 732B60, &entry->key for 732E90) and returns whether
// the entry may be touched. thiscall ret 4.
struct CacheFilterVTable {
    void* slot0;
    bool(__thiscall* accepts)(void* self, const void* key);  // +0x04
};

// The cache object 0x00732B60 and 0x00732E90 operate on. The lock at +0x58 (tag
// 0x00FFDD34) is held as raw bytes: thread::Mutex is alignas(8), and a member of
// that type would pad this object to 0xA0 while the game's ResourceCache ends at
// 0x9C - the mutex itself sits 8-aligned at +0x58 anyway. Code views it with the
// CacheMutex helper in the .cpp.
struct ResourceCache {
    void* vtable;            // +0x00
    uint8_t unknown04[0x18]; // +0x04..+0x1B
    CacheMap map;            // +0x1C
    void* lruAnchorNext;     // +0x3C — handle of the first entry (anchor's next)
    void* lruAnchorPrev;     // +0x40 — handle of the last entry (anchor's prev)
    uint32_t anchorSize;     // +0x44 (list sentinel bookkeeping; not read here)
    void* pendingBegin;      // +0x48 — entries queued for release
    void* pendingEnd;        // +0x4C
    void* pendingCap;        // +0x50
    uint8_t gap54[4];        // +0x54..+0x57
    uint8_t mutex[0x30];     // +0x58 — thread::Mutex storage (see above)
    uint8_t gap88[4];        // +0x88..+0x8B
    uint32_t budget;         // +0x8C — target cache size in bytes
    uint32_t minKeep;        // +0x90 — trimming floor unless forced
    uint32_t pendingBytes;   // +0x94 — bytes queued for release
    uint32_t cachedBytes;    // +0x98 — bytes currently cached
};
TS3_X86_OFFSET(ResourceCache, map, 0x1C);
TS3_X86_OFFSET(ResourceCache, lruAnchorNext, 0x3C);
TS3_X86_OFFSET(ResourceCache, pendingBegin, 0x48);
TS3_X86_OFFSET(ResourceCache, mutex, 0x58);
TS3_X86_OFFSET(ResourceCache, budget, 0x8C);
TS3_X86_OFFSET(ResourceCache, minKeep, 0x90);
TS3_X86_OFFSET(ResourceCache, pendingBytes, 0x94);
TS3_X86_OFFSET(ResourceCache, cachedBytes, 0x98);
TS3_X86_SIZE(ResourceCache, 0x9C);

// ---------------------------------------------------------------------------
// The ten entry points.
// ---------------------------------------------------------------------------

// TS3W.exe 0x00727290 (cdecl): stores `value` into the global object slot at
// 0x011D3C84
uint32_t SwapResourceSlotA(uint32_t value);
// TS3W.exe 0x0072A520 (cdecl): the same swap on the slot at 0x011D3C88.
uint32_t SwapResourceSlotB(uint32_t value);

// TS3W.exe 0x007293E0 (thiscall ret 4): under the container lock, AddRefs the job
// of every node whose tag matches, collects them into a vector, then outside the
// lock releases each collected job and frees the vector buffer. Returns the last
// callee's eax (Unlock's when nothing was collected, 0x004E3F70's when it was).
int32_t ReleaseTaggedJobs(ResourceDirContainer* self, uint32_t tag);

// TS3W.exe 0x0072AD90 (thiscall ret 8): if the 0x011CDD00 singleton exists, runs
// the game's lookup through it (0x0058C4F0 with callback 0x0072ACC0 and a 16-byte
// context {self, name, wcslen(name), flag=1}); returns the context's flag byte,
// which the callback is expected to overwrite.
uint8_t LookupResource(void* self, void* src, const uint16_t* name);

// TS3W.exe 0x0072EC00 (cdecl): the DirectoryManager singleton at 0x011D3C90,
// created on first use (20 bytes, vtable 0x00FFDAE8) under the mutex at 0x011D3C98.
void* GetDirectoryManager();

// TS3W.exe 0x00731DD0 (thiscall ret 0): releases 0x100 of the entry's refcount
// when it has more than 0x100 left, otherwise drops one and destroys the entry
// through vtable slot 2 when the count reaches zero. Returns the remaining count
// in the fast path, 0 otherwise.
int32_t ReleaseCacheEntryRef(CacheEntry* self);

// TS3W.exe 0x00732130 (thiscall ret 0xC): erases `node` from the bucket, pushes it
// onto the map's free list, decrements elementCount, and fills *out with the
// successor pair (next node, bucket to continue from). Returns `out`.
CacheMapIter* EraseCacheMapNode(CacheMap* self, CacheMapIter* out, CacheHashNode* node,
                                CacheHashNode** bucket);

// TS3W.exe 0x00732B60 (thiscall ret 4): walks the hash buckets, optionally filtered
// by `filter` (slot 1), unlinks matching entries from the LRU, accounts their bytes
// and collects them; afterwards decrements each collected entry's refcount. Returns
// the last callee's eax as 0x00731DD0 / 0x004E3F70 produce it.
int32_t ReleaseCacheRefs(ResourceCache* self, void* filter);

// TS3W.exe 0x00732E90 (thiscall ret 0xC): drains the pending-release vector, then
// unlinks LRU entries until the cached bytes fit `budget - bytes` (or `force` says
// keep going down to `minKeep`), collecting them for release; returns the number of
// bytes freed.
uint32_t TrimCacheTo(ResourceCache* self, uint32_t bytes, void* filter, uint8_t force);

// TS3W.exe 0x009953C0 (cdecl): looks the DirectoryManager table entry up and formats
// the key into the {begin,end} span object at `out`; 1 on success, 0 when there is
// no manager or the entry is missing.
uint8_t FormatResourceKey(void* out);

}  // namespace ts3::resourcesystem
