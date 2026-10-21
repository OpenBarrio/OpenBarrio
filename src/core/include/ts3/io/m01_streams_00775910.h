#pragma once
#include <cstdint>
namespace ts3::io::m01 {
// TS3W.exe 0x00775910: tail-calls handle removal and Release; preserves the helper's EAX, not a bool Close result.
uint32_t Close(uint32_t handle);
}
