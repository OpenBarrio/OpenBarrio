#include "ts3/resource/m03_delivery_catalog.h"

namespace ts3::resource::m03::delivery_catalog {
uint8_t Activate(Catalog* self) {
    if (Enabled(self) == 0)
        Enabled(self) = 1;
    return 1;
}

uint8_t Deactivate(Catalog* self) {
    if (Enabled(self) != 0) {
        // Reload this receiver's current table on every call. As in the original,
        // resolve the slot before clearing, and clear before invoking it.
        const auto callback = static_cast<const CatalogTable*>(self->vtable)->slots[10];
        Enabled(self) = 0;
        callback(self, 0);
    }
    return 1;
}

uint32_t TypeIdentity(const CatalogPointerView*) {
    return 0xB5962C35u;
}

void SetAttachment(CatalogPointerView* self, CatalogAttachment* replacement) {
    auto* old = self->attachment;
    if (old != replacement) {
        if (old)
            old->vtable->release(old, 1);
        // Deliberately after release: callback/reentrant changes are visible
        // during release, but this outer transition publishes last.
        self->attachment = replacement;
    }
}
}

#include <cstring>
namespace ts3::resource::m03::delivery_catalog {
namespace {
CatalogIterator* GameFind(CatalogMap* map, CatalogIterator* out, const uint32_t* key) {
    return CallGameMethod<CatalogIterator*, 0x00AD3F40>(map, out, key);
}
int GameLock(ts3::thread::Mutex* mutex, const ts3::thread::ThreadTime& timeout) {
    return CallGameMethod<int, 0x004E16F0>(mutex, &timeout);
}
int GameUnlock(ts3::thread::Mutex* mutex) {
    return CallGameMethod<int, 0x004E17B0>(mutex);
}
}
const InsertionBoundaries& OriginalInsertionBoundaries() {
    static const InsertionBoundaries boundaries{GameFind, GameLock, GameUnlock};
    return boundaries;
}
void InsertUnlinked(Catalog* self, CatalogEntry* node, const InsertionBoundaries& boundaries) {
    if (node->listNext || node->listPrev) return;
    CatalogIterator found;
    boundaries.find(&self->map, &found, node->key);
    if (found.node == self->map.buckets[self->map.bucketCount]) return;
    void* const link = &node->listNext;
    node->listPrev = self->lruAnchorPrev;
    node->listNext = &self->lruAnchorNext;
    self->lruAnchorPrev = link;
    // The old tail is a link-subobject handle, possibly the owner's sentinel.
    // Copy the native pointer into its first field without inventing a full node.
    std::memcpy(node->listPrev, &link, sizeof(link));
    self->cachedBytes += node->size;
    self->pendingBytes -= node->size;
}
void InsertUnlinkedLocked(Catalog* self, CatalogEntry* node, const InsertionBoundaries& boundaries) {
    auto* const mutex = reinterpret_cast<ts3::thread::Mutex*>(self->mutex);
#if defined(_M_IX86)
    const auto& timeout = *reinterpret_cast<const ts3::thread::ThreadTime*>(0x00FFDD34);
#else
    const auto& timeout = ts3::thread::kTimeoutNone;
#endif
    boundaries.lock(mutex, timeout);
    InsertUnlinked(self, node, boundaries);
    boundaries.unlock(mutex);
}
}

