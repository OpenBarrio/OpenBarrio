#pragma once
#include "ts3/io/memory_stream.h"

namespace ts3::io::m01 {
// TS3W.exe 0x00455A00: release a MemoryStream reference; counts <=1 destroy without decrementing (including zero/negative counts).
int32_t __fastcall MemoryRelease(MemoryStream* self, void*);
}
