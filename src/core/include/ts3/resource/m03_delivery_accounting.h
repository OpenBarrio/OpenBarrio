#pragma once
#include "ts3/thread/mutex.h"
#include "ts3/resource/compressed_index.h"

namespace ts3::resource::m03::accounting {
// Bounded access views, NOT complete catalog/node allocation-size declarations.
// Only the used fields are named; unknown bytes remain uninterpreted.
struct OwnerView {
    unsigned char unused00[0x58];
    ts3::thread::Mutex mutex;
    unsigned char unused88[0x0c];
    uint32_t unlinkedBytes;
    uint32_t linkedBytes;
};
struct LinkView; // Opaque identity of the two-pointer link subobject (not a node base).
struct NodeView {
    void* vtable; // Widened independently: packedReferences is never inside this pointer.
    uint32_t packedReferences;
    unsigned char unused00[0x18 - sizeof(void*) - sizeof(uint32_t)];
    LinkView* previous;
    LinkView* next;
    uint32_t size;
    OwnerView* owner;
};
TS3_X86_OFFSET(OwnerView, mutex, 0x58);
TS3_X86_OFFSET(OwnerView, unlinkedBytes, 0x94);
TS3_X86_OFFSET(OwnerView, linkedBytes, 0x98);
TS3_X86_OFFSET(NodeView, vtable, 0);
TS3_X86_OFFSET(NodeView, packedReferences, 4);
static_assert(offsetof(NodeView, packedReferences) % alignof(uint32_t) == 0);
TS3_X86_OFFSET(NodeView, previous, 0x18);
TS3_X86_OFFSET(NodeView, next, 0x1c);
TS3_X86_OFFSET(NodeView, size, 0x20);
TS3_X86_OFFSET(NodeView, owner, 0x24);

inline LinkView* LinkOf(NodeView* node) {
    return reinterpret_cast<LinkView*>(&node->previous);
}
// Exact external boundary only: no accounting/provider policy. Null boundary uses
// the original EA mutex on x86, and the canonical native Mutex on x64.
struct MutexBoundary {
    void* context;
    int (*lock)(void*, ts3::thread::Mutex&, const ts3::thread::ThreadTime&);
    int (*unlock)(void*, ts3::thread::Mutex&);
};
// Slot signatures are bounded used contracts, not a full EA vtable declaration.
#if defined(_M_IX86)
using ReleasePredicate = uint32_t(__thiscall*)(NodeView*);
using ReleaseDelete = void(__thiscall*)(NodeView*, uint32_t);
#else
using ReleasePredicate = uint32_t(*)(NodeView*);
using ReleaseDelete = void(*)(NodeView*, uint32_t);
#endif
struct ReleaseTable {
    void* unused0[2];
    ReleaseDelete destroy;
    void* unused3[2];
    ReleasePredicate predicate;
};
TS3_X86_OFFSET(ReleaseTable, destroy, 8);
TS3_X86_OFFSET(ReleaseTable, predicate, 0x14);
// Cross-family 7328B0 only. The accounting view is NOT cast to native Catalog/
// CacheEntry: their independently expanded layouts are not interchangeable.
struct ReleaseBoundary {
    void* context;
    void (*insertLocked)(void*, OwnerView*, NodeView*);
};
// TS3W.exe 0x00731D40: two atomic packed decrements, signed branch/count, AL-only virtual gate and current-table deletion. Wrap and XADD-result ZF retained.
int32_t Release(NodeView* node, const ReleaseBoundary* boundary = nullptr);
// TS3W.exe 0x00731D00: notify the real idle transition at count one, then atomically add 0x100 and return signed packed count. Flags, wrap and unsafe inputs retained.
int32_t AddRef(NodeView* node, const MutexBoundary* boundary = nullptr);
// TS3W.exe 0x00731CC0: set size directly when detached, otherwise call Account with the node's old size. No null-node repair or EAX return contract.
void SetSize(NodeView* node, uint32_t newSize);
// TS3W.exe 0x00731F60: charge new-old to the linkage-selected counter and publish size under the mutex. Unsigned wrap and ignored lock result retained.
void Account(OwnerView* owner, NodeView* node, uint32_t oldSize, uint32_t newSize,
             const MutexBoundary* boundary = nullptr);
// TS3W.exe 0x00731FF0: repair both neighbors and store the previous node base in out; return out. Receiver unused; invalid null links retain EA's unsafe low-address accesses.
NodeView** Unlink(void* receiver, NodeView** out, NodeView* cursor);
// TS3W.exe 0x007320D0: unlink a linked node and transfer its size to idle accounting under the mutex; return unlock result. Unsigned wrap and ignored lock result retained.
int MakeIdle(OwnerView* owner, NodeView* node, const MutexBoundary* boundary = nullptr);
}

