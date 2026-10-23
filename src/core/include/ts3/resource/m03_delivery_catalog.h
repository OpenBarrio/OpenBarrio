#pragma once
#include "ts3/resourcesystem/clu_727290_ResourceSystem.h"
#include "ts3/resource/m03_manager_contracts.h"

namespace ts3::resource::m03::delivery_catalog {
// Canonical owner, not a second guessed allocation layout. The flag occupies
// byte 4 of the owner's formerly unknown prefix on both pointer widths.
using Catalog = ts3::resourcesystem::ResourceCache;
TS3_X86_OFFSET(Catalog, unknown04, 4);
static_assert(sizeof(void*) != 4 || offsetof(Catalog, unknown04) + 4 == 8);
inline uint8_t& Enabled(Catalog* self) { return self->unknown04[4]; }

// Only the true external slot10 boundary; no whole-operation provider.
#if defined(_M_IX86)
using ReleaseCallback = int32_t(__thiscall*)(Catalog*, void*);
#else
using ReleaseCallback = int32_t(*)(Catalog*, void*);
#endif
struct CatalogTable { ReleaseCallback slots[11]; };
TS3_X86_OFFSET(CatalogTable, slots[10], 0x28);

// TS3W.exe 0x00731EA0: set zero flag to one, preserving noncanonical nonzero bytes; returns AL1.
uint8_t Activate(Catalog* self);
// TS3W.exe 0x00731EB0: clear enabled byte before own slot10(self,0), then return AL1.
uint8_t Deactivate(Catalog* self);

struct CatalogAttachment;
#if defined(_M_IX86)
using AttachmentRelease = int32_t(__thiscall*)(CatalogAttachment*, uint32_t);
#else
using AttachmentRelease = int32_t(*)(CatalogAttachment*, uint32_t);
#endif
struct AttachmentTable { AttachmentRelease release; };
struct CatalogAttachment { const AttachmentTable* vtable; };
TS3_X86_OFFSET(CatalogAttachment, vtable, 0);
TS3_X86_OFFSET(AttachmentTable, release, 0);

// Borrowed USED-prefix view, NOT an allocation-size claim or ResourceCache
// alias: the shared cache's +44 is labelled a uint32_t sentinel size. Here
// the body proves it is a native object pointer. Do not narrow it to that word.
struct CatalogPointerView {
    void* vtable;
    uint8_t unused04[0x40];
    CatalogAttachment* attachment;
};
TS3_X86_OFFSET(CatalogPointerView, attachment, 0x44);
static_assert(sizeof(CatalogPointerView::attachment) == sizeof(void*));

// TS3W.exe 0x00731F20: return the exact full-dword type identity B5962C35; no receiver reads.
uint32_t TypeIdentity(const CatalogPointerView* self);
// TS3W.exe 0x00731F30: on pointer change release old primary slot0(flags1) BEFORE publishing new.
// thiscall ret4; EAX is undefined on no-release paths, so no return value is exposed.
void SetAttachment(CatalogPointerView* self, CatalogAttachment* replacement);
}

namespace ts3::resource::m03::delivery_catalog {
using CatalogEntry = ts3::resourcesystem::CacheEntry;
using CatalogMap = ts3::resourcesystem::CacheMap;
using CatalogIterator = ts3::resourcesystem::CacheMapIter;
TS3_X86_OFFSET(CatalogEntry, key, 8);
TS3_X86_OFFSET(CatalogEntry, listNext, 0x18);
TS3_X86_OFFSET(CatalogEntry, listPrev, 0x1c);
TS3_X86_OFFSET(CatalogEntry, size, 0x20);
TS3_X86_OFFSET(CatalogMap, buckets, 4);
TS3_X86_OFFSET(CatalogMap, bucketCount, 8);
static_assert(offsetof(CatalogEntry, listPrev) == offsetof(CatalogEntry, listNext) + sizeof(void*));
static_assert(offsetof(Catalog, lruAnchorPrev) == offsetof(Catalog, lruAnchorNext) + sizeof(void*));

// Only external AD3F40 and mutex boundaries; no policy/insertion provider.
// find writes both iterator fields and returns out (thiscall ret8 in the image).
struct InsertionBoundaries {
    CatalogIterator* (*find)(CatalogMap*, CatalogIterator* out, const uint32_t* key);
    int (*lock)(ts3::thread::Mutex*, const ts3::thread::ThreadTime&);
    int (*unlock)(ts3::thread::Mutex*);
};
const InsertionBoundaries& OriginalInsertionBoundaries();
// TS3W.exe 0x00732570: if both links are null and lookup is not end, append and transfer uint32 size.
// ECX owner, one node stack argument, ret4; incidental EAX is not a result contract.
void InsertUnlinked(Catalog* self, CatalogEntry* node,
    const InsertionBoundaries& boundaries = OriginalInsertionBoundaries());
// TS3W.exe 0x007328B0: lock +58 with FFDD34 timeout, call the same insertion, then unlock; ret4.
void InsertUnlinkedLocked(Catalog* self, CatalogEntry* node,
    const InsertionBoundaries& boundaries = OriginalInsertionBoundaries());
}

namespace ts3::resource::m03::delivery_catalog {
// Existing canonical borrowed prefix, not a complete vector allocation claim.
using PointerVector = ts3::resourcesystem::InlineGameVec;
TS3_X86_OFFSET(PointerVector, begin, 0);
TS3_X86_OFFSET(PointerVector, cursor, 4);
TS3_X86_OFFSET(PointerVector, cap, 8);
TS3_X86_OFFSET(PointerVector, tag, 0xc);
TS3_X86_OFFSET(PointerVector, inlineBegin, 0x10);
// Only the external named-byte allocator/free; no insertion/copy/growth provider.
struct RangeMemoryBoundaries {
    void* (*allocate)(size_t bytes, const char* tag, uint32_t flags, uint32_t extra,
        const char* file, uint32_t line);
    void (*free)(void*);
};
const RangeMemoryBoundaries& OriginalRangeMemoryBoundaries();
// TS3W.exe 0x007326B0: insert pointer range in place, or grow/copy/free before publishing.
// ECX vector; position, first, last, unused stack arguments; ret16; no defined EAX result.
void InsertPointerRange(PointerVector* self, void** position, void* const* first,
    void* const* last, void* unused,
    const RangeMemoryBoundaries& memory = OriginalRangeMemoryBoundaries());
}

