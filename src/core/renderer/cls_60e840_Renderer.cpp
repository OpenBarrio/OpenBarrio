// Unit cls_60e840_Renderer — TS3W.exe 0x0060E840 / 0x00694120 / 0x006946A0 / 0x006E89F0 /
// 0x006EB020 / 0x006EEAF0 / 0x006F3C70. See the header for the object map.
//
// Every callee that is not yet rewritten is reached through the layout.h bridge, so both
// x86 targets run the game code and x64 targets fail closed. Game-global helpers mirror
// clu_5a00a0_Routing.cpp (no-ops on x64).
//
// EA behaviour kept on purpose (see the unit report):
//  * 0x006E89F0 binds the descriptor through 0x00722F50 even when the object came back
//    null, and runs 0x00722FC0 unconditionally on the release paths (null-unsafe).
//  * 0x006E89F0 leaves the spike object global 0x11D1870 dangling (not zeroed) when the
//    face descriptor fails to load.
//  * 0x006EB020 returns with the caller's upper EAX bytes intact (the original instruction sequence); the
//    rewrite preserves them on the handled path and cannot recover them otherwise.
//  * 0x006EEAF0 returns the last free's result, which is stale EAX for an empty list.

#include <cstdint>
#include <cstring>

#include "ts3/layout.h"
#include "ts3/renderer/cls_60e840_Renderer.h"