namespace ts3::resource::m03::delivery_catalog {
namespace {
void* GameRangeAllocate(size_t bytes, const char* tag, uint32_t flags, uint32_t extra,
    const char* file, uint32_t line) {
    return CallGameFunction<void*, 0x004E3F90>(bytes, tag, flags, extra, file, line);
}
void GameRangeFree(void* storage) { CallGameFunction<void, 0x004E3F70>(storage); }
size_t PointerCount(void* const* first, void* const* last) {
    // A null/null vector is valid; do not subtract two null C++ pointers.
    return first == last ? 0 : size_t(last - first);
}
void CopyPointers(void** to, void* const* from, size_t count) {
    if (count) std::memmove(to, from, count * sizeof(void*));
}
}
const RangeMemoryBoundaries& OriginalRangeMemoryBoundaries() {
    static const RangeMemoryBoundaries memory{GameRangeAllocate, GameRangeFree};
    return memory;
}
void InsertPointerRange(PointerVector* self, void** position, void* const* first,
    void* const* last, void* unused, const RangeMemoryBoundaries& memory) {
    (void)unused;
    if (first == last) return;
    auto* const begin = static_cast<void**>(self->begin);
    auto* const end = static_cast<void**>(self->cursor);
    const size_t incoming = PointerCount(first, last);
    const size_t suffix = PointerCount(position, end);
    if (incoming <= PointerCount(end, static_cast<void**>(self->cap))) {
        if (incoming < suffix) {
            CopyPointers(end, end - incoming, incoming);
            self->cursor = end + incoming;
            CopyPointers(position + incoming, position, suffix - incoming);
            CopyPointers(position, first, incoming);
        } else {
            CopyPointers(end, first + suffix, incoming - suffix);
            self->cursor = end + incoming - suffix;
            CopyPointers(static_cast<void**>(self->cursor), position, suffix);
            self->cursor = end + incoming;
            CopyPointers(position, first, suffix);
        }
        return;
    }
    const size_t size = PointerCount(begin, end);
    const size_t doubled = size ? 2 * size : 1;
    const size_t capacity = doubled > size + incoming ? doubled : size + incoming;
#if defined(_M_IX86)
    const auto* file = reinterpret_cast<const char*>(0x00F98980);
#else
    const auto* file = "c:\\BuildAgent\\slc1-build110\\CMBuild\\Core\\include\\EASTL/allocator.h";
#endif
    auto* const storage = static_cast<void**>(memory.allocate(capacity * sizeof(void*),
        self->tag, 0, 0, file, 0xe7));
    const size_t prefix = PointerCount(begin, position);
    CopyPointers(storage, begin, prefix);
    CopyPointers(storage + prefix, first, incoming);
    CopyPointers(storage + prefix + incoming, position, suffix);
    // Free sees the OLD vector fields and completed new copies. Inline identity
    // is the pointer stored at +10 on x86, not the address receiver+10 itself.
    if (begin && begin != self->inlineBegin) memory.free(begin);
    self->cursor = storage + size + incoming;
    self->begin = storage;
    self->cap = storage + capacity;
}
}

namespace ts3::resource::m03::delivery_catalog {
namespace {
void GameDestroyCatalog(Catalog* self) {
    CallGameMethod<void, 0x00732450>(self);
}
void GameDeleteCatalog(void* self) {
    CallGameFunction<void, 0x004E4230>(self);
}
}
const DestructionBoundaries& OriginalDestructionBoundaries() {
    static const DestructionBoundaries boundaries{GameDestroyCatalog, GameDeleteCatalog};
    return boundaries;
}
Catalog* DeleteCatalog(Catalog* self, uint32_t flags, const DestructionBoundaries& boundaries) {
    boundaries.destroy(self);
    if ((static_cast<uint8_t>(flags) & 1u) != 0) boundaries.free(self);
    return self;
}
}

namespace ts3::resource::m03::delivery_catalog {
void ShrinkCallback(Catalog* self, TrimBoundary trim) {
    trim(self, self->budget, nullptr, 1);
}
}

namespace ts3::resource::m03::delivery_catalog {
#if defined(_M_IX86)
namespace {
SchedulingService* GetSchedulingService() {
    return CallGameFunction<SchedulingService*, 0x004AFD20>();
}
}
const SchedulingBoundaries& OriginalSchedulingBoundaries() {
    static const SchedulingBoundaries boundaries{GetSchedulingService,
        reinterpret_cast<ChangeNotification>(0x00733970)};
    return boundaries;
}
#endif
uint8_t ScheduleChanges(void* self, const SchedulingBoundaries& boundaries) {
    auto* service = boundaries.manager();
    if (!service) return 0;
    const auto dispatch = service->vtable->schedule;
    return dispatch(service, 1, boundaries.notification, self, nullptr, nullptr);
}
uint8_t UnscheduleChanges(void* self, const SchedulingBoundaries& boundaries) {
    auto* service = boundaries.manager();
    if (!service) return 0;
    const auto dispatch = service->vtable->schedule;
    return dispatch(service, 0, boundaries.notification, self, nullptr, nullptr);
}
}

