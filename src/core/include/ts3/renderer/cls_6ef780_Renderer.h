#pragma once
// Unit cls_6ef780_Renderer — TS3W.exe 0x006EF780
// (src/core/renderer/cls_6ef780_Renderer.cpp).
//
// 0x006EF780 is the scene-manager/renderer constructor. It installs the base and
// derived vtables, clears the object, builds the 22-entry culling-flag adapter table
// and the 16-entry visualisation adapter table, registers the object with the global
// config services (0x5A00A0) and reads the mission/quality settings (0x58AAD0). It is
// called from 0x7EB160, 0xD62640 and 0xEC9CD0 and returns `this` on every path.
//
// The object layout is 32-bit x86 only; the x64 build keeps the function as a stub.

#include <cstdint>

#include "ts3/layout.h"

namespace ts3::renderer {

// The constructor writes the 32-bit game layout over [0, 0x558); the object stays an
// opaque byte blob because only the constructor's stores are modelled here.
struct SceneManager6EF780 {
  std::uint8_t raw[0x558];
};
TS3_X86_SIZE(SceneManager6EF780, 0x558);

// TS3W.exe 0x006EF780: 5679-byte scene-manager constructor, __thiscall (no stack
// args, ret 0); returns `self`. It is declared with the game ABI (self in ECX, dummy
// EDX) so the replace hook uses it directly (registration abi="free-function").
void* __fastcall Construct6EF780(void* self, void* /*edx*/);

}  // namespace ts3::renderer