#include "ts3/locomotion/clu_471020_Locomotion.h"

#include <cstring>

namespace ts3::locomotion {

// 0x00471020. The object fields are accessed by byte offset, avoiding an invented
// pointer-bearing layout: +4 is one byte, +0x48 is a 32-bit word on both targets.
bool HasLocomotionState(const void* self)
{
    const auto* bytes = static_cast<const unsigned char*>(self);
    std::uint32_t state = 0;
    std::memcpy(&state, bytes + 0x48, sizeof(state));
    return state == 0 && bytes[4] != 0;
}

}  // namespace ts3::locomotion
