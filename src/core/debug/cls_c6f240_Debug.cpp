// Rewrites the three functions of unit cls_c6f240_Debug (see the header for the layout, the
// call edges, and why the fields are reached by byte offset). Differential coverage:
// src/exehost/cls_c6f240_Debug_test.cpp plus the autotest row for Initialize.
#include "ts3/debug/cls_c6f240_Debug.h"

#include <cstddef>
#include <cstdint>
#include <cstring>

namespace ts3 { namespace debug {
namespace {

// The object is a raw 68-byte block - the callers hand over stack memory, not a constructed
// C++ object - so fields are read and written as bytes instead of through a struct pointer.
template <class T>
T Load(const void* object, size_t offset)
{
    T value;
    std::memcpy(&value, static_cast<const uint8_t*>(object) + offset, sizeof value);
    return value;
}

template <class T>
void Store(void* object, size_t offset, T value)
{
    std::memcpy(static_cast<uint8_t*>(object) + offset, &value, sizeof value);
}

uint32_t* BucketArray(const void* object, size_t offset)
{
    return reinterpret_cast<uint32_t*>(
        static_cast<uintptr_t>(Load<uint32_t>(object, offset)));
}

using L = CopyKeyFilterLayout;

// The literals the original constructor stores, read off the disassembly of 0x00C6FB60: the
// class vtable 0x01079C70, the shared static bucket array at 0x0114C900 (which holds
// {0, 0xFFFFFFFF, 0xFFFFFFFF, 0xFFFFFFFE}), and the name string at 0x01079C5C.
constexpr uint32_t kVtable = 0x01079C70;
constexpr uint32_t kStaticBuckets = 0x0114C900;
constexpr uint32_t kFilterName = 0x01079C5C;

}  // namespace

// TS3W.exe 0x00C6FB60 (__thiscall, no stack args, plain ret). Writes the object from
// constants and returns `this`. The order follows the original's stores; +0x04 and +0x24 are
// deliberately left alone, exactly as the original leaves them. Idempotent by construction,
// which is what makes the compare hook safe to run both sides on one live object.
CopyKeyFilter* CopyKeyFilter::Initialize()
{
    void* const self = this;
    Store(self, offsetof(L, vtable), kVtable);
    Store(self, offsetof(L, lo), 1.0f);   // the original movss-loads 1.0 from 0x0107A538
    Store(self, offsetof(L, hi), 2.0f);   // ... and 2.0 from 0x00F98A58
    Store(self, offsetof(L, name), kFilterName);
    Store(self, offsetof(L, count), 1u);
    Store(self, offsetof(L, cleared), 0u);
    Store(self, offsetof(L, pad1C), 0u);
    Store(self, offsetof(L, buckets), kStaticBuckets);
    Store(self, offsetof(L, lo2), 1.0f);
    Store(self, offsetof(L, hi2), 2.0f);
    Store(self, offsetof(L, name2), kFilterName);
    Store(self, offsetof(L, count2), 1u);
    Store(self, offsetof(L, buckets2), kStaticBuckets);
    Store(self, offsetof(L, cleared2), 0u);
    Store(self, offsetof(L, pad3C), 0u);
    return this;
}

// TS3W.exe 0x00C6F240 (__thiscall, ret 4, vtable slot 1): "is this key copied?".
//
// The two maps are the subobjects at this+0x04 and this+0x24, so the helpers see their bucket
// array / element index as [sub+4] / [sub+8] - the object's +0x08/+0x0C and +0x28/+0x2C. Each
// find helper (0x00592EC0 for map1, 0x0072DAD0 for map2) fills the caller's 8-byte scratch
// buffer and returns its address; the node it wrote is compared with buckets[index], the
// table's end sentinel, so "found" means "not the sentinel". map1 is keyed by the dword at
// key+8, map2 by the key itself.
//
// EA leaves the upper 24 bits of EAX holding the scratch-buffer address on both return paths
// (the original instruction sequence / the original instruction sequence), so only AL is part of the contract; a C++ bool return is exactly
// that contract.
bool CopyKeyFilter::IsFiltered(const void* key)
{
#if defined(_M_IX86)
    uint8_t* const self = reinterpret_cast<uint8_t*>(this);
    uint32_t scratch[2];  // the original's uninitialized 8-byte slot; each helper writes its
                          // first dword before the caller reads it

    const uint32_t sentinel1 = BucketArray(self, offsetof(L, buckets))[Load<uint32_t>(self,
                                                                      offsetof(L, count))];
    void* const found1 = ts3::CallGameMethod<void*, 0x00592EC0>(
        self + offsetof(L, mapSlot), scratch, static_cast<const uint8_t*>(key) + 8);
    if (Load<uint32_t>(found1, 0) != sentinel1)
        return true;

    const uint32_t sentinel2 = BucketArray(self, offsetof(L, buckets2))[Load<uint32_t>(self,
                                                                       offsetof(L, count2))];
    void* const found2 = ts3::CallGameMethod<void*, 0x0072DAD0>(
        self + offsetof(L, map2Slot), scratch, key);
    return Load<uint32_t>(found2, 0) != sentinel2;
#else
    (void)key;
    ts3::NotRewritten("cls_c6f240_Debug IsFiltered (walks 32-bit game bucket addresses)");
#endif
}

// TS3W.exe 0x00C6FBD0 (__thiscall, ret 4, vtable slot 0): the scalar deleting destructor.
// The C1 destructor (0x00AA9550, another unit) tears down both maps, and the raw memory is
// released through the game's free (0x004E3F70, unit clu_4e3f70_Scratch) only when bit 0 of
// the flag is set - the original tests just that bit (the original instruction sequence). Returns the
// pointer it was given.
CopyKeyFilter* CopyKeyFilter::Destroy(uint32_t deleteFlags)
{
    CopyKeyFilter* const self = this;
    ts3::CallGameMethod<void, 0x00AA9550>(self);
    if ((deleteFlags & 1u) != 0u)
        ts3::CallGameFunction<void, 0x004E3F70>(self);
    return self;
}

} }  // namespace ts3::debug
