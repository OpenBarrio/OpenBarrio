#pragma once
#include "ts3/io/m01_streams.h"
namespace ts3::io::m01 {
// TS3W.exe 0x00775BA0: allocate a managed array and read the whole stream from zero; failed reads leave the cursor changed.
uint32_t __cdecl ToArray(uint32_t handle);
}