namespace ts3::resource::m03::delivery_catalog {
namespace {
constexpr uint32_t kReloadTag = 0x01CCE04Du;
constexpr int32_t kReloadSentinel = -1;
#if defined(_M_IX86)
SchedulingService* GameRoutingService() {
    return CallGameFunction<SchedulingService*, 0x004AFD20>();
}
ReloadManager* GameRoutingManager() {
    return CallGameFunction<ReloadManager*, 0x00727280>();
}
#endif
}
#if defined(_M_IX86)
const ReloadRoutingBoundaries& OriginalReloadRoutingBoundaries() {
    static const ReloadRoutingBoundaries boundaries{GameRoutingService, GameRoutingManager};
    return boundaries;
}
#endif
void ReloadRoute(const ts3::resource::ResourceKey* key, const ReloadRoutingBoundaries& boundaries) {
    // No service null guard: the original dereferences the getter result directly.
    auto* const service = boundaries.service();
    auto* const candidate = service->vtable->lookup(service, key->type, kReloadSentinel);
    if (candidate != nullptr && candidate->vtable->identity(candidate) == kReloadTag) {
        auto* const manager = boundaries.manager();
        if (manager != nullptr) manager->vtable->route(manager, key);
        return;
    }
    auto* const object = service->vtable->fallback(service, key);
    if (object == nullptr) return;
    // The local is zero-initialized before the output call and is released once
    // afterwards even when the call's AL is zero.
    void* temporary = nullptr;
    const uint8_t routed = object->vtable->output(object, key, &temporary);
    if (routed != 0) {
        // Re-read the CURRENT service table after the output call, as EA does.
        const auto dispatch = service->vtable->dispatch;
        dispatch(service, key, temporary, nullptr, nullptr, nullptr);
    }
    if (temporary != nullptr) {
        auto* const owned = static_cast<ReloadTemporary*>(temporary);
        owned->vtable->release(owned);
    }
}
}

#define WIN32_LEAN_AND_MEAN
#ifndef NOMINMAX
#define NOMINMAX
#endif
#include <windows.h>

namespace ts3::resource::m03::delivery_catalog {
#if defined(_M_IX86)
namespace {
// The genuine external imports, read from the same IAT cells the raw body calls,
// so a runtime import patch is observed exactly as the original observes it.
void NotificationImportEnter(void*, void* critical) {
    using Import = void(WINAPI*)(CRITICAL_SECTION*);
    const auto enter = reinterpret_cast<Import>(*reinterpret_cast<void**>(0x00F9512C));
    enter(static_cast<CRITICAL_SECTION*>(critical));
}
void NotificationImportLeave(void*, void* critical) {
    using Import = void(WINAPI*)(CRITICAL_SECTION*);
    const auto leave = reinterpret_cast<Import>(*reinterpret_cast<void**>(0x00F95130));
    leave(static_cast<CRITICAL_SECTION*>(critical));
}
NotificationIterator* NotificationGameLookup(NotificationMap* map,
    NotificationIterator* out, const uint32_t* keyType) {
    return CallGameMethod<NotificationIterator*, 0x00C45F60>(map, out, keyType);
}
void NotificationGameQueue(void* receiver, const ts3::resource::ResourceKey* key,
    ChangeNotification callback) {
    CallGameMethod<void, 0x00733770>(receiver, key, callback);
}
}
void ReloadNotification(void* database, const ts3::resource::ResourceKey* key, void*) {
    (void)database;
    ReloadRoute(key, OriginalReloadRoutingBoundaries());
}
const NotificationBoundaries& OriginalNotificationBoundaries() {
    static const NotificationBoundaries boundaries{nullptr, &NotificationImportEnter,
        &NotificationImportLeave, &NotificationGameLookup, &NotificationGameQueue,
        &ReloadNotification};
    return boundaries;
}
#endif
void NotificationCallback(void* database, const ts3::resource::ResourceKey* key,
    NotificationContext* context, const NotificationBoundaries& boundaries) {
    (void)database;
    // The raw body initializes the mode/callback locals before entering the lock;
    // both are snapshots that later external calls must not be able to change.
    uint32_t mode = 0;
    ChangeNotification callback = boundaries.fallback;
    void* const critical = &context->critical;
    boundaries.enter(boundaries.context, critical);
    NotificationIterator local;
    NotificationIterator* const returned = boundaries.lookup(&context->map, &local, &key->type);
    // The returned iterator pointer is what the body dereferences, not the local.
    NotificationNode* const node = returned->node;
    // CURRENT bucket count then CURRENT bucket array, both read after the lookup
    // while the lock is still held; the raw body loads +0x138 before +0x134.
    const uint32_t count = context->map.bucketCount;
    NotificationNode** const buckets = context->map.buckets;
    if (node != buckets[count]) {
        mode = node->mode;
        callback = node->callback;
    }
    boundaries.leave(boundaries.context, critical);
    // After the unlock only the saved context ADDRESS is used (the raw body keeps
    // it in EBX); nothing at the context is read, and the callback is never
    // invoked: 733770 owns queueing it.
    if (mode == 0) {
        boundaries.queue(reinterpret_cast<uint8_t*>(context) + offsetof(NotificationContext, queue0),
            key, callback);
    } else if (mode == 1) {
        boundaries.queue(reinterpret_cast<uint8_t*>(context) + offsetof(NotificationContext, queue1),
            key, callback);
    }
}
}

