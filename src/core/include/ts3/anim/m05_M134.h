#pragma once
#include <cstdint>
namespace ts3::anim::m134 {
// TS3W.exe 0x0084D7E0: Refresh animated-body style distances and flags; preserves early count return and reverse slow-out accumulation.
uintptr_t __fastcall Refresh(void* self, void* unused, uint32_t style, uint32_t flags, uint32_t arg, uint32_t extraFlags, uint8_t a, uint8_t b);
}