namespace ts3::renderer {
namespace {

// ---- game globals (x86 only; fail-closed elsewhere, like clu_5a00a0_Routing) ----
constexpr std::uintptr_t kCallbackHost = 0x11CECC8;    // 0x0060E2C0's singleton
constexpr std::uintptr_t kRegistryObject = 0x1158948;  // 0x006940A0's `this`
constexpr std::uintptr_t kRegistryFlag = 0x1158988;    // byte set by 0x006946A0
constexpr std::uintptr_t kRenderer = 0x11D7530;        // texture owner for 0x0079A160
constexpr std::uintptr_t kSettings = 0x11CDDE0;        // 0x00523860's `this`
constexpr std::uintptr_t kReadyFlag = 0x11D188C;       // byte: 0x006E89F0 has run
constexpr std::uintptr_t kSpikeTexture = 0x11D1884;
constexpr std::uintptr_t kSpikeObject = 0x11D1870;
constexpr std::uintptr_t kFaceTexture = 0x11D1880;
constexpr std::uintptr_t kFaceObject = 0x11D186C;
constexpr std::uintptr_t kShimmerTexture = 0x11D187C;
constexpr std::uintptr_t kShimmerObject = 0x11D1874;
constexpr std::uintptr_t kSeaTexture = 0x11D1888;
constexpr std::uintptr_t kSeaObject = 0x11D1878;

// x86 targets read and write the game's memory; on x64 the helpers fail closed (see
// clu_5a00a0_Routing.cpp for the same pattern).
inline void* GameGlobalPointer(std::uintptr_t address) {
#if defined(_M_IX86)
  return *reinterpret_cast<void* const*>(address);
#else
  (void)address;
  return nullptr;
#endif
}
inline void GameGlobalSet32(std::uintptr_t address, std::uint32_t value) {
#if defined(_M_IX86)
  *reinterpret_cast<std::uint32_t*>(address) = value;
#else
  (void)address;
  (void)value;
#endif
}
inline void GameGlobalSet8(std::uintptr_t address, std::uint8_t value) {
#if defined(_M_IX86)
  *reinterpret_cast<std::uint8_t*>(address) = value;
#else
  (void)address;
  (void)value;
#endif
}
inline std::uint8_t GameGlobal8(std::uintptr_t address) {
#if defined(_M_IX86)
  return *reinterpret_cast<const std::uint8_t*>(address);
#else
  (void)address;
  return 1;  // pretend the initialiser ran: nothing to do without the game
#endif
}
// Pointer <-> the 32-bit dwords the game stores (x86 only, like BitsU32 in the routing
// unit). The 64-bit targets never store game pointers through these.
inline std::uint32_t Bits(const void* p) {
  return static_cast<std::uint32_t>(reinterpret_cast<std::uintptr_t>(p));
}
inline void* Ptr(std::uint32_t bits) {
  return reinterpret_cast<void*>(static_cast<std::uintptr_t>(bits));
}
// A string living in the game image.
template <unsigned kAddress>
inline const void* GamePtr() {
  return reinterpret_cast<const void*>(static_cast<std::uintptr_t>(kAddress));
}
// 0x006EEAF0 walks the intrusive list head at `this`: [node] = next, [node+8] = payload.
inline void* ListLink(const void* p) {
#if defined(_M_IX86)
  void* value = nullptr;
  std::memcpy(&value, p, sizeof value);  // one 32-bit game word
  return value;
#else
  (void)p;
  return nullptr;
#endif
}

}  // namespace

std::uint32_t RendererCallbackHost::SetCallback(std::uint32_t fn, std::uint32_t userdata) {
  callback = reinterpret_cast<const void*>(static_cast<std::uintptr_t>(fn));
  this->userdata = Ptr(userdata);
  return fn;
}

std::int32_t ResetRendererCallback694120() {
  auto* host = ts3::CallGameFunction<void*, 0x0060E2C0>();
  // The original stores (0,0) then ends with xor eax,eax; SetCallback's return
  // value is that same constant 0, so one expression covers both.
  return static_cast<std::int32_t>(
      static_cast<RendererCallbackHost*>(host)->SetCallback(0, 0));
}

SceneManager29* SceneManager29::InstallCallback() {
  auto* host = ts3::CallGameFunction<void*, 0x0060E2C0>();
  static_cast<RendererCallbackHost*>(host)->SetCallback(
      0x00694690, static_cast<std::uint32_t>(reinterpret_cast<std::uintptr_t>(this)));
  ts3::CallGameMethod<void, 0x006940A0>(reinterpret_cast<void*>(kRegistryObject));
  GameGlobalSet8(kRegistryFlag, 1);
  return this;
}

std::int32_t SceneManager29::HandleMessage(std::int32_t message,
                                            [[maybe_unused]] std::int32_t arg) {
  // 0x006EB020 reads only the message dword; arg is dropped by the original.
  std::uint32_t stale{};  // value-init: an unhandled message has no handler result
  if (static_cast<std::uint32_t>(message) == 0xFC6AA1FEu)
    stale = ts3::CallGameMethod<std::uint32_t, 0x006E9270>(this);
  // The original writes only AL, leaving the upper bytes of the event handler's result;
  // keep them so both sides agree byte for byte.
  return static_cast<std::int32_t>((stale & 0xFFFFFF00u) | 1u);
}

std::int32_t SceneManager29::ClearNodeList() {
  // x64 has no 32-bit list: ListLink is fail-closed and the first game call stops
  // through layout.h, so there is no separate fake-return branch to maintain.
  void* const head = static_cast<void*>(this);
  void* node = ListLink(head);
  std::int32_t result = 0;
  while (node != head) {
    void* const current = node;
    void* const payload = ListLink(static_cast<const std::uint8_t*>(current) + 8);
    node = ListLink(current);
    if (payload) ts3::CallGameMethod<void, 0x0059A990>(payload);
    result = ts3::CallGameFunction<std::int32_t, 0x004E3F70>(current);
  }
  return result;
}

SceneManager29* SceneManager29::DeletingDtor(std::uint32_t flags) {
  ts3::CallGameMethod<void, 0x006F1800>(this);
  if (flags & 1u) ts3::CallGameFunction<std::int32_t, 0x004E3F70>(static_cast<void*>(this));
  return this;
}

void InitPostProcess6E89F0() {
#if defined(_M_IX86)
  // 0x004AECE0 — descriptor loader; the caller checks the low byte of the result.
  const auto LoadDescriptor = [](void* buf, const void* name, void* extra) {
    return ts3::CallGameFunction<std::uint8_t, 0x004AECE0>(
        buf, name, std::uint32_t{0x00B2D882}, extra, std::uint32_t{0}, std::uint32_t{0});
  };
  // 0x0079A160 — texture id, callee-pops.
  const auto TextureId = [](void* renderer, const void* name) {
    return ts3::CallGameMethod<std::uint32_t, 0x0079A160>(renderer, name, 1u, 5u);
  };
  // 0x004E3F90 + 0x00723000 — 24-byte post process object.
  const auto NewPostProcess = [] {
    void* const mem =
        ts3::CallGameFunction<void*, 0x004E3F90>(24u, GamePtr<0x00FF8464>(), 0u, 0u, 0u, 0u);
    return mem ? ts3::CallGameMethod<void*, 0x00723000>(mem) : static_cast<void*>(nullptr);
  };
  // 0x00722F50 — bind the descriptor; the original passes the object without a null
  // check even though the allocator can fail.
  const auto BindDescriptor = [](void* obj, void* buf) {
    ts3::CallGameMethod<void, 0x00722F50>(obj, buf, 0u, 0u, 0u);
  };
  // The original's per-object teardown: 0x00722FC0 runs unconditionally, then the
  // destructor and free only when the global still holds an object.
  const auto ReleaseObject = [](std::uintptr_t global) {
    void* obj = GameGlobalPointer(global);
    ts3::CallGameMethod<void, 0x00722FC0>(obj);
    obj = GameGlobalPointer(global);
    if (!obj) return;
    ts3::CallGameMethod<void, 0x00723050>(obj);
    ts3::CallGameFunction<std::int32_t, 0x004E3F70>(obj);
  };

  if (GameGlobal8(kReadyFlag) != 0) return;
  void* const renderer = GameGlobalPointer(kRenderer);
  std::uint8_t buf[16] = {};

  if (!LoadDescriptor(buf, GamePtr<0x00FF8494>(), nullptr)) return;  // L"MummyCurseSpikes"
  GameGlobalSet32(kSpikeTexture, TextureId(renderer, GamePtr<0x00FF847C>()));
  void* obj = NewPostProcess();
  GameGlobalSet32(kSpikeObject, Bits(obj));
  BindDescriptor(obj, buf);
  if (!LoadDescriptor(buf, GamePtr<0x00FF8444>(), nullptr)) {  // L"MummyCurseFace"
    ReleaseObject(kSpikeObject);  // EA: 0x11D1870 keeps the freed pointer
    return;
  }
  GameGlobalSet32(kFaceTexture, TextureId(renderer, GamePtr<0x00FF8430>()));
  obj = NewPostProcess();
  GameGlobalSet32(kFaceObject, Bits(obj));
  BindDescriptor(obj, buf);

  void* extra = nullptr;
  const void* shimmerName;
  if (ts3::CallGameMethod<std::uint8_t, 0x00523860>(reinterpret_cast<void*>(kSettings),
                                                    std::uint32_t{0x8000}) != 0) {
    extra = ts3::CallGameFunction<void*, 0x007D7680>(16u);
    shimmerName = GamePtr<0x00FF840C>();  // L"EP08_heatshimmer"
  } else {
    shimmerName = GamePtr<0x00FF83F4>();  // L"heatshimmer"
  }
  if (!LoadDescriptor(buf, shimmerName, extra)) {
    ReleaseObject(kSpikeObject);
    GameGlobalSet32(kSpikeObject, 0);
    ReleaseObject(kFaceObject);
    GameGlobalSet32(kFaceObject, 0);
    return;
  }
  GameGlobalSet32(kShimmerTexture, TextureId(renderer, GamePtr<0x00FF83E0>()));
  obj = NewPostProcess();
  GameGlobalSet32(kShimmerObject, Bits(obj));
  BindDescriptor(obj, buf);

  if (ts3::CallGameMethod<std::uint8_t, 0x00523860>(reinterpret_cast<void*>(kSettings),
                                                    std::uint32_t{0x40000}) == 0) {
    GameGlobalSet8(kReadyFlag, 1);  // EA: the flag is set even without the sea object
    return;
  }
  if (!LoadDescriptor(buf, GamePtr<0x00FF83C0>(), nullptr)) {  // L"seaWaterSigned"
    ReleaseObject(kSpikeObject);
    GameGlobalSet32(kSpikeObject, 0);
    ReleaseObject(kFaceObject);
    GameGlobalSet32(kFaceObject, 0);
    ReleaseObject(kShimmerObject);
    GameGlobalSet32(kShimmerObject, 0);
    return;
  }
  GameGlobalSet32(kSeaTexture, TextureId(renderer, GamePtr<0x00FF83B0>()));
  obj = NewPostProcess();
  GameGlobalSet32(kSeaObject, Bits(obj));
  BindDescriptor(obj, buf);
  GameGlobalSet8(kReadyFlag, 1);
#endif
}

}  // namespace ts3::renderer
