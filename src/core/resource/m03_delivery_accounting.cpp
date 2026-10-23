#include "ts3/resource/m03_delivery_accounting.h"
#include <atomic>
#include <bit>
#include <cstddef>
#include <cstring>
#include <stdexcept>

namespace ts3::resource::m03::accounting {
int32_t Release(NodeView* node, const ReleaseBoundary* boundary) {
    static_assert(std::atomic_ref<uint32_t>::is_always_lock_free);
    std::atomic_ref<uint32_t> word(node->packedReferences);
    const uint32_t first = word.fetch_add(0xffffff01u, std::memory_order_seq_cst) - 255u;
    if (std::bit_cast<int32_t>(first) >= 256) {
        const uint32_t second = word.fetch_add(0xffffffffu, std::memory_order_seq_cst) - 1u;
        const int32_t count = std::bit_cast<int32_t>(second) >> 8;
        if (count == 1) {
            OwnerView* owner = node->owner;
            if (owner) {
                if (boundary) boundary->insertLocked(boundary->context, owner, node);
                else {
#if defined(_M_IX86)
                    ts3::CallGameMethod<void, 0x007328b0>(owner, node);
#else
                    throw std::logic_error("Release requires native catalog insertion boundary");
#endif
                }
            }
        }
        return count; // Saved before a reentrant external call; never reload self.
    }
    const auto* table = static_cast<const ReleaseTable*>(node->vtable);
    if (static_cast<uint8_t>(table->predicate(node)) != 0) {
        // JNE in EA consumes XADD's addition ZF, NOT the returned old EAX.
        const uint32_t second = word.fetch_add(0xffffffffu, std::memory_order_seq_cst) - 1u;
        if (second == 0) {
            table = static_cast<const ReleaseTable*>(node->vtable);
            table->destroy(node, 1);
        }
    }
    return 0; // No self access after the potentially deleting callback.
}

int32_t AddRef(NodeView* node, const MutexBoundary* boundary) {
    static_assert(std::atomic_ref<uint32_t>::is_always_lock_free);
    static_assert(std::atomic_ref<uint32_t>::required_alignment <= alignof(uint32_t));
    std::atomic_ref<uint32_t> word(node->packedReferences);
    if ((word.load(std::memory_order_relaxed) & 0xffffff00u) == 0x100u) {
        OwnerView* owner = node->owner;
        if (owner) MakeIdle(owner, node, boundary);
    }
    const uint32_t old = word.fetch_add(0x100u, std::memory_order_seq_cst);
    return std::bit_cast<int32_t>(old + 0x100u) >> 8;
}

void SetSize(NodeView* node, uint32_t newSize) {
    OwnerView* owner = node->owner;
    if (owner) Account(owner, node, node->size, newSize);
    else node->size = newSize;
}

void Account(OwnerView* owner, NodeView* node, uint32_t oldSize, uint32_t newSize,
             const MutexBoundary* boundary)
{
    const uint32_t delta = newSize - oldSize;
#if defined(_M_IX86)
    const auto& label = *reinterpret_cast<const ts3::thread::ThreadTime*>(0x00ffdd34);
#else
    const ts3::thread::ThreadTime label = ts3::thread::kTimeoutNone;
#endif
    if (boundary) boundary->lock(boundary->context, owner->mutex, label);
    else {
#if defined(_M_IX86)
        ts3::CallGameMethod<int, 0x004e16f0>(&owner->mutex, &label);
#else
        owner->mutex.Lock(label);
#endif
    }
    if (node->previous || node->next)
        owner->linkedBytes += delta;
    else
        owner->unlinkedBytes += delta;
    node->size = newSize;
    if (boundary) boundary->unlock(boundary->context, owner->mutex);
    else {
#if defined(_M_IX86)
        ts3::CallGameMethod<int, 0x004e17b0>(&owner->mutex);
#else
        owner->mutex.Unlock();
#endif
    }
}
}

