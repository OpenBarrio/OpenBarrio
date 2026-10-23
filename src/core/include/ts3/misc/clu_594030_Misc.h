#pragma once
#include <cstdint>

namespace ts3::misc {
// TS3W.exe 0x00594030: store two dwords in the global pair table and return the unchecked index; x86 address wrap is preserved.
int32_t SetGlobalPair(int32_t index, uint32_t first, uint32_t second);
} // namespace ts3::misc