namespace ts3::resource::m03::delivery_catalog {
// Only the two external lifetime boundaries. Callbacks take full native pointers;
// destroy ends the receiver's logical lifetime, free may reclaim its storage.
struct DestructionBoundaries {
    void (*destroy)(Catalog*);
    void (*free)(void*);
};
const DestructionBoundaries& OriginalDestructionBoundaries();
// TS3W.exe 0x00732890: destroy unconditionally, free iff low-byte bit0, return original self.
// ECX self, one flags dword, ret4; never inspect receiver after either callback.
Catalog* DeleteCatalog(Catalog* self, uint32_t flags,
    const DestructionBoundaries& boundaries = OriginalDestructionBoundaries());
}

namespace ts3::resource::m03::delivery_catalog {
// Genuine external 00732E90 boundary; default is the existing portable owner.
using TrimBoundary = uint32_t (*)(Catalog*, uint32_t, void*, uint8_t);
TS3_X86_OFFSET(Catalog, budget, 0x8c);
// TS3W.exe 0x00733120: reread current budget and trim same receiver, null filter, force1.
// ECX receiver, plain ret; untyped scheduler callback has no proven observable result.
void ShrinkCallback(Catalog* self, TrimBoundary trim = ts3::resourcesystem::TrimCacheTo);
}

namespace ts3::resource::m03::delivery_catalog {
// Borrowed primary prefix only; neither dispatcher reads its subject's fields.
struct SchedulingService;
// Genuine 00733970 is cdecl(database*, key*, context*), plain ret. It is
// registered, never executed here; third argument is the monitor (S+0C),
// second is the key (S+08). Reuse the existing manager notification ABI.
using ChangeNotification = ts3::resource::m03::manager::FilterNotify;
// 00733CC0/00733CF0 forward only slot9. 00733340 observes three further slots of
// the SAME service object; each is read from the current table at its call site.
struct ReloadCandidate;
struct ReloadManager;
struct ReloadFallback;
struct ReloadTemporary;
#if defined(_M_IX86)
using ScheduleSlot = uint8_t(__thiscall*)(SchedulingService*, uint32_t,
    ChangeNotification, void*, void*, void*);
using ReloadDispatchSlot = void(__thiscall*)(SchedulingService*,
    const ts3::resource::ResourceKey* key, void* temporary, void*, void*, void*);
using ReloadLookupSlot = ReloadCandidate*(__thiscall*)(SchedulingService*,
    uint32_t type, int32_t sentinel);
using ReloadFallbackSlot = ReloadFallback*(__thiscall*)(SchedulingService*,
    const ts3::resource::ResourceKey* key);
using ReloadIdentitySlot = uint32_t(__thiscall*)(ReloadCandidate*);
using ReloadRouteSlot = void(__thiscall*)(ReloadManager*,
    const ts3::resource::ResourceKey* key);
using ReloadOutputSlot = uint8_t(__thiscall*)(ReloadFallback*,
    const ts3::resource::ResourceKey* key, void** output);
using ReloadReleaseSlot = void(__thiscall*)(ReloadTemporary*);
#else
using ScheduleSlot = uint8_t(*)(SchedulingService*, uint32_t,
    ChangeNotification, void*, void*, void*);
using ReloadDispatchSlot = void(*)(SchedulingService*,
    const ts3::resource::ResourceKey* key, void* temporary, void*, void*, void*);
using ReloadLookupSlot = ReloadCandidate*(*)(SchedulingService*,
    uint32_t type, int32_t sentinel);
using ReloadFallbackSlot = ReloadFallback*(*)(SchedulingService*,
    const ts3::resource::ResourceKey* key);
using ReloadIdentitySlot = uint32_t(*)(ReloadCandidate*);
using ReloadRouteSlot = void(*)(ReloadManager*,
    const ts3::resource::ResourceKey* key);
using ReloadOutputSlot = uint8_t(*)(ReloadFallback*,
    const ts3::resource::ResourceKey* key, void** output);
using ReloadReleaseSlot = void(*)(ReloadTemporary*);
#endif
// All 23 low entries are pointer-sized; the unused ones stay opaque so the
// slots 00733340 uses keep their x86 offsets (0x14/0x2c/0x58) and their slot
// index on x64, where the offset grows with the pointer width.
struct SchedulingTable {
    void* unused00[5];
    ReloadDispatchSlot dispatch;  // slot5  +0x14
    void* unused18[3];
    ScheduleSlot schedule;        // slot9  +0x24
    void* unused28;
    ReloadLookupSlot lookup;      // slot11 +0x2c
    void* unused30[10];
    ReloadFallbackSlot fallback;  // slot22 +0x58
};
struct SchedulingService { const SchedulingTable* vtable; };
TS3_X86_OFFSET(SchedulingTable, dispatch, 0x14);
TS3_X86_OFFSET(SchedulingTable, schedule, 0x24);
TS3_X86_OFFSET(SchedulingTable, lookup, 0x2c);
TS3_X86_OFFSET(SchedulingTable, fallback, 0x58);
TS3_X86_OFFSET(SchedulingService, vtable, 0);
struct SchedulingBoundaries {
    SchedulingService* (*manager)(); // genuine cdecl004AFD20, reread on each call
    ChangeNotification notification; // genuine external00733970 identity mapping
};
#if defined(_M_IX86)
const SchedulingBoundaries& OriginalSchedulingBoundaries();
#endif
// TS3W.exe 0x00733CC0: get current service, slot9(1,733970,self,null,null), or AL0.
// ECX self, plain ret; caller0059EE21 consumes AL, upper EAX is not exposed.
uint8_t ScheduleChanges(void* self, const SchedulingBoundaries& boundaries);
// TS3W.exe 0x00733CF0: get current service, slot9(0,733970,self,null,null), or null getter AL0.
// ECX self, plain ret; lifecycle0059ED06 ignores result; retain external slot's AL.
uint8_t UnscheduleChanges(void* self, const SchedulingBoundaries& boundaries);
}

