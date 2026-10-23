#pragma once
#include <cstddef>
#include <cstdint>

namespace ts3::ui {

// TS3W.exe 0x00722F00: returns TRUE when this+0x10 is non-null and its low two bits are clear.
struct UiFlagPredicate {
    uint8_t pad00[0x10];
    uintptr_t handle;
    bool IsAlignedHandle() const;
};
static_assert(offsetof(UiFlagPredicate, handle) == 0x10, "game field offset");

} // namespace ts3::ui