namespace ts3::resource::m03::delivery_catalog {
#if defined(_M_IX86)
namespace {
void NotificationGameDestroyMap(NotificationMap* map, NotificationNode** array, uint32_t count) {
    // Genuine 00AEBDB0 is thiscall ret8: ECX map, then array and count on the stack.
    CallGameMethod<void, 0x00AEBDB0>(map, array, count);
}
void NotificationGameFreeStorage(void* storage) {
    CallGameFunction<void, 0x004E3F70>(storage);
}
void NotificationGameTeardownQueue(void* receiver) {
    // Genuine 007333E0 is thiscall ret0 and its integer result is never consumed.
    CallGameMethod<void, 0x007333E0>(receiver);
}
void NotificationImportDeleteCritical(void* critical) {
    // Read the live import cell so a runtime import patch is observed exactly as
    // the raw body's indirect call observes it.
    using Import = void(WINAPI*)(CRITICAL_SECTION*);
    const auto remove = reinterpret_cast<Import>(*reinterpret_cast<void**>(0x00F95128));
    remove(static_cast<CRITICAL_SECTION*>(critical));
}
void NotificationGameBaseTeardown(void* self) {
    // Genuine 00731CF0 is thiscall; it writes the base vtable at [self+0].
    CallGameMethod<void, 0x00731CF0>(self);
}
}
const NotificationTeardownBoundaries& OriginalNotificationTeardownBoundaries() {
    static const NotificationTeardownBoundaries boundaries{NotificationGameDestroyMap,
        NotificationGameFreeStorage, NotificationGameTeardownQueue,
        NotificationImportDeleteCritical, NotificationGameBaseTeardown};
    return boundaries;
}
#endif
#if defined(_M_IX86)
void** NotificationPublicationCell() {
    // The genuine global 0x011D3E2C is 32 bits in the image; the rewritten x86
    // code still targets that process-global cell.
    return reinterpret_cast<void**>(static_cast<uintptr_t>(0x011D3E2C));
}
#else
namespace {
// The full-width x64 publication cell, the native analogue of the x86 global.
void* g_notificationPublication = nullptr;
}
void** NotificationPublicationCell() {
    return &g_notificationPublication;
}
#endif
void TeardownNotificationMonitor(NotificationContext* self,
    const NotificationTeardownBoundaries& boundaries) {
    NotificationMap* const map = &self->map;
    // Snapshot the array and count BEFORE the external map destroy; the destroy
    // may replace either current value and it must still receive these two.
    NotificationNode** const snapshotArray = map->buckets;
    const uint32_t snapshotCount = map->bucketCount;
    boundaries.destroyMap(map, snapshotArray, snapshotCount);
    // Reload the CURRENT count and compare unsigned (> 1), then clear the uint32
    // state word unconditionally before any further callback, then reload the
    // CURRENT array for the free.
    const uint32_t currentCount = map->bucketCount;
    map->state = 0;
    if (currentCount > 1u)
        boundaries.freeStorage(map->buckets);
    // receiver+0xB0 is torn down before receiver+0x30, exactly as the raw body.
    auto* const bytes = reinterpret_cast<uint8_t*>(self);
    boundaries.teardownQueue(bytes + offsetof(NotificationContext, queue1));
    boundaries.teardownQueue(bytes + offsetof(NotificationContext, queue0));
    boundaries.deleteCritical(bytes + offsetof(NotificationContext, critical));
    // The SUBJECT performs the unconditional publication clear itself, after the
    // delete-lock callback and before the terminal base teardown.
    *NotificationPublicationCell() = nullptr;
    boundaries.baseTeardown(self);
    // No read of self after the terminal base teardown.
}
}

