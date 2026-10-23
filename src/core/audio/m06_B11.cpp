#include "ts3/audio/m06_B11.h"
#include "ts3/layout.h"
#include <cstring>
#include <atomic>
#if defined(_WIN32)
#define WIN32_LEAN_AND_MEAN
#define NOMINMAX
#include <windows.h>
#endif
namespace ts3::audio {
TS3_X86_OFFSET(B11Bucket, next, 0xb8);
TS3_X86_OFFSET(B11Entry, retained, 0x14);
TS3_X86_OFFSET(B11SoundArray, mutex, 0x38);
TS3_X86_SIZE(B11SoundArray, 0x78);
namespace {
void Free(void* p) { CallGameFunction<void, 0x004E3F70>(p); }
void Critical(void* p, bool initialize) {
#if defined(_M_IX86)
    using Fn = void(__stdcall*)(void*);
    auto slot = initialize ? 0x00F95124u : 0x00F95128u;
    (*reinterpret_cast<Fn*>(slot))(p);
#else
    if (initialize) InitializeCriticalSection(static_cast<CRITICAL_SECTION*>(p));
    else DeleteCriticalSection(static_cast<CRITICAL_SECTION*>(p));
#endif
}
const void* Datum(uintptr_t address) {
#if defined(_M_IX86)
    return reinterpret_cast<const void*>(address);
#else
    static const char empty[] = "";
    (void)address; return empty;
#endif
}
void Tables(void* self) {
    auto** p = static_cast<void**>(self);
    p[0] = const_cast<void*>(Datum(0x01028DD8));
    p[1] = const_cast<void*>(Datum(0x01028DC4));
}
}
void B11Clear(B11Bucket** buckets, uint32_t count) {
    for (uint32_t i = 0; i < count; ++i) {
        auto* bucket = buckets[i];
        while (bucket) {
            auto* next = bucket->next; // captured before callbacks
            auto* entry = bucket->map.head;
            while (entry) {
                CallGameMethod<void, 0x008715D0>(&bucket->map, entry->key);
                auto* following = entry->next;
                if (entry->retained) CallGameVirtual<void, 1>(entry->retained);
                // EA recycles only after Release, and keeps the old next/ref fields.
                entry->freeNext = bucket->map.free;
                bucket->map.free = entry;
                entry = following;
            }
            Free(bucket);
            bucket = next;
        }
        buckets[i] = nullptr;
    }
}
B11SoundArray* B11ConstructArray(B11SoundArray* self, const char* name, uint32_t mode, uint32_t flag) {
    self->vtable = const_cast<void*>(Datum(0x0107396C));
#if defined(_WIN32)
    InterlockedExchange(reinterpret_cast<volatile LONG*>(&self->refs), 0);
#else
    self->refs = 0;
#endif
    self->vtable = const_cast<void*>(Datum(0x01028D3C));
    self->gain = 1.0f; self->pitch = 2.0f;
    self->mapLabel = Datum(0x01028D1C);
    self->one = 1; self->empty = Datum(0x0114C900);
    self->zero14 = nullptr; self->zero20 = 0;
    self->queueLabel = Datum(0x01028CFC);
    self->first = &self->first; self->last = &self->first;
    Critical(self->mutex, true);
    self->nameBegin = self->nameEnd = self->nameCapacity = nullptr;
    self->nameLabel = Datum(0x01028CE4);
    CallGameMethod<void, 0x00405C00>(&self->nameBegin, name);
    self->active = 0; self->mode = mode; self->flag = static_cast<uint8_t>(flag); self->update = 1.0f;
    return self;
}
void* B11DeleteArray(void* self, uint32_t flags) {
    CallGameMethod<void, 0x00872220>(self);
    if (flags & 1) Free(self);
    return self;
}
uint32_t B11Load(void* self, void* resource) {
    const uint32_t type = CallGameMethod<uint32_t, 0x0089F6C0>(resource);
    switch (type) {
    case 0x0A29E8BE: return CallGameMethod<uint32_t, 0x00868B20>(self, resource);
    case 0x0A29E8E6: return CallGameMethod<uint32_t, 0x008686F0>(self, resource);
    case 0x0A320CC2: return CallGameMethod<uint32_t, 0x00868820>(self, resource);
    case 0x0CC8F7E5: return CallGameMethod<uint32_t, 0x008688E0>(self, resource);
    default: return CallGameMethod<uint32_t, 0x008BCBB0>(self, resource);
    }
}
void* B11Construct(void* self) {
    CallGameMethod<void, 0x008C0720>(self);
    Tables(self); Critical(static_cast<uint8_t*>(self) + 0x27bb0, true);
    return self;
}
uintptr_t B11Destruct(void* self) {
    Tables(self); Critical(static_cast<uint8_t*>(self) + 0x27bb0, false);
    return CallGameMethod<uintptr_t, 0x008BFEB0>(self);
}
uint32_t B11Initialize(void* self, void* config) {
    uint32_t result = CallGameMethod<uint32_t, 0x008B3C20>(self, config);
    if (!(result & 0xff)) return result;
    CallGameFunction<void, 0x00F14E10>(0);
    result = CallGameFunction<uint32_t, 0x00F14920>();
    return (result & 0xffffff00u) | 1;
}
uintptr_t B11Start(void* self) {
    CallGameVirtual<void, 17>(self, uint32_t(0x044888B0));
    CallGameVirtual<void, 19>(self, uint32_t(0x034753A0), uint32_t(1));
    CallGameVirtual<void, 26>(self);
    return CallGameMethod<uintptr_t, 0x008B4730>(self, 0.0f);
}
uintptr_t B11Stop(void* self) {
    CallGameFunction<void, 0x00F02FC0>();
    return CallGameMethod<uintptr_t, 0x008B0AE0>(self);
}
uintptr_t B11Samples(void* self) { return CallGameMethod<uintptr_t, 0x008BE390>(self); }
uintptr_t B11Voices(void* self) { return CallGameMethod<uintptr_t, 0x008BF250>(self); }
uintptr_t B11Update(void* self, float a, float b) {
    // The x87 load/store can quiet signaling NaNs; do not substitute integer copies.
#if defined(_M_IX86)
    __asm { fld a }
    __asm { fstp a }
    __asm { fld b }
    __asm { fstp b }
#endif
    return CallGameMethod<uintptr_t, 0x008BFAE0>(self, a, b);
}
void* B11DeleteSecondary(void* self, uint32_t flags) {
    return B11Delete(static_cast<uint8_t*>(self) - sizeof(void*), flags);
}
void* B11Delete(void* self, uint32_t flags) {
    B11Destruct(self);
    if (flags & 1) Free(self);
    return self;
}
}
