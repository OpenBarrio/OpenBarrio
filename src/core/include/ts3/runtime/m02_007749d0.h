#pragma once
#include <cstdint>
namespace ts3::runtime::m02 {
// TS3W.exe 0x007749D0: Allocate a stopped stopwatch and register its tagged handle; negative modes and null registration on allocation failure are retained.
uint32_t __cdecl CreateStopwatch(int32_t mode);
}