namespace ts3::resource::m03::accounting {
// Independent borrowed pool view, NOT OwnerView or a complete class-size claim.
// The raw Windows critical section has no EA Mutex lockCount/timeout wrapper.
struct PoolView {
    unsigned char opaque00[8];
    alignas(void*) unsigned char critical[sizeof(void*) == 4 ? 24 : 40];
    unsigned char opaque20[8];
    unsigned char slots[0x80000];
    uint32_t bitmap[128];
};
TS3_X86_OFFSET(PoolView, critical, 8);
TS3_X86_OFFSET(PoolView, slots, 0x28);
TS3_X86_OFFSET(PoolView, bitmap, 0x80028);
struct PoolBoundary {
    void* context;
    void* allocator;
    void (*enter)(void*, void*);
    void (*leave)(void*, void*);
    void (*deallocate)(void*, void*, void*);
};
// TS3W.exe 0x00733D40: return an in-range 128-byte slot by OR under the raw critical section, otherwise deallocate externally. Floors interior pointers; second stack argument unused; no EAX contract.
void ReturnPool(PoolView* pool, void* block, uint32_t unused, const PoolBoundary& boundary = {});
}

namespace ts3::resource::m03::accounting {
// Independent USED activity prefix, not a packed-reference node or complete class.
// Storage holds an initialized Windows CRITICAL_SECTION at its host-native size.
struct ActivityView {
    unsigned char opaque00[0xb0];
    void* retained;
    unsigned char opaqueB4[4];
    alignas(void*) unsigned char critical[sizeof(void*) == 4 ? 24 : 40];
    unsigned char opaqueD0[8];
    uint32_t activity;
    unsigned char opaqueDC[4];
    uint32_t timestampLow;
    uint32_t timestampHigh;
    unsigned char opaqueE8[0x14];
    uint32_t finalState;
};
TS3_X86_OFFSET(ActivityView, retained, 0xb0);
TS3_X86_OFFSET(ActivityView, finalState, 0xfc);
TS3_X86_OFFSET(ActivityView, critical, 0xb8);
TS3_X86_OFFSET(ActivityView, activity, 0xd8);
TS3_X86_OFFSET(ActivityView, timestampLow, 0xe0);
TS3_X86_OFFSET(ActivityView, timestampHigh, 0xe4);
struct ActivityBoundary {
    void* context;
    void (*enter)(void*, void*);
    void (*leave)(void*, void*);
    int (*counter)(void*, uint32_t*);
};
// TS3W.exe 0x007347C0: locked modulo-32 subtract two, reload, timestamp only at one, then unlock; no meaningful EAX contract.
void ReleaseActivity(ActivityView* self, const ActivityBoundary& boundary = {});
}

namespace ts3::resource::m03::accounting {
// Only actual external service calls, never a provider for the subject algorithm.
struct AcquireBoundary {
    void* context;
    void (*enter)(void*, void*);
    void (*leave)(void*, void*);
    uint32_t (*initialize)(void*, ActivityView*, uint32_t, uint32_t, uint32_t);
    uint32_t (*enable)(void*, ActivityView*, uint32_t);
    uint32_t (*cleanup)(void*, void*);
    uint32_t (*query)(void*, ActivityView*, uint32_t, uint32_t);
};
// TS3W.exe 0x00734710: acquire activity, initialize first use, release retained handle and query after unlock. AL-only gate/result, modulo overflow and separate reloads retained.
bool AcquireActivity(ActivityView* self, const AcquireBoundary& boundary = {});
}

