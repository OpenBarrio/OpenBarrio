#include "ts3/io/m01_streams_004deac0.h"
#include <xmmintrin.h>

namespace ts3::io::m01 {
uint32_t __fastcall MemorySetOption(MemoryStreamLayout* self, void*, uint32_t option, float value)
{
    // The default outside the jump table returns option-1; holes return
    // table index 4. Cases 4 and 8 leave their dispatch index in EAX.
    if (option - 1u > 15u)
        return option - 1u;
    switch (option) {
    case 1:
        self->stream.resizable = value != 0.0f; // unordered is true, -0 is false
        return self->stream.resizable ? 1u : 0u;
    case 4:
        if (value < 1.0f) value = 1.0f;
        self->stream.growthFactor = value;
        return 1u;
    case 8:
        if (value < 0.0f) value = 0.0f;
        self->stream.growthIncrement = _mm_cvtt_ss2si(_mm_set_ss(value));
        return 2u;
    case 16:
        // CVTTSS2SI returns INT_MIN for NaN, infinity and overflow, not a
        // saturating conversion; unlike option 8 there is no lower clamp.
        self->option16 = _mm_cvtt_ss2si(_mm_set_ss(value));
        return static_cast<uint32_t>(self->option16);
    default:
        return 4u;
    }
}
}
