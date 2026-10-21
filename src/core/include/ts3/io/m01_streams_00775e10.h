#pragma once
#include "ts3/io/m01_streams.h"
namespace ts3::io::m01 {
// TS3W.exe 0x00775E10: write a managed string's converted bytes with a base-128 length prefix; preserve partial writes and AL-only success.
int32_t __cdecl WriteStringUTF8(uint32_t handle, uint32_t managed);
}
