#pragma once
#include "ts3/layout.h"

namespace ts3::ui {
// Known prefix only; the initializer also uses storage beyond this prefix.
struct UiValue4f56d0 {
    void* vtable;
    uint32_t untouched;
    uint16_t flags;
    uint16_t size;
    // TS3W.exe 0x004F56D0: initialize the value prefix and delegate its payload initialization.
    UiValue4f56d0* Construct(const void* value);
};
TS3_X86_OFFSET(UiValue4f56d0, vtable, 0);
TS3_X86_OFFSET(UiValue4f56d0, untouched, 4);
TS3_X86_OFFSET(UiValue4f56d0, flags, 8);
TS3_X86_OFFSET(UiValue4f56d0, size, 10);
TS3_X86_SIZE(UiValue4f56d0, 12);
} // namespace ts3::ui