namespace ts3::resource::m03::accounting {
namespace {
NodeView* Container(LinkView* link) {
    return link ? reinterpret_cast<NodeView*>(reinterpret_cast<uintptr_t>(link) - offsetof(NodeView, previous)) : nullptr;
}
}
NodeView** Unlink(void*, NodeView** out, NodeView* cursor) {
    NodeView* next = Container(cursor->next);
    NodeView* previous = Container(cursor->previous);
    // Do not turn EA's invalid-null low-address writes into successful erases.
    // Valid rings include a fully backed sentinel with actual link members.
    next->previous = previous ? LinkOf(previous) : nullptr;
    previous->next = LinkOf(next);
    *out = previous;
    return out;
}
int MakeIdle(OwnerView* owner, NodeView* node, const MutexBoundary* boundary) {
#if defined(_M_IX86)
    const auto& label = *reinterpret_cast<const ts3::thread::ThreadTime*>(0x00ffdd34);
#else
    const ts3::thread::ThreadTime label = ts3::thread::kTimeoutNone;
#endif
    if (boundary) boundary->lock(boundary->context, owner->mutex, label);
    else {
#if defined(_M_IX86)
        ts3::CallGameMethod<int, 0x004e16f0>(&owner->mutex, &label);
#else
        owner->mutex.Lock(label);
#endif
    }
    if (node->previous || node->next) {
        NodeView* out;
        Unlink(reinterpret_cast<unsigned char*>(owner) + 0x3c, &out, node);
        node->next = nullptr;
        node->previous = nullptr;
        owner->linkedBytes -= node->size;
        owner->unlinkedBytes += node->size;
    }
    if (boundary) return boundary->unlock(boundary->context, owner->mutex);
#if defined(_M_IX86)
    return ts3::CallGameMethod<int, 0x004e17b0>(&owner->mutex);
#else
    return owner->mutex.Unlock();
#endif
}
}

#define WIN32_LEAN_AND_MEAN
#define NOMINMAX
#include <windows.h>
#include <stdexcept>
namespace ts3::resource::m03::accounting {
void ReturnPool(PoolView* pool, void* block, uint32_t, const PoolBoundary& boundary) {
    // uintptr_t subtraction preserves x86 modulo arithmetic AND full native VA.
    const uintptr_t delta = reinterpret_cast<uintptr_t>(block) - reinterpret_cast<uintptr_t>(pool->slots);
    if (delta < 0x80000u) {
        const size_t index = delta >> 7;
        if (boundary.enter) boundary.enter(boundary.context, pool->critical);
        else EnterCriticalSection(reinterpret_cast<CRITICAL_SECTION*>(pool->critical));
        pool->bitmap[index >> 5] |= uint32_t(1) << (index & 31);
        if (boundary.leave) boundary.leave(boundary.context, pool->critical);
        else LeaveCriticalSection(reinterpret_cast<CRITICAL_SECTION*>(pool->critical));
    } else {
        if (boundary.deallocate) boundary.deallocate(boundary.context, boundary.allocator, block);
        else {
#if defined(_M_IX86)
            auto* allocator = *reinterpret_cast<void**>(0x011cb864);
            ts3::CallGameMethod<void, 0x004e5920>(allocator, block);
#else
            // Native integration must supply its allocator's deallocation seam;
            // it must not silently use a different heap for foreign allocations.
            throw std::logic_error("ReturnPool requires native allocator deallocation boundary");
#endif
        }
    }
}
}

namespace ts3::resource::m03::accounting {
void ReleaseActivity(ActivityView* self, const ActivityBoundary& boundary) {
    static_assert(sizeof(CRITICAL_SECTION) == sizeof(self->critical));
    static_assert(alignof(ActivityView) >= alignof(CRITICAL_SECTION));
    static_assert(std::atomic_ref<uint32_t>::is_always_lock_free);
    static_assert(std::atomic_ref<uint32_t>::required_alignment <= alignof(uint32_t));
    if (boundary.enter) boundary.enter(boundary.context, self->critical);
    else EnterCriticalSection(reinterpret_cast<CRITICAL_SECTION*>(self->critical));
    std::atomic_ref<uint32_t> count(self->activity);
    count.fetch_sub(2u, std::memory_order_seq_cst);
    // EA explicitly reloads; the XADD old value is not the branch operand.
    if (count.load(std::memory_order_seq_cst) == 1u) {
        alignas(LARGE_INTEGER) uint32_t words[2];
        if (boundary.counter) boundary.counter(boundary.context, words);
        else {
            LARGE_INTEGER value;
            QueryPerformanceCounter(&value);
            words[0] = value.LowPart;
            words[1] = static_cast<uint32_t>(value.HighPart);
        }
        self->timestampLow = words[0];
        self->timestampHigh = words[1];
    }
    if (boundary.leave) boundary.leave(boundary.context, self->critical);
    else LeaveCriticalSection(reinterpret_cast<CRITICAL_SECTION*>(self->critical));
}
}

