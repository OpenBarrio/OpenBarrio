// cls_4ad650_x.cpp - portable rewrite of the TS3W.exe I/O record read/write family
// (misc/, 25 entry points at 0x004AD650..0x00F20360, 2118 bytes).
//
// Every game convention is kept: the entry points are __thiscall with `this` in ECX and
// the original `ret N` stack cleanup (the C++ signatures below carry exactly the original
// stack arguments; on x86 the compiler emits the same ret N).  Calls into units that are
// not rewritten go through the bridge in ts3/layout.h; the memory-stream helper
// 0x004DEC90 is verified (see ts3/io/memory_stream.h) and called directly.
//
// Layout (constructor 0x004AE2E0): primary object PackedFileRecordWrite (vtable
// 0x00FB2AD4 at +0x00, read subobject vtable 0x00FB2A98 at +0x28, MemoryStream at +0x2C,
// embedded object at +0x58, byte at +0x280).  The read subobject entries take
// PackedFileRecordRead (primary + 0x28) as `this`; `this->field258` is primary + 0x280.
#include "ts3/misc/cls_4ad650_x.h"

#include <cstdint>

#include <intrin.h>

#include "ts3/io/memory_stream.h"
#include "ts3/layout.h"

namespace ts3 {
namespace misc {
namespace {

#if defined(_M_IX86)
#define TS3_THIS_CALL __thiscall
#else
#define TS3_THIS_CALL
#endif

// The game passes the address of the global at 0x00FB2CD0; the value keeps the timeout
// behavior while the exehost tests patch the mutex entry points.
constexpr uint32_t kTimeoutNone = 0xFFFFFFFFu;

// The interface id the read subobject reports from vtable slot 3 (0x004ADA10).
constexpr uint32_t kRecordClassId = 0x12E4A892u;

// Raw field helpers (the game objects are byte-accurate overlays, never C++ objects).
template <class T>
T& Field(void* base, size_t offset)
{
    return *reinterpret_cast<T*>(static_cast<uint8_t*>(base) + offset);
}

template <class T>
T Field(const void* base, size_t offset)
{
    return *reinterpret_cast<const T*>(static_cast<const uint8_t*>(base) + offset);
}

// The original reads pointers out of 32-bit argument dwords (a0/a1 in the manager
// machines); this keeps the same arithmetic without lying about the parameter type.
inline void* IntToPtr(uint32_t value)
{
    return reinterpret_cast<void*>(static_cast<uintptr_t>(value));
}

// Calls a game virtual by *byte* offset into the vtable, matching the original call sites
// exactly (they index [vtable + 0xNN], not by slot number).
template <class R, class... Args>
R VirtualCallOffset(void* object, uintptr_t byteOffset, Args... args)
{
#if defined(_M_IX86)
    using Method = R(TS3_THIS_CALL*)(void*, Args...);
    void* const* const* vtable = static_cast<void* const* const*>(object);
    const uint8_t* entry = reinterpret_cast<const uint8_t*>(*vtable) + byteOffset;
    return reinterpret_cast<Method>(*reinterpret_cast<Method const*>(entry))(object, args...);
#else
    (void)object;
    (void)byteOffset;
    ((void)args, ...);
    AbortNotRewritten("a game virtual method");
    return R{};
#endif
}

// The embedded object at +0x30 of the read subobject, probed through byte offset 0x10.
bool ActiveEmbedded(PackedFileRecordRead* self, void** active)
{
    void* candidate = reinterpret_cast<uint8_t*>(self) + 0x30;
    if (VirtualCallOffset<uint32_t>(candidate, 0x10) == 0) return false;
    *active = candidate;
    return true;
}

// The +0x04 embedded object is the fallback the original calls when the probe is zero.
void* FallbackEmbedded(PackedFileRecordRead* self)
{
    return reinterpret_cast<uint8_t*>(self) + 0x04;
}

}  // namespace

// ---------------------------------------------------------------------------
// 0x004AD650
// ---------------------------------------------------------------------------
int32_t PackedFileRecordClearResource(PackedFileRecordWrite* self)
{
    self->resourceState = 0;
    self->resource = nullptr;
    return 0;
}

// ---------------------------------------------------------------------------
// 0x004AD660
// ---------------------------------------------------------------------------
PackedFileRecordRead* PackedFileRecordReadView(PackedFileRecordWrite* self)
{
    if (self == nullptr) return nullptr;
    return reinterpret_cast<PackedFileRecordRead*>(reinterpret_cast<uint8_t*>(self) + 0x28);
}

// ---------------------------------------------------------------------------
// 0x004AD970: clears the resource fields and +0x280, then asks the +0x58 object's slot 4
// whether it is readable; if so runs slot 6, copies the 260-character file name through
// slot 17 and forwards it to the game function 0x004DBBD0 (cdecl) whose result is returned.
// ---------------------------------------------------------------------------
int32_t PackedFileRecordResetAndDiscard(PackedFileRecordWrite* self)
{
    uint8_t* const writer = reinterpret_cast<uint8_t*>(self) + 0x58;
    self->resourceState = 0;
    self->resource = nullptr;
    Field<uint8_t>(self, 0x280) = 0;
    if (VirtualCallOffset<uint32_t>(writer, 0x10) == 0) return 0;
    VirtualCallOffset<void>(writer, 0x18);
    wchar_t fileName[260];
    VirtualCallOffset<void>(writer, 0x44, fileName, 260);
    return CallGameFunction<int32_t, 0x004DBBD0>(fileName);
}

// ---------------------------------------------------------------------------
// 0x004AD9D0: returns object+0x58 when its slot 4 reports active, else object+0x2C.
// ---------------------------------------------------------------------------
void* PackedFileRecordActiveWriter(PackedFileRecordWrite* self)
{
    uint8_t* const candidate = reinterpret_cast<uint8_t*>(self) + 0x58;
    if (VirtualCallOffset<uint32_t>(candidate, 0x10) != 0) return candidate;
    return reinterpret_cast<uint8_t*>(self) + 0x2C;
}

// ---------------------------------------------------------------------------
// 0x004AD9F0
// The fallback sets only AL (mov al,1); the original leaves EAX's upper bits alone.
// ---------------------------------------------------------------------------
uint8_t PackedFileRecordReleaseResource(PackedFileRecordWrite* self)
{
    if (self->resourceState == 0) return 1;
    return static_cast<uint8_t>(VirtualCallOffset<uint32_t>(self->resource, 0x3C, self));
}

// ---------------------------------------------------------------------------
// 0x00F20360
// ---------------------------------------------------------------------------
void* PackedFileRecordResource(const PackedFileRecordWrite* self)
{
    return self->resource;
}

// ---------------------------------------------------------------------------
// 0x004AF100: returns `this` for 0x02269ED1 and 0xEE3F516E, else 0 (ret 4).
// ---------------------------------------------------------------------------
void* PackedFileRecordQueryInterfaceById(void* self, uint32_t id)
{
    if (id == 0x02269ED1u) return self;
    if (id == 0xEE3F516Eu) return self;
    return nullptr;
}

// ---------------------------------------------------------------------------
// 0x004ADA10
// ---------------------------------------------------------------------------
uint32_t PackedFileRecordReadClassId(PackedFileRecordRead* self)
{
    (void)self;
    return kRecordClassId;
}

// ---------------------------------------------------------------------------
// 0x004ADA20 / 0x004ADA50 / 0x004ADA90 / 0x004ADAD0 / 0x004ADB10 / 0x004ADB40 /
// 0x004ADB80: the same probe-and-forward shape for vtable slots 7..13 of the embedded
// +0x30 object (or of +0x04 when the probe reports inactive).  The original tail-jumps to
// the second target, so both paths return the callee's full EAX.
// ---------------------------------------------------------------------------
void* PackedFileRecordReadDispatch1C(PackedFileRecordRead* self)
{
    void* active = nullptr;
    void* target = ActiveEmbedded(self, &active) ? active : FallbackEmbedded(self);
    return VirtualCallOffset<void*>(target, 0x1C);
}

uint32_t PackedFileRecordReadDispatch20(PackedFileRecordRead* self, uint32_t a0)
{
    void* active = nullptr;
    void* target = ActiveEmbedded(self, &active) ? active : FallbackEmbedded(self);
    return VirtualCallOffset<uint32_t>(target, 0x20, a0);
}

uint32_t PackedFileRecordReadDispatch24(PackedFileRecordRead* self, uint32_t a0)
{
    void* active = nullptr;
    void* target = ActiveEmbedded(self, &active) ? active : FallbackEmbedded(self);
    return VirtualCallOffset<uint32_t>(target, 0x24, a0);
}

uint32_t PackedFileRecordReadDispatch28(PackedFileRecordRead* self, uint32_t a0, uint32_t a1)
{
    void* active = nullptr;
    void* target = ActiveEmbedded(self, &active) ? active : FallbackEmbedded(self);
    return VirtualCallOffset<uint32_t>(target, 0x28, a0, a1);
}

uint32_t PackedFileRecordReadDispatch2C(PackedFileRecordRead* self)
{
    void* active = nullptr;
    void* target = ActiveEmbedded(self, &active) ? active : FallbackEmbedded(self);
    return VirtualCallOffset<uint32_t>(target, 0x2C);
}

uint32_t PackedFileRecordReadDispatch30(PackedFileRecordRead* self, uint32_t a0, uint32_t a1)
{
    void* active = nullptr;
    void* target = ActiveEmbedded(self, &active) ? active : FallbackEmbedded(self);
    return VirtualCallOffset<uint32_t>(target, 0x30, a0, a1);
}

uint32_t PackedFileRecordReadDispatch34(PackedFileRecordRead* self)
{
    void* active = nullptr;
    void* target = ActiveEmbedded(self, &active) ? active : FallbackEmbedded(self);
    return VirtualCallOffset<uint32_t>(target, 0x34);
}

// ---------------------------------------------------------------------------
// 0x004ADBC0: the write/stream machine (ret 8).
//   fast: [this-4] != 0 and +0x30 slot 4 non-zero -> +0x30 slot 14 (a0, a1)
//   slow: (stream slot 7 + a1) >= 2,000,000 and +0x258 == 0 and 0x004AD810 accepts the
//         primary object -> reset the stream (0x004DEC90) with the resource's slot 19
//         result; when the state byte is still zero run 0x004AD920 on the primary and
//         return +0x30 slot 14 (a0, a1).  Only AL is defined on the intermediate path.
// ---------------------------------------------------------------------------
uint8_t PackedFileRecordReadDispatch38(PackedFileRecordRead* self, uint32_t a0, uint32_t a1)
{
    uint8_t* const base = reinterpret_cast<uint8_t*>(self);
    uint8_t* const active = base + 0x30;
    if (Field<int32_t>(self, -4) == 0) return 0;  // state clear: the original returns AL=0 at once
    if (VirtualCallOffset<uint32_t>(active, 0x10) != 0) {
        return static_cast<uint8_t>(VirtualCallOffset<uint32_t>(active, 0x38, a0, a1));
    }
    uint8_t* const stream = base + 0x04;
    if (static_cast<uint32_t>(VirtualCallOffset<int32_t>(stream, 0x1C)) + a1 < 0x1E8480u ||
        Field<uint8_t>(self, 0x258) != 0 ||
        CallGameMethod<uint8_t, 0x004AD810>(base - 0x28) == 0) {
        return static_cast<uint8_t>(VirtualCallOffset<uint32_t>(active, 0x38, a0, a1));
    }
    uint32_t key = 0;
    void* const streamField = Field<void*>(stream, 4);
    if (streamField != nullptr) key = Field<uint32_t>(streamField, 8);
    uint8_t state = static_cast<uint8_t>(
        VirtualCallOffset<uint32_t>(active, 0x38, key, VirtualCallOffset<int32_t>(stream, 0x1C)));
    if (state != 0) {
        state = static_cast<uint8_t>(VirtualCallOffset<uint32_t>(active, 0x38, a0, a1));
    }
    void* const resource = Field<void*>(self, -8);
    void* const allocator = VirtualCallOffset<void*>(resource, 0x4C);
    reinterpret_cast<io::MemoryStream*>(stream)->SetData(
        nullptr, 0, false, true, reinterpret_cast<allocator::ICoreAllocator*>(allocator));
    if (state == 0) {
        CallGameMethod<void, 0x004AD920>(base - 0x28);
        return static_cast<uint8_t>(VirtualCallOffset<uint32_t>(active, 0x38, a0, a1));
    }
    return state;  // the original returns SetData's EAX with AL replaced by the state byte
}

// ---------------------------------------------------------------------------
// 0x004AE440: deleting destructor (ret 4).  The game destructor 0x004AE3C0 runs first,
// then only the low byte of `flags` is tested before the free (0x004E4230, cdecl).
// ---------------------------------------------------------------------------
void* PackedFileRecordDelete(PackedFileRecordWrite* self, uint32_t flags)
{
    CallGameMethod<void, 0x004AE3C0>(self);
    if ((flags & 0x01) != 0) CallGameFunction<void, 0x004E4230>(self);
    return self;
}

// ---------------------------------------------------------------------------
// 0x004AE2D0
// ---------------------------------------------------------------------------
void* PackedFileRecordReadDelete(PackedFileRecordRead* self, uint32_t flags)
{
    return PackedFileRecordDelete(reinterpret_cast<PackedFileRecordWrite*>(reinterpret_cast<uint8_t*>(self) - 0x28),
                                  flags);
}

// ---------------------------------------------------------------------------
// 0x004B35A0: deleting destructor of the manager (ret 4); same shape as 0x004AE440 with
// the game destructor 0x004B30B0.
// ---------------------------------------------------------------------------
void* DpfManagerDelete(DpfManager* self, uint32_t flags)
{
    CallGameMethod<void, 0x004B30B0>(self);
    if ((flags & 0x01) != 0) CallGameFunction<void, 0x004E4230>(self);
    return self;
}

// ---------------------------------------------------------------------------
// 0x004B05F0: mutex-protected load machine (ret 0x18).  The original keeps the out
// pointer in the a3 argument's stack slot; a local keeps the same behavior without
// writing the caller's slot (the value is only read back inside this function).
// ---------------------------------------------------------------------------
uint8_t DpfManagerLoad(DpfManager* self, uint32_t a0, uint32_t a1, uint32_t a2,
                                     uint32_t a3, uint32_t a4, uint32_t a5)
{
    uint8_t* const mutex = reinterpret_cast<uint8_t*>(self) + 0x170;
    CallGameMethod<int32_t, 0x004E16F0>(mutex, &kTimeoutNone);
    uint8_t result = 0;
    const uint32_t factoryKey = a5 != 0 ? a5 : a0;
    void* const factory = VirtualCallOffset<void*>(self, 0x58, factoryKey);
    if (factory != nullptr) {
        result = static_cast<uint8_t>(VirtualCallOffset<uint32_t>(factory, 0x18, factoryKey, a1));
        if (result != 0) {
            CallGameMethod<int32_t, 0x004E17B0>(mutex);
            return 1;
        }
    }
    void* prepared = reinterpret_cast<void*>(static_cast<uintptr_t>(a3));
    if (prepared == nullptr) {
        prepared = VirtualCallOffset<void*>(self, 0x44, a0);
        if (prepared == nullptr) {
            CallGameMethod<int32_t, 0x004E17B0>(mutex);
            return result;
        }
    }
    void* committed = reinterpret_cast<void*>(static_cast<uintptr_t>(a4));
    if (committed == nullptr) {
        committed = VirtualCallOffset<void*>(self, 0x2C, Field<uint32_t>(IntToPtr(a0), 8), 0xFFFFFFFFu);
        if (committed == nullptr) {
            CallGameMethod<int32_t, 0x004E17B0>(mutex);
            return result;
        }
    }
    if (a1 == 0) {
        CallGameMethod<int32_t, 0x004E17B0>(mutex);
        return 1;
    }
    uint32_t out = 0;
    if (VirtualCallOffset<uint32_t>(prepared, 0x34, a0, &out, 1, 3, 1, 0) != 0) {
        result = static_cast<uint8_t>(
            VirtualCallOffset<uint32_t>(committed, 0x1C, out, a1, a2, Field<uint32_t>(IntToPtr(a0), 8)));
    }
    if (result != 0) {
        if (a5 != 0) {
            CallGameMethod<void, 0x004AF0E0>(Field<void*>(IntToPtr(a1), 0), a5);
        }
        VirtualCallOffset<void>(self, 0x5C, Field<void*>(IntToPtr(a1), 0), 1);
    }
    if (out != 0) {
        VirtualCallOffset<void>(IntToPtr(out), 0x08);
        CallGameMethod<int32_t, 0x004E17B0>(mutex);
        return result;
    }
    CallGameMethod<int32_t, 0x004E17B0>(mutex);
    return result;
}

// ---------------------------------------------------------------------------
// 0x004B0750: the same machine without the factory lookup, mutex and a5 (ret 0x14).
// ---------------------------------------------------------------------------
uint8_t DpfManagerAdd(DpfManager* self, uint32_t a0, uint32_t a1, uint32_t a2,
                                    uint32_t a3, uint32_t a4)
{
    uint8_t local = 0;
    void* prepared = reinterpret_cast<void*>(static_cast<uintptr_t>(a3));
    if (prepared == nullptr) {
        prepared = VirtualCallOffset<void*>(self, 0x44, a0);
        if (prepared == nullptr) return local;
    }
    void* committed = reinterpret_cast<void*>(static_cast<uintptr_t>(a4));
    if (committed == nullptr) {
        committed = VirtualCallOffset<void*>(self, 0x2C, Field<uint32_t>(IntToPtr(a0), 8), 0xFFFFFFFFu);
        if (committed == nullptr) return local;
    }
    uint32_t out = 0;
    if (VirtualCallOffset<uint32_t>(prepared, 0x34, a0, &out, 1, 3, 1, 0) != 0) {
        local = static_cast<uint8_t>(
            VirtualCallOffset<uint32_t>(committed, 0x24, out, a1, a2, Field<uint32_t>(IntToPtr(a0), 8)));
    } else {
        local = 0;
    }
    if (out != 0) VirtualCallOffset<void>(IntToPtr(out), 0x08);
    return local;
}

// ---------------------------------------------------------------------------
// 0x004B2770: factory-set teardown.  The factory set is a game container whose iterator
// starts at container+4 and ends at the container itself; nodes link through +0x1C, carry
// a vector of 8-byte key/value pairs at +4/+8, and values are at node+0x10.  The game
// helpers 0x006A1BE0 (insert), 0x006A0850 (erase by key), 0x004E0B20 (in-order successor)
// and 0x004E3F70 (free) do the work; the container layout is reproduced byte for byte.
// ---------------------------------------------------------------------------
uint8_t DpfManagerShutdown(DpfManager* self)
{
    uint8_t* const base = reinterpret_cast<uint8_t*>(self);
    if (self->flag04 == 0) return 1;

    // Release every +0xA0 vector entry whose +0x04 word is 1, then destroy the range.
    void** begin = *reinterpret_cast<void***>(base + 0xA0);
    void** end = *reinterpret_cast<void***>(base + 0xA4);
    self->flag04 = 0;
    for (void** it = begin; it != end; it += 2) {
        void* object = it[0];
        if (Field<uint32_t>(object, 4) == 1) VirtualCallOffset<void>(object, 0x08);
    }
    CallGameMethod<void, 0x004B0E10>(base + 0xA0, *reinterpret_cast<void**>(base + 0xA0),
                                     *reinterpret_cast<void**>(base + 0xA4));
    *reinterpret_cast<void**>(base + 0xA4) = *reinterpret_cast<void**>(base + 0xA0);

    // Collect the factory set from the +0x7C hash table into the embedded tree.
    alignas(8) uint8_t tree[0x28] = {};
    uint8_t* const container = tree + 0x0C;  // container fields +0x00 .. +0x14
    uint8_t* const treeThis = tree + 0x08;   // 0x006A1BE0/0x006A0850 take container - 4
    uint8_t* const insertOut = tree;         // the insert result lives at container - 0xC
    *reinterpret_cast<void**>(container + 0x00) = container;
    *reinterpret_cast<void**>(container + 0x04) = container;
    *reinterpret_cast<uint32_t*>(container + 0x08) = 0;
    *reinterpret_cast<uint8_t*>(container + 0x0C) = 0;
    *reinterpret_cast<uint32_t*>(container + 0x10) = 0;
    *reinterpret_cast<const char**>(container + 0x14) = "ResourceSystem/ResourceMan/Manager/factorySet";

    void** bucket = *reinterpret_cast<void***>(base + 0x7C);
    void* const sentinel = bucket[Field<uint32_t>(base, 0x80)];
    void** node = reinterpret_cast<void**>(bucket[0]);
    if (node == nullptr) {
        do {
            ++bucket;
        } while (*bucket == nullptr);
        node = reinterpret_cast<void**>(*bucket);
    }
    while (node != sentinel) {
        for (void** entry = reinterpret_cast<void**>(node[1]);
             entry != reinterpret_cast<void**>(node[2]); entry += 2) {
            int32_t key = static_cast<int32_t>(reinterpret_cast<uintptr_t>(entry[0]));
            CallGameMethod<void, 0x006A1BE0>(treeThis, insertOut, &key, 0);
        }
        node = reinterpret_cast<void**>(node[7]);  // +0x1C
        if (node == nullptr) {
            do {
                ++bucket;
                node = reinterpret_cast<void**>(*bucket);
            } while (node == nullptr);
        }
    }

    // Walk the set in order: value->slot 3 gates value->slot 5.
    for (void* iterator = *reinterpret_cast<void**>(container + 0x04); iterator != container;
         iterator = CallGameFunction<void*, 0x004E0B20>(iterator)) {
        void* value = Field<void*>(iterator, 0x10);
        if (VirtualCallOffset<uint32_t>(value, 12) != 0) VirtualCallOffset<void>(value, 20);
    }

    // Free the set's nodes through the game's erase/free helpers.
    for (void* item = *reinterpret_cast<void**>(container + 0x08); item != nullptr;) {
        void* const next = Field<void*>(item, 4);
        CallGameMethod<void, 0x006A0850>(treeThis, Field<void*>(item, 0));
        CallGameFunction<void, 0x004E3F70>(item);
        item = next;
    }

    // Reset the container and destroy the +0x78 container's range.
    *reinterpret_cast<void**>(container + 0x00) = container;
    *reinterpret_cast<void**>(container + 0x04) = container;
    *reinterpret_cast<uint32_t*>(container + 0x08) = 0;
    *reinterpret_cast<uint8_t*>(container + 0x0C) = 0;
    *reinterpret_cast<uint32_t*>(container + 0x10) = 0;
    CallGameMethod<void, 0x004B2260>(base + 0x78, *reinterpret_cast<void**>(base + 0x7C),
                                     *reinterpret_cast<void**>(base + 0x80));
    *reinterpret_cast<uint32_t*>(base + 0x84) = 0;

    // Release the +0x30 vector under the +0x48 mutex.
    CallGameMethod<int32_t, 0x004E16F0>(base + 0x48, &kTimeoutNone);
    void** vectorBegin = *reinterpret_cast<void***>(base + 0x30);
    void** vectorEnd = *reinterpret_cast<void***>(base + 0x34);
    for (void** it = vectorBegin; it != vectorEnd; it += 2) {
        void* object = it[0];
        if (VirtualCallOffset<uint32_t>(object, 0x10) != 0) VirtualCallOffset<void>(object, 0x08);
    }
    for (void** it = vectorBegin; it != vectorEnd; it += 2) {
        void* object = it[0];
        if (object != nullptr) VirtualCallOffset<void>(reinterpret_cast<uint8_t*>(object) + 4, 0x04);
    }
    *reinterpret_cast<void**>(base + 0x34) = *reinterpret_cast<void**>(base + 0x30);
    CallGameMethod<int32_t, 0x004E17B0>(base + 0x48);
    return 1;
}

// ---------------------------------------------------------------------------
// 0x004AFD90
// ---------------------------------------------------------------------------
uint8_t DpfManagerUnused0(DpfManager* self, uint32_t a0, uint32_t a1, uint32_t a2,
                                        uint32_t a3, uint32_t a4, uint32_t a5, uint32_t a6,
                                        uint32_t a7, uint32_t a8, uint32_t a9)
{
    (void)self;
    (void)a0;
    (void)a1;
    (void)a2;
    (void)a3;
    (void)a4;
    (void)a5;
    (void)a6;
    (void)a7;
    (void)a8;
    (void)a9;
    return 0;
}

// ---------------------------------------------------------------------------
// 0x00924180: lock xadd on +0x04, dec; on zero pin to 1 and call vtable slot 2 with 1.
// ---------------------------------------------------------------------------
int32_t RecordRunsRelease(RecordRuns* self)
{
    volatile long* const count = reinterpret_cast<volatile long*>(&self->referenceCount);
    const long previous = _InterlockedDecrement(count);
    if (previous == 0) {
        _InterlockedExchange(count, 1);
        if (self != nullptr) VirtualCallOffset<void>(self, 0x08, 1);
    }
    return static_cast<int32_t>(previous);
}

// ---------------------------------------------------------------------------
// 0x0053B170: installs the base interface vtable 0x00FA1B78, then frees through the
// cdecl 0x004E3F70 when bit 0 of `flags` is set (ret 4).
// ---------------------------------------------------------------------------
void* InterfaceFa1b78Destroy(void* self, uint32_t flags)
{
    *reinterpret_cast<uintptr_t*>(self) = 0x00FA1B78u;
    if ((flags & 0x01) != 0) CallGameFunction<void, 0x004E3F70>(self);
    return self;
}

}  // namespace misc
}  // namespace ts3