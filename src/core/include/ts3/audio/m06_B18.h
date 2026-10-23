#pragma once
#include <cstdint>
#include <cstddef>
namespace ts3::audio::b18impl {
struct Primitive {
    void* primary; void* secondary; void* interfaceTable;
    uint32_t state; uint8_t enabled; uint8_t padding[3]; float value;
    void* resource; void* auxiliary; uint32_t hash;
};
struct Curve { void* begin; void* end; void* capacity; const void* label; void* inlineBegin; uint8_t storage[0x348]; uint32_t state; };
struct alignas(16) Listener { uint8_t vectors[0x40]; Curve curves[3]; uint8_t active,flag; uint8_t padding[6]; uint32_t word0,word1; alignas(16) float orientation[4]; float velocity[4]; };
// TS3W.exe 0x0086D1A0: Load listener curves from a retained property view, testing only AL.
bool LoadListener(Listener*, void*);
// TS3W.exe 0x0086DD80: Free non-inline curve allocations in reverse order without clearing fields.
void DestroyListener(Listener*);
// TS3W.exe 0x0086DDD0: Initialize listener vectors, three curves, flags and homogeneous orientation.
Listener* ConstructListener(Listener*);
// TS3W.exe 0x0086DE70: Forward weather mode as a float through primary slot 14.
uintptr_t UpdateWeather(Primitive*, uint32_t, uint32_t);
// TS3W.exe 0x0086DED0: Notify before storing a changed value; unordered values count as changed.
void SetValue(Primitive*, float);
// TS3W.exe 0x0086DF00: Adjust secondary this and tail-call release.
int32_t ReleaseSecondary(void*);
// TS3W.exe 0x0086DF10: Adjust secondary this and tail-call addref.
int32_t AddRefSecondary(void*);
// TS3W.exe 0x0086DF20: Construct the weather primitive interface views.
Primitive* ConstructWeather(Primitive*);
// TS3W.exe 0x0086DF60: Lazily hash the weather primitive name.
uint32_t WeatherName(Primitive*);
// TS3W.exe 0x0086DF90: Forward lot classification, doing nothing when the world singleton is absent.
uintptr_t UpdateLot(Primitive*, uint32_t, uint32_t);
// TS3W.exe 0x0086DFE0: Construct the lot primitive interface views.
Primitive* ConstructLot(Primitive*);
// TS3W.exe 0x0086E020: Lazily hash the lot primitive name.
uint32_t LotName(Primitive*);
// TS3W.exe 0x0086E050: Adjust secondary this and invoke the primitive deleting destructor.
void* DeleteSecondary(void*, uint32_t);
// TS3W.exe 0x0086E060: Forward the adjusted manager scalar; preserves the null-path low-address read bug.
uintptr_t UpdateManager(Primitive*, uint32_t, uint32_t);
}
// ABI adapter view: no storage; game this remains the observed object address.
namespace ts3::audio {
struct b18 {
    bool LoadListener(void* source) { return b18impl::LoadListener(reinterpret_cast<b18impl::Listener*>(this),source); }
    void DestroyListener() { b18impl::DestroyListener(reinterpret_cast<b18impl::Listener*>(this)); }
    void* ConstructListener() { return b18impl::ConstructListener(reinterpret_cast<b18impl::Listener*>(this)); }
    uintptr_t UpdateWeather(uint32_t a,uint32_t b) { return b18impl::UpdateWeather(reinterpret_cast<b18impl::Primitive*>(this),a,b); }
    void SetValue(float v) { b18impl::SetValue(reinterpret_cast<b18impl::Primitive*>(this),v); }
    void* ConstructWeather() { return b18impl::ConstructWeather(reinterpret_cast<b18impl::Primitive*>(this)); }
    uint32_t WeatherName() { return b18impl::WeatherName(reinterpret_cast<b18impl::Primitive*>(this)); }
    uintptr_t UpdateLot(uint32_t a,uint32_t b) { return b18impl::UpdateLot(reinterpret_cast<b18impl::Primitive*>(this),a,b); }
    void* ConstructLot() { return b18impl::ConstructLot(reinterpret_cast<b18impl::Primitive*>(this)); }
    uint32_t LotName() { return b18impl::LotName(reinterpret_cast<b18impl::Primitive*>(this)); }
    uintptr_t UpdateManager(uint32_t a,uint32_t b) { return b18impl::UpdateManager(reinterpret_cast<b18impl::Primitive*>(this),a,b); }
};
}