namespace ts3::resource::m03::accounting {
bool AcquireActivity(ActivityView* self, const AcquireBoundary& b) {
#if !defined(_M_IX86)
    if (!b.initialize || !b.enable || !b.cleanup || !b.query)
        throw std::logic_error("AcquireActivity requires native external service integration");
#endif
    if (b.enter) b.enter(b.context, self->critical);
    else EnterCriticalSection(reinterpret_cast<CRITICAL_SECTION*>(self->critical));
    bool success = true;
    std::atomic_ref<uint32_t> count(self->activity);
    count.fetch_add(2u, std::memory_order_seq_cst);
    if (count.load(std::memory_order_seq_cst) == 2u) {
        uint32_t initialized = 0;
        if (b.initialize) initialized = b.initialize(b.context, self, 1, 6, 0);
        else {
#if defined(_M_IX86)
            initialized = ts3::CallGameMethod<uint32_t, 0x0072d790>(self, 1u, 6u, 0u);
#endif
        }
        if (static_cast<uint8_t>(initialized) == 0) {
            count.exchange(0u, std::memory_order_seq_cst);
            success = false;
        } else {
            count.fetch_add(1u, std::memory_order_seq_cst);
            auto enable = [&](uint32_t value) {
                if (b.enable) b.enable(b.context, self, value);
                else {
#if defined(_M_IX86)
                    ts3::CallGameMethod<uint32_t, 0x0072c6c0>(self, value);
#endif
                }
            };
            enable(1);
            void* current = self->retained;
            if (current) {
                if (b.cleanup) b.cleanup(b.context, current);
                else {
#if defined(_M_IX86)
                    ts3::CallGameMethod<uint32_t, 0x0072db80>(current);
#endif
                }
                self->retained = nullptr;
            }
            enable(0);
        }
    }
    if (b.leave) b.leave(b.context, self->critical);
    else LeaveCriticalSection(reinterpret_cast<CRITICAL_SECTION*>(self->critical));
    if (self->finalState == 2u) {
        uint32_t value = 0;
        if (b.query) value = b.query(b.context, self, 0, 0);
        else {
#if defined(_M_IX86)
            value = ts3::CallGameMethod<uint32_t, 0x0072c930>(self, 0u, 0u);
#endif
        }
        self->finalState = value <= 1000u ? 1u : 0u;
    }
    return success;
}
}

namespace ts3::resource::m03::accounting {
bool CheckWatcherChanged(WatcherView* self, const WatchBoundary& boundary) {
#if !defined(_M_IX86)
    // Native has no original image to bridge to: absent typed captures must fail
    // explicitly rather than pretend to provide a file/metadata/release service.
    if (!boundary.identity || !boundary.size || !boundary.stamp || !boundary.release)
        throw std::logic_error("CheckWatcherChanged requires native external file-service integration");
#endif
    // 72C920: thiscall on self+8; the FULL returned identity is saved and reused.
    void* identity = boundary.identity ? boundary.identity(boundary.context, &self->reader)
#if defined(_M_IX86)
        : ts3::CallGameMethod<void*, 0x0072C920>(&self->reader);
#else
        : nullptr; // Unreachable: the native precheck demanded the typed capture.
#endif
    // 4DBC60: cdecl(identity), caller pops 4; the FULL EAX is compared as one dword.
    const uint32_t size = boundary.size ? boundary.size(boundary.context, identity)
#if defined(_M_IX86)
        : ts3::CallGameFunction<uint32_t, 0x004DBC60>(identity);
#else
        : 0u; // Unreachable: the native precheck demanded the typed capture.
#endif
    if (size == self->size) {
        // 4DBCF0: cdecl(identity, 2), caller pops 8; EDX:EAX is one 64-bit result.
        const uint64_t stamp = boundary.stamp ? boundary.stamp(boundary.context, identity, 2u)
#if defined(_M_IX86)
            : ts3::CallGameFunction<uint64_t, 0x004DBCF0>(identity, 2u);
#else
            : 0u; // Unreachable: the native precheck demanded the typed capture.
#endif
        // Exact original order: full low dword first, then the high dword, both
        // short-circuiting; a size mismatch never reaches the second query at all.
        if (static_cast<uint32_t>(stamp) == self->stampLow &&
            static_cast<uint32_t>(stamp >> 32) == self->stampHigh)
            return false; // AL=0: retained pointer left intact, no cleanup call.
    }
    // The pointer is RELOADED only here, after the query callbacks, so query-side
    // edits are honored; the release callback runs BEFORE the field is cleared.
    void* retained = self->retained;
    if (retained) {
        if (boundary.release) boundary.release(boundary.context, retained);
        else {
#if defined(_M_IX86)
            ts3::CallGameMethod<uint32_t, 0x0072DB80>(retained);
#endif
        }
        self->retained = nullptr;
    }
    return true; // AL=1 even when the reloaded pointer was already null.
}
}

