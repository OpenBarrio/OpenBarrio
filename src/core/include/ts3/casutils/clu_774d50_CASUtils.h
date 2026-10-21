// CAS utility helpers from TS3W.exe 0x00774D50..0x00774F23.
#pragma once

#include <cstdint>

namespace ts3::casutils {

// 0x00774D50, __cdecl, ret 0. Writes the low 1/2/3/4 bytes according to size.
uint8_t StoreBytes774D50(void* destination, uint32_t value, int32_t size);

// 0x00774E30, __cdecl, ret 0. Invalidates the cached CAS object when identifiers match.
uint32_t ResolveCasObject774E30(uint32_t id);

// 0x00774E90, __cdecl, ret 0. Stores a value in a CAS object property.
int32_t SetCasProperty774E90(uint32_t id, uint32_t value, int32_t size);

}  // namespace ts3::casutils
