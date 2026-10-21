#pragma once
#include "ts3/io/m01_streams.h"

namespace ts3::io::m01 {
// TS3W.exe 0x00774DC0: look up a tagged stream handle and cache a borrowed successful result; failures preserve stale cache cells.
IStream* __cdecl LookupStream(uint32_t handle);
}