namespace ts3::resource::m03::delivery_catalog {
#if defined(_M_IX86)
namespace {
// The genuine import, read from the same IAT cell the raw 00733610 call targets,
// so a runtime import patch is observed exactly as the original observes it.
void QueueImportInitializeLock(void* critical) {
    using Import = void(WINAPI*)(CRITICAL_SECTION*);
    const auto initialize = reinterpret_cast<Import>(*reinterpret_cast<void**>(0x00F95124));
    initialize(static_cast<CRITICAL_SECTION*>(critical));
}
// Genuine thiscall ret8 targets. Their incidental EAX is unused by 00733610.
void QueueImportTimerInitialize(void* timer, uint32_t mode, uint8_t start) {
    CallGameMethod<void*, 0x004F35B0>(timer, mode, start);
}
void QueueImportTimerSync(void* timer, uint32_t advance, uint8_t start) {
    CallGameMethod<void*, 0x004F34F0>(timer, advance, start);
}
const void* const kManifestTable = reinterpret_cast<const void*>(0x0114C900);
const float* const kFirstValue = reinterpret_cast<const float*>(0x0107A538);
const float* const kSecondValue = reinterpret_cast<const float*>(0x00F98A58);
const char* const kRequestedName = reinterpret_cast<const char*>(0x00FFDEF0);
const char* const kPendingName = reinterpret_cast<const char*>(0x00FFDEC0);
}
const ReloadQueueBoundaries& OriginalReloadQueueBoundaries() {
    static const ReloadQueueBoundaries boundaries{&QueueImportInitializeLock,
        &QueueImportTimerInitialize, &QueueImportTimerSync, kManifestTable, kFirstValue,
        kSecondValue, kRequestedName, kPendingName};
    return boundaries;
}
#endif
ReloadQueue* InitializeReloadQueue(ReloadQueue* self,
    const ReloadQueueBoundaries& boundaries) {
    boundaries.initLock(&self->critical);
    boundaries.initTimer(&self->timer, 4, 0);
    boundaries.syncTimer(&self->timer, 0, 1);
    // The raw body loads both cells once, AFTER every external call, and reuses
    // the two values for both manifests.
    const float first = *boundaries.firstValue;
    const float second = *boundaries.secondValue;
    ReloadManifest& requested = self->requested;
    requested.first = first;
    requested.second = second;
    requested.name = boundaries.requestedName;
    requested.state = 0;
    requested.zero = 0;
    requested.count = 1;
    requested.table = boundaries.manifestTable;
    ReloadManifest& pending = self->pending;
    pending.first = first;
    pending.second = second;
    pending.name = boundaries.pendingName;
    pending.table = boundaries.manifestTable;
    pending.state = 0;
    pending.zero = 0;
    pending.count = 1;
    return self;
}
}

namespace ts3::resource::m03::delivery_catalog {
#if defined(_M_IX86)
namespace {
// Genuine 0059ECC0 is thiscall ret8: ECX self, then the hash and the name on the
// stack. It writes the base table identity, the hash, a zero state word and the
// x86 name pointer.
void MonitorCreateBase(void* self, uint32_t hash, const void* typeName) {
    CallGameMethod<void*, 0x0059ECC0>(self, hash, typeName);
}
}
const MonitorConstructionBoundaries& OriginalMonitorConstructionBoundaries() {
    static const MonitorConstructionBoundaries boundaries{&MonitorCreateBase,
        reinterpret_cast<const void*>(0x00FFDF58), 0xB23536D5u,
        reinterpret_cast<const void*>(0x00FFDF8C),
        reinterpret_cast<const char*>(0x00FFDF24),
        OriginalReloadQueueBoundaries()};
    return boundaries;
}
#endif
NotificationContext* InitializeChangeMonitor(NotificationContext* self,
    const MonitorConstructionBoundaries& boundaries) {
    boundaries.baseConstruct(self, boundaries.typeHash, boundaries.typeName);
    // The raw body publishes receiver+0x10 BEFORE installing the table and
    // before initializing the lock; every later observation sees that value.
    *NotificationPublicationCell() = &self->critical;
    MonitorBase(self)->table = boundaries.monitorTable;
    boundaries.queue.initLock(&self->critical);
    auto* const bytes = reinterpret_cast<uint8_t*>(self);
    // The real nesting: both queue receivers are constructed through the SAME
    // portable InitializeReloadQueue, never a capture.
    InitializeReloadQueue(reinterpret_cast<ReloadQueue*>(
        bytes + offsetof(NotificationContext, queue0)), boundaries.queue);
    InitializeReloadQueue(reinterpret_cast<ReloadQueue*>(
        bytes + offsetof(NotificationContext, queue1)), boundaries.queue);
    ReloadManifest* const manifest = MonitorManifest(self);
    manifest->first = *boundaries.queue.firstValue;
    manifest->second = *boundaries.queue.secondValue;
    manifest->name = boundaries.pendingName;
    manifest->state = 0;
    manifest->zero = 0;
    manifest->count = 1;
    manifest->table = boundaries.queue.manifestTable;
    return self;
}
}

