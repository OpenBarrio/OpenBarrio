#pragma once
#include "ts3/io/memory_stream.h"

namespace ts3::io::m01 {
// TS3W.exe 0x004DEDE0: attaches a shared holder, sets size/capacity and resets position; self-attach still resets size/position.
bool __fastcall AttachShared(MemoryStream* self, void*, SharedBuffer* buffer, uint32_t size);
}
