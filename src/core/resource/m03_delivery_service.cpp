#include "ts3/resource/m03_delivery_service.h"

#include <cstddef>

#include "ts3/resource/m03_delivery_accounting.h"

namespace ts3::resource::m03::delivery {
uint32_t ConditionalService(ServiceView* self)
{
    auto* adjusted = &self->adjusted;
    const uint32_t locked = adjusted->vtable->lock(adjusted);
    if (static_cast<uint8_t>(locked) == 0)
        return locked;
    // Lock can synchronously replace the interface's vtable.
    const uint32_t finalized = adjusted->vtable->finalize(adjusted);
    return (finalized & 0xffffff00u) | 1u;
}
}

namespace ts3::resource::m03::delivery {
uint32_t UpdateAndFinalize(ServiceInterfaceView* self, void* argument, BaseUpdate base)
{
    const uint32_t updated = base(self, argument);
    if (static_cast<uint8_t>(updated) == 0)
        return updated;
    const uint32_t finalized = self->vtable->finalize(self);
    return (finalized & 0xffffff00u) | static_cast<uint8_t>(updated);
}

uint32_t GatedUpdateAndFinalize(ServiceInterfaceView* self, void* argument)
{
    const uint32_t locked = self->vtable->lock(self);
    if (static_cast<uint8_t>(locked) == 0)
        return locked;
    // 0x00624F70 is exactly xor al,al; ret 4. It ignores receiver and
    // argument, touches no memory, and returns false. No image fallback.
    (void)argument;
    constexpr uint8_t updated = 0;
    const uint32_t finalized = self->vtable->finalize(self);
    return (finalized & 0xffffff00u) | updated;
}
}

namespace ts3::resource::m03::delivery {
const char16_t* SelectPrimaryPath(const EndpointOwnerView* self)
{
    return self->primaryPath.begin != self->primaryPath.end ? self->primaryPath.begin : nullptr;
}
const char16_t* SelectSecondaryPath(const EndpointOwnerView* self)
{
    return self->secondaryPath.begin != self->secondaryPath.end ? self->secondaryPath.begin : nullptr;
}
}

namespace ts3::resource::m03::delivery {
uint32_t NormalizeStatus(RetainedOwnerView* self, OwnerExternal activate)
{
    const uint32_t word = activate(self);
    return (word & 0xffffff00u) | (static_cast<uint8_t>(word) != 0 ? 1u : 0u);
}

uint32_t CleanupRetained(RetainedOwnerView* self, OwnerExternal base, ReleaseExternal release)
{
    uint32_t word = base(self);
    if (auto* handle = self->retained) {
        word = release(handle);
        self->retained = nullptr;
    }
    return (word & 0xffffff00u) | 1u;
}
}

namespace ts3::resource::m03::delivery {
uint32_t DispatchRetained(RetainedOwnerView* self, void* arg1, void* arg2,
                          const DispatchExternals& external)
{
    uint32_t result = 0;
    external.set(self, 1);
    if (auto* handle = self->retained) {
        result = external.handle(handle, arg1, arg2);
    } else if (static_cast<uint8_t>(self->vtable->lock(self)) != 0) {
        result = external.base(self, arg1, arg2);
        // The external callback may replace the owner's vtable.
        self->vtable->finalize(self);
    }
    external.set(self, 0);
    return result;
}

uint32_t DispatchRetainedSix(RetainedOwnerView* self, const ResourceKey* key, void** outRecord, uint32_t flags,
                             uint32_t mode, uint8_t flagByte, MemoryDBRecordInfo* info,
                             const RetainedSixExternals& external)
{
    if (outRecord == nullptr && info == nullptr) {
        external.set(self, 1);
        bool handled = false;
        uint32_t handleWord = 0;
        // The raise callback may synchronously publish a handle: reload the field after it.
        if (auto* handle = self->retained) {
            handleWord = external.handle(handle, key);
            handled = true;
        }
        // The clear always runs; its EAX supplies the upper bytes of a handled result.
        const uint32_t cleared = external.set(self, 0);
        if (handled)
            return (cleared & 0xffffff00u) | static_cast<uint8_t>(handleWord);
        // Null handle (or no probe): the ordinary path runs AFTER the clear.
    }
    const uint32_t locked = self->vtable->lock(self);
    if (static_cast<uint8_t>(locked) == 0)
        return locked;
    // The genuine 0x0072D470 read can replace the owner table: reload before slot21.
    uint32_t word = external.read(self, key, outRecord, flags, mode, flagByte, info);
    const uint8_t readLow = static_cast<uint8_t>(word);
    if (readLow == 0 || outRecord == nullptr)
        word = self->vtable->finalize(self);
    return (word & 0xffffff00u) | readLow;
}
}

