#pragma once
#include "ts3/io/m01_streams.h"
namespace ts3::io::m01 {
// TS3W.exe 0x004DEAC0: sets memory stream options, preserving NaNs, SSE integer-indefinite conversions and dispatch EAX returns.
uint32_t __fastcall MemorySetOption(MemoryStreamLayout* self, void*, uint32_t option, float value);
}
