#include "ts3/math/m04_corners.h"
#include <cstdint>
#include <cstring>

namespace ts3::math {
Float4Value* BoxCorners(const UnionBox* bounds, Float4Value* out) noexcept {
    // Access object representations, not floating-point values: even signalling
    // NaNs and subnormals are copied without evaluation or DAZ conversion.
    const auto* source = reinterpret_cast<const unsigned char*>(bounds);
    auto* destination = reinterpret_cast<unsigned char*>(out);
    for (unsigned corner = 0; corner != 8; ++corner) {
        std::uint32_t vector[4];
        std::memcpy(vector, source + ((corner & 4) ? 16 : 0), 4);
        std::memcpy(vector + 1, source + ((corner & 2) ? 20 : 4), 4);
        std::memcpy(vector + 2, source + ((corner & 1) ? 24 : 8), 4);
        vector[3] = 0x3f800000u;
        // Complete this corner before reading bounds for the next one. Character
        // accesses deliberately permit aliasing canonical bounds and output views.
        std::memcpy(destination + corner * 16, vector, 16);
    }
    return out;
}
}
