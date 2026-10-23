#include "ts3/world/clu_82a720_World.h"

namespace ts3::world {
// 0x0082A720, __thiscall, ret 4; only AL is the return contract.
bool WorldGate82A720::Accept(WorldCandidate82A720* candidate)
{
    if (gated && active) {
        if (!allowed)
            return false;
        pending = 0;
    }
    if (!candidate || candidate->identifier == 0)
        return false;
#if defined(_M_IX86)
    const uint32_t mask = *reinterpret_cast<const uint32_t*>(0x011e90fc);
#else
    AbortNotRewritten("WorldGate82A720 eligibility mask at 0x011e90fc");
    const uint32_t mask = 0;
#endif
    if ((candidate->flags & mask) != 0)
        return true;
    // This dependency returns a full int, not a boolean in AL.
    return CallGameMethod<int, 0x0068ec50>(candidate) != 0;
}
} // namespace ts3::world
