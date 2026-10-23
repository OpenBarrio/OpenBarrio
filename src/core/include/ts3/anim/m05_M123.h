#pragma once
#include "ts3/layout.h"
namespace ts3::anim {
struct m123 {
    // TS3W.exe 0x00844F40: load locomotion styles, builders, defaults, progressions and weather effects. Keeps AL-only tests/returns and skipFor parsing quirks.
    static uint32_t LoadStyleDefinitions(void* self);
    // Instance adapter for the registered thiscall hook; no native fields are accessed here.
    uint32_t LoadStyleDefinitions() { return LoadStyleDefinitions(this); }
};
}
