#pragma once
#include "ts3/io/m01_streams.h"
namespace ts3::io::m01 {
// TS3W.exe 0x00776180: read a length-prefixed UTF8 string into a managed string. Prefix-read failure returns success; invalid handles leave output untouched.
int32_t __cdecl ReadStringUTF8(uint32_t handle, uint32_t* result);
}
