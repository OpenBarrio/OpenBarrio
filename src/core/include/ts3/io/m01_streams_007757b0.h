#pragma once
#include "ts3/io/m01_streams.h"

namespace ts3::io::m01 {
// TS3W.exe 0x007757B0: create a resizable empty stream and transfer a reference to a scripting handle, releasing on registration failure.
uint32_t __cdecl Create();
}