namespace ts3::resource::m03::delivery_catalog {
// 00733340's observed lookup result (slot11 output): only slot6 (+18) is used,
// returning the full 32-bit identity compared against 01CCE04D.
struct ReloadCandidateTable { void* unused00[6]; ReloadIdentitySlot identity; };
struct ReloadCandidate { const ReloadCandidateTable* vtable; };
TS3_X86_OFFSET(ReloadCandidateTable, identity, 0x18);
TS3_X86_OFFSET(ReloadCandidate, vtable, 0);
// Genuine cdecl 00727280 getter result: only slot8 (+20) is used; its result is
// ignored, so no return contract is claimed for it.
struct ReloadManagerTable { void* unused00[8]; ReloadRouteSlot route; };
struct ReloadManager { const ReloadManagerTable* vtable; };
TS3_X86_OFFSET(ReloadManagerTable, route, 0x20);
TS3_X86_OFFSET(ReloadManager, vtable, 0);
// 00733340 fallback result: slot6 (+18) is called with the key and the address
// of a zero-initialized local; only its AL is tested, so the exposed result is
// a raw byte rather than a normalized bool.
struct ReloadFallbackTable { void* unused00[6]; ReloadOutputSlot output; };
struct ReloadFallback { const ReloadFallbackTable* vtable; };
TS3_X86_OFFSET(ReloadFallbackTable, output, 0x18);
TS3_X86_OFFSET(ReloadFallback, vtable, 0);
// Temporary published through the fallback output; only slot1 (+4) is invoked,
// as the subject's terminal release. Its result is not exposed.
struct ReloadTemporaryTable { void* unused00; ReloadReleaseSlot release; };
struct ReloadTemporary { const ReloadTemporaryTable* vtable; };
TS3_X86_OFFSET(ReloadTemporaryTable, release, 4);
TS3_X86_OFFSET(ReloadTemporary, vtable, 0);
// Only the two genuine getters are external boundaries. Every other dependency
// is dispatched through the CURRENT fixture-owned table of the receiver at the
// same point the original re-reads it. There is no portable default on x64:
// callers must supply the real getters (genuine x86 identities below).
struct ReloadRoutingBoundaries {
    SchedulingService* (*service)(); // genuine cdecl 004AFD20, reread on each call
    ReloadManager* (*manager)();     // genuine cdecl 00727280, reread on each call
};
#if defined(_M_IX86)
const ReloadRoutingBoundaries& OriginalReloadRoutingBoundaries();
#endif
// TS3W.exe 0x00733340: route one reload notification through the current service.
// Reads only the key's type word (+8) and forwards it with sentinel -1 to the
// current service slot11. A result whose slot6 identity equals 01CCE04D routes
// through the 00727280 manager slot8 with the original key; a null manager or
// lookup falls through to slot22, whose object slot6(key,&local) fills a zero-
// initialized local. If that call's AL is nonzero, the CURRENT table slot5 is
// invoked as (key, local, null, null, null); regardless of AL a nonnull local is
// released once through its current slot1 as a terminal dispatch. No service
// null guard exists in the original. One caller-cleaned cdecl argument, plain
// ret; every path's EAX is the last callee's incidental value, so no result is
// exposed (the genuine 00733970 sibling callback is cdecl void).
void ReloadRoute(const ts3::resource::ResourceKey* key,
    const ReloadRoutingBoundaries& boundaries);
}