namespace ts3::resource::m03::delivery {
uint32_t CreateRetainedDatabaseIndex(DatabaseIndexOwnerView* self, DatabaseIndexStream* stream, uint32_t mode,
                                     uint32_t size, uint32_t unused, uint32_t stampLow, uint32_t stampHigh,
                                     const DatabaseIndexExternals& external)
{
    // a4 is never read by the original; keep it out of every store path.
    (void)unused;
    RetainedOwnerView* database = &self->database;
    // The acquire's EAX is discarded by the original (BL was already 1 before the call).
    external.lock(database, 1);
    uint8_t carried = 1;
    if (database->retained == nullptr && self->gate == 0) {
        // Original six-argument allocator literals: 0x24 requested bytes, the type-name
        // pointer (original-only raw address 0x00FFD980), flags 0, reserved 0, no debug file
        // and line 0. A null allocation skips the constructor entirely.
        const DatabaseIndexAllocationRequest request{0x24u, "ResourceSystem/DatabaseIndex", 0u, 0u, nullptr, 0u};
        DatabaseIndexHandle* const allocation = external.allocate(request);
        DatabaseIndexHandle* constructed = allocation != nullptr ? external.construct(allocation) : nullptr;
        // EA stores the constructor's RETURN (null when neither ran) before testing it, so a
        // side effect on the handle field is overwritten even on this failure path.
        database->retained = constructed;
        if (constructed == nullptr) {
            carried = 0;
        } else {
            external.retain(constructed);
            // The retain callback may synchronously replace the handle: reload for the load.
            DatabaseIndexHandle* const current = database->retained;
            carried = static_cast<uint8_t>(external.load(current, stream, mode));
            if (carried != 0) {
                // Same store order as the original: +0xFC, +0xF8, +0x100; a4 never stored.
                self->stampHigh = stampHigh;
                self->stampLow = stampLow;
                self->fileSize = size;
            }
        }
    }
    // Exactly one unlock on every path; its EAX supplies the result's upper 24 bits.
    const uint32_t unlocked = external.lock(database, 0);
    return (unlocked & 0xffffff00u) | carried;
}
}

namespace ts3::resource::m03::delivery {
uint32_t AcquireRetainedOutputs(DatabaseIndexOwnerView* self, DatabaseIndexHandle** outHandle, uint32_t* outSize,
                                uint32_t* outStamp, const RetainedAcquireExternals& external)
{
    RetainedOwnerView* const interior = &self->database;
    DatabaseIndexHandle* local = nullptr;
    external.lock(interior, 1);
    if (self->database.retained == nullptr) {
        // The slot20 callback can synchronously replace the interface table, so slot21 is
        // read from the reloaded table; only AL decides whether slot21 runs at all.
        if (static_cast<uint8_t>(interior->vtable->lock(interior)) != 0)
            interior->vtable->finalize(interior);
    }
    // The callbacks may publish or clear the handle: reload before choosing the path.
    DatabaseIndexHandle* const retained = self->database.retained;
    if (retained != nullptr) {
        local = retained;
        external.retain(local);
        // Read the metadata after the retain callback; the cached words may have changed.
        outStamp[0] = self->stampLow;
        outStamp[1] = self->stampHigh;
        outSize[0] = self->fileSize;
        outSize[1] = 0;
    } else if (self->gate != 0) {
        // Original six-argument allocator literals: 0x24 requested bytes and the semantic
        // type name (the original-only raw 0x00FFD980 identity stays in the game image).
        const DatabaseIndexAllocationRequest request{0x24u, "ResourceSystem/DatabaseIndex", 0u, 0u, nullptr, 0u};
        DatabaseIndexHandle* const allocation = external.allocate(request);
        if (allocation == nullptr) {
            local = nullptr; // 0x735319 clears the local result and skips the constructor
        } else {
            // The constructor's RETURN, not the allocation, is the local result; a null
            // return skips the retain/metadata path and is never published to self+0xB8.
            local = external.construct(allocation);
            if (local != nullptr) {
                external.retain(local);
                RetainedTemporary temporary{};
                temporary.typeName = "ResourceSystem/DatabaseIndex";
                external.prepare(interior, &temporary, 0);
                external.loadTemporary(local, &temporary);
                const wchar_t* const source = external.acquireSource(interior);
                const uint64_t stamp = external.convertTimestamp(source, 2);
                outStamp[0] = static_cast<uint32_t>(stamp);
                outStamp[1] = static_cast<uint32_t>(stamp >> 32);
                outSize[0] = external.convertSize(source);
                // The first member may have been published or cleared by the callbacks; the
                // second output word is stored before the free, exactly as in the original.
                void* const first = temporary.begin;
                outSize[1] = 0;
                if (first != nullptr)
                    external.releaseTemporary(first);
            }
        }
    }
    external.lock(interior, 0);
    *outHandle = local;
    return local != nullptr ? 1u : 0u;
}
}