namespace ts3::resource::m03::accounting {
// Independent USED view of the 0x007343C0 receiver only: the +0xB8 field is a retained
// pointer released through 72DB80. It is NOT the wave7/8 ActivityView critical section
// at +0xB8, and this is no complete class or allocation-size claim. Reached only through
// vtable 00FFE0D0 slot 3 (xref 00FFE0DC); no direct code caller exists.
struct WatchReaderView {
    // 72C920 reads exactly one dword at +0x1C of this inline self+8 subobject.
    unsigned char unused00[0x1c - (sizeof(void*) - 4)];
    void* identity; // Opaque full-width file identity handed to 4DBC60/4DBCF0.
};
struct WatcherView {
    unsigned char unused00[8];
    WatchReaderView reader;
    unsigned char unused28[0xb8 - 8 - sizeof(WatchReaderView)];
    void* retained;
    unsigned char unusedBC[0xf8 - 0xb8 - sizeof(void*)];
    uint32_t stampLow;
    uint32_t stampHigh;
    uint32_t size;
    unsigned char unused104[0x10];
};
TS3_X86_OFFSET(WatchReaderView, identity, 0x1c);
TS3_X86_OFFSET(WatcherView, reader, 8);
TS3_X86_OFFSET(WatcherView, retained, 0xb8);
TS3_X86_OFFSET(WatcherView, stampLow, 0xf8);
TS3_X86_OFFSET(WatcherView, stampHigh, 0xfc);
TS3_X86_OFFSET(WatcherView, size, 0x100);
// Only actual external file-identity/metadata/release leaves, never a provider for the
// subject algorithm. 72C920 is thiscall on self+8 returning the FULL EAX identity;
// 4DBC60 is cdecl(identity) with caller-popped 4; 4DBCF0 is cdecl(identity, mode) with
// caller-popped 8 returning the full 64-bit EDX:EAX pair; 72DB80 is thiscall on the
// retained pointer and may destroy it (never the receiver).
struct WatchBoundary {
    void* context;
    void* (*identity)(void*, WatchReaderView*);
    uint32_t (*size)(void*, void*);
    uint64_t (*stamp)(void*, void*, uint32_t);
    uint32_t (*release)(void*, void*);
};
// TS3W.exe 0x007343C0: query the watched file's size, then its 64-bit stamp with mode 2,
// compare full dwords in exact order; on any mismatch reload the CURRENT retained pointer
// AFTER the query callbacks, release it through 72DB80 BEFORE clearing the field and
// return AL=1 (also when it was already null); on full match return AL=0 with the pointer
// intact and no cleanup. AL-only result (upper EAX dirty), full-width compares and
// short-circuit (no second query after a size mismatch) retained.
bool CheckWatcherChanged(WatcherView* self, const WatchBoundary& boundary = {});
}

namespace ts3::resource::m03::accounting {
// Independent USED view of the 0x007341F0 receiver only (cls_532c30_x, reached solely
// through vtable 00FFE054 slot 1, xref 00FFE058; slot 0 is the shared 009D6C70 FB2CBC
// base scalar delete). The initializing producer 0x007098C0 stores the vtable word, then
// copies the four canonical ResourceKey VALUE words to +8..+0x14 in order (this[2]=a2[0],
// this[3]=a2[1], this[4]=a2[2], this[5]=a2[3]); the same placement is produced by 7354F0
// and AD5540, which both leave +4 unwritten. The words are resource instance/type/group
// identities, never pointers. Bytes past +0x18 stay uninterpreted: no complete class or
// allocation-size claim.
struct KeyRecordView {
    void* identity; // +0: original vtable word; never read by 0x007341F0.
    uint32_t unused04; // +4: producer-uninitialized gap; never read.
    ResourceKey key; // +8..+0x14: four 32-bit value words (instanceLow/High, type, group).
};
TS3_X86_OFFSET(KeyRecordView, identity, 0);
TS3_X86_OFFSET(KeyRecordView, unused04, 4);
TS3_X86_OFFSET(KeyRecordView, key, 8);
// TS3W.exe 0x007341F0: four full 32-bit key-value compares in EA's exact order - word0
// (input+0 vs self+8), word2 (input+8 vs self+0x10), word3 (input+C vs self+0x14), word1
// (input+4 vs self+C) - each short-circuiting to full EAX 0; full EAX 1 iff all four are
// equal. Raw thiscall ECX receiver, one stack key pointer, ret 4. No calls, no writes,
// no null repair, no ignored high bits.
uint32_t KeyRecordEquals(const KeyRecordView* self, const ResourceKey* key);
}

