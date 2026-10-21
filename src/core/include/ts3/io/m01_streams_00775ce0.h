#pragma once
#include "ts3/io/m01_streams.h"
namespace ts3::io::m01 {
// TS3W.exe 0x00775CE0: writes a UTF16 array as UTF8 with adapter lifetime; start is checked but ignored.
int32_t __cdecl WriteCharArrayUTF8(uint32_t handle, uint32_t managed, int32_t start, int32_t count);
}
