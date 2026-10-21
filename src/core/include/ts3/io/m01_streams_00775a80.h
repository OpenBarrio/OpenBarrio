#pragma once
#include "ts3/io/m01_streams.h"
namespace ts3::io::m01 {
// TS3W.exe 0x00775A80: copy source from offset zero to destination; retain partial progress and AL-only status checks.
int32_t __cdecl CopyStream(uint32_t destination, uint32_t source);
}
