#pragma once

#include <cstdint>

namespace ts3::gameutils {

// TS3W.exe 0x00799D10: __thiscall (this) -> int.
int32_t __fastcall AdvanceState(void* self, void* edx);

// TS3W.exe 0x00799D60: __thiscall (this) -> void.
void __fastcall ClearMirror(void* self, void* edx);

}  // namespace ts3::gameutils
