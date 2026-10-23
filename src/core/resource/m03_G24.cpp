#include "ts3/resource/m03_G24.h"
#include <atomic>
#include <cstring>
namespace ts3::resource::m03::g24 {
#if defined(_M_IX86)
namespace {
uint8_t* Bytes(void* self) { return static_cast<uint8_t*>(self); }
void Store32(void* self, unsigned offset, uint32_t value) { std::memcpy(Bytes(self) + offset, &value, 4); }
void Store16(void* self, unsigned offset, uint16_t value) { std::memcpy(Bytes(self) + offset, &value, 2); }
void Store8(void* self, unsigned offset, uint8_t value) { std::memcpy(Bytes(self) + offset, &value, 1); }
uint32_t Load32(const void* self, unsigned offset) { uint32_t value; std::memcpy(&value, Bytes(const_cast<void*>(self)) + offset, 4); return value; }
uint8_t Load8(const void* self, unsigned offset) { return Bytes(const_cast<void*>(self))[offset]; }
void StorePointer(void* self, unsigned offset, const void* pointer) { Store32(self, offset, static_cast<uint32_t>(reinterpret_cast<uintptr_t>(pointer))); }
void* LoadPointer(const void* self, unsigned offset) { return reinterpret_cast<void*>(static_cast<uintptr_t>(Load32(self, offset))); }
void* Slot(void* self, unsigned offset) { return Bytes(self) + offset; }
// Last-ref release: decrement +4, and when the old count was one reset it to one before the slot-0 delete.
void Release(void* object) {
    if (!object) return;
    auto* count = reinterpret_cast<int32_t*>(Bytes(object) + 4);
    std::atomic_ref<int32_t> ref(*count);
    if (ref.fetch_sub(1) == 1) {
        ref.store(1);
        CallGameVirtual<void, 0>(object, 1);
    }
}
}
#endif
void* __fastcall Construct(void* self, void*) {
#if defined(_M_IX86)
    Store32(self, 0x000, 0x00FFDFE0u);
    CallGameMethod<void, 0x004B33B0>(Slot(self, 0x008), static_cast<void*>(nullptr));
    Store32(self, 0x1D8, 0x00FB2CBCu);
    Store32(self, 0x000, 0x00FFE2F0u);
    Store32(self, 0x008, 0x00FFE250u);
    Store32(self, 0x1D8, 0x00FFE244u);
    Store32(self, 0x1DC, 0);
    Store32(self, 0x1E0, 0);
    Store32(self, 0x1E4, 0);
    Store32(self, 0x1E8, 0x00FD8248u);
    Store32(self, 0x1EC, 0x6B20C4F3u);
    Store8(self, 0x1F0, 0);
    Store8(self, 0x1F1, 1);
    Store32(self, 0x1F4, 0);
    Store32(self, 0x1F8, 0);
    Store32(self, 0x1FC, 0);
    Store32(self, 0x200, 0x00FFE478u);
    Store32(self, 0x204, 0);
    Store32(self, 0x208, 0);
    Store32(self, 0x20C, 0);
    Store32(self, 0x210, 0x00FFE448u);
    CallGameMethod<void, 0x004E17E0>(Slot(self, 0x218), static_cast<void*>(nullptr), uint8_t{1});
    CallGameMethod<void, 0x004E17E0>(Slot(self, 0x248), static_cast<void*>(nullptr), uint8_t{1});
    CallGameMethod<void, 0x004E17E0>(Slot(self, 0x278), static_cast<void*>(nullptr), uint8_t{1});
    Store32(self, 0x2A8, 0);
    Store32(self, 0x2AC, 0);
    Store32(self, 0x2B0, 0);
    Store32(self, 0x2B4, 0x00FFE428u);
    Store32(self, 0x2B8, 0);
    Store32(self, 0x2BC, 0);
    Store32(self, 0x2C0, 0);
    Store32(self, 0x2C4, 0x00FFE408u);
    CallGameMethod<void, 0x004F35B0>(Slot(self, 0x2C8), uint32_t{4}, uint32_t{0});
    Store32(self, 0x2E8, 0x00FFE3E8u);
    StorePointer(self, 0x2E0, Slot(self, 0x2E0));
    StorePointer(self, 0x2E4, Slot(self, 0x2E0));
    StorePointer(self, 0x2FC, Slot(self, 0x300));
    StorePointer(self, 0x2F0, Slot(self, 0x300));
    StorePointer(self, 0x2EC, Slot(self, 0x300));
    StorePointer(self, 0x2F4, Slot(self, 0x508));
    Store16(self, 0x300, 0);
    Store32(self, 0x2F8, 0x00FA15D8u);
    StorePointer(self, 0x518, Slot(self, 0x51C));
    StorePointer(self, 0x50C, Slot(self, 0x51C));
    StorePointer(self, 0x508, Slot(self, 0x51C));
    StorePointer(self, 0x510, Slot(self, 0x724));
    Store16(self, 0x51C, 0);
    Store32(self, 0x514, 0x00FA15D8u);
    Store32(self, 0x724, 0);
    Store32(self, 0x728, 0);
    Store32(self, 0x72C, 0);
    Store32(self, 0x730, 0x00FFE3CCu);
    Store32(self, 0x734, 0);
    Store32(self, 0x738, 0);
    Store32(self, 0x73C, 0);
    Store32(self, 0x740, 0x00FFE3B0u);
    Store32(self, 0x750, 0x00FFE3A0u);
    Store32(self, 0x744, 0x011C7E54u);
    Store32(self, 0x748, 0x011C7E54u);
    Store32(self, 0x74C, 0x011C7E56u);
    StorePointer(self, 0x758, Slot(self, 0x758));
    StorePointer(self, 0x75C, Slot(self, 0x758));
    Store32(self, 0x760, 0x00FFE38Cu);
    Store8(self, 0x764, 0);
    CallGameMethod<void, 0x007306B0>(Slot(self, 0x768));
    Store32(self, 0x7F0, 0);
    Store32(self, 0x7F4, 0);
    Store32(self, 0x7F8, 0);
    Store32(self, 0x7FC, 0);
    Store32(self, 0x800, 0);
    Store32(self, 0x804, 0);
    Store32(self, 0x808, 0);
    if (void* tuning = CallGameFunction<void*, 0x005A00A0>())
        CallGameVirtual<void, 6>(tuning, reinterpret_cast<void*>(uintptr_t{0x00FF829C}), uint32_t{10000});
    if (void* tuning = CallGameFunction<void*, 0x005A00A0>())
        CallGameMethod<void, 0x00735460>(tuning, reinterpret_cast<void*>(uintptr_t{0x00FFE218}), self,
                                         reinterpret_cast<void*>(uintptr_t{0x00733F10}), static_cast<void*>(nullptr));
    if (void* tuning = CallGameFunction<void*, 0x005A00A0>())
        CallGameVirtual<void, 6>(tuning, static_cast<void*>(nullptr), uint32_t{0x7FFFFFFF});
    return self;
#else
    NotRewritten("m03.G24.Construct");
#endif
}
int __fastcall Stop(void* self, void*) {
#if defined(_M_IX86)
    CallGameMethod<void, 0x00736CB0>(self, uint32_t{1});
    CallGameMethod<void, 0x00730270>(Slot(self, 0x768));
    if (LoadPointer(self, 0x1E0)) {
        CallGameVirtual<void, 21>(Slot(self, 0x008), uint32_t{0}, LoadPointer(self, 0x1E0), uint32_t{0});
        // The original re-reads the field after every call (register clobber): a callee that swaps
        // the database must be observed by the next step, exactly like the game's reloads do.
        CallGameFunction<int32_t, 0x00731ED0>();
        CallGameVirtual<void, 2>(LoadPointer(self, 0x1E0));
        Release(LoadPointer(self, 0x1E0));
        Store32(self, 0x1E0, 0);
    }
    if (LoadPointer(self, 0x1E4)) {
        CallGameVirtual<void, 21>(Slot(self, 0x008), uint32_t{0}, LoadPointer(self, 0x1E4), uint32_t{0});
        CallGameVirtual<void, 2>(LoadPointer(self, 0x1E4));
        Release(LoadPointer(self, 0x1E4));
        Store32(self, 0x1E4, 0);
    }
    if (void* async = CallGameFunction<void*, 0x00727280>())
        CallGameVirtual<void, 2>(async);
    uint8_t* queue = static_cast<uint8_t*>(CallGameFunction<void*, 0x0072A510>());
    *queue = 1;
    uint32_t result;
    do {
        result = CallGameMethod<uint32_t, 0x0072A730>(queue, uint32_t{0x7FFFFFFF}, int32_t{-1});
    } while (static_cast<uint8_t>(result) != 0);
    return static_cast<int>(result);
#else
    NotRewritten("m03.G24.Stop");
#endif
}
int __fastcall Update(void* self, void*, int budget) {
#if defined(_M_IX86)
    if (Load8(self, 0x1F0) != 0 && budget < 250) budget = 250;
    uint8_t* queue = static_cast<uint8_t*>(CallGameFunction<void*, 0x0072A510>());
    CallGameMethod<void, 0x0072A730>(queue, uint32_t{0x7FFFFFFF}, static_cast<uint32_t>(budget));
    CallGameMethod<uint32_t, 0x00732E90>(LoadPointer(self, 0x1E0), uint32_t{0}, static_cast<void*>(nullptr), uint32_t{0});
    CallGameMethod<uint32_t, 0x00732E90>(LoadPointer(self, 0x1E4), uint32_t{0}, static_cast<void*>(nullptr), uint32_t{0});
    uint32_t begin = 0, end = 0;
    CallGameMethod<int32_t, 0x004E16F0>(Slot(self, 0x248), reinterpret_cast<void*>(uintptr_t{0x00FFDFC4}));
    if ((Load32(self, 0x2C8) | Load32(self, 0x2CC)) != 0) {
        if (CallGameMethod<uint64_t, 0x004F32C0>(Slot(self, 0x2C8)) > 1000) {
            CallGameMethod<void, 0x004F3220>(Slot(self, 0x2C8));
            begin = Load32(self, 0x204);
            Store32(self, 0x204, 0);
            end = Load32(self, 0x208);
            Store32(self, 0x208, 0);
            Store32(self, 0x20C, 0);
        }
    }
    CallGameMethod<int32_t, 0x004E17B0>(Slot(self, 0x248));
    while (begin != end) {
        void* entry = LoadPointer(reinterpret_cast<void*>(static_cast<uintptr_t>(end - 4)), 0);
        end -= 4;
        CallGameVirtual<void, 5>(self, CallGameMethod<void*, 0x004DA9D0>(entry));
    }
    if (Load8(self, 0x1F1) != 0) {
        CallGameMethod<int32_t, 0x004E16F0>(Slot(self, 0x218), reinterpret_cast<void*>(uintptr_t{0x00FFDFC4}));
        uint32_t watcher = Load32(self, 0x2B8), watcherEnd = Load32(self, 0x2BC);
        while (watcher != watcherEnd) {
            CallGameVirtual<void, 6>(LoadPointer(reinterpret_cast<void*>(static_cast<uintptr_t>(watcher)), 0), uint32_t{0});
            watcher += 4;
        }
        CallGameMethod<int32_t, 0x004E17B0>(Slot(self, 0x218));
    }
    if (void* async = CallGameFunction<void*, 0x00727280>()) {
        uint8_t deadline[32];
        CallGameMethod<void, 0x004F35B0>(deadline, uint32_t{4}, uint32_t{0});
        CallGameMethod<uint32_t, 0x004F34F0>(deadline, static_cast<uint32_t>(budget), uint32_t{0});
        CallGameVirtual<void, 4>(async, static_cast<void*>(deadline));
    }
    CallGameMethod<void, 0x00730740>(Slot(self, 0x768));
    const uint32_t result = CallGameMethod<uint32_t, 0x00736CB0>(self, uint32_t{0});
    if (begin != 0)
        return static_cast<int>(CallGameFunction<uintptr_t, 0x004E3F70>(reinterpret_cast<void*>(static_cast<uintptr_t>(begin))));
    return static_cast<int>(result);
#else
    NotRewritten("m03.G24.Update");
#endif
}
}