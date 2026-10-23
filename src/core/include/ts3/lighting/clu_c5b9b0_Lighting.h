#pragma once

#include <cstdint>

namespace ts3::lighting {

// TS3W.exe 0x00C5B9B0: return 1 iff object flags at +0x90 contain any bit of 0xE400.
std::int32_t __cdecl HasLightingFlags(const void* object);

} // namespace ts3::lighting
