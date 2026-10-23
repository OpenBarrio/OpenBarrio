#pragma once

#include <cstdint>

namespace ts3::lotrenderer {

struct LotRendererC7CCC0 {
    // Opaque prefix through the first accessed state field; remaining extent is not needed by this method.
    unsigned char opaque[0x60];

    // TS3W.exe 0x00C7CCC0: tests whether the requested pair matches this lot renderer and its state is active.
    bool Matches(int32_t first, int32_t second) const;
};

} // namespace ts3::lotrenderer
