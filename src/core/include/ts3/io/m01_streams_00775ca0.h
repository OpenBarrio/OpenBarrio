#pragma once
#include "ts3/io/m01_streams.h"

namespace ts3::io::m01 {
// TS3W.exe 0x00775CA0: allocate a StreamHost binding and register it, even if null; return slot 18's EAX without cleanup.
uint32_t __cdecl RegisterStreamBindings(void* service);
}