namespace ts3::resource::m03::accounting {
// Wave11: TWO DIFFERENT USED paired UTF16 bucket-redistribution contracts; the similar
// loops never license aliasing the node layouts. 0x00734860 is called only by 0x007356D0
// (xref 0x00735794), whose insert allocates 12-byte nodes via 0x004E3F90(12, ...) and
// stores the key pointer at +0, the mapped payload at +4 and the chain link at +8.
// 0x00734940 is called only by 0x007357F0 (xref 0x007358AB), whose insert allocates
// 8-byte nodes via 0x004E3F90(8, ...) and stores the key pointer at +0 and the chain
// link at +4. Both maps borrow the same USED words (+4 buckets, +8 old count, +0x1C
// allocator identity for 0x004E3F90's name argument) but stay SEPARATE bounded views.
// Wave13 expanded two more USED cells of the SAME wide map (0x007356D0 reads +0xC and
// reaches the policy at +0x10 through 0x004E09B0): elementCount +0xC and the growth
// policy prefix +0x10..+0x1B below. Every other byte stays uninterpreted. No complete
// EASTL hashtable class, element-count or allocation-size claim is made here.
struct PairedRehashPolicyView {
    float maxLoadFactor; // +0x10 in the map: 1.0f in game data; read by 0x004E09B0.
    float growthFactor;  // +0x14 in the map: 2.0f; read by 0x004E09B0.
    uint32_t nextResize; // +0x18 in the map: growth threshold; MUTATED by 0x004E09B0.
};
struct PairedWideNodeView {
    const char16_t* key; // +0: NUL-terminated UTF16 code units; hashed, never modified.
    void* payload; // +4: 0x007356D0's mapped-value store; redistribution never touches it.
    PairedWideNodeView* next; // +8: bucket chain link.
};
struct PairedCompactNodeView {
    const char16_t* key; // +0: NUL-terminated UTF16 code units; hashed, never modified.
    PairedCompactNodeView* next; // +4: bucket chain link; there is no payload word.
};
template <class Node>
struct PairedMapView {
    unsigned char unused00[4];
    Node** buckets; // +4 on x86: old bucket heads; freed externally only when count > 1.
    uint32_t bucketCount; // +8: current count; republished as newBucketCount on return.
    uint32_t elementCount; // +0xC: stored element count; 0x007356D0 increments it once per fresh insert.
    PairedRehashPolicyView policy; // +0x10: growth policy prefix reached through 0x004E09B0.
    const void* allocatorIdentity; // +0x1C on x86: passed unchanged as the allocation name.
};
using PairedWideMapView = PairedMapView<PairedWideNodeView>;
using PairedCompactMapView = PairedMapView<PairedCompactNodeView>;
TS3_X86_OFFSET(PairedWideNodeView, key, 0);
TS3_X86_OFFSET(PairedWideNodeView, payload, 4);
TS3_X86_OFFSET(PairedWideNodeView, next, 8);
TS3_X86_SIZE(PairedWideNodeView, 12);
TS3_X86_OFFSET(PairedCompactNodeView, key, 0);
TS3_X86_OFFSET(PairedCompactNodeView, next, 4);
TS3_X86_SIZE(PairedCompactNodeView, 8);
TS3_X86_OFFSET(PairedWideMapView, buckets, 4);
TS3_X86_OFFSET(PairedWideMapView, bucketCount, 8);
TS3_X86_OFFSET(PairedWideMapView, elementCount, 0xc);
TS3_X86_OFFSET(PairedWideMapView, policy, 0x10);
TS3_X86_OFFSET(PairedWideMapView, allocatorIdentity, 0x1c);
TS3_X86_SIZE(PairedWideMapView, 0x20);
TS3_X86_OFFSET(PairedCompactMapView, buckets, 4);
TS3_X86_OFFSET(PairedCompactMapView, bucketCount, 8);
TS3_X86_OFFSET(PairedCompactMapView, elementCount, 0xc);
TS3_X86_OFFSET(PairedCompactMapView, policy, 0x10);
TS3_X86_OFFSET(PairedCompactMapView, allocatorIdentity, 0x1c);
TS3_X86_SIZE(PairedCompactMapView, 0x20);
// True external allocation boundaries only, captured with the exact compiled cdecl ABIs:
// 0x004E3F90(size_t bytes, const void* allocatorIdentity, uint32_t flags, uint32_t zero,
// const void* sourceFile 0x00F98980, uint32_t line 0xE7) and 0x004E3F70(void* block).
// They provide memory and tracing; they never implement hashing, node moves, the free
// predicate or publication.
struct RedistributeBoundary {
    void* context;
    void* (*allocate)(void* context, size_t bytes, const void* allocatorIdentity, uint32_t flags,
                      uint32_t zero, const void* sourceFile, uint32_t line);
    void (*release)(void* context, void* block);
};
// TS3W.exe 0x00734860 (wide nodes, next +8): allocate newCount pointer slots plus one
// trailing 0xFFFFFFFF sentinel cell, clear the count slots, rehash every node of every
// old bucket (ascending index, popping the head, FNV-1 multiply-then-XOR over nonzero
// UTF16 units from 0x811C9DC5) into hash % newCount by exact-identity prepend - so each
// destination chain ends up reversed - then release the old table only when the old count
// exceeds one and publish the new pointer and count. thiscall ECX map, one stack uint32,
// ret 4; EAX holds newBucketCount only on the freed path (oldCount > 1), so no other EAX
// promise is made. newCount 0 with any node in the table is EA's division by zero and is
// not repaired; callers must pass a safe count (see the review note).
uint32_t RedistributeWide(PairedWideMapView* map, uint32_t newBucketCount,
                          const RedistributeBoundary* boundary = nullptr);
// TS3W.exe 0x00734940: the same published algorithm over the COMPACT set nodes (key +0,
// next +4). Identical ABI, sentinel, free predicate and reversal; only the chain offset
// differs, and the two node contracts are never aliased.
uint32_t RedistributeCompact(PairedCompactMapView* map, uint32_t newBucketCount,
                             const RedistributeBoundary* boundary = nullptr);
}