namespace ts3::resource::m03::accounting {
// 0x007341F0 exact EA compare order: word0 (input+0 vs self+8), word2 (input+8 vs
// self+0x10), word3 (input+C vs self+0x14), word1 (input+4 vs self+C). All four are
// full 32-bit value compares; every mismatch short-circuits to a full EAX 0 exactly
// like the original branches, and only the final fall-through yields full EAX 1.
uint32_t KeyRecordEquals(const KeyRecordView* self, const ResourceKey* key) {
    if (key->instanceLow != self->key.instanceLow) return 0;
    if (key->type != self->key.type) return 0;
    if (key->group != self->key.group) return 0;
    if (key->instanceHigh != self->key.instanceHigh) return 0;
    return 1;
}
}

namespace ts3::resource::m03::accounting {
namespace {
// One truthful production seam for both 0x00734860 and 0x00734940: the two loops are
// byte-for-byte identical except the chain offset, which is selected by the SEPARATE
// bounded node views. The template never aliases the two node contracts.
template <class MapView, class Node>
uint32_t RedistributePairedBuckets(MapView* map, uint32_t newBucketCount,
                                   const RedistributeBoundary* boundary) {
    // 0x0073488x: one allocation of newBucketCount+1 pointer slots (count*4+4 on x86),
    // named by the map's +0x1C allocator identity with flags 0x400000, source file
    // 0x00F98980
    const size_t slotBytes = size_t(newBucketCount) * sizeof(void*) + sizeof(void*);
    void* fresh;
    if (boundary) fresh = boundary->allocate(boundary->context, slotBytes, map->allocatorIdentity,
                                             0x400000u, 0u, reinterpret_cast<const void*>(0x00F98980), 0xE7u);
    else {
#if defined(_M_IX86)
        fresh = ts3::CallGameFunction<void*, 0x004E3F90>(slotBytes, map->allocatorIdentity, 0x400000u,
                                                         0u, reinterpret_cast<const void*>(0x00F98980), 0xE7u);
#else
        // No production allocator exists natively; foreign ownership is not guessed.
        throw std::logic_error("bucket redistribution requires native allocation boundary");
#endif
    }
    // Ordinary production byte clearing of the count slots; the trailing cell is the
    // sentinel, written right after the clear exactly like the original.
    std::memset(fresh, 0, size_t(newBucketCount) * sizeof(void*));
    Node** freshBuckets = static_cast<Node**>(fresh);
    // Numeric 0xFFFFFFFF sentinel representation, zero-extended on every width; it is
    // never dereferenced as a node pointer.
    *reinterpret_cast<uintptr_t*>(freshBuckets + newBucketCount) = static_cast<uintptr_t>(0xFFFFFFFFu);
    // Old buckets are visited by ascending index; each pass pops the CURRENT head,
    // re-reading the published head until the bucket drains.
    for (uint32_t oldIndex = 0; oldIndex < map->bucketCount; ++oldIndex) {
        while (Node* node = map->buckets[oldIndex]) {
            // FNV-1 over UTF16 code units: multiply first, then XOR the unit; the hash
            // stops at the first zero (NUL) unit. Not FNV-1a, not bytes.
            const char16_t* cursor = node->key;
            uint32_t hash = 0x811C9DC5u;
            while (*cursor) {
                hash = (hash * 0x01000193u) ^ static_cast<uint16_t>(*cursor);
                ++cursor;
            }
            // EA's unsigned division: hash % newBucketCount. With any node present and
            // newBucketCount 0 this is the original's division by zero, kept unfixed.
            const uint32_t destination = hash % newBucketCount;
            Node* successor = node->next; // read before the link is republished
            map->buckets[oldIndex] = successor; // pop the exact node identity
            node->next = freshBuckets[destination]; // prepend to the destination head
            freshBuckets[destination] = node;
        }
    }
    // The old table is released only when it was separately allocated (count > 1); the
    // count 0/1 table is embedded in the map and must not reach the allocator. The
    // release still observes the OLD pointer and count: publication comes after it.
    if (map->bucketCount > 1u) {
        if (boundary) boundary->release(boundary->context, map->buckets);
        else {
#if defined(_M_IX86)
            ts3::CallGameFunction<void, 0x004E3F70>(map->buckets);
#else
            throw std::logic_error("bucket redistribution requires native release boundary");
#endif
        }
    }
    map->buckets = freshBuckets;
    map->bucketCount = newBucketCount;
    // EAX is newBucketCount on the freed path (oldCount > 1); the other paths carry no
    // original EAX contract, and this return makes no wider promise.
    return newBucketCount;
}
}

uint32_t RedistributeWide(PairedWideMapView* map, uint32_t newBucketCount,
                          const RedistributeBoundary* boundary) {
    return RedistributePairedBuckets<PairedWideMapView, PairedWideNodeView>(map, newBucketCount, boundary);
}

uint32_t RedistributeCompact(PairedCompactMapView* map, uint32_t newBucketCount,
                             const RedistributeBoundary* boundary) {
    return RedistributePairedBuckets<PairedCompactMapView, PairedCompactNodeView>(map, newBucketCount, boundary);
}
}

