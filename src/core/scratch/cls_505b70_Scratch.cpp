#include "ts3/scratch/cls_505b70_Scratch.h"

#include <atomic>

namespace ts3::scratch {

// 0x00505B70: AddRef on the interlocked refcount at +0x4C. The original does
// the original instruction sequence — wraps mod 2^32 and returns the
// new count, so the rewrite adds through uint32_t to stay defined.
int32_t UiTextBatch::AddRef()
{
    std::atomic_ref<uint32_t> count(refcount);
    return int32_t(count.fetch_add(1u, std::memory_order_relaxed) + 1u);
}

// 0x00506550: ctor. Writes the vtable and the four allocator tags, self-links the list
// head at +0x04, zeroes +0x10..+0x24 and the two vector regions, clears the refcount
// (the original uses _InterlockedExchange, a plain store is the same value), then sets
// the cursor to the list head. Returns this.
UiTextBatch* UiTextBatch::Construct()
{
    vtable = reinterpret_cast<void* const*>(uintptr_t(0x00FBA3DC));
    vertsTag = reinterpret_cast<const char*>(uintptr_t(0x00FBA3CC));
    headNext = reinterpret_cast<UiTextBatchNode*>(&headNext);
    headPrev = reinterpret_cast<UiTextBatchNode*>(&headNext);
    listSize = 0;
    listField14 = 0;
    cursor = nullptr;
    vertsBuffer = nullptr;
    partsEnd = 0;
    partsCapacity = 0;
    partsTag = reinterpret_cast<const char*>(uintptr_t(0x00FBA3BC));
    partsBegin = nullptr;
    partsEnd2 = 0;
    partsCapacity2 = 0;
    textSpansTag = reinterpret_cast<const char*>(uintptr_t(0x00FBA3A8));
    textSpansBegin = nullptr;
    textSpansEnd = 0;
    textSpansCapacity = 0;
    glyphInfosTag = reinterpret_cast<const char*>(uintptr_t(0x00FBA390));
    refcount = 0;
    cursor = reinterpret_cast<UiTextBatchNode*>(&headNext);
    return this;
}

// 0x00509890: dtor. Resets the vtable, runs the element teardown (0x00508BC0, same-class
// helper that is not in units.csv — bridged), frees textSpansBegin, partsBegin, vertsBuffer
// then every list node in head-first order through operator delete (0x004E3F70), sets the
// base vtable and returns the last free's eax (0 when nothing was freed). EA keeps the
// dangling pointers and list links in place — only the vtable changes. The last eax is
// unobservable in-game (the sole caller, 0x00509C40, ignores it) but is reproduced where
// the frees themselves ran.
int32_t UiTextBatch::Dtor()
{
    vtable = reinterpret_cast<void* const*>(uintptr_t(0x00FBA3DC));
    CallGameMethod<void, 0x00508BC0>(this);
    int32_t result = 0;
    if (textSpansBegin) result = CallGameFunction<int32_t, 0x004E3F70>(static_cast<void*>(textSpansBegin));
    if (partsBegin) result = CallGameFunction<int32_t, 0x004E3F70>(static_cast<void*>(partsBegin));
    // The original re-reads every field into eax, so a null later field discards an earlier
    // free's return: only the verts check (the last of the three, always executed) survives
    // into the node loop.
    result = vertsBuffer ? CallGameFunction<int32_t, 0x004E3F70>(static_cast<void*>(vertsBuffer)) : 0;
    const auto head = reinterpret_cast<UiTextBatchNode*>(&headNext);
    for (UiTextBatchNode* node = headNext; node != head;) {
        UiTextBatchNode* next = node->next;  // read before the free, like the original
        result = CallGameFunction<int32_t, 0x004E3F70>(static_cast<void*>(node));
        node = next;
    }
    vtable = reinterpret_cast<void* const*>(uintptr_t(0x00FA1B78));
    return result;
}

// 0x00509C40: scalar deleting dtor (ret 4). Runs the dtor, frees this when the flag byte's
// low bit is set, returns this.
UiTextBatch* UiTextBatch::DeletingDtor(uint32_t flags)
{
    Dtor();
    if (flags & 1u) CallGameFunction<int32_t, 0x004E3F70>(static_cast<void*>(this));
    return this;
}

}  // namespace ts3::scratch