namespace ts3::resource::m03::accounting {
// Wave12: 0x00734D10 receiver view (IDA class DatabaseShadowWatchersSignaled_3,
// primary table 0x00FFE2F0 slot 5; sole xref is that table word at 0x00FFE304 - it is
// reached only through virtual dispatch, so the thiscall/ret4 ABI is the table's).
// Constructor 0x00736DB0 proves the used cells: the ADJUSTED interface subobject at
// +8 (it stores its own table word 0x00FFE250 there), the FIRST of the three
// EA::Thread::Mutex objects it initializes (+0x218, then +0x248, +0x278), and the
// {begin,end,capacity,tag} vector cells at +0x2B8/+0x2BC/+0x2C0/+0x2C4 whose tag is
// "ResourceSystem/DatabaseInfoList" (dwords 174..177). Only those cells are declared:
// this is NOT a complete allocation-size claim and never the keys-family owner.
struct NotifyInterfaceView {
    void* table; // +0 of the +8 subobject: CURRENT interface table word (0x00FFE250 in game).
};
struct NotifyOwnerView {
    unsigned char unused00[8];
    NotifyInterfaceView interface; // +8: embedded adjusted subobject, receiver of slot 19.
    unsigned char unused0C[0x218 - 8 - sizeof(NotifyInterfaceView)];
    ts3::thread::Mutex mutex; // +0x218: recursive EA mutex held across the whole pass.
    unsigned char unused248[0x2B8 - 0x218 - sizeof(ts3::thread::Mutex)];
    void** retained;            // +0x2B8: array of retained-object pointers (4-byte EA slots).
    void* const* retainedEnd;   // +0x2BC: one-past-end, snapshotted AFTER the lock.
};
TS3_X86_OFFSET(NotifyOwnerView, interface, 8);
TS3_X86_OFFSET(NotifyOwnerView, mutex, 0x218);
TS3_X86_OFFSET(NotifyOwnerView, retained, 0x2B8);
TS3_X86_OFFSET(NotifyOwnerView, retainedEnd, 0x2BC);
// Honest pointer-slot arrays: the EA slot numbers (1,2,3,12,19) select entries, so the
// native layout is the real pointer-sized table, never a hard-coded x86 byte offset.
struct NotifyObjectTable {
    void* slots[4]; // slot1 +4: resource getter; slot2 +8: prefix operand; slot3 +0xC: gate.
};
struct NotifyResourceTable {
    void* slots[13]; // slot12 +0x30: scratch key-list collector.
};
struct NotifyInterfaceTable {
    void* slots[20]; // slot19 +0x4C: filtered delivery of one resource.
};
TS3_X86_SIZE(NotifyObjectTable, 0x10);
TS3_X86_SIZE(NotifyResourceTable, 0x34);
TS3_X86_SIZE(NotifyInterfaceTable, 0x50);
// The four-word temporary: EA zeroes the first three words and stores the table
// identity 0x00FFE150 ("ResourceSystem/KeyList") in the fourth. Producer 0x007D7A50
// builds the identical shape (CopyKeyList begin/end/capacity/tag), so the first three
// words stay full-width pointer fields on native; only the first word is read back.
struct NotifyScratch {
    void* begin;     // +0: re-read AFTER both callbacks; freed through 0x004E3F70 when non-null.
    void* end;       // +4 on x86.
    void* capacity;  // +8 on x86.
    const void* tag; // +0xC on x86: the 0x00FFE150 table identity.
};
TS3_X86_OFFSET(NotifyScratch, begin, 0);
TS3_X86_OFFSET(NotifyScratch, end, 4);
TS3_X86_OFFSET(NotifyScratch, capacity, 8);
TS3_X86_OFFSET(NotifyScratch, tag, 0xC);
TS3_X86_SIZE(NotifyScratch, 0x10);
// Actual lock/unlock/free seams only, never a provider for the notification algorithm.
struct NotifyLeaves {
    void* context;
    int (*lock)(void*, ts3::thread::Mutex&, const ts3::thread::ThreadTime&); // 0x004E16F0
    int (*unlock)(void*, ts3::thread::Mutex&);                               // 0x004E17B0
    void (*release)(void*, void* block);                                     // 0x004E3F70
};
// Genuine external comparison boundary: 0x004ED480, canonical production form
// ts3::resourcesystem::HasPathPrefix(prefix, path, mode). The subject always passes
// mode 5 and consumes ONLY the low AL of the returned word. A native host must
// install this typed binding (the canonical mode-5 folding table is image data);
// no traversal or match policy crosses this seam.
using NotifyFilterPrefix = uint32_t (*)(const char16_t* prefix, const char16_t* path, int mode);
NotifyFilterPrefix SetNotifyFilterPrefix(NotifyFilterPrefix binding);
// The 0x00FFE150 "ResourceSystem/KeyList" identity stored in the fourth scratch word.
const void* NotifyScratchTag();
// TS3W.exe 0x00734D10: lock owner+0x218 with the 0x00FFDFC4 timeout label, snapshot
// +0x2B8/+0x2BC once, then per retained object: optional prefix filter (object slot 2
// operand, 0x004ED480 mode 5, low AL only), eligibility gate (slot 3, low AL only),
// resource getter (slot 1, full pointer), zeroed {begin,end,capacity}+tag scratch,
// resource slot 12 with (scratch,0), owner+8 interface slot 19 with (resource,scratch),
// and free of the re-read final scratch first word via 0x004E3F70. Unlock once, even
// for an empty array. Every virtual table is reloaded before its call; the snapshot
// bounds are never recomputed. EA quirks kept: AL-only gates with dirty upper results,
// equality-only begin/end check (no ordering repair for a corrupt inverted range), and
// no EAX return contract (the original's EAX is incidental unlock residue).
void NotifyRetained(NotifyOwnerView* owner, const char16_t* filter, const NotifyLeaves* leaves = nullptr);
}

