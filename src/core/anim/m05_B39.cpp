#include "ts3/anim/m05_B39.h"
#include <xmmintrin.h>
#include <cstring>
#include <limits>
namespace ts3::anim::b39 {
void __fastcall SetTarget(Vehicle* self, void*, float value) { self->target = value; }
uintptr_t __fastcall Stop(Vehicle* self, void*) {
    self->speed = 0.0f;
    self->target = 0.0f;
    if (!(self->flags & 4) && self->route) {
        CallGameVirtual<uintptr_t,30>(self->host, self->route, uint32_t(0));
        self->route = nullptr;
    }
    const bool mode = self->mode != 0;
    auto result = CallGameVirtual<uintptr_t,20>(self->host, mode ? 2 : 3, uint32_t(0), uint32_t(0));
    if (!mode) result = CallGameVirtual<uintptr_t,17>(self->host);
    return result;
}
uintptr_t __fastcall SetRoute(Vehicle* self, void*, const Range* range, uint32_t flags) {
    if (self->route) {
        CallGameVirtual<uintptr_t,30>(self->host, self->route, uint32_t(0));
        self->route = nullptr;
    }
    auto begin = range->begin;
    if (begin == range->end) return reinterpret_cast<uintptr_t>(begin);
    return CallGameVirtual<uintptr_t,29>(self->host, &self->route, begin, flags);
}
uintptr_t __fastcall Position(void* self, void*, float* out) {
    auto bytes = static_cast<unsigned char*>(self);
    auto source = *reinterpret_cast<void**>(bytes - sizeof(void*));
    CallGameVirtual<uintptr_t,7>(source, out);
    float lateral; std::memcpy(&lateral, bytes + 0xa0, 4);
    const __m128 v = _mm_set_ps(0.0f, lateral, 0.0f, 0.0f);
    alignas(16) float rotation[4];
    // The position callback may replace the borrowed host pointer.
    source = *reinterpret_cast<void**>(bytes - sizeof(void*));
    auto result = CallGameVirtual<uintptr_t,9>(source, rotation);
    const __m128 q = _mm_load_ps(rotation);
    const __m128 q9 = _mm_shuffle_ps(q,q,9);
    const __m128 q18 = _mm_shuffle_ps(q,q,18);
    __m128 t = _mm_sub_ps(_mm_mul_ps(_mm_shuffle_ps(v,v,18),q9), _mm_mul_ps(_mm_shuffle_ps(v,v,9),q18));
    t = _mm_add_ps(t, _mm_mul_ps(_mm_shuffle_ps(q,q,255),v));
    __m128 r = _mm_sub_ps(_mm_mul_ps(_mm_shuffle_ps(t,t,18),q9), _mm_mul_ps(_mm_shuffle_ps(t,t,9),q18));
    r = _mm_mul_ps(r, _mm_set1_ps(2.0f));
    r = _mm_add_ps(r,v);
    _mm_store_ps(out,_mm_add_ps(r,_mm_load_ps(out)));
    return result;
}
float* __fastcall CopyValue(void* self, void*, float* out) {
    _mm_store_ps(out,_mm_load_ps(reinterpret_cast<const float*>(static_cast<unsigned char*>(self)+0x18)));
    return out;
}
StringRecord* __fastcall InitNames(StringRecord* self, void*) {
#if defined(_M_IX86)
    const auto empty = reinterpret_cast<const char16_t*>(0x011c7e54);
    self[0] = {empty, empty, empty+1, reinterpret_cast<const void*>(0x010277a4)};
    self[1] = {empty, empty, empty+1, reinterpret_cast<const void*>(0x01027780)};
#else
    static const char16_t empty[2] = {};
    static const char cruise[] = "Locomotion/VehicleBody/Cruise";
    static const char decelerate[] = "Locomotion/VehicleBody/Decelerate";
    self[0] = {empty,empty,empty+1,cruise};
    self[1] = {empty,empty,empty+1,decelerate};
#endif
    return self;
}
uint32_t __fastcall VehicleType(void*, void*) { return 0x0754f112u; }
uint32_t __fastcall SecondaryType(void*, void*) { return 0x0754f112u; }
uintptr_t __fastcall Forward(void* self, void*) { return CallGameVirtual<uintptr_t,3>(self); }
uint32_t __fastcall AccumulatorType(void*, void*) { return 0x34d414c0u; }
void* __fastcall Query(void* self, void*, uint32_t id) {
    if (id == 0x34d414c0u) return self;
    if (id == 0x74d3f00cu) return self;
    return nullptr;
}
void __fastcall Clear(void* self, void*) {
    _mm_store_ps(reinterpret_cast<float*>(static_cast<unsigned char*>(self)+0x10),_mm_setzero_ps());
}
const float* __fastcall Accumulate(void* self, void*, const float* input, float scale) {
    auto value = reinterpret_cast<float*>(static_cast<unsigned char*>(self)+0x10);
    const __m128 product = _mm_mul_ps(_mm_set1_ps(scale),_mm_load_ps(input));
    _mm_store_ps(value,_mm_add_ps(product,_mm_load_ps(value)));
    return input;
}
bool __fastcall Reset(void* self, void*) {
    auto b = static_cast<unsigned char*>(self);
    const float values[4] = {-1.0f,-1.0f,std::numeric_limits<float>::max(),std::numeric_limits<float>::max()};
    std::memcpy(b+0x0c,values,16);
    const int32_t priority = 2; std::memcpy(b+0x1c,&priority,4);
    b[0x20] = 0;
    b[0x21] = 0;
    return false;
}
int32_t __fastcall Request(void* self, void*, int32_t priority, uint32_t flag) {
    auto b = static_cast<unsigned char*>(self);
    b[0x20] = 1;
    b[0x21] = b[0x21] || static_cast<uint8_t>(flag) ? 1 : 0;
    int32_t previous; std::memcpy(&previous,b+0x1c,4);
    if (priority < previous) std::memcpy(b+0x1c,&priority,4);
    return priority;
}
}
