// Reimplementation of CAS class unit cls_4a6d60_CAS (TS3W.exe 0x004A6D60..0x0072C610).
// Layout and ABI notes are in the header; the differential test lives in
// src/exehost/cls_4a6d60_CAS_batch_test.cpp.
//
// Every function is written as the original: same offsets, same order of writes, same
// bridge convention for the dependencies that are not rewritten yet (x86 only - in x64
// builds the game-facing calls report themselves instead). The two functions that belong
// to the rewritten packed-file pilot (0x004A6D60/0x004A6DC0) call EnsureOpen (0x004A6860)
// directly, because packed_file.h declares and packed_file.cpp implements it.
#include "ts3/cas/cls_4a6d60_CAS.h"

#include <cstring>

namespace ts3::cas {
namespace {

// Byte-addressed field access: every offset below is the original's.
template <class T>
T& At(void* base, size_t offset)
{
    return *reinterpret_cast<T*>(reinterpret_cast<uint8_t*>(base) + offset);
}

// A pointer as the 32-bit game keeps it (the x64 build truncates, but it has no game).
uint32_t PtrValue(const void* pointer)
{
    return static_cast<uint32_t>(reinterpret_cast<uintptr_t>(pointer));
}

// Release through the interface convention of these objects: the vtable pointer lives at
// `p + 4` and slot 1 is the release.
uint32_t ReleaseInterface(void* p)
{
#if defined(_M_IX86)
    return CallGameVirtual<uint32_t, 1>(reinterpret_cast<void*>(reinterpret_cast<uint8_t*>(p) + 4));
#else
    (void)p;
    return 0;
#endif
}

#if defined(_M_IX86)
constexpr uintptr_t kCrcTableAddress = 0x0114D330;

// 0x00C0FD60 is a one-byte script bridge stub in the image (TS3W.exe has it as a plain
// return), called with -1 when the user-version crc leaves a non-zero remainder mod 7.
void ReportUserVersionMismatch()
{
    CallGameFunction<void, 0x00C0FD60>(-1);
}
#else
// The game's table is not loaded in a 64-bit host, so build the same table (poly
// 0x04C11DB7, MSB-first) the image holds at 0x0114D330.
const uint32_t* BuildCrcTable()
{
    static uint32_t table[256];
    static bool ready = false;
    if (!ready) {
        for (uint32_t i = 0; i != 256; ++i) {
            uint32_t value = i << 24;
            for (int bit = 0; bit != 8; ++bit)
                value = (value & 0x80000000u) ? (value << 1) ^ 0x04C11DB7u : (value << 1);
            table[i] = value;
        }
        ready = true;
    }
    return table;
}
#endif

const uint32_t* CrcTable()
{
#if defined(_M_IX86)
    return reinterpret_cast<const uint32_t*>(kCrcTableAddress);
#else
    return BuildCrcTable();
#endif
}

// 0x00FB24F8 holds the game's infinite ThreadTime (0xFFFFFFFF).
void LockRecordMutex(resource::DatabasePackedFile& file)
{
#if defined(_M_IX86)
    const uint32_t infinite = 0xFFFFFFFFu;
    CallGameMethod<int, 0x004E16F0>(&file.recordMutex, &infinite);
#else
    (void)file;
    AbortNotRewritten("the DBPF record mutex");
#endif
}

void UnlockRecordMutex(resource::DatabasePackedFile& file)
{
#if defined(_M_IX86)
    CallGameMethod<int, 0x004E17B0>(&file.recordMutex);
#else
    (void)file;
    AbortNotRewritten("the DBPF record mutex");
#endif
}

}  // namespace

// TS3W.exe 0x004FA4C0, __cdecl. A negative length never enters the loop (the original
// compares the cursor against data + length as unsigned pointers).
uint32_t Crc32(const uint8_t* data, int32_t length, uint32_t crc, uint8_t invert)
{
    const uint32_t* const table = CrcTable();
    const uint8_t* const end = data + length;
    for (const uint8_t* cursor = data; cursor < end; ++cursor)
        crc = table[static_cast<uint32_t>(*cursor) ^ (crc >> 24)] ^ (crc << 8);
    return invert ? ~crc : crc;
}

// TS3W.exe 0x004A6D60, thiscall ret 8.
uint8_t DbpfUserVersionView::GetUserVersion(uint32_t* major, uint32_t* minor)
{
    auto& file = *reinterpret_cast<resource::DatabasePackedFile*>(this);
    LockRecordMutex(file);
    const bool have = file.openFlags != 0 || file.EnsureOpen();
    if (have) {
        *major = file.userVersionMajor;
        *minor = file.userVersionMinor;
    }
    UnlockRecordMutex(file);
    return have ? 1 : 0;
}

// TS3W.exe 0x004A6DC0, thiscall ret 8. The original stores minor first, then major.
void DbpfUserVersionView::SetNewUserVersion(uint32_t major, uint32_t minor)
{
    auto& file = *reinterpret_cast<resource::DatabasePackedFile*>(this);
    LockRecordMutex(file);
    file.newUserVersionMinor = minor;
    file.newUserVersionMajor = major;
    file.headerDirty = true;
    UnlockRecordMutex(file);
}

// TS3W.exe 0x0072C580, __stdcall ret 0xC (the file is the first stack argument).
uint8_t __stdcall VerifyUserVersionCrc(DbpfUserVersionView* file, const uint8_t* data, uint32_t length)
{
    uint32_t major = 0;
    uint32_t minor = 0;
    if (!file->GetUserVersion(&major, &minor))
        return 0;
    const uint32_t crc = Crc32(data, static_cast<int32_t>(length), 0xFFFFFFFFu, 0);
    // The user version is the pair (major, minor) where minor is the inverted crc of the
    // major value; 0x0072C610 writes exactly that pair (see below).
    const uint32_t check = Crc32(reinterpret_cast<const uint8_t*>(&major), 4, crc, 1);
    if (check != minor)
        return 0;
    if (check % 7 != 0) {
#if defined(_M_IX86)
        ReportUserVersionMismatch();
#endif
    }
    return 1;
}

// TS3W.exe 0x0072C600, thiscall ret 0.
DNameNode* DNameNode::Construct()
{
    vtable = reinterpret_cast<void*>(static_cast<uintptr_t>(0x00FE2594u));
    field04 = 0;
    return this;
}

// TS3W.exe 0x0072C610, thiscall ret 0xC (the node arrives in ecx).
uint8_t RefreshUserVersionFromNode(DNameNode* node, DbpfUserVersionView* file, const uint8_t* data, uint32_t length)
{
    const uint32_t crc = Crc32(data, static_cast<int32_t>(length), 0xFFFFFFFFu, 0);
#if defined(_M_IX86)
    // The image's import slot for QueryPerformanceCounter holds a function pointer; the
    // game reads the low dword of the counter it fills in.
    using QueryPerformanceCounterFn = int(__stdcall*)(int64_t*);
    const auto query = *reinterpret_cast<QueryPerformanceCounterFn*>(static_cast<uintptr_t>(0x00F953B8));
    int64_t counter = 0;
    query(&counter);
    uint32_t value = static_cast<uint32_t>(counter);
    const uint32_t wanted = node != nullptr ? node->field04 : 0;
    uint32_t check = 0;
    do {
        ++value;
        check = Crc32(reinterpret_cast<const uint8_t*>(&value), 4, crc, 1);
    } while (check % 7 != wanted);
    file->SetNewUserVersion(value, check);
#else
    (void)node;
    (void)file;
    (void)crc;
    AbortNotRewritten("the user-version counter");
#endif
    return 1;
}

// TS3W.exe 0x005BBB70, thiscall ret 0xC.
CompositedTextureMediator* CompositedTextureMediator::Construct(void* memory, int32_t arg1, int32_t arg2, int32_t arg3)
{
    auto* self = reinterpret_cast<CompositedTextureMediator*>(memory);
    self->vtable = reinterpret_cast<void*>(static_cast<uintptr_t>(0x00FE2580u));
    self->field04 = 0;
    self->node.Construct();
    self->node.vtable = reinterpret_cast<void*>(static_cast<uintptr_t>(0x00FE2594u));
    self->name = reinterpret_cast<const char*>(static_cast<uintptr_t>(0x00FE2540u));  // CAS/CompositorCache/MRUList
    // The list anchor at +0x10 (the node's +0x08) points at itself.
    self->node.field08 = reinterpret_cast<uintptr_t>(&self->node.field08);
    self->list14 = self->node.field08;
    self->key = arg1;  // cdq: the 64-bit field holds the sign-extended value
    self->magic28 = 0x04D82D90u;
    self->field2C = 0;
    self->field30 = nullptr;
    self->field34 = 0;
    self->field38 = arg3;
    self->field3C = arg2;
    self->field40 = arg1;
#if defined(_M_IX86)
    if (CallGameFunction<uint8_t, 0x0058B100>("UnboundedTCC", static_cast<void*>(nullptr)) != 0)
        self->field38 = -1;
#endif
    return self;
}

// TS3W.exe 0x005BB960, thiscall ret 4 (a tail call into 0x005BB770).
uint32_t CompositedTextureMediator::ReleaseKey()
{
#if defined(_M_IX86)
    return CallGameMethod<uint32_t, 0x005BB770>(field30);
#else
    AbortNotRewritten("the compositor-cache key release");
    return 0;
#endif
}

// TS3W.exe 0x005BBF60, thiscall ret 0.
uint32_t CompositedTextureMediator::Destroy()
{
    uint32_t result = 0;
    vtable = reinterpret_cast<void*>(static_cast<uintptr_t>(0x00FE2580u));
    node.vtable = reinterpret_cast<void*>(static_cast<uintptr_t>(0x00FE2594u));
#if defined(_M_IX86)
    CallGameMethod<void, 0x005BBC10>(this);  // drops the reference the cache holds
    if (field30 != nullptr)
        result = ReleaseInterface(field30);
    void** const anchor = reinterpret_cast<void**>(&node.field08);
    for (void* item = *anchor; item != anchor;) {
        void* const doomed = item;
        item = *reinterpret_cast<void**>(item);
        result = CallGameFunction<uint32_t, 0x004E3F70>(doomed);
    }
#else
    (void)result;
    AbortNotRewritten("the compositor mediator destructor");
#endif
    vtable = reinterpret_cast<void*>(static_cast<uintptr_t>(0x00FA1B78u));
    return result;
}

// TS3W.exe 0x005BC490, thiscall ret 4.
CompositedTextureMediator* CompositedTextureMediator::DeleteAndDestroy(int flags)
{
    Destroy();
    if ((flags & 1) != 0) {
#if defined(_M_IX86)
        CallGameFunction<void, 0x004E3F70>(this);
#endif
    }
    return this;
}

// TS3W.exe 0x006CE900, thiscall ret 4: ecx is the +4 base sub-object of the compositor.
MaterialTextureCompositor* MaterialTextureCompositorBase4::Delete(int flags)
{
    auto* self = reinterpret_cast<MaterialTextureCompositor*>(reinterpret_cast<uint8_t*>(this) - 4);
    return self->DeleteAndDestroy(flags);
}

// TS3W.exe 0x006CBDB0, thiscall ret 4.
MaterialTextureCompositor* MaterialTextureCompositor::DeleteBase(int flags)
{
    vtable2 = reinterpret_cast<void*>(static_cast<uintptr_t>(0x00FF5B44u));
    if ((flags & 1) != 0) {
#if defined(_M_IX86)
        CallGameFunction<void, 0x004E3F70>(this);
#endif
    }
    return this;
}

// TS3W.exe 0x006CEB20, thiscall ret 0.
MaterialTextureCompositor* MaterialTextureCompositor::Construct()
{
    vtable2 = reinterpret_cast<void*>(static_cast<uintptr_t>(0x00FF5B44u));
    vtable = reinterpret_cast<void*>(static_cast<uintptr_t>(0x00FF5E50u));
    vtable2 = reinterpret_cast<void*>(static_cast<uintptr_t>(0x00FF5E48u));
    map.weight10 = 1.0f;   // 0x0107A538
    map.weight14 = 2.0f;   // 0x00F98A58
    map.name = reinterpret_cast<const char*>(static_cast<uintptr_t>(0x00FE2540u));  // CAS/MaterialTextureCompositor/mCacheChecks
    map.bucketCount = 1;
    map.buckets = reinterpret_cast<CacheMapEntry**>(static_cast<uintptr_t>(0x0114C900u));  // the empty array
    map.count0C = 0;
    map.field18 = 0;
    field30 = PtrValue("CAS/MaterialTextureCompositor/RequestsCompletedNotifiers");
    // The map's +0x20/+0x24 links are this object's +0x28 list head.
    const uint32_t head28 = PtrValue(reinterpret_cast<uint8_t*>(this) + 0x28);
    map.field20 = head28;
    map.field24 = head28;
    list3C[0] = reinterpret_cast<uintptr_t>(list3C);
    list3C[1] = reinterpret_cast<uintptr_t>(list3C);
    mediator = nullptr;
    resource = nullptr;
    list44[0] = reinterpret_cast<uintptr_t>(list44);
    list44[1] = reinterpret_cast<uintptr_t>(list44);
    list6C[0] = reinterpret_cast<uintptr_t>(list6C);
    list6C[1] = reinterpret_cast<uintptr_t>(list6C);
    std::memset(unknown4C, 0, sizeof unknown4C);
    field298 = -1;
    At<uint32_t>(this, 0x29C) = 0;
    std::memset(reinterpret_cast<uint8_t*>(this) + 0x2A4, 0, 0x824);
    fieldACC = 0;
    fieldAC8 = -1;
    Rebuild(1, nullptr, nullptr);
    return this;
}

// TS3W.exe 0x006CE7A0, thiscall ret 0. The +0x3C list is spliced and drained; its nodes
// (allocated elsewhere) carry a reference-counted payload at node + 0x20.
uint32_t MaterialTextureCompositor::DestroyCore()
{
    uint32_t result = 0;
    vtable = reinterpret_cast<void*>(static_cast<uintptr_t>(0x00FF5E50u));
    vtable2 = reinterpret_cast<void*>(static_cast<uintptr_t>(0x00FF5E48u));
    std::memset(reinterpret_cast<uint8_t*>(this) + 0x2A4, 0, 0x824);
    At<uint32_t>(this, 0xACC) = 0;
#if defined(_M_IX86)
    {
        uint8_t* const bytes = reinterpret_cast<uint8_t*>(this);
        auto* const list = reinterpret_cast<void**>(bytes + 0x3C);
        auto* const tailAt = reinterpret_cast<void**>(bytes + 0x44);
        if (*tailAt != tailAt) {
            auto* head = reinterpret_cast<void**>(*reinterpret_cast<void**>(bytes + 0x40));
            auto* back = reinterpret_cast<void**>(*reinterpret_cast<void**>(bytes + 0x48));
            *head = *tailAt;
            reinterpret_cast<void**>(*tailAt)[1] = head;
            *back = list;
            *reinterpret_cast<void**>(bytes + 0x40) = back;
            *tailAt = tailAt;
            *reinterpret_cast<void**>(bytes + 0x48) = tailAt;
        }
        while (*reinterpret_cast<void**>(bytes + 0x40) != static_cast<void*>(list)) {
            auto* node = static_cast<uint8_t*>(*reinterpret_cast<void**>(bytes + 0x40));
            auto* base = node != nullptr ? node - 24 : nullptr;
            *reinterpret_cast<void**>(node + 4) = list;
            *reinterpret_cast<void**>(bytes + 0x40) = *reinterpret_cast<void**>(node + 4);
            auto* payload = base != nullptr ? reinterpret_cast<uint32_t*>(*reinterpret_cast<void**>(base + 32)) : nullptr;
            if (base != nullptr)
                *reinterpret_cast<void**>(base + 32) = nullptr;
            if (payload != nullptr) {
                uint32_t* const counter = payload + 1;  // the vtable sits at payload + 4
                const uint32_t left = counter[1] - 1;
                counter[1] = left;
                if (left == 0) {
                    counter[1] = 1;
                    CallGameVirtual<void, 0>(counter, 1);
                }
            }
            if (base != nullptr) {
                // EA re-reads the payload here; it is always null after the store above.
                auto* counter = reinterpret_cast<uint32_t*>(*reinterpret_cast<void**>(base + 32));
                if (counter != nullptr) {
                    const uint32_t left = counter[2] - 1;
                    counter[1] = left;
                    if (left == 0) {
                        counter[1] = 1;
                        CallGameVirtual<void, 0>(counter, 1);
                    }
                }
                CallGameFunction<void, 0x004E3F70>(base);
            }
        }
    }
    if (mediator != nullptr) {
        void* const saved = mediator;
        mediator = nullptr;
        result = CallGameVirtual<uint32_t, 1>(saved);
    }
    if (resource != nullptr)
        result = ReleaseInterface(resource);
    if (mediator != nullptr)
        result = CallGameVirtual<uint32_t, 1>(mediator);
    CallGameMethod<uint32_t, 0x006CC920>(reinterpret_cast<uint8_t*>(this) + 0x28);
    result = CallGameFunction<uint32_t, 0x006CDAA0>(map.buckets, map.bucketCount);
    const bool manyBuckets = map.bucketCount > 1;
    map.count0C = 0;
    if (manyBuckets)
        result = CallGameFunction<uint32_t, 0x004E3F70>(map.buckets);
#else
    (void)result;
    AbortNotRewritten("the compositor destructor");
#endif
    vtable2 = reinterpret_cast<void*>(static_cast<uintptr_t>(0x00FF5B44u));
    vtable = reinterpret_cast<void*>(static_cast<uintptr_t>(0x00FB2CBCu));
    return result;
}

// TS3W.exe 0x006CEC10, thiscall ret 4.
MaterialTextureCompositor* MaterialTextureCompositor::DeleteAndDestroy(int flags)
{
    DestroyCore();
    if ((flags & 1) != 0) {
#if defined(_M_IX86)
        CallGameFunction<void, 0x004E3F70>(this);
#endif
    }
    return this;
}

// TS3W.exe 0x006CBF90, thiscall ret 0xC. `create` selects the "Co_0" / "Wo" service name;
// the game builds a 512-byte source string, resolves a resource through 0x0058ABA0,
// allocates the mediator with EaNew(72, "CAS/CompositedTextureMediator") and swaps it in.
void MaterialTextureCompositor::Rebuild(int8_t create, void* arg2, void* resource)
{
#if defined(_M_IX86)
    uint8_t source[512];
    (void)arg2;
    if (mediator != nullptr) {
        CallGameMethod<void, 0x005BBC10>(this);
        void* const old = mediator;
        if (old != nullptr) {
            mediator = nullptr;
            CallGameVirtual<void, 1>(old);
        }
    }
    void* resolved = nullptr;
    if (create != 0) {
        CallGameFunction<void, 0x004FF380>(source, reinterpret_cast<void*>(static_cast<uintptr_t>(0x00FF5BECu)), 256);
        resolved = CallGameFunction<void*, 0x0058ABA0>(reinterpret_cast<void*>(static_cast<uintptr_t>(0x00FE4E70u)), static_cast<const void*>(L"Co_0"), 0x10000000u);
    } else {
        CallGameFunction<void, 0x005BB4D0>(resource, reinterpret_cast<void*>(static_cast<uintptr_t>(0x00FF5BA0u)), source, 256);
        resolved = CallGameFunction<void*, 0x0058ABA0>(reinterpret_cast<void*>(static_cast<uintptr_t>(0x00FE4E70u)), static_cast<const void*>(L"Wo"), 0x20000000u);
    }
    void* replacement = nullptr;
    void* const memory = CallGameFunction<void*, 0x004E3F90>(72, "CAS/CompositedTextureMediator", static_cast<void*>(nullptr));
    if (memory != nullptr)
        replacement = CompositedTextureMediator::Construct(memory, 0, 0, static_cast<int32_t>(reinterpret_cast<intptr_t>(resolved)));
    void* const previous = mediator;
    if (replacement != previous) {
        if (replacement != nullptr)
            CallGameVirtual<uint32_t, 0>(replacement);
        mediator = replacement;
        if (previous != nullptr)
            CallGameVirtual<uint32_t, 1>(previous);
    }
    if (mediator != nullptr) {
        At<uint32_t>(mediator, 0x34) = PtrValue(reinterpret_cast<uint8_t*>(this) + 4);
        CallGameFunction<void, 0x005BC6D0>(source);
    }
#else
    (void)create;
    (void)arg2;
    (void)resource;
    AbortNotRewritten("the composited-texture mediator rebuild");
#endif
}

// TS3W.exe 0x006CC0A0, thiscall ret 0.
void MaterialTextureCompositor::ReleaseResource()
{
    if (resource == nullptr)
        return;
    Rebuild(1, nullptr, nullptr);
#if defined(_M_IX86)
    void* const service = CallGameFunction<void*, 0x004AFD20>();
    if (service != nullptr)
        CallGameVirtual<void, 13>(service, 0, PtrValue(resource), -1000);
    CallGameVirtual<void, 2>(resource);
    void* const saved = resource;
    resource = nullptr;
    if (saved != nullptr)
        ReleaseInterface(saved);
#else
    AbortNotRewritten("the composited-texture resource release");
#endif
}

// TS3W.exe 0x006CC570, thiscall ret 4. The 0x418-byte frame object is the cache request
// the original builds and hands to the cache it creates.
void MaterialTextureCompositor::RefreshWithResource(void* resource)
{
    InitRequestList();
    void* created = nullptr;
    uint8_t filled = 0;
#if defined(_M_IX86)
    if (CallGameFunction<uint8_t, 0x0058B100>("IgnoreWorldCache", static_cast<void*>(nullptr)) == 0) {
        alignas(16) uint8_t request[0x418];
        CallGameFunction<void, 0x004EC290>(request);
        if (CallGameFunction<uint8_t, 0x005BB410>(resource, request) != 0) {
            if (CallGameFunction<void*, 0x004E41F0>(176, "CAS/Compositor/Cache", static_cast<void*>(nullptr)) != nullptr) {
                void* const cache = CallGameFunction<void*, 0x0072CC60>(0);
                if (cache != nullptr) {
                    CallGameVirtual<void, 0>(reinterpret_cast<uint8_t*>(cache) + 4);
                    created = cache;
                    if (CallGameVirtual<uint8_t, 1>(cache) != 0) {
                        const uint32_t key = CallGameFunction<uint32_t, 0x00F626F0>(request);
                        CallGameVirtual<void, 11>(cache, key);
                        if (CallGameVirtual<uint8_t, 6>(cache, 1, 3, 0) != 0) {
                            void* const service = CallGameFunction<void*, 0x004AFD20>();
                            if (CallGameVirtual<uint8_t, 13>(service, 1, PtrValue(cache), -1000) != 0) {
                                uint32_t holder = PtrValue(cache);
                                CallGameFunction<void, 0x00C620B0>(&holder);
                                filled = 1;
                            }
                        }
                    }
                }
            }
        }
    }
    Rebuild(filled, created, resource);
    if (created != nullptr)
        ReleaseInterface(created);
#else
    (void)filled;
    AbortNotRewritten("the composited-texture cache refresh");
#endif
}

// TS3W.exe 0x006CCB40, thiscall ret 8. 59653134 = 0x038E3C0E, 59653228 = 0x038E3C6C.
uint8_t MaterialTextureCompositor::HandleMessage(int32_t message, void* resource)
{
    if (message == 59653134) {
        RefreshWithResource(resource);
        return 1;
    }
    if (message == 59653228) {
        ReleaseResource();
        return 1;
    }
    return 0;
}

// TS3W.exe 0x006CCA00, thiscall ret 4. The part table at +0x2A0 is walked bucket by bucket
// (empty buckets are skipped) until the sentinel node pointer stored at +0xAC4.
uint8_t MaterialTextureCompositor::ClearMatchingPartTypes(const uint32_t* key)
{
    const uintptr_t sentinel = partTable[521];
    uintptr_t* cursor = partTable;
    uintptr_t node = *cursor;
    if (node == 0) {
        do {
            ++cursor;
            node = *cursor;
        } while (node == 0);
    }
    if (node != sentinel) {
        do {
            if (At<uint32_t>(reinterpret_cast<void*>(node), 8) == key[0] &&
                At<uint32_t>(reinterpret_cast<void*>(node), 12) == key[1]) {
                const uintptr_t target = At<uintptr_t>(reinterpret_cast<void*>(node), 0x20) + 0x48;
                At<uint32_t>(reinterpret_cast<void*>(target), 0) = 0;
                At<uint32_t>(reinterpret_cast<void*>(target), 4) = 0;
                At<uint32_t>(reinterpret_cast<void*>(target), 8) = 0;
                At<uint32_t>(reinterpret_cast<void*>(target), 12) = 0;
            }
            node = At<uintptr_t>(reinterpret_cast<void*>(node), 0);
            if (node == 0) {
                do {
                    ++cursor;
                    node = *cursor;
                } while (node == 0);
            }
        } while (node != sentinel);
    }
    return 1;
}

// TS3W.exe 0x00722E80, __cdecl. The global list head lives at 0x0115B874; +0x08 holds the
// payload 0x006A0850 releases, and the three dwords after the head are reset.
int InitRequestList()
{
#if defined(_M_IX86)
    // 0x0115B870 is the global request-list object the game passes in ecx; the payload to
    // release is its +0x0C node pointer (0x0115B87C).
    auto* const list = reinterpret_cast<void*>(static_cast<uintptr_t>(0x0115B870u));
    const uint32_t payload = At<uint32_t>(list, 0x0C);
    CallGameMethod<void, 0x006A0850>(list, payload);
    *reinterpret_cast<uint32_t*>(static_cast<uintptr_t>(0x0115B874u)) = 0x0115B874u;
    *reinterpret_cast<uint32_t*>(static_cast<uintptr_t>(0x0115B878u)) = 0x0115B874u;
    *reinterpret_cast<uint32_t*>(static_cast<uintptr_t>(0x0115B87Cu)) = 0;
    *reinterpret_cast<uint8_t*>(static_cast<uintptr_t>(0x0115B880u)) = 0;
    *reinterpret_cast<uint32_t*>(static_cast<uintptr_t>(0x0115B884u)) = 0;
#else
    AbortNotRewritten("the global request list");
#endif
    return 0;
}

}  // namespace ts3::cas