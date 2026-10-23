#include "ts3/anim/m05_M55.h"
namespace ts3::anim::m55 {
static void** SpecializedTable(bool second) {
#if defined(_M_IX86)
    return reinterpret_cast<void**>(second ? 0x00fc2420 : 0x00fc23e0);
#else
    static void* tables[2][11]{};
    return tables[second ? 1 : 0];
#endif
}
void* __stdcall CreateScalar(void* host, void* storage) {
    if (!storage) return nullptr;
    return m07::Construct(static_cast<m07::Evaluator*>(storage), nullptr, host);
}
void* __stdcall CreateBlend(void* host, void* storage) {
    if (!storage) return nullptr;
    return m10::Construct(static_cast<m10::Blend*>(storage), host);
}
void* __stdcall CreateFirst(void* host, void* storage) {
    if (!storage) return nullptr;
    auto* result = m07::Construct(static_cast<m07::Evaluator*>(storage), nullptr, host);
    result->base.vtable = SpecializedTable(false);
    return storage;
}
void* __stdcall CreateSecond(void* host, void* storage) {
    if (!storage) return nullptr;
    auto* result = m07::Construct(static_cast<m07::Evaluator*>(storage), nullptr, host);
    result->base.vtable = SpecializedTable(true);
    return storage;
}
void* __stdcall CreateBuffer(void* host, void* storage) {
    if (!storage) return nullptr;
    return m16::Construct(static_cast<m16::Evaluator*>(storage), host);
}
void* __stdcall CreateConstant(void* host, void* storage) {
    if (!storage) return nullptr;
    return m17::ConstructConstant(static_cast<m17::Constant*>(storage), host);
}
}