namespace ts3::resource::m03::delivery_catalog {
// ---------------------------------------------------------------------------
// Wave9: the genuine handler lookup/notification callback 00733970.
// ---------------------------------------------------------------------------
// The handler record the context+0x130 map chains: the type-key dword C45F60
// hashes, the full 32-bit routing mode and the full-width callback pointer the
// body snapshots under the lock, then the chain link C45F60 walks at +0xC on
// x86 (NOT the CacheHashNode +0x18 layout, so the types stay separate).
struct NotificationNode {
    uint32_t key;                // +0x00 type word hashed by the C45F60 lookup
    uint32_t mode;               // +0x04 full 32-bit mode, never a byte
    ChangeNotification callback; // +0x08 full callback pointer identity
    NotificationNode* next;      // +0x0C x86 chain link walked by C45F60
};
TS3_X86_OFFSET(NotificationNode, mode, 4);
TS3_X86_OFFSET(NotificationNode, callback, 8);
TS3_X86_OFFSET(NotificationNode, next, 0xc);

// C45F60's iterator pair: the found (or sentinel) record and the bucket slot to
// keep walking from. Distinct from CacheMapIter because the chained node type
// is the handler record above, not CacheHashNode.
struct NotificationIterator {
    NotificationNode* node;
    NotificationNode** bucket;
};
TS3_X86_SIZE(NotificationIterator, 8);

// The dword-keyed chain map at context+0x130. Only buckets (+4) and bucketCount
// (+8) are read by the wave9 subject or C45F60; the +0xC word is the container
// state word 00733880 clears unconditionally (never the cache's elementCount: no
// wave9 path reads it, and the teardown writes it as a uint32).
struct NotificationMap {
    void* storage;                // +0x00 opaque allocator word (not read)
    NotificationNode** buckets;   // +0x04 bucket array; buckets[count] is sentinel
    uint32_t bucketCount;         // +0x08 full 32-bit bucket count/index
    uint32_t state;               // +0x0C uint32 container state, zeroed on teardown
};
TS3_X86_OFFSET(NotificationMap, buckets, 4);
TS3_X86_OFFSET(NotificationMap, bucketCount, 8);
TS3_X86_OFFSET(NotificationMap, state, 0xc);

// ---------------------------------------------------------------------------
// Wave11 layout types: the reload queue 00733610 constructs and the prefix the
// monitor construction 007337F0 carries. NotificationContext's two queue
// members and manifest tail are exactly this storage.
// ---------------------------------------------------------------------------
// Genuine 004F35B0: thiscall ret8 (timer, uint32 mode, uint8 start); it zeroes
// the 0x20-byte timer, writes the mode word and period 1.0f and takes a first
// timestamp only when start is nonzero. Genuine 004F34F0: thiscall ret8
// (timer, uint32 advance, uint8 start); it takes a performance counter and
// stores the 64-bit deadline at +0x18. Both leave an incidental EAX the raw
// 00733610 caller ignores; only the receivers and the exact pushed argument
// order are used here.
using QueueLockInitialize = void (*)(void* critical);
using QueueTimerInitialize = void (*)(void* timer, uint32_t mode, uint8_t start);
using QueueTimerSync = void (*)(void* timer, uint32_t advance, uint8_t start);

// The 0x20-byte helper-owned timer at queue+0x20. The queue body only hands out
// its address to the captured services and never reads or writes a field.
struct ReloadQueueTimer {
    uint64_t count;    // +0x00
    uint64_t ticks;    // +0x08
    uint32_t mode;     // +0x10
    float period;      // +0x14
    uint64_t deadline; // +0x18
};
TS3_X86_OFFSET(ReloadQueueTimer, ticks, 0x08);
TS3_X86_OFFSET(ReloadQueueTimer, mode, 0x10);
TS3_X86_OFFSET(ReloadQueueTimer, period, 0x14);
TS3_X86_OFFSET(ReloadQueueTimer, deadline, 0x18);
TS3_X86_SIZE(ReloadQueueTimer, 0x20);

// The 0x1C-byte manifest 00733610 stores twice (+0x44 requested, +0x64 pending)
// and 007337F0 stores once over its map: an x86 image table identity, the full
// 32-bit count/state words, the two shared float cells and the manifest name.
// The constructor writes 7 dwords of the 0x20-byte stride, so the stride gap
// stays untouched.
struct ReloadManifest {
    const void* table;   // genuine image table identity 0x0114C900
    uint32_t count;      // 1
    uint32_t state;      // 0
    float first;         // from the 0x0107A538 cell (3F800000)
    float second;        // from the 0x00F98A58 cell (40000000)
    uint32_t zero;       // 0
    const char* name;    // per-manifest string identity
};
TS3_X86_OFFSET(ReloadManifest, count, 4);
TS3_X86_OFFSET(ReloadManifest, state, 8);
TS3_X86_OFFSET(ReloadManifest, first, 0xc);
TS3_X86_OFFSET(ReloadManifest, second, 0x10);
TS3_X86_OFFSET(ReloadManifest, zero, 0x14);
TS3_X86_OFFSET(ReloadManifest, name, 0x18);
TS3_X86_SIZE(ReloadManifest, 0x1c);

// One reload queue, exact x86 layout: the critical section 00733610 initializes
// at +0x00 (24 bytes on x86), the helper timer at +0x20, the requested manifest
// at +0x44 and the pending manifest at +0x64. x86 size 0x80 matches the queue
// stride 007337F0 places (context+0x30 and context+0xB0). The x64 layout is its
// own: the host-sized lock grows, so the later members move with the width.
#if defined(_M_IX86)
struct ReloadQueue {
    alignas(void*) unsigned char critical[24];
    uint8_t padAfterLock[8];        // timer at +0x20
    ReloadQueueTimer timer;         // +0x20
    uint8_t gap40[4];               // +0x40 stride gap
    ReloadManifest requested;       // +0x44
    uint8_t gap60[4];               // +0x60 stride gap
    ReloadManifest pending;         // +0x64
};
#else
struct ReloadQueue {
    alignas(void*) unsigned char critical[40];
    uint8_t padAfterLock[8];        // timer at +0x30 (host-sized lock)
    ReloadQueueTimer timer;         // +0x30
    uint8_t gap40[8];               // +0x50 gap
    ReloadManifest requested;       // +0x58
    ReloadManifest pending;         // +0x80
};
#endif
TS3_X86_OFFSET(ReloadQueue, timer, 0x20);
TS3_X86_OFFSET(ReloadQueue, gap40, 0x40);
TS3_X86_OFFSET(ReloadQueue, requested, 0x44);
#if defined(_M_IX86)
TS3_X86_OFFSET(ReloadQueue, gap60, 0x60);
#endif
TS3_X86_OFFSET(ReloadQueue, pending, 0x64);
TS3_X86_SIZE(ReloadQueue, 0x80);

// The prefix the genuine base construction 0059ECC0 writes at the monitor: the
// base table identity at +0 (immediately replaced by the monitor table), the
// full 32-bit type hash at +4, a zero state word at +8 and, on x86 only, the
// full 32-bit type-name pointer at +0xC. The x64 name argument stays with the
// base capture instead: a full-width pointer cannot occupy the +0xC slot
// without overwriting the lock at +0x10.
struct MonitorBasePrefix {
    const void* table;   // x86 +0x00 base table identity (0059ECC0)
    uint32_t hash;       // x86 +0x04 (B23536D5)
    uint32_t state;      // x86 +0x08 (zeroed)
#if defined(_M_IX86)
    const void* name;    // x86 +0x0C genuine 00FFDF8C type-name pointer
#endif
};
TS3_X86_OFFSET(MonitorBasePrefix, hash, 4);
TS3_X86_OFFSET(MonitorBasePrefix, state, 8);
#if defined(_M_IX86)
TS3_X86_OFFSET(MonitorBasePrefix, name, 0xc);
#endif
static_assert(sizeof(MonitorBasePrefix) == 0x10);

// The two queue receivers genuine 733770 is invoked on (context+0x30 and
// context+0xB0) and 007337F0 constructs through 00733610: the real typed queue
// object with its exact x86 used extent. Wave9/10 callers only hand out the
// address.
using NotificationQueueReceiver = ReloadQueue;

// The notification context (the callback's third argument). x86 offsets are the
// raw body's; the native layout is its own (the raw Windows critical section is
// host-sized, so later members move with the pointer width).
struct NotificationContext {
    uint8_t unknown00[0x10];
    alignas(void*) unsigned char critical[sizeof(void*) == 4 ? 24 : 40];
    uint8_t padAfterLock[sizeof(void*) == 4 ? 8 : 1]; // x86: puts queue0 at +0x30
    NotificationQueueReceiver queue0; // +0x30 x86 receiver queued for mode 0
    NotificationQueueReceiver queue1; // +0xB0 x86 receiver queued for mode 1
    NotificationMap map;              // +0x130 x86 handler map
    // Wave11: the constructor's third manifest starts at map+sizeof(void*)
    // (over the map's buckets/count/state words) and extends past the map view.
    // With the map this makes the x86 receiver exactly the 0x150 bytes the
    // genuine caller 00733A10 allocates.
    uint8_t manifestStorage[sizeof(void*) + sizeof(ReloadManifest) - sizeof(NotificationMap)];
};
TS3_X86_OFFSET(NotificationContext, critical, 0x10);
TS3_X86_OFFSET(NotificationContext, queue0, 0x30);
TS3_X86_OFFSET(NotificationContext, queue1, 0xb0);
TS3_X86_OFFSET(NotificationContext, map, 0x130);
TS3_X86_OFFSET(NotificationContext, manifestStorage, 0x140);
TS3_X86_SIZE(NotificationContext, 0x150);

// Wave11 views over that storage: the base prefix at the receiver start and the
// third manifest at map+sizeof(void*) (x86 +0x134: its table extends the map's
// buckets word, its count the bucketCount word and its state the state word).
inline MonitorBasePrefix* MonitorBase(NotificationContext* self) {
    return reinterpret_cast<MonitorBasePrefix*>(self);
}
inline ReloadManifest* MonitorManifest(NotificationContext* self) {
    return reinterpret_cast<ReloadManifest*>(reinterpret_cast<uint8_t*>(&self->map) + sizeof(void*));
}

// Only the three genuine external operations plus the notify-ABI default
// callback identity. enter/leave take a caller-owned capture context first; the
// default x86 implementations call the real 0xF9512C/0xF95130 imports exactly as
// the raw body's IAT call does.
using NotificationLock = void (*)(void* context, void* critical);
using NotificationLookup = NotificationIterator* (*)(NotificationMap* map,
    NotificationIterator* out, const uint32_t* keyType);
using NotificationQueue = void (*)(void* receiver,
    const ts3::resource::ResourceKey* key, ChangeNotification callback);
struct NotificationBoundaries {
    void* context;               // passthrough for the captured enter/leave
    NotificationLock enter;
    NotificationLock leave;
    NotificationLookup lookup;
    NotificationQueue queue;
    ChangeNotification fallback; // genuine identity queued on a map miss
};
#if defined(_M_IX86)
const NotificationBoundaries& OriginalNotificationBoundaries();
#endif
// TS3W.exe 0x00733970: cdecl(ignored database*, canonical key*, context*), plain
// ret, no result contract. Locks context+0x10, looks the key's type word (+8,
// ResourceKey::type) up in the context+0x130 map through the genuine C45F60,
// dereferences the RETURNED iterator pointer to the record (it need not be the
// supplied output local), compares the record against the CURRENT
// buckets[bucketCount] sentinel, snapshots the record's full-dword mode and full
// callback pointer under the lock, unlocks the SAME critical section, then asks
// the 733770 queue of the mode's receiver (context+0x30 for mode 0, context+0xB0
// for mode 1) to queue the original key with the snapshotted callback. Every
// other uint32 mode does nothing. A map miss supplies the boundary fallback
// (genuine 733340 production through the x86 adapter). The subject never invokes
// the callback itself, keeps the raw body's missing map null guard and reads no
// receiver after the unlock-side external calls.
void NotificationCallback(void* database, const ts3::resource::ResourceKey* key,
    NotificationContext* context, const NotificationBoundaries& boundaries);
#if defined(_M_IX86)
// Typed adapter from the notification ABI to the real 733340 production: ignores
// database/context exactly as the original default handler does and forwards only
// the key to ReloadRoute with the genuine x86 getters.
void ReloadNotification(void* database, const ts3::resource::ResourceKey* key, void* context);
#endif

// ---------------------------------------------------------------------------
// Wave10: the genuine monitor teardown 00733880 and its publication clearing.
// ---------------------------------------------------------------------------
// The map destroy is the genuine external 00AEBDB0: thiscall ret8, walks the
// PASSED array over the PASSED count, unlinks every nonnull record through its
// +0xC chain link, frees it through the genuine 004E3F70 and zeroes the bucket
// slot. It stays an external leaf, captured, never reimplemented here.
using NotificationMapDestroy = void (*)(NotificationMap* map,
    NotificationNode** array, uint32_t count);
// Genuine external 004E3F70: cdecl one pointer, called with the CURRENT array.
using NotificationStorageFree = void (*)(void* storage);
// Genuine external 007333E0 queue teardown: thiscall one receiver, ret0; its
// incidental EAX is ignored by the raw body and is not exposed.
using NotificationQueueTeardown = void (*)(void* receiver);
// DeleteCriticalSection through the live 0xF95128 import cell.
using NotificationCriticalDelete = void (*)(void* critical);
// Genuine external 00731CF0 base teardown: thiscall, writes the base vtable at
// [receiver+0]. It may end the receiver's logical lifetime; the subject reads
// nothing after it.
using NotificationBaseTeardown = void (*)(void* self);

// Only the five genuine external operations, nothing else.
struct NotificationTeardownBoundaries {
    NotificationMapDestroy destroyMap;        // 00AEBDB0 (thiscall ret8)
    NotificationStorageFree freeStorage;      // 004E3F70 (cdecl)
    NotificationQueueTeardown teardownQueue;  // 007333E0 (thiscall, EAX unused)
    NotificationCriticalDelete deleteCritical;// 0F95128 import (WINAPI)
    NotificationBaseTeardown baseTeardown;    // 00731CF0 (thiscall, EAX incidental)
};
#if defined(_M_IX86)
// Genuine x86 bindings, read at each call: the free/queue/base entries are the
// real addresses and deleteCritical reads the live import cell. There is no
// portable default; x64 callers must supply the real full-width operations.
const NotificationTeardownBoundaries& OriginalNotificationTeardownBoundaries();
#endif

// The real publication cell. 007337F0 stores monitor+0x10 in the genuine x86
// global 0x011D3E2C and 00733A70 recovers the monitor as value-0x10; x64 owns a
// real full-width static cell. The SUBJECT clears *cell itself, so this accessor
// is never a captured boundary and never an emulated write.
void** NotificationPublicationCell();

// TS3W.exe 0x00733880: notification monitor teardown. ECX monitor, no stack
// arguments, plain ret through the terminal base teardown; the only caller
// 007338F0 (a data-offset vtable slot at 00FFDF60) and that caller's consumer
// read no result, so nothing is returned and no EAX value is invented.
// Snapshots map+0x134 array and +0x138 count BEFORE the external map destroy,
// then clears the uint32 state word at +0x13C unconditionally and reloads the
// CURRENT count: an unsigned current count > 1 frees the CURRENT array through
// the genuine 004E3F70. Then tears down receiver+0xB0 as the first queue and
// receiver+0x30 as the second, deletes the receiver+0x10 critical section,
// clears the publication cell unconditionally and tail-calls the base teardown
// with the original receiver. Preserve this order: every earlier callback may
// change the current count/array/state/publication, and the receiver must not be
// read after the terminal base teardown.
void TeardownNotificationMonitor(NotificationContext* self,
    const NotificationTeardownBoundaries& boundaries);
#if defined(_M_IX86)
// The same subject over the genuine x86 external bindings.
inline void TeardownNotificationMonitor(NotificationContext* self) {
    TeardownNotificationMonitor(self, OriginalNotificationTeardownBoundaries());
}
#endif
}

