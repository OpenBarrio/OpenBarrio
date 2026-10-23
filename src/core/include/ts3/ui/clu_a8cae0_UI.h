#pragma once

#include <cstdint>

namespace ts3::ui {

struct UiPredicateA8CAE0 {
    uint8_t pad00[0xB8];
    uint32_t stateFlags;
    uint8_t padBC[0x14];
    uint32_t conditionFlags;

    // TS3W.exe 0x00A8CAE0: reports the high state bit or any condition bit in 0xE00.
    uint32_t PredicateA8CAE0() const;
};

} // namespace ts3::ui
