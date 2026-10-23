#include "ts3/render/m07_B07.h"
#include <cstring>
#include <xmmintrin.h>
namespace ts3::render::m07::b07 {
namespace {
constexpr uint32_t compare[] = {1,2,3,4,5,6,7,8};
constexpr uint32_t stencil[] = {1,2,3,4,5,6,7,8};
constexpr uint32_t blendOperation[] = {1,2,4,5,3};
constexpr uint32_t blend[] = {1,2,3,4,5,6,7,8,9,10,11,14,15};
constexpr uint32_t fill[] = {1,2,3};
DeviceHost* Set(DeviceHost* self, uint32_t state, uint32_t value) {
    // 0060E860 returns its receiver in EAX on every path, including suppression.
    return ts3::CallGameMethod<DeviceHost*, 0x0060E860>(self, state, value);
}
uint32_t ReadState(const DeviceHost* self, size_t imageOffset) {
    uint32_t value;
    std::memcpy(&value, self->opaque0100 + imageOffset - 0x100, sizeof(value));
    return value;
}
uint32_t AlphaReference(float reference) {
    // MULSS/ADDSS round separately; EA's bit-based float-to-byte path keeps
    // its peculiar NaN/infinity and overflowing subtraction results.
    __m128 scaled = _mm_mul_ss(_mm_set_ss(reference), _mm_set_ss(255.0f / 256.0f));
    __m128 biased = _mm_add_ss(scaled, _mm_set_ss(49152.0f));
    uint32_t bits = static_cast<uint32_t>(_mm_cvtsi128_si32(_mm_castps_si128(biased)));
    uint32_t shifted = bits + 0xB8C00000u;
    uint32_t low = (shifted & 0x80000000u) ? 0u : (shifted & 0xFFu);
    uint32_t high = ((255u - shifted) & 0x80000000u) ? 255u : 0u;
    return low | high;
}
}
DeviceHost* SetDepthWrite(DeviceHost* self, uint32_t value) {
    return Set(self, 14, value & 0xFFu);
}
DeviceHost* SetDepthCompare(DeviceHost* self, uint32_t value) {
    return Set(self, 23, compare[value]);
}
DeviceHost* SetStencilEnable(DeviceHost* self, uint32_t value) {
    return Set(self, 52, value & 0xFFu);
}
DeviceHost* SetStencilCompare(DeviceHost* self, uint32_t value, uint32_t reference, uint32_t mask) {
    Set(self, 56, compare[value]);
    Set(self, 57, reference);
    return Set(self, 58, mask);
}
uint32_t GetStencilReference(DeviceHost* self) {
    return ReadState(self, 0x228);
}
DeviceHost* SetStencilWriteMask(DeviceHost* self, uint32_t value) {
    return Set(self, 59, value);
}
DeviceHost* SetStencilOperations(DeviceHost* self, uint32_t fail, uint32_t depthFail, uint32_t pass) {
    Set(self, 53, stencil[fail]);
    Set(self, 54, stencil[depthFail]);
    return Set(self, 55, stencil[pass]);
}
DeviceHost* SetAlphaTestEnable(DeviceHost* self, uint32_t value) {
    return Set(self, 15, value & 0xFFu);
}
DeviceHost* SetAlphaCompare(DeviceHost* self, uint32_t value, float reference) {
    Set(self, 25, compare[value]);
    return Set(self, 24, AlphaReference(reference));
}
DeviceHost* SetBlendOperation(DeviceHost* self, uint32_t value) {
    return Set(self, 171, blendOperation[value]);
}
DeviceHost* SetBlendEnable(DeviceHost* self, uint32_t value) {
    return Set(self, 27, value & 0xFFu);
}
DeviceHost* SetBlendFactors(DeviceHost* self, uint32_t source, uint32_t destination) {
    Set(self, 19, blend[source]);
    return Set(self, 20, blend[destination]);
}
DeviceHost* SetFillMode(DeviceHost* self, uint32_t value) {
    return Set(self, 8, fill[value]);
}

#if defined(_M_IX86)
DeviceHost* __fastcall HookSetDepthWrite(DeviceHost* self, void*, uint32_t value) { return SetDepthWrite(self ,value); }
DeviceHost* __fastcall HookSetDepthCompare(DeviceHost* self, void*, uint32_t value) { return SetDepthCompare(self ,value); }
DeviceHost* __fastcall HookSetStencilEnable(DeviceHost* self, void*, uint32_t value) { return SetStencilEnable(self ,value); }
DeviceHost* __fastcall HookSetStencilCompare(DeviceHost* self, void*, uint32_t value, uint32_t reference, uint32_t mask) { return SetStencilCompare(self ,value, reference, mask); }
uint32_t __fastcall HookGetStencilReference(DeviceHost* self, void*) { return GetStencilReference(self); }
DeviceHost* __fastcall HookSetStencilWriteMask(DeviceHost* self, void*, uint32_t value) { return SetStencilWriteMask(self ,value); }
DeviceHost* __fastcall HookSetStencilOperations(DeviceHost* self, void*, uint32_t fail, uint32_t depthFail, uint32_t pass) { return SetStencilOperations(self ,fail, depthFail, pass); }
DeviceHost* __fastcall HookSetAlphaTestEnable(DeviceHost* self, void*, uint32_t value) { return SetAlphaTestEnable(self ,value); }
DeviceHost* __fastcall HookSetAlphaCompare(DeviceHost* self, void*, uint32_t value, float reference) { return SetAlphaCompare(self ,value, reference); }
DeviceHost* __fastcall HookSetBlendOperation(DeviceHost* self, void*, uint32_t value) { return SetBlendOperation(self ,value); }
DeviceHost* __fastcall HookSetBlendEnable(DeviceHost* self, void*, uint32_t value) { return SetBlendEnable(self ,value); }
DeviceHost* __fastcall HookSetBlendFactors(DeviceHost* self, void*, uint32_t source, uint32_t destination) { return SetBlendFactors(self ,source, destination); }
DeviceHost* __fastcall HookSetFillMode(DeviceHost* self, void*, uint32_t value) { return SetFillMode(self ,value); }
#endif
}
