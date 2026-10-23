#pragma once
#include <cstdint>

namespace ts3::locomotion {
// Original 0x0059D440: key-to-bit-mask lookup.
int32_t LookupLocomotionMask(int32_t key);
// Original 0x007D7630: set the locomotion mask field when value > 1.
int32_t* SetLocomotionMask(int32_t value, int32_t* word);
}
