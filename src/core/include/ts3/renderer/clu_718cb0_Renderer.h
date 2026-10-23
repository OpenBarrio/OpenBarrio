#pragma once
// Unit clu_718cb0_Renderer — TS3W.exe 0x00718CB0 / 0x00723000 / 0x00F81D70
// (src/core/renderer/clu_718cb0_Renderer.cpp).
//
// 0x00723000 is the 20-byte POD zeroer used for the two globals below and by other
// renderer objects (18 callers); 0x00F81D70 is a static initialiser that clears those two
// globals and registers the teardown thunk 0x00F921F0 through the image's atexit import.
// 0x00718CB0 is the renderer's per-frame visibility decision: it asks the render target
// 0x00718CB0's owner at +0x210 whether the frame is active, derives a boolean from the
// camera/flags state and the list at +0x190, and writes that boolean back through virtual
// slot 16 of the same object (the early return for byte +0x217 pushes 0).

#include <cstdint>

#include "ts3/layout.h"

namespace ts3::renderer {

// 0x00723000's `this`: five 32-bit words, all cleared. Returned in EAX.
struct Blob20 {
  std::uint32_t words[5];

  // 0x00723000 __thiscall, ret 0. Zeroes all five words and returns `this`.
  Blob20* Clear();
};
TS3_X86_SIZE(Blob20, 20);

// The `this` of 0x00718CB0; only the fields the decision reads/writes are named, the
// rest stays opaque. +0x210 is the render target, +0x190 the intrusive list head,
// +0x38/+0x44/+0x208/+0x217/+0x1B8 the state words the original inspects.
struct RendererState718 {
  std::uint8_t opaque[0x2C0];

  // 0x00718CB0 __thiscall, ret 0. Per-frame visibility update (see the unit header).
  void UpdateVisibility();
};

// 0x00F81D70 __cdecl, ret 0. Clears the two 20-byte globals and calls the image's
// atexit import (0x00F7E9A1) with 0x00F921F0. Returns the import's result.
std::int32_t InitF81D70();

}  // namespace ts3::renderer