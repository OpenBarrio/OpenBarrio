#pragma once
#include <cstdint>

namespace ts3::cas {
// TS3W.exe 0x00AB5B60: repeat a 16-byte record, returning the end; preserve sequential overlapping reads and null-slot skipping.
void* Fill16(void* destination, std::uint32_t count, const void* source);
}
