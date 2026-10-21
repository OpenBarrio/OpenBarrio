#pragma once
#include "ts3/io/m01_streams.h"
namespace ts3::io::m01 {
// TS3W.exe 0x00776420: read UTF8 into a managed UTF16 buffer; EA validates but ignores start.
int32_t __cdecl ReadCharArrayUTF8(uint32_t handle, uint32_t managed, int32_t start, int32_t count);
}
