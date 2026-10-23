#pragma once
// Unit clu_417130_Renderer — TS3W.exe 0x00417130 (src/core/renderer/clu_417130_Renderer.cpp).
//
// 0x00417130 is a one-shot static initialiser: the first call sets the guard byte
// 0x11B02A0 and runs 0x00446270 on the data object 0x1146108; later calls do nothing.
// The function returns whatever 0x00446270 returned on the initialising path and stale
// EAX otherwise (the four callers all ignore it).

#include <cstdint>

namespace ts3::renderer {

// 0x00417130 __cdecl, ret 0. Guarded one-shot initialiser described above.
std::int32_t Init417130();

}  // namespace ts3::renderer