#include "ts3/resourcesystem/clu_455970_ResourceSystem.h"
namespace ts3::resource::m03::accounting {
namespace {
NotifyFilterPrefix notifyFilterPrefix = nullptr;

// The CURRENT table word is re-read from the receiver before every single call,
// exactly like the original's repeated [esi]/[edi+8] loads; no slot is ever cached.
void* const* NotifyTableOf(const void* self) {
    return *static_cast<void* const* const*>(self);
}
const char16_t* NotifyCallOperand(void* self, void* slot) {
#if defined(_M_IX86)
    return reinterpret_cast<const char16_t* (__fastcall*)(void*, void*)>(slot)(self, nullptr);
#else
    return reinterpret_cast<const char16_t* (*)(void*)>(slot)(self);
#endif
}
uint32_t NotifyCallEligible(void* self, void* slot) {
#if defined(_M_IX86)
    return reinterpret_cast<uint32_t (__fastcall*)(void*, void*)>(slot)(self, nullptr);
#else
    return reinterpret_cast<uint32_t (*)(void*)>(slot)(self);
#endif
}
void* NotifyCallResource(void* self, void* slot) {
#if defined(_M_IX86)
    return reinterpret_cast<void* (__fastcall*)(void*, void*)>(slot)(self, nullptr);
#else
    return reinterpret_cast<void* (*)(void*)>(slot)(self);
#endif
}
void NotifyCallCollect(void* self, void* slot, NotifyScratch* scratch, uint32_t zero) {
#if defined(_M_IX86)
    reinterpret_cast<void (__fastcall*)(void*, void*, NotifyScratch*, uint32_t)>(slot)(self, nullptr, scratch, zero);
#else
    reinterpret_cast<void (*)(void*, NotifyScratch*, uint32_t)>(slot)(self, scratch, zero);
#endif
}
void NotifyCallDeliver(void* self, void* slot, void* resource, NotifyScratch* scratch) {
#if defined(_M_IX86)
    reinterpret_cast<void (__fastcall*)(void*, void*, void*, NotifyScratch*)>(slot)(self, nullptr, resource, scratch);
#else
    reinterpret_cast<void (*)(void*, void*, NotifyScratch*)>(slot)(self, resource, scratch);
#endif
}
}

NotifyFilterPrefix SetNotifyFilterPrefix(NotifyFilterPrefix binding) {
    const auto previous = notifyFilterPrefix;
    notifyFilterPrefix = binding;
    return previous;
}

const void* NotifyScratchTag() {
#if defined(_M_IX86)
    return reinterpret_cast<const void*>(0x00FFE150); // "ResourceSystem/KeyList"
#else
    // The same table identity as a native constant; x86 carries the image word.
    static constexpr char kKeyListTag[] = "ResourceSystem/KeyList";
    return static_cast<const void*>(kKeyListTag);
#endif
}

void NotifyRetained(NotifyOwnerView* owner, const char16_t* filter, const NotifyLeaves* leaves) {
#if defined(_M_IX86)
    const auto& label = *reinterpret_cast<const ts3::thread::ThreadTime*>(0x00FFDFC4);
#else
    const ts3::thread::ThreadTime label = ts3::thread::kTimeoutNone;
#endif
    ts3::thread::Mutex& mutex = owner->mutex;
    if (leaves && leaves->lock) leaves->lock(leaves->context, mutex, label);
    else {
#if defined(_M_IX86)
        ts3::CallGameMethod<int, 0x004E16F0>(&mutex, &label);
#else
        mutex.Lock(label);
#endif
    }
    // Both cells are snapshotted AFTER the lock and BEFORE any external call; the
    // loop compares against the SAVED end, so callback edits of the owner cells are
    // never observed mid-pass (EA's saved-local compare, kept exactly).
    void** cursor = owner->retained;
    void* const* const last = owner->retainedEnd;
    if (cursor != last) {
        do {
            void* const object = *cursor; // the CURRENT slot, re-read every iteration
            bool eligible = true;
            if (filter) {
                const char16_t* operand = NotifyCallOperand(object, NotifyTableOf(object)[2]);
                uint32_t accepted;
                if (notifyFilterPrefix) accepted = notifyFilterPrefix(filter, operand, 5);
                else {
#if defined(_M_IX86)
                    accepted = ts3::resourcesystem::HasPathPrefix(filter, operand, 5) ? 1u : 0u;
#else
                    (void)operand;
                    throw std::logic_error("NotifyRetained requires a native path-prefix binding");
#endif
                }
                // Only the low byte of the comparison word is tested; dirty upper
                // result bits must neither match nor leak into the gate.
                eligible = (accepted & 0xFFu) != 0u;
            }
            if (eligible && (NotifyCallEligible(object, NotifyTableOf(object)[3]) & 0xFFu) != 0u) {
                // FULL returned resource pointer, from a freshly reloaded table.
                void* const resource = NotifyCallResource(object, NotifyTableOf(object)[1]);
                NotifyScratch scratch{}; // begin/end/capacity zeroed...
                scratch.tag = NotifyScratchTag(); // ...and the table identity word
                NotifyCallCollect(resource, NotifyTableOf(resource)[12], &scratch, 0);
                NotifyInterfaceView* const interface = &owner->interface;
                NotifyCallDeliver(interface, NotifyTableOf(interface)[19], resource, &scratch);
                // The FIRST word is re-read here, AFTER both callbacks: cleanup follows
                // the final field, never a pointer captured before the delivery call.
                if (scratch.begin) {
                    if (leaves && leaves->release) leaves->release(leaves->context, scratch.begin);
                    else {
#if defined(_M_IX86)
                        ts3::CallGameFunction<void, 0x004E3F70>(scratch.begin);
#else
                        throw std::logic_error("NotifyRetained requires a native release boundary");
#endif
                    }
                }
            }
            ++cursor;
        } while (cursor != last);
    }
    if (leaves && leaves->unlock) leaves->unlock(leaves->context, mutex);
    else {
#if defined(_M_IX86)
        ts3::CallGameMethod<int, 0x004E17B0>(&mutex);
#else
        mutex.Unlock();
#endif
    }
}
}

