#pragma once

#include <cstdint>

namespace ts3::sacs {

// TS3W.exe 0x0040B840: __thiscall (this, requested UTF-16 capacity), ret 4.
uintptr_t __fastcall ReserveClipboardString(void* string, void* edx, uint32_t requested);

// TS3W.exe 0x004E9AA0: __thiscall (this, destination UTF-16 string), ret 4.
uint8_t __fastcall PasteClipboardText(void* state, void* edx, void* destination);

}  // namespace ts3::sacs
