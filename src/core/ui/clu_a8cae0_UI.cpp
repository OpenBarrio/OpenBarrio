#include "ts3/ui/clu_a8cae0_UI.h"

namespace ts3::ui {

uint32_t UiPredicateA8CAE0::PredicateA8CAE0() const
{
    // TS3W.exe 0x00A8CAE0: read dwords at this+0xB8 and this+0xD0; return 1
    // if stateFlags bit 31 or conditionFlags mask 0xE00 is set, otherwise 0.
    return ((stateFlags & 0x80000000u) != 0 || (conditionFlags & 0xE00u) != 0) ? 1u : 0u;
}

} // namespace ts3::ui
