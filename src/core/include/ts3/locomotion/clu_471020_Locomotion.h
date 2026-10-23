#pragma once

#include <cstdint>

namespace ts3::locomotion {

// TS3W.exe 0x00471020: reports true when the state dword at +0x48 is zero and the byte at +4 is nonzero.
bool HasLocomotionState(const void* self);

}  // namespace ts3::locomotion
