// Stage 3 m03 batch B02 implementations. Evidence: a disassembler 00733DA0:101,
// 00734000:47, 00734030:204, 007343B0:8 and IDA decompiles 20261019-07xxxx.
#include "ts3/resource/m03_B02.h"
#include "ts3/scratch/clu_4e3f70_Scratch.h" // Free4E3F70 (verified 0x004E3F70)
#include <bit>
#include <stdexcept>

#if defined(_WIN32)
#define WIN32_LEAN_AND_MEAN
#define NOMINMAX
#include <windows.h>
#endif

namespace ts3::resource::m03::system {
namespace {
// Game-image words the original stamps into the object (data identities, never called).
template <unsigned Address>
void* GameWord()
{
    return reinterpret_cast<void*>(static_cast<uintptr_t>(Address));
}
} // namespace

// TS3W.exe 0x007343B0: adjustor thunk +8 onto 0x0072C920 (the original instruction sequence),
// i.e. the identity word of the reader subobject at watcher +0x24. Full width.
void* WatcherIdentity(ts3::resource::m03::accounting::WatcherView* watcher)
{
    return watcher->reader.identity;
}

void* PoolDeletingDtor(ts3::resource::m03::accounting::PoolView* pool, uint32_t flags,
                       const PoolDestroyBoundary& boundary)
{
    *reinterpret_cast<void**>(pool) = GameWord<kPoolVtable>();
    if (boundary.deleteSection)
        boundary.deleteSection(boundary.context, pool->critical);
    else
        DeleteCriticalSection(reinterpret_cast<CRITICAL_SECTION*>(pool->critical));
    *reinterpret_cast<void**>(pool) = GameWord<kPoolBaseVtable>();
    // the original instruction sequence: only flag bit 0 decides the free.
    if (flags & 1u) {
        if (boundary.freeBlock)
            boundary.freeBlock(boundary.context, boundary.allocatorIdentity, pool);
        else {
#if defined(_M_IX86)
            ts3::scratch::Free4E3F70(pool);
#else
            throw std::logic_error("PoolDeletingDtor requires a native free boundary");
#endif
        }
    }
    return pool;
}

void* AllocatePool(ts3::resource::m03::accounting::PoolView* pool, uint32_t a0, void* unused1,
                   void* a2, uint32_t a3, void* a4, const PoolAllocBoundary& boundary)
{
    (void)unused1; // the original body never reads its second stack argument
    const auto external = [&]() -> void* {
        if (boundary.allocate)
            return boundary.allocate(boundary.context, boundary.allocatorIdentity, a0, a3, a4, a2);
#if defined(_M_IX86)
        void* allocator = *reinterpret_cast<void**>(0x011CB864);
        return ts3::CallGameMethod<void*, 0x004E6A20>(allocator, a0, a3, a4, a2);
#else
        throw std::logic_error("AllocatePool requires a native allocator boundary");
#endif
    };
    // Unsigned range guards (`ja`): exactly 0x80 and 4 are still in range.
    if (a0 > 0x80u || a3 > 4u)
        return external();
    if (boundary.enter)
        boundary.enter(boundary.context, pool->critical);
    else
        EnterCriticalSection(reinterpret_cast<CRITICAL_SECTION*>(pool->critical));
    size_t word = 0;
    while (word < 128 && pool->bitmap[word] == 0)
        ++word;
    if (word == 128) {
        if (boundary.leave)
            boundary.leave(boundary.context, pool->critical);
        else
            LeaveCriticalSection(reinterpret_cast<CRITICAL_SECTION*>(pool->critical));
        return external();
    }
    // Isolate the lowest set bit; EA's shift chain yields exactly its bit index.
    const uint32_t bit = pool->bitmap[word] & (0u - pool->bitmap[word]);
    const auto bitIndex = static_cast<unsigned>(std::countr_zero(bit));
    pool->bitmap[word] &= ~bit;
    if (boundary.leave)
        boundary.leave(boundary.context, pool->critical);
    else
        LeaveCriticalSection(reinterpret_cast<CRITICAL_SECTION*>(pool->critical));
    return pool->slots + ((static_cast<uintptr_t>(word) * 32 + bitIndex) * 128);
}

void* DispatchFiltered(DispatchOwnerView* owner, const ResourceKey* key, void* pass1,
                       DispatchTargetView* target, void* pass3, void* pass4,
                       const DispatchBoundary& boundary)
{
    // 0x007D80E0 normalized-key construction into the 16-byte stack temp.
    ResourceKey temp;
    if (boundary.resolveKey)
        boundary.resolveKey(boundary.context, &temp, key);
    else {
#if defined(_M_IX86)
        ts3::CallGameFunction<ResourceKey*, 0x007D80E0>(&temp, key);
#else
        throw std::logic_error("DispatchFiltered requires a native key-resolution boundary");
#endif
    }
    if (target) {
        const auto deliver = reinterpret_cast<DispatchDeliverSlot>(
            static_cast<void* const*>(target->table)[13]);
        return deliver(target, &temp, pass1, pass3, pass4, 1, 0);
    }
    DispatchInterfaceView& interface = owner->interface;
    const auto lookup =
        reinterpret_cast<DispatchLookupSlot>(static_cast<void* const*>(interface.table)[17]);
    auto* found = static_cast<DispatchTargetView*>(lookup(&interface, &temp));
    if (!found)
        // the original instruction sequence clears only the low byte of the lookup result (which is zero here).
        return reinterpret_cast<void*>(reinterpret_cast<uintptr_t>(found) & ~uintptr_t(0xFF));
    // The lookup slot (real 0x004AFDA0) is thiscall ret 4, so the stack is balanced here
    // exactly as on the direct path: both push (&temp, pass1, pass3, pass4, 1, 0).
    const auto deliver =
        reinterpret_cast<DispatchDeliverSlot>(static_cast<void* const*>(found->table)[13]);
    return deliver(found, &temp, pass1, pass3, pass4, 1, 0);
}

#if defined(_M_IX86)
void* __fastcall HookDispatch(DispatchOwnerView* self, void*, const ResourceKey* key,
                             void* a1, DispatchTargetView* target, void* a3, void* a4)
{ return DispatchFiltered(self, key, a1, target, a3, a4); }
void* __fastcall HookPoolDtor(accounting::PoolView* self, void*, uint32_t flags)
{ return PoolDeletingDtor(self, flags); }
void* __fastcall HookAllocate(accounting::PoolView* self, void*, uint32_t a0, void* a1,
                             void* a2, uint32_t a3, void* a4)
{ return AllocatePool(self, a0, a1, a2, a3, a4); }
#endif

} // namespace ts3::resource::m03::system
