#pragma once
#include <cstdint>

namespace ts3::speedtree {
// TS3W.exe 0x00F82DA0: construct the global and register its destructor, returning the registration result.
int32_t InitializeGlobal();
}