namespace ts3::resource::m03::delivery {
uint8_t RefreshOnForceOrDeadline(DeadlineRefreshView* self, uint32_t force,
                                 const DeadlineRefreshExternals& external)
{
    // Two exact full-dword gates, not byte tests: anything but 1 fails them.
    if (self->state != 1u)
        return 0;
    if (self->gate != 1u)
        return 1;
    external.enterCriticalSection(self->criticalSection);
    if (self->gate != 1u) {
        // The enter callback changed the gate: no refresh on this path.
        external.leaveCriticalSection(self->criticalSection);
        return 1;
    }
    if (static_cast<uint8_t>(force) != 0) {
        // EA reads only the low argument byte; the refresh's AL is discarded.
        external.refresh(self);
    } else {
        uint64_t now = 0;
        external.queryPerformanceCounter(&now);
        // The clock callback may mutate the cached window: read both words only now.
        const uint64_t base = (static_cast<uint64_t>(self->baseHigh) << 32) | self->baseLow;
        const uint64_t deadline = (static_cast<uint64_t>(self->deadlineHigh) << 32) | self->deadlineLow;
        // Modulo-64 elapsed, then UNSIGNED high/low comparison: equality refreshes.
        const uint64_t elapsed = now - base;
        if (elapsed < deadline) {
            external.leaveCriticalSection(self->criticalSection);
            return 0;
        }
        external.refresh(self);
    }
    // Every entered path that does not return early runs exactly one leave.
    external.leaveCriticalSection(self->criticalSection);
    return 1;
}
}

namespace ts3::resource::m03::delivery {
namespace {
// Layout compatibility with the accounting-established key record: the message is the
// same borrowed prefix the used vtable 0x00FFE054 slot +4 (KeyRecordEquals) consumes.
static_assert(offsetof(KeySignalMessage, key) == offsetof(accounting::KeyRecordView, key),
              "signal message key placement matches the accounting key record");
#if defined(_M_IX86)
uint32_t __fastcall KeySignalIdentityEquals(const KeySignalMessage* self, void*,
                                            const ResourceKey* key)
#else
uint32_t KeySignalIdentityEquals(const KeySignalMessage* self, const ResourceKey* key)
#endif
{
    // The original identity is the cls_532c30_x table 0x00FFE054; its only used method is
    // slot +4. Reuse the rewritten 0x007341F0 instead of re-deriving the compare.
    return accounting::KeyRecordEquals(
        reinterpret_cast<const accounting::KeyRecordView*>(self), key);
}
const KeySignalVtable kKeySignalIdentity{nullptr, &KeySignalIdentityEquals};
} // namespace

const KeySignalVtable* KeySignalIdentity()
{
    return &kKeySignalIdentity;
}

void SignalKeyWatchers(SignalReceiverView* self, const ResourceKey* key,
                       const KeySignalExternals& external)
{
    // Owned local backing: the sentinel collector and the message. The original keeps them
    // adjacent in its own frame; no used contract observes their relative placement.
    struct LocalSignal {
        WatcherCollector collector;
        KeySignalMessage message;
    };
    LocalSignal local; // +4 of the message stays uninitialized, as in the original.
    local.message.identity = KeySignalIdentity();
    // Copy ALL four key words to owned backing BEFORE any external call, in the original's
    // store order (a0[0], a0[4], a0[8], a0[0xC]): the collection callback may change the
    // caller's key and the message is the pre-call snapshot.
    local.message.key.instanceLow = key->instanceLow;
    local.message.key.instanceHigh = key->instanceHigh;
    local.message.key.type = key->type;
    local.message.key.group = key->group;
    local.collector.next = &local.collector;
    local.collector.prev = &local.collector;
    local.collector.typeName = "ResourceSystem/FRCacheList";
    // The original reads [self+8] and passes self+8 as the receiver; production passes the
    // embedded base member address, never the primary object.
    external.collect(&self->embedded, &local.collector);
    auto* const sentinel = reinterpret_cast<WatcherNode*>(&local.collector);
    // Notifications: reload the CURRENT node's next link AFTER the observer call, so links
    // rewired by the observer are honored, and pass the same owned message to every node.
    for (WatcherNode* node = static_cast<WatcherNode*>(local.collector.next); node != sentinel;) {
        external.notify(node->observer, &local.message);
        node = node->next;
    }
    // Cleanup: reload the sentinel's next link (a notification may have replaced it) and
    // read each node's next link BEFORE releasing the node.
    for (WatcherNode* node = static_cast<WatcherNode*>(local.collector.next); node != sentinel;) {
        WatcherNode* const next = node->next;
        external.release(node);
        node = next;
    }
}
}
