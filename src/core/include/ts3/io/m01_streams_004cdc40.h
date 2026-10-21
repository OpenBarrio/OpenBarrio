#pragma once
#include "ts3/io/m01_streams.h"

namespace ts3::io::m01 {
// TS3W.exe 0x004CDC40: release the buffer, destroy the stream base, optionally delete, and return this. Leaves the buffer field dangling.
MemoryStream* __fastcall DestroyMemory(MemoryStream* self, void*, uint32_t flags);
}
