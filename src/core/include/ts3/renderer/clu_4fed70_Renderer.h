#pragma once
// Unit clu_4fed70_Renderer — TS3W.exe 0x004FED70 / 0x0083A6B0
// (src/core/renderer/clu_4fed70_Renderer.cpp).
//
// Both entries are argument-shuffling wrappers around the lot-renderer entry 0x004FEA70
// (unit clu_4f35b0_LotRenderer, not rewritten yet, reached through the layout.h bridge):
//   0x004FED70(a1, a2, a3) -> 0x004FEA70(a1, a2, a3, 1)
//   0x0083A6B0(a1)         -> 0x004FED70(a1, 0, 10)
// The arguments are forwarded untouched (word-for-word), so they are typed as the 32-bit
// game words the original moves.

#include <cstdint>

namespace ts3::renderer {

// 0x004FED70 __cdecl, ret 0. Forwards three words and a constant 1 to 0x004FEA70.
std::int32_t Sub4FED70(std::uint32_t a1, std::uint32_t a2, std::uint32_t a3);

// 0x0083A6B0 __cdecl, ret 0. Calls 0x004FED70(a1, 0, 10).
std::int32_t Sub83A6B0(std::uint32_t a1);

}  // namespace ts3::renderer