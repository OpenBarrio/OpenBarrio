#include "ts3/resource/m03_C05.h"
#include <stdexcept>
namespace ts3::resource::m03::c05 {
const void* nativeRegistrationTable = nullptr;
void* __fastcall Allocate5(Pool* self, void* dummy, uintptr_t a, uintptr_t b, uintptr_t ignoredC, uintptr_t ignoredD, uintptr_t e) {
    return self->vtable->allocate3(self, nullptr, a, b, e);
}
void* __fastcall Allocate7(Pool* self, void* dummy, uintptr_t a, uintptr_t b, uintptr_t ignoredC, uintptr_t ignoredD, uintptr_t e, uintptr_t f, uintptr_t g) {
    return self->vtable->allocate5(self, nullptr, a, b, e, f, g);
}
uint64_t __fastcall Zero64(void*, void*, uintptr_t) {
    return 0;
}
uint8_t __fastcall DriverFalse(void*, void*, uintptr_t, uintptr_t, uintptr_t) {
    return 0;
}
void* __fastcall SetCallback(CallbackOwner* self, void*, void* value) {
    self->callback = value;
    return value;
}
void* __fastcall LoaderValue(Loader* self, void*) {
    return self->value;
}
uint8_t __fastcall FactoryFalse(void*, void*, uintptr_t, uintptr_t, uintptr_t, uintptr_t, uintptr_t, uintptr_t, uintptr_t, uintptr_t, uintptr_t) {
    return 0;
}
int32_t __fastcall RefCount(RefObject* self, void*) {
    return self->packed >> 8;
}
void* __fastcall Allocate3(Pool* self, void*, uintptr_t a, uintptr_t b, uintptr_t c) {
    return self->vtable->allocate5(self, nullptr, a, b, c, 4, 0);
}
void* __fastcall KeyAddress(void* self, void*) {
    return static_cast<uint8_t*>(self) + 0x2c;
}
uint32_t __fastcall DriverZero(void*, void*, uintptr_t, uintptr_t) {
    return 0;
}
uint32_t __fastcall DatabaseOne(void*, void*) {
    return 1;
}
uintptr_t __fastcall Invoke(Callback* self, void*, intptr_t offset) {
    return self->function(self->context + offset, nullptr);
}
void* __fastcall CacheValue(Cache* self, void*) {
    return self->value;
}
const void* __fastcall RegistrationTable(void*, void*) {
    #if defined(_M_IX86)
    return reinterpret_cast<const void*>(0x01001ae0);
#else
    if (!nativeRegistrationTable) throw std::logic_error("m03_C05 native registration table not installed");
    return nativeRegistrationTable;
#endif
}
}
