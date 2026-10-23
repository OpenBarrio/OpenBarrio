#pragma once
// Unit cls_60e840_Renderer — TS3W.exe 0x0060E840 / 0x00694120 / 0x006946A0 / 0x006E89F0 /
// 0x006EB020 / 0x006EEAF0 / 0x006F3C70 (src/core/renderer/cls_60e840_Renderer.cpp).
//
// 0x0060E840 stores the renderer's callback pair on the object 0x0060E2C0 hands out
// (global 0x11CECC8); 0x00694120 clears that pair, 0x006946A0 installs a fresh one
// (callback 0x00694690 on the installing object). 0x006E89F0 lazily builds the post
// process objects, 0x006EB020 forwards a renderer event, 0x006EEAF0 tears the node
// list down, 0x006F3C70 is the scalar deleting destructor.
//
// The `this` objects are not all known: 0x006946A0's owner is the object 0x006EF780
// passes in (only forwarded and returned here), the 0x006EB020 / 0x006F3C70 owner is
// the SceneManager_29-like object of this region (vtable 0x00FF91B8). Both are opaque
// below; the rewrites never read their bodies, only pass `this` on.

#include <cstdint>

#include "ts3/layout.h"

namespace ts3::renderer {

// 0x0060E840's `this`; layout of +0x00..+0xEB belongs to clu_60e2c0_Renderer's object.
struct RendererCallbackHost {
  uint8_t leading[0xEC];
  const void* callback;  // +0xEC the thunk to run (0x00694690, 0 for none)
  void* userdata;        // +0xF0 the object to run it on (the installing object)

  // 0x0060E840 __thiscall, ret 8. The game pushes a 32-bit code address and a 32-bit
  // object pointer (x86); the pair is stored as-is and the callback comes back.
  std::uint32_t SetCallback(std::uint32_t fn, std::uint32_t userdata);
};
TS3_X86_OFFSET(RendererCallbackHost, callback, 0xEC);
TS3_X86_OFFSET(RendererCallbackHost, userdata, 0xF0);
TS3_X86_SIZE(RendererCallbackHost, 0xF4);

// The thiscall entries of this region.
struct SceneManager29 {
  // 0x006946A0 __thiscall, ret 0: sets callback 0x00694690 on the callback host to
  // `this`, runs 0x006940A0 on 0x1158948, sets byte 0x1158988, returns `this`.
  SceneManager29* InstallCallback();

  // 0x006EB020 __thiscall, ret 8: renderer event hook. Only 0xFC6AA1FE is handled
  // (0x006E9270). Returns 1; the original leaves the upper EAX bytes stale.
  std::int32_t HandleMessage(std::int32_t message, std::int32_t arg);

  // 0x006EEAF0 __thiscall, ret 0: walks the intrusive list at `*this`, tearing each
  // node's payload down (0x0059A990) and freeing the node (0x004E3F70). Returns the
  // last free's result; the original leaves EAX stale when the list is empty.
  std::int32_t ClearNodeList();

  // 0x006F3C70 __thiscall, ret 4: scalar deleting destructor (0x006F1800 then free
  // when bit 0 of `flags` is set), returns `this`.
  SceneManager29* DeletingDtor(std::uint32_t flags);
};

// 0x00694120 __cdecl, ret 0: clears the callback pair on the callback host, returns 0.
std::int32_t ResetRendererCallback694120();

// 0x006E89F0 __cdecl, ret 0: one-shot initialiser of the post process objects
// (curse spikes / curse face / heat shimmer / under sea), gated by byte 0x11D188C.
void InitPostProcess6E89F0();

}  // namespace ts3::renderer