namespace ts3::resource::m03::delivery_catalog {
// ---------------------------------------------------------------------------
// Wave11: reload queue construction 00733610 and change monitor construction
// 007337F0. Every initialization store, its order, the publication and the real
// nesting of the queue production belong to these two bodies. Only the genuine
// external services (the live 0x00F95124 InitializeCriticalSection import,
// 004F35B0, 004F34F0), the genuine base construction 0059ECC0 and the genuine
// constant/table/storage identities are captured or supplied.
// ---------------------------------------------------------------------------
// The identities the two bodies store: the 0x0114C900 image table, the two
// shared float cells 0x0107A538 (3F800000) and 0x00F98A58 (40000000), and the
// four genuine name strings. x86 callers may use the genuine defaults; x64
// callers must supply real full-width native tables/storage (no portable
// default exists on x64).
struct ReloadQueueBoundaries {
    QueueLockInitialize initLock;    // genuine live 0x00F95124 import
    QueueTimerInitialize initTimer;  // genuine 004F35B0
    QueueTimerSync syncTimer;        // genuine 004F34F0
    const void* manifestTable;       // genuine 0x0114C900 table identity
    const float* firstValue;         // genuine 0x0107A538 cell (3F800000)
    const float* secondValue;        // genuine 0x00F98A58 cell (40000000)
    const char* requestedName;       // genuine 0x00FFDEF0
    const char* pendingName;         // genuine 0x00FFDEC0
};
#if defined(_M_IX86)
// Genuine x86 bindings, read at each call: the lock entry reads the live import
// cell and the timer entries are the real 004F35B0/004F34F0 addresses.
const ReloadQueueBoundaries& OriginalReloadQueueBoundaries();
#endif
// TS3W.exe 0x00733610: construct one reload queue in place (thiscall, no stack
// arguments, plain ret; EAX is the receiver on every path). Calls
// InitializeCriticalSection on receiver+0x00 through the live import, then the
// genuine timer 004F35B0 on receiver+0x20 with the pushed argument pair (4,0)
// and 004F34F0 on the SAME timer with (0,1). Only AFTER those calls it loads
// the two float cells once and stores the two manifests:
// requested  first/second/name/state/zero/count/table;
// pending    first/second/name/table/state/zero/count.
// Unowned bytes (the lock contents, the timer fields, the stride gaps) stay
// untouched.
ReloadQueue* InitializeReloadQueue(ReloadQueue* self,
    const ReloadQueueBoundaries& boundaries);
#if defined(_M_IX86)
inline ReloadQueue* InitializeReloadQueue(ReloadQueue* self) {
    return InitializeReloadQueue(self, OriginalReloadQueueBoundaries());
}
#endif

// Genuine 0059ECC0 base construction: thiscall ret8 (self, uint32 hash,
// const void* typeName); writes the base table identity at [self], the hash at
// +4, a zero state word at +8 and, on x86 only, the type-name pointer at +0xC.
using MonitorBaseConstruct = void (*)(void* self, uint32_t hash, const void* typeName);
struct MonitorConstructionBoundaries {
    MonitorBaseConstruct baseConstruct; // genuine 0059ECC0
    const void* monitorTable;           // genuine 0x00FFDF58 table identity
    uint32_t typeHash;                  // B23536D5
    const void* typeName;               // genuine 0x00FFDF8C name identity
    const char* pendingName;            // genuine 0x00FFDF24 name identity
    ReloadQueueBoundaries queue;        // forwarded to both nested queue bodies
};
#if defined(_M_IX86)
const MonitorConstructionBoundaries& OriginalMonitorConstructionBoundaries();
#endif
// TS3W.exe 0x007337F0: construct the change monitor in place (thiscall, no
// stack arguments, plain ret; EAX is the receiver). Runs the captured base
// construction first, then publishes receiver+0x10 into the real publication
// cell, installs the monitor table at [receiver] and initializes the
// receiver+0x10 lock; THEN constructs receiver+0x30 and receiver+0xB0 through
// the SAME InitializeReloadQueue production (not a capture). Only after both
// nested constructions it loads 0x0107A538 then 0x00F98A58 into the manifest
// and stores name, state, zero, count and the 0x0114C900 table in the raw
// order. The publication cell keeps whatever earlier external code left until
// the subject publishes; the nested queue calls observe that published value.
NotificationContext* InitializeChangeMonitor(NotificationContext* self,
    const MonitorConstructionBoundaries& boundaries);
#if defined(_M_IX86)
inline NotificationContext* InitializeChangeMonitor(NotificationContext* self) {
    return InitializeChangeMonitor(self, OriginalMonitorConstructionBoundaries());
}
#endif
}

