#pragma once
#include "ts3/layout.h"
#include <cstdint>
namespace ts3::audio::m06_B12 {
struct Service {
    void* vtable;
    uint32_t base[3];
    void* manager;
    uint32_t reserved;
    alignas(void*) uint8_t inner[0xB8];
};
TS3_X86_OFFSET(Service, manager, 0x10);
TS3_X86_OFFSET(Service, inner, 0x18);
TS3_X86_SIZE(Service, 0xD0);
// TS3W.exe 0x00868EE0: Construct the audio service and its embedded service interface.
Service* Construct(Service* self);
// TS3W.exe 0x00868F10: Return the service type identifier (adapted from FileType's constant leaf).
const void* TypeId(Service* self);
// TS3W.exe 0x00868F20: Allocate, retain and initialize the manager; publish before callbacks.
bool Start(Service* self);
// TS3W.exe 0x00868FA0: Stop/release the manager before stopping the embedded service.
uintptr_t Stop(Service* self);
// TS3W.exe 0x00868FF0: Ignore both arguments and perform the original no-op cleanup.
void Idle(Service* self, uint32_t a, uint32_t b);
// TS3W.exe 0x00869000: Tick the embedded service then forward the second argument; keep x87 NaN quieting.
uintptr_t Tick(Service* self, uint32_t ignored, float elapsed);
// TS3W.exe 0x00869030: Destroy the embedded service and base, optionally freeing self on flag bit zero.
Service* Destroy(Service* self, uint32_t flags);
// TS3W.exe 0x00869060: Register Audio/Service when the service registry lacks its identifier.
void RegisterService();
// TS3W.exe 0x008690D0: Schedule the manager callback then invoke its slot four with zero.
void Activate(Service* self);
// TS3W.exe 0x00869130: Copy a surface-name string with strlcpy length/truncation semantics.
uint32_t SurfaceName(uint32_t surface, void* output, uint32_t capacity);
// TS3W.exe 0x00869340: Return the device byte at +270 or zero (GetErrorLine twin).
uint8_t DeviceFlag();
// TS3W.exe 0x00869360: Check signed byte coordinates against exclusive XY and inclusive Z bounds.
uint32_t Contains(const void* self, const int8_t* coordinates);
// TS3W.exe 0x008693B0: Test either object flag D0:12 or 9C:6.
uint32_t HasFlag(const void* self);
// Zero-storage ABI view used by the generated thiscall adapters.
struct Api {
    Service* Construct() { return m06_B12::Construct(reinterpret_cast<Service*>(this)); }
    void* TypeId() { return const_cast<void*>(m06_B12::TypeId(reinterpret_cast<Service*>(this))); }
    bool Start() { return m06_B12::Start(reinterpret_cast<Service*>(this)); }
    uintptr_t Stop() { return m06_B12::Stop(reinterpret_cast<Service*>(this)); }
    void Idle(uint32_t a, uint32_t b) { m06_B12::Idle(reinterpret_cast<Service*>(this),a,b); }
    uintptr_t Tick(uint32_t a, float b) { return m06_B12::Tick(reinterpret_cast<Service*>(this),a,b); }
    Service* Destroy(uint32_t flags) { return m06_B12::Destroy(reinterpret_cast<Service*>(this),flags); }
    void Activate() { m06_B12::Activate(reinterpret_cast<Service*>(this)); }
    uint32_t Contains(void* c) { return m06_B12::Contains(this,static_cast<const int8_t*>(c)); }
    uint32_t HasFlag() { return m06_B12::HasFlag(this); }
};
}