#include <algorithm>
#include <cmath>
namespace ts3::resource::m03::accounting {
namespace {
// EASTL's 256-entry prime table as compiled into TS3W.exe at 0x00FB8558 (the same
// table 0x004E09B0 searches through 0x6D8BD0); identical to the canonical copy in
// compressed_index.cpp, repeated here because that copy is translation-private.
const uint32_t kPairedPrimes[256] = {
    2u, 3u, 5u, 7u, 11u, 13u, 17u, 19u, 23u, 29u, 31u, 37u, 41u, 43u, 47u, 53u, 59u, 61u, 67u, 71u, 73u, 79u, 83u,
    89u, 97u, 103u, 109u, 113u, 127u, 137u, 139u, 149u, 157u, 167u, 179u, 193u, 199u, 211u, 227u, 241u, 257u, 277u,
    293u, 313u, 337u, 359u, 383u, 409u, 439u, 467u, 503u, 541u, 577u, 619u, 661u, 709u, 761u, 823u, 887u, 953u,
    1031u, 1109u, 1193u, 1289u, 1381u, 1493u, 1613u, 1741u, 1879u, 2029u, 2179u, 2357u, 2549u, 2753u, 2971u,
    3209u, 3469u, 3739u, 4027u, 4349u, 4703u, 5087u, 5503u, 5953u, 6427u, 6949u, 7517u, 8123u, 8783u, 9497u,
    10273u, 11113u, 12011u, 12983u, 14033u, 15173u, 16411u, 17749u, 19183u, 20753u, 22447u, 24281u, 26267u,
    28411u, 30727u, 33223u, 35933u, 38873u, 42043u, 45481u, 49201u, 53201u, 57557u, 62233u, 67307u, 72817u,
    78779u, 85229u, 92203u, 99733u, 107897u, 116731u, 126271u, 136607u, 147793u, 159871u, 172933u, 187091u,
    202409u, 218971u, 236897u, 256279u, 277261u, 299951u, 324503u, 351061u, 379787u, 410857u, 444487u,
    480881u, 520241u, 562841u, 608903u, 658753u, 712697u, 771049u, 834181u, 902483u, 976369u, 1056323u,
    1142821u, 1236397u, 1337629u, 1447153u, 1565659u, 1693859u, 1832561u, 1982627u, 2144977u, 2320627u,
    2510653u, 2716249u, 2938679u, 3179303u, 3439651u, 3721303u, 4026031u, 4355707u, 4712381u, 5098259u,
    5515729u, 5967347u, 6456007u, 6984629u, 7556579u, 8175383u, 8844859u, 9569143u, 10352717u, 11200489u,
    12117689u, 13109983u, 14183539u, 15345007u, 16601593u, 17961079u, 19431899u, 21023161u, 22744717u,
    24607243u, 26622317u, 28802401u, 31160981u, 33712729u, 36473443u, 39460231u, 42691603u, 46187573u,
    49969847u, 54061849u, 58488943u, 63278561u, 68460391u, 74066549u, 80131819u, 86693767u, 93793069u,
    101473717u, 109783337u, 118773397u, 128499677u, 139022417u, 150406843u, 162723577u, 176048909u,
    190465427u, 206062531u, 222936881u, 241193053u, 260944219u, 282312799u, 305431229u, 330442829u,
    357502601u, 386778277u, 418451333u, 452718089u, 489790921u, 529899637u, 573292817u, 620239453u,
    671030513u, 725980837u, 785430967u, 849749479u, 919334987u, 994618837u, 1076067617u, 1164186217u,
    1259520799u, 1362662261u, 1474249943u, 1594975441u, 1725587117u, 1866894511u, 2019773507u,
    2185171673u, 2364114217u, 2557710269u, 2767159799u, 2993761039u, 3238918481u, 3504151727u,
    3791104843u, 4101556399u, 4294967291u};
uint32_t PairedNextPrime(uint32_t atLeast) {
    const uint32_t* p = std::lower_bound(kPairedPrimes, kPairedPrimes + 256, atLeast);
    return p == kPairedPrimes + 256 ? kPairedPrimes[255] : *p;
}
}

PairedWideRehashDecision* PairedWideRehashRequired(PairedRehashPolicyView* policy,
                                                   PairedWideRehashDecision* decision,
                                                   uint32_t bucketCount, uint32_t elementCount,
                                                   uint32_t elementAdd) {
    const uint32_t wanted = elementCount + elementAdd;
    if (wanted > policy->nextResize) {
        const uint32_t counted = bucketCount == 1u ? 0u : bucketCount; // 0x004E09CD
        // 0x004E09D6..0x004E0A0A: minBuckets = wanted/maxLoadFactor, buckets = counted,
        // both as float compares. In the used integer-valued domain every product,
        // quotient and the 0xC00 ceil conversion are exact integers.
        const float minBuckets = static_cast<float>(wanted) / policy->maxLoadFactor;
        const float buckets = static_cast<float>(counted);
        if (buckets < minBuckets) {
            // 0x004E0A12..0x004E0A5E: grown = growthFactor*buckets (mulss), target =
            // the larger of grown and minBuckets, converted under the round-up control
                       // word; the prime is the table's first entry >= that count.
            const float grown = policy->growthFactor * buckets;
            const float target = grown <= minBuckets ? minBuckets : grown;
            // The original converts through the x87 stack and CRT ceil (double), then
            // fistp to int64 under the round-up control word; the double mirror is
            // exact for every integer-valued case this domain produces.
            const uint32_t prime = PairedNextPrime(
                static_cast<uint32_t>(static_cast<int64_t>(std::ceil(static_cast<double>(target)))));
            policy->nextResize = static_cast<uint32_t>(
                static_cast<int64_t>(std::ceil(static_cast<double>(prime) *
                                               static_cast<double>(policy->maxLoadFactor))));
            decision->grow = 1; // 0x004E0ABA/+4: byte flag then count
            decision->newBucketCount = prime;
        } else {
            // 0x004E0AD3: even a no-rehash answer republishes the threshold.
            policy->nextResize = static_cast<uint32_t>(
                static_cast<int64_t>(std::ceil(static_cast<double>(counted) *
                                               static_cast<double>(policy->maxLoadFactor))));
            decision->grow = 0;
            decision->newBucketCount = 0;
        }
    } else {
        // 0x004E09C3 early return: decision {0,0} and NO policy mutation at all.
        decision->grow = 0;
        decision->newBucketCount = 0;
    }
    return decision;
}

PairedWideInsertResult* InsertWide(PairedWideMapView* map, PairedWideInsertResult* out,
                                   const PairedWideInsertPair* pair, const void* /*unusedThird:
                                   the original never reads its third stack argument
                                   (sole caller 0x00737F38 pushes a zeroed byte; the
                                   body's only stack reads are arg1/arg2 cells)*/,
                                   const RedistributeBoundary* boundary) {
    // 0x7356DC..0x735700: the borrowed key pointer is saved FIRST and used for both
    // hashing and the identity compare; it stays valid across all callbacks below
    // while the pair words are re-read for the node copy after them.
    const char16_t* key = pair->key;
    uint32_t hash = 0x811C9DC5u;
    for (const char16_t* cursor = key; *cursor; ++cursor)
        hash = (hash * 0x01000193u) ^ static_cast<uint16_t>(*cursor); // FNV-1: multiply, then XOR
    // 0x735709 DIV: unsigned hash % bucketCount; EA's divide by zero with count 0 is
    // not repaired (positive count is the caller's precondition).
    const uint32_t bucket = hash % map->bucketCount;
    PairedWideNodeView** slot = map->buckets + bucket;
    for (PairedWideNodeView* node = *slot; node; node = node->next)
        if (node->key == key) { // 0x735724: POINTER identity, never string contents
            // 0x7357CF..0x7357E3: exact write order node, inserted byte, bucket slot.
            out->node = node;
            out->inserted = 0;
            out->bucketSlot = slot;
            return out; // full EAX = out; no policy call, allocation, release or count
        }
    // 0x735733..0x735747: the true growth decision on the embedded policy (ECX map+0x10)
    // with (decision record, bucket count, element count, 1). The original's local
    // decision record reuses the saved-key stack slot; that aliasing is not observable
    // here because the saved pointer is never needed again after the compare loop.
    PairedWideRehashDecision decision;
#if defined(_M_IX86)
    ts3::CallGameMethod<PairedWideRehashDecision*, 0x004E09B0>(&map->policy, &decision,
                                                               map->bucketCount, map->elementCount,
                                                               1u);
#else
    // No game image natively: the same bounded used policy, proven equal to the
    // original body by the differential grid in the exehost suite.
    PairedWideRehashRequired(&map->policy, &decision, map->bucketCount, map->elementCount, 1u);
#endif
    // 0x735747..0x735762: one 12-byte node with the exact compiled cdecl argument
    // list of 0x004E3F90: (12, map+0x1C identity, flags 0, zero 0, 0x00F98980, 0xE7).
    void* block;
    if (boundary) block = boundary->allocate(boundary->context, size_t(12), map->allocatorIdentity,
                                             0u, 0u, reinterpret_cast<const void*>(0x00F98980), 0xE7u);
    else {
#if defined(_M_IX86)
        block = ts3::CallGameFunction<void*, 0x004E3F90>(size_t(12), map->allocatorIdentity, 0u, 0u,
                                                         reinterpret_cast<const void*>(0x00F98980), 0xE7u);
#else
        // Native foreign ownership is never guessed: the boundary is required.
        throw std::logic_error("paired wide insert requires native allocation boundary");
#endif
    }
    auto* node = static_cast<PairedWideNodeView*>(block);
    if (node) {
        // 0x735769..0x735774: the CURRENT pair words are re-read only here, AFTER the
        // decision and allocation callbacks, so callback edits of the pair are honored.
        node->key = pair->key;
        node->payload = pair->payload;
    }
    // 0x735777: EA's unconditional next clear even after the failed-allocation test -
    // the original still dereferences a null block here; kept, not repaired.
    node->next = nullptr;
    uint32_t slotIndex = bucket;
    if (decision.grow != 0) {
        // 0x735785..0x735794: recompute hash % newCount BEFORE the rehash, then run
        // the REAL production 0x00734860 body (wave-676 RedistributeWide) with the SAME
        // map and boundary; it reallocates, redistributes and republishes the table.
        slotIndex = hash % decision.newBucketCount;
        RedistributeWide(map, decision.newBucketCount, boundary);
    }
    // 0x735799..0x7357B7: prepend to the head of the CURRENT (post-growth) bucket and
    // increment the element count once, with the table pointer RELOADED after the
    // possible rehash. uint32 wrap kept.
    PairedWideNodeView** current = map->buckets + slotIndex;
    node->next = *current;
    *current = node;
    ++map->elementCount;
    // 0x7357AF..0x7357C5: exact output write order node, inserted byte, bucket slot,
    // with the CURRENT table pointer loaded again for the slot address.
    out->node = node;
    out->inserted = 1;
    out->bucketSlot = map->buckets + slotIndex;
    return out; // full EAX = out
}
}