namespace ts3::resource::m03::delivery_catalog {
// ---------------------------------------------------------------------------
// Wave12: notification queue dispatch 00733AA0 over the wave11 ReloadQueue.
// ---------------------------------------------------------------------------
// The dispatch stores its waiting notifications in the two chained hash maps
// the wave11 constructor already initialized: the requested map at queue+0x40
// and the pending map at queue+0x60 on x86. The genuine helpers (007336A0,
// 00B33670, 00626610, 007332B0) see the map base (the allocator word before
// the buckets pointer), so this view keeps that opaque word first. It is never
// read or written by this unit.
struct NotificationQueueNode;
// Genuine cdecl callback: the moved notification is invoked with the node.
using NotificationDispatch = void (*)(NotificationQueueNode*);
// One chained-map node: the four key dwords the genuine 007336A0 hashes with
// key[0]^key[3], the notification callback at +0x10, the retry budget at +0x14
// and the bucket chain at +0x18 on x86. The x64 layout is its own: the
// full-width callback moves the retry/chain members with it.
struct NotificationQueueNode {
    uint32_t key[4];                 // +0x00..+0x0F
    NotificationDispatch dispatch;   // +0x10 moved notification callback
    uint32_t retry;                  // +0x14 uint32 retry budget (50 on move)
    NotificationQueueNode* next;     // +0x18 bucket-chain link
};
TS3_X86_OFFSET(NotificationQueueNode, dispatch, 0x10);
TS3_X86_OFFSET(NotificationQueueNode, retry, 0x14);
TS3_X86_OFFSET(NotificationQueueNode, next, 0x18);

// The map base the genuine helpers receive. buckets[bucketCount] is the
// sentinel slot (the value the dispatch snapshots once after 00626610);
// elementCount is the uint32 count 00733AA0 rechecks after every erase.
struct NotificationQueueMap {
    void* allocStorage;              // +0x00 opaque, never touched here
    NotificationQueueNode** buckets; // +0x04 buckets[bucketCount] sentinel slot
    uint32_t bucketCount;            // +0x08
    uint32_t elementCount;           // +0x0C
};
TS3_X86_OFFSET(NotificationQueueMap, buckets, 4);
TS3_X86_OFFSET(NotificationQueueMap, bucketCount, 8);
TS3_X86_OFFSET(NotificationQueueMap, elementCount, 0xc);

// 007336A0's output tuple: the found-or-inserted node, its bucket slot and the
// byte at +0x08 that is nonzero only for a freshly inserted node.
struct NotificationQueueInsert {
    NotificationQueueNode* node;     // +0x00
    NotificationQueueNode** bucket;  // +0x04
    uint8_t inserted;                // +0x08 uint8; only this byte is tested
};
TS3_X86_OFFSET(NotificationQueueInsert, inserted, 8);

// 00626610/00B33670/007332B0's iterator pair.
struct NotificationQueueIter {
    NotificationQueueNode* node;
    NotificationQueueNode** bucket;
};
TS3_X86_SIZE(NotificationQueueIter, 8);

// The genuine maps reuse the SAME storage wave11 filled: the source map's
// buckets word is requested.table and the destination's is pending.table.
// The map base is the (never touched) word before that pointer.
inline NotificationQueueMap* SourceQueueMap(ReloadQueue* queue) {
    return reinterpret_cast<NotificationQueueMap*>(
        reinterpret_cast<uint8_t*>(&queue->requested) - offsetof(NotificationQueueMap, buckets));
}
inline NotificationQueueMap* DestinationQueueMap(ReloadQueue* queue) {
    return reinterpret_cast<NotificationQueueMap*>(
        reinterpret_cast<uint8_t*>(&queue->pending) - offsetof(NotificationQueueMap, buckets));
}

// The 00733AA0 dispatch results through only these genuine virtual boundaries:
// service slot+0x44(entry) returns a result object or null; result slot+0x34
// (entry,&local,1,6,1,0) returns AL; local slot+0x20 and slot+0x08 release it;
// service slot+0x08 is the service release slot, which this build never
// reaches (see the quirk in the DispatchNotificationQueue contract below).
// All slot offsets are x86 and move with the pointer width on x64.
struct NotificationService;
struct NotificationResult;
struct NotificationLocal;
#if defined(_M_IX86)
using NotificationLookupSlot = NotificationResult*(__thiscall*)(NotificationService*,
    NotificationQueueNode*);
using NotificationResultCallSlot = uint8_t(__thiscall*)(NotificationResult*,
    NotificationQueueNode*, NotificationLocal**, uint32_t, uint32_t, uint32_t, uint32_t);
using NotificationServiceReleaseSlot = void(__thiscall*)(NotificationService*);
using NotificationLocalReleaseSlot = void(__thiscall*)(NotificationLocal*);
using NotificationLocalFinishSlot = void(__thiscall*)(NotificationLocal*);
#else
using NotificationLookupSlot = NotificationResult*(*)(NotificationService*,
    NotificationQueueNode*);
using NotificationResultCallSlot = uint8_t(*)(NotificationResult*,
    NotificationQueueNode*, NotificationLocal**, uint32_t, uint32_t, uint32_t, uint32_t);
using NotificationServiceReleaseSlot = void (*)(NotificationService*);
using NotificationLocalReleaseSlot = void (*)(NotificationLocal*);
using NotificationLocalFinishSlot = void (*)(NotificationLocal*);
#endif
struct NotificationServiceTable {
    void* unused00[2];
    NotificationServiceReleaseSlot release; // +0x08 slot2
    void* unused0C[14];
    NotificationLookupSlot lookup;          // +0x44 slot17
};
TS3_X86_OFFSET(NotificationServiceTable, release, 0x08);
TS3_X86_OFFSET(NotificationServiceTable, lookup, 0x44);
struct NotificationService { const NotificationServiceTable* vtable; };

struct NotificationResultTable {
    void* unused00[13];
    NotificationResultCallSlot call;        // +0x34 slot13
};
TS3_X86_OFFSET(NotificationResultTable, call, 0x34);
struct NotificationResult { const NotificationResultTable* vtable; };

struct NotificationLocalTable {
    void* unused00[2];
    NotificationLocalReleaseSlot release;   // +0x08 slot2
    void* unused0C[5];
    NotificationLocalFinishSlot finish;     // +0x20 slot8
};
TS3_X86_OFFSET(NotificationLocalTable, release, 0x08);
TS3_X86_OFFSET(NotificationLocalTable, finish, 0x20);
struct NotificationLocal { const NotificationLocalTable* vtable; };

// External boundaries only: the live try-lock/time/unlock imports and the five
// genuine helpers. They are captures, not a drain implementation.
using QueueTryLock = bool (*)(void* critical);
using QueueQueryCounter = void (*)(int64_t* counter);
using QueueUnlock = void (*)(void* critical);
using QueueInsertOrFind = NotificationQueueInsert* (*)(NotificationQueueMap* map,
    NotificationQueueInsert* out, NotificationQueueNode* entry, uint32_t flag);
using QueueErase = NotificationQueueIter* (*)(NotificationQueueMap* map,
    NotificationQueueIter* out, NotificationQueueNode* entry, NotificationQueueNode** bucket);
using QueueBegin = NotificationQueueIter* (*)(NotificationQueueMap* map,
    NotificationQueueIter* out);
using QueueAdvance = void (*)(NotificationQueueIter* iter);
using QueueService = NotificationService* (*)();
struct NotificationQueueBoundaries {
    QueueTryLock tryLock;             // live 0F95218 TryEnterCriticalSection
    QueueQueryCounter queryCounter;   // live 0F953B8 QueryPerformanceCounter
    QueueUnlock unlock;               // live 0F95130 LeaveCriticalSection
    QueueInsertOrFind insertOrFind;   // genuine 007336A0 (thiscall ret0C, flag unread)
    QueueErase erase;                 // genuine 00B33670 (thiscall ret0C)
    QueueBegin begin;                 // genuine 00626610 (thiscall ret4)
    QueueAdvance advance;             // genuine 007332B0 (thiscall plain ret)
    QueueService service;             // genuine 004AFD20 getter (cdecl)
};
#if defined(_M_IX86)
const NotificationQueueBoundaries& OriginalNotificationQueueBoundaries();
#endif
// TS3W.exe 0x00733AA0: drain the requested map into the pending map, then
// dispatch the pending notifications. ECX queue (monitor+0x30 / monitor+0xB0
// through the 00733D20/00733D30 wrappers), no stack arguments, plain ret; EAX
// is the last callee's incidental value and no result contract exists.
// On a failed try-lock the body returns without unlock/time/helper calls. On
// success it takes one QueryPerformanceCounter sample and computes
// deadline(queue+0x38/+0x3C) - time as an unsigned 64-bit subtraction; it
// unlocks and returns unless the difference's high word sign bit is set
// (zero/nonnegative differences return), and also returns on an empty source
// map. The source drain reloads the CURRENT buckets pointer every iteration,
// scans the first nonnull bucket, moves the entry through the genuine
// 007336A0 with a dword whose low byte is zero and whose upper bytes the
// genuine callee never reads, copies entry+0x10 into a freshly inserted
// destination entry and sets its uint32 retry to 50, erases the source entry
// through 00B33670 and repeats while the CURRENT source count is nonzero.
// THEN it unlocks BEFORE the 004AFD20 service getter and the destination walk:
// a 00626610 begin, a once-only sentinel snapshot through the CURRENT
// buckets[bucketCount], and for each non-end entry a genuine service slot+0x44
// lookup. A null lookup erases and reloads. A nonnull result calls slot+0x34
// with (entry,&local,1,6,1,0): on AL true the unguarded local slot+0x20 runs,
// the local is cleared and released through slot+0x08 when nonnull, the node's
// callback is invoked, and the entry is erased/reloaded. The service release
// slot+0x08 is then tested against the just-cleared local slot, so it is DEAD
// code: the callback's cdecl argument is still on the stack at the release
// read, making esp+14h alias esp+10h, and the local is already null. The
// saved service pointer is used only by the slot+0x44 lookups. On AL false the
// old uint32 retry is read, the decremented value is written back BEFORE
// testing the old one, a nonzero old retry erases/reloads while a zero old
// retry advances the iterator through the genuine 007332B0, and a nonnull
// local is then released.
void DispatchNotificationQueue(ReloadQueue* self,
    const NotificationQueueBoundaries& boundaries);
#if defined(_M_IX86)
// The same subject over the genuine x86 external bindings (live imports and
// the real helper addresses). The real getters are never reimplemented here.
inline void DispatchNotificationQueue(ReloadQueue* self) {
    DispatchNotificationQueue(self, OriginalNotificationQueueBoundaries());
}
#endif
}