namespace ts3::resource::m03::accounting {
// Wave13: 0x007356D0's USED argument records. The pair is caller-owned storage whose
// words the body re-reads AFTER the growth/allocation callbacks (the borrowed hashing
// key pointer saved before them may legitimately differ from the stored pointer). The
// result record is 12 bytes on x86; the original writes +0 node, +4 bucket slot address
// and the inserted BYTE at +8, leaving +9..+0xB untouched.
struct PairedWideInsertPair {
    const char16_t* key; // +0: borrowed NUL-terminated UTF16 key pointer (identity key).
    void* payload; // +4: mapped value word copied into a fresh node after the callbacks.
};
struct PairedWideInsertResult {
    PairedWideNodeView* node; // +0: found node on a hit, the fresh node on a miss.
    PairedWideNodeView** bucketSlot; // +4 on x86: ADDRESS of the bucket head cell used.
    uint8_t inserted; // +8: 1 fresh insert, 0 existing identity hit.
    unsigned char untouched[3]; // +9..+0xB on x86: never written by the original.
};
TS3_X86_OFFSET(PairedWideInsertPair, key, 0);
TS3_X86_OFFSET(PairedWideInsertPair, payload, 4);
TS3_X86_SIZE(PairedWideInsertPair, 8);
TS3_X86_OFFSET(PairedWideInsertResult, node, 0);
TS3_X86_OFFSET(PairedWideInsertResult, bucketSlot, 4);
TS3_X86_OFFSET(PairedWideInsertResult, inserted, 8);
TS3_X86_SIZE(PairedWideInsertResult, 12);
// The exact used output record of 0x004E09B0 (thiscall ECX policy, four stack arguments
// (decision*, bucketCount, elementCount, elementAdd), ret 0x10): grow flag BYTE at +0,
// requested new bucket count at +4; the policy's +0x18 threshold is mutated in place.
// +1..+3 of the record stay unwritten in the original (it reuses a stack slot).
struct PairedWideRehashDecision {
    uint8_t grow;
    unsigned char pad[3];
    uint32_t newBucketCount;
};
TS3_X86_OFFSET(PairedWideRehashDecision, newBucketCount, 4);
TS3_X86_SIZE(PairedWideRehashDecision, 8);
// Portable exact bounded used form of 0x004E09B0's decision (max load factor 1.0,
// growth factor 2.0, EASTL's 256-entry prime table semantics; integer-valued float
// domain, so the 0xC00 ceil control word equals plain ceil). Same mutations and
// outputs as the original; used natively where no game image exists. This helper
// carries no independent function credit - the subject is 0x007356D0.
PairedWideRehashDecision* PairedWideRehashRequired(PairedRehashPolicyView* policy,
                                                   PairedWideRehashDecision* decision,
                                                   uint32_t bucketCount, uint32_t elementCount,
                                                   uint32_t elementAdd);
// TS3W.exe 0x007356D0: thiscall ECX map, THREE stack arguments (out record, pair,
// unused third) and ret 0xC. FNV-1 hash (multiply 0x01000193 then XOR, basis
// 0x811C9DC5) of the pair's borrowed NUL-terminated UTF16 key, bucket = hash %
// map+8 (unsigned; the original's division by zero with count 0 is not repaired),
// POINTER-identity traversal of the 12-byte nodes (never string contents). A hit
// writes node, the bucket cell address and inserted BYTE 0 and never allocates,
// grows, recounts or touches the policy. A miss asks the growth policy (true
// 0x004E09B0 with ECX map+0x10 on x86, the portable bounded form natively), then
// allocates one 12-byte node through the exact cdecl 0x004E3F90 ABI (bytes 12,
// map+0x1C identity, flags 0, zero 0, file 0x00F98980, line 0xE7), copies the
// CURRENT pair words only AFTER those callbacks, clears next (kept as an
// unconditional store even after the original's failed-allocation test), and on a
// nonzero grow flag rehashes through the REAL production RedistributeWide (the
// 0x00734860 body) with hash % newCount recomputed BEFORE it. The node is then
// prepended to the CURRENT table's bucket head, the element count is incremented
// once (uint32 wrap kept), and the record gets node, inserted BYTE 1 and the
// CURRENT post-growth bucket cell address, in that order. Full EAX = out on both
// paths. The third stack argument is proven unread (sole caller 0x00737F38 pushes a
// zeroed byte; the body only ever reads its arg1/arg2 stack cells).
PairedWideInsertResult* InsertWide(PairedWideMapView* map, PairedWideInsertResult* out,
                                   const PairedWideInsertPair* pair, const void* unusedThird,
                                   const RedistributeBoundary* boundary = nullptr);
}