namespace ts3::resource::m03::delivery_catalog {
#if defined(_M_IX86)
namespace {
// The live import cells: the same 32-bit addresses the raw 00733AA0 body calls
// indirectly, so a runtime import patch is observed exactly as the original.
bool QueueImportTryLock(void* critical) {
    using Import = BOOL(WINAPI*)(CRITICAL_SECTION*);
    const auto tryEnter = reinterpret_cast<Import>(*reinterpret_cast<void**>(0x00F95218));
    return tryEnter(static_cast<CRITICAL_SECTION*>(critical)) != 0;
}
void QueueImportQueryCounter(int64_t* counter) {
    using Import = BOOL(WINAPI*)(LARGE_INTEGER*);
    const auto query = reinterpret_cast<Import>(*reinterpret_cast<void**>(0x00F953B8));
    query(reinterpret_cast<LARGE_INTEGER*>(counter));
}
void QueueImportUnlock(void* critical) {
    using Import = void(WINAPI*)(CRITICAL_SECTION*);
    const auto leave = reinterpret_cast<Import>(*reinterpret_cast<void**>(0x00F95130));
    leave(static_cast<CRITICAL_SECTION*>(critical));
}
// Genuine thiscall ret0C/ret4/plain-ret helpers. The 007336A0 third stack
// argument is never read by its body, so it stays an untyped dword here.
NotificationQueueInsert* QueueGameInsertOrFind(NotificationQueueMap* map,
    NotificationQueueInsert* out, NotificationQueueNode* entry, uint32_t flag) {
    return CallGameMethod<NotificationQueueInsert*, 0x007336A0>(map, out, entry, flag);
}
NotificationQueueIter* QueueGameErase(NotificationQueueMap* map,
    NotificationQueueIter* out, NotificationQueueNode* entry, NotificationQueueNode** bucket) {
    return CallGameMethod<NotificationQueueIter*, 0x00B33670>(map, out, entry, bucket);
}
NotificationQueueIter* QueueGameBegin(NotificationQueueMap* map, NotificationQueueIter* out) {
    return CallGameMethod<NotificationQueueIter*, 0x00626610>(map, out);
}
void QueueGameAdvance(NotificationQueueIter* iter) {
    CallGameMethod<void, 0x007332B0>(iter);
}
NotificationService* QueueGameService() {
    return CallGameFunction<NotificationService*, 0x004AFD20>();
}
}
const NotificationQueueBoundaries& OriginalNotificationQueueBoundaries() {
    static const NotificationQueueBoundaries boundaries{QueueImportTryLock,
        QueueImportQueryCounter, QueueImportUnlock, QueueGameInsertOrFind, QueueGameErase,
        QueueGameBegin, QueueGameAdvance, QueueGameService};
    return boundaries;
}
#endif

void DispatchNotificationQueue(ReloadQueue* self,
    const NotificationQueueBoundaries& boundaries) {
    void* const critical = &self->critical;
    if (!boundaries.tryLock(critical)) return;
    int64_t counter = 0;
    boundaries.queryCounter(&counter);
    // Raw: mov ecx,[queue+0x38]; sub ecx,[time]; mov eax,[queue+0x3C];
    // sbb eax,[time+4]; test eax,eax; jg return; jl process; test ecx,ecx;
    // jae return. Compute the 64-bit difference unsigned and inspect the sign
    // bit of the high word, so no host signed overflow is involved: the body
    // proceeds only when the difference is negative.
    const uint64_t difference = self->timer.deadline - static_cast<uint64_t>(counter);
    if ((difference >> 63) == 0) {
        boundaries.unlock(critical);
        return;
    }
    NotificationQueueMap* const source = SourceQueueMap(self);
    if (source->elementCount == 0) {
        boundaries.unlock(critical);
        return;
    }
    NotificationQueueMap* const destination = DestinationQueueMap(self);
    while (source->elementCount != 0) {
        // The CURRENT buckets pointer is reloaded on every iteration; null
        // bucket slots are skipped until the first nonnull entry. A nonempty
        // source has one, and the caller owns the finite backing.
        NotificationQueueNode** bucket = source->buckets;
        NotificationQueueNode* entry = *bucket;
        while (entry == nullptr) {
            ++bucket;
            entry = *bucket;
        }
        // Raw byte [esp+14h]=0 then push dword [esp+14h]; the upper bytes are
        // stale stack contents, and the genuine 007336A0 reads neither the
        // low nor the upper bytes of the argument, so no initialized value is
        // promised beyond the required zero low byte.
        uint32_t flag = 0;
        NotificationQueueInsert inserted;
        boundaries.insertOrFind(destination, &inserted, entry, flag);
        if (inserted.inserted != 0) {
            inserted.node->dispatch = entry->dispatch;
            inserted.node->retry = 50;
        }
        NotificationQueueIter removed;
        boundaries.erase(source, &removed, entry, bucket);
    }
    // Unlock BEFORE the service getter and the whole destination walk.
    boundaries.unlock(critical);
    NotificationService* const service = boundaries.service();
    NotificationQueueIter iter;
    boundaries.begin(destination, &iter);
    // The sentinel is loaded through the CURRENT bucket-count/array pair ONCE,
    // after the begin call, and every later comparison uses this snapshot.
    const uint32_t sentinelCount = destination->bucketCount;
    NotificationQueueNode** const sentinelBuckets = destination->buckets;
    NotificationQueueNode* const sentinel = sentinelBuckets[sentinelCount];
    while (iter.node != sentinel) {
        // The raw body reloads the saved service pointer from its stack slot on
        // every iteration; it is the same pointer at all times.
        NotificationService* const current = service;
        NotificationResult* const result = current->vtable->lookup(current, iter.node);
        if (result == nullptr) {
            NotificationQueueIter removed;
            NotificationQueueIter* const next =
                boundaries.erase(destination, &removed, iter.node, iter.bucket);
            iter.node = next->node;
            iter.bucket = next->bucket;
        } else {
            NotificationLocal* local = nullptr;
            const uint8_t dispatched =
                result->vtable->call(result, iter.node, &local, 1, 6, 1, 0);
            if (dispatched != 0) {
                // Raw dereferences the local without a null guard.
                local->vtable->finish(local);
                NotificationLocal* const released = local;
                local = nullptr;
                if (released != nullptr) released->vtable->release(released);
                iter.node->dispatch(iter.node);
                // Raw quirk: at the release the callback's cdecl argument is
                // still pushed, so the receiver is read from esp+14h, which
                // aliases the just-cleared local slot (esp+10h) and is null.
                // The saved service pointer is used only by the lookups; the
                // release branch is dead code and is reproduced as such.
                if (local != nullptr) local->vtable->release(local);
                NotificationQueueIter removed;
                NotificationQueueIter* const next =
                    boundaries.erase(destination, &removed, iter.node, iter.bucket);
                iter.node = next->node;
                iter.bucket = next->bucket;
            } else {
                // Write the wrapped decrement BEFORE testing the old value.
                const uint32_t old = iter.node->retry;
                iter.node->retry = old - 1;
                if (old != 0) {
                    NotificationQueueIter removed;
                    NotificationQueueIter* const next =
                        boundaries.erase(destination, &removed, iter.node, iter.bucket);
                    iter.node = next->node;
                    iter.bucket = next->bucket;
                } else {
                    boundaries.advance(&iter);
                }
                if (local != nullptr) local->vtable->release(local);
            }
        }
    }
}
}
