#pragma once
#include <cstdint>

namespace ts3::math {
// TS3W.exe 0x00555060: return the input pointer with the greater signed value, first on ties.
// Both pointers must designate live readable int32 objects; ownership stays with the caller.
const std::int32_t* MaxSignedReference(const std::int32_t* first,
                                     const std::int32_t* second) noexcept;
}
