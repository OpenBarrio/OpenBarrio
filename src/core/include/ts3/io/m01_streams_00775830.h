#pragma once
#include "ts3/io/m01_streams.h"
namespace ts3::io::m01 {
// TS3W.exe 0x00775830: copy a managed buffer into a fixed MemoryStream and transfer a reference to a script handle; roll back on failure.
uint32_t __cdecl CreateFromBuffer(uint32_t managed);
}
