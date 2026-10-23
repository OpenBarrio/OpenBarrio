#include "ts3/anim/m05_M46.h"
#include "ts3/layout.h"
#include "ts3/anim/m05_B07.h"
#include "ts3/anim/m05_B08.h"
#include "ts3/anim/m05_B09.h"
#include <cstring>
namespace ts3::anim {
uintptr_t M46SetKeys(M46Sync* self, const void* keys, const float* durations, const float* weights, uint32_t count) {
    auto* end = static_cast<const unsigned char*>(keys) + size_t(count)*16;
    CallGameMethod<void,0x007E8820>(&self->keys, keys, end, count);
    CallGameMethod<void,0x0056DBE0>(&self->cumulative, count);
    CallGameMethod<void,0x0056DBE0>(&self->weights, count);
    float sum = 0.0f;
    for (uint32_t i=0; i<count; ++i) {
        // Sequential single-precision ADDSS, including the initial addition to +0.
        sum = durations[i] + sum;
        static_cast<float*>(self->cumulative.begin)[i] = sum;
        static_cast<float*>(self->weights.begin)[i] = weights[i];
    }
    self->latch = 0;
    uintptr_t result = CallGameMethod<uintptr_t,0x0056DCF0>(self);
    while (self->index < 2) {
        result = CallGameMethod<uintptr_t,0x0056D470>(self);
        if ((result & 0xff) == 0) break;
    }
    return result;
}
Float4Value* M46Sample(ScratchM46* self, Float4Value* out, uint32_t a, uint32_t b, float t) {
    if (self->source) {
        // The wrapper itself does FLD/FSTP before forwarding (not a bitwise float copy).
#if defined(_M_IX86)
        __asm { fld t }
        __asm { fstp t }
#else
        uint32_t bits; std::memcpy(&bits, &t, 4);
        if ((bits & 0x7f800000u) == 0x7f800000u && (bits & 0x007fffffu)) {
            bits |= 0x00400000u; std::memcpy(&t, &bits, 4);
        }
#endif
        b07::AdvanceE090(out, self->source, a, b, t, 0, nullptr, self->channel);
    } else {
        std::memset(out, 0, sizeof(*out));
    }
    return out;
}
uint8_t M46Read(ScratchM46* self, void* stream) {
    // Stream pointers widen on native builds; status follows the third pointer-sized slot.
    struct Stream { void* vtable; uint32_t value4; void* object; uint32_t valueC; void* context; uint8_t good; };
    auto* s = static_cast<Stream*>(stream);
    void* binding = CallGameFunction<void*,0x004DDBD0>(s->object, uint32_t(1), s->context);
    CallGameMethod<void,0x007D77C0>(s, binding);
    if (!s->good) return 0;
    b08::SaveOptional(static_cast<Controller*>(self->bindings), reinterpret_cast<b08::StreamState*>(s));
    return s->good;
}
uint8_t M46Write(ScratchM46* self, void* stream) {
    struct Stream { void* vtable; uint32_t value4; void* object; uint32_t valueC; void* context; uint8_t good; };
    auto* s = static_cast<Stream*>(stream);
    uint32_t type = ~uint32_t(0);
    void* binding = CallGameFunction<void*,0x004DD700>(s->object, &type, s->context);
    CallGameMethod<void,0x007D77C0>(s, binding);
    if (!s->good || type > 1) return 0;
    void* value = b09impl::ReadController(self->base.hostSlot, reinterpret_cast<b09impl::Reader*>(s));
    CallGameMethod<void,0x00CF3C70>(&self->bindings, value);
    return s->good;
}
ScratchM46* M46Construct(ScratchM46* self, void* host) {
    b08::Construct(&self->base, host);
#if defined(_M_IX86)
    self->base.vtable = reinterpret_cast<void*>(0x00FC2068);
    self->base.secondary8 = reinterpret_cast<void*>(0x00FC2060);
    self->base.secondaryC = reinterpret_cast<void*>(0x00FC205C);
    self->interface28 = reinterpret_cast<void*>(0x00FC1E5C);
#else
    AbortNotRewritten("Scratch controller game vtables");
#endif
    self->source = nullptr;
    self->bindings = nullptr;
    self->channel = 0;
    self->value3c = 0;
    self->value40 = 0;
    self->flags[0] = 0;
    self->flags[1] = 0;
    self->flags[2] = 0;
    self->time = 0.0f;
    return self;
}
void* M46DeleteSecondary(void* secondary, uint32_t flags) {
    // Same adjustment-and-forward as ResourceBaseDestroySecondary, but from interface +0x0c.
    auto* self = reinterpret_cast<unsigned char*>(secondary) - offsetof(Controller, secondaryC);
    return CallGameMethod<void*,0x0056E7A0>(self, flags);
}
}
