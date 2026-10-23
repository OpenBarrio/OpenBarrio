// Unit clu_718cb0_Renderer — TS3W.exe 0x00718CB0 / 0x00723000 / 0x00F81D70. See the
// header for the object map.
//
// All callees are unrewritten units and go through the layout.h bridge:
//   0x00723560/0x007235B0 (clu_718cb0's siblings elsewhere), 0x006E8330 (renderer
//   context getter), 0x006FA850, 0x006C93D0, 0x00692770, 0x006FAC50 (list node helpers).
// The render target's virtual slots are read with CallGameVirtual.
//
// EA behaviour kept on purpose:
//  * the value handed to virtual slot 16 is built with the original instruction sequence after the original instruction sequence, so
//    only AL carries the decision and the upper EAX bytes are stale. This rewrite passes
//    the zero-extended boolean; only the low byte is meaningful and the game's callee
//    reads no more (noted in the report).
//  * the float gate is `fcompi`/`ja`: 0.2f > result sends the object down the same
//    "hidden" path as a found node; NaN falls through into the list walk. The condition
//    is written as !(0.2f > f) so NaN keeps the original path.
//  * the list walk's second check tests bit 2 of the byte at [node->+0x38 + 0x38], not of
//    the node.

#include <cstdint>

#include "ts3/layout.h"
#include "ts3/renderer/clu_718cb0_Renderer.h"

namespace ts3::renderer {
namespace {

// Game globals (x86 addresses in the mapped image).
constexpr std::uintptr_t kBlobA = 0x011D0DA8;
constexpr std::uintptr_t kBlobB = 0x011D0DC0;
constexpr std::uintptr_t kAtexitImport = 0x00F7E9A1;
constexpr std::uintptr_t kAtexitThunk = 0x00F921F0;

inline std::uint32_t Read32(const void* base, std::size_t offset) {
  return *reinterpret_cast<const std::uint32_t*>(static_cast<const std::uint8_t*>(base) + offset);
}
inline std::uint8_t Read8(const void* base, std::size_t offset) {
  return *reinterpret_cast<const std::uint8_t*>(static_cast<const std::uint8_t*>(base) + offset);
}
inline void* ReadPtr(const void* base, std::size_t offset) {
  return *reinterpret_cast<void* const*>(static_cast<const std::uint8_t*>(base) + offset);
}

}  // namespace

Blob20* Blob20::Clear() {
  words[0] = 0;
  words[1] = 0;
  words[2] = 0;
  words[3] = 0;
  words[4] = 0;
  return this;
}

std::int32_t InitF81D70() {
#if defined(_M_IX86)
  // mov ecx,0x11d0da8; call 0x723000 / mov ecx,0x11d0dc0; call 0x723000
  reinterpret_cast<Blob20*>(kBlobA)->Clear();
  reinterpret_cast<Blob20*>(kBlobB)->Clear();
  // push 0xf921f0; call 0xf7e9a1; pop ecx
  return ts3::CallGameFunction<std::int32_t, kAtexitImport>(
      reinterpret_cast<void*>(kAtexitThunk));
#else
  ts3::AbortNotRewritten("clu_718cb0_Renderer global init");
  return 0;
#endif
}

void RendererState718::UpdateVisibility() {
  auto* const self = reinterpret_cast<std::uint8_t*>(this);
  void* const target = ReadPtr(self, 0x210);
  if (!target) return;
  // (*(u8 (__thiscall**)(void*))(*target + 0x10))(target)
  if (ts3::CallGameVirtual<std::uint8_t, 4>(target) == 0) return;
  if (Read8(self, 0x217) != 0) {
    // (*(void (__thiscall**)(void*, int))(*target + 0x40))(target, 0)
    ts3::CallGameVirtual<void, 16>(target, 0);
    return;
  }

  void* const context = ts3::CallGameFunction<void*, 0x006E8330>();
  const std::uint8_t contextFlag = context ? Read8(context, 0x2A2) : 0;

  const std::uint32_t flags = Read32(self, 0x44);
  bool visible;
  if ((flags & 1u) == 0 && contextFlag == 0) {
    visible = false;
  } else {
    visible = true;
    void* const node = ReadPtr(self, 0x38);
    if (node) {
      if ((flags & 0x80u) == 0) {
        visible = (Read32(node, 0x30) & 8u) != 0;
        if ((Read32(node, 0x30) & 8u) == 0) goto store;
      }
      if ((flags & 4u) == 0)
        visible = (Read32(node, 0x38) & 4u) == 0;
      if (!visible) goto store;
      {
        const std::uint32_t probe = ts3::CallGameFunction<std::uint32_t, 0x006FA850>();
        if (static_cast<std::uint8_t>(probe) == 0 && contextFlag == 0) {
          visible = false;
          goto store;
        }
      }
      void* const item = ReadPtr(self, 0x190);
      visible = true;
      if (item) {
        if ((ts3::CallGameMethod<std::uint8_t, 0x006C93D0>(item) & 0x80u) == 0) {
          visible = (Read32(node, 0x30) & 8u) != 0;
          if ((Read32(node, 0x30) & 8u) == 0) goto store;
        }
      }
    }

    // Walk the intrusive list at +0x190 while the frame's 0.2f gate holds.
    {
      void* walk = ReadPtr(self, 0x190);
      if (walk) {
        const float gate = ts3::CallGameMethod<float, 0x00692770>(walk);
        if (!(0.2f > gate)) {
          for (;;) {
            if ((ts3::CallGameMethod<std::uint8_t, 0x006C93D0>(walk) & 4u) == 0) {
              void* const inner = ReadPtr(walk, 0x38);
              if (inner && (Read8(inner, 0x38) & 4u) != 0) {
                visible = false;
                goto store;
              }
            }
            walk = ts3::CallGameMethod<void*, 0x006FAC50>(walk);
            if (!walk) goto afterList;
          }
        }
        visible = false;
        goto store;
      }
    }

  afterList:
    {
      void* const context2 = ts3::CallGameFunction<void*, 0x006E8330>();
      if (!context2) goto store;
      if (Read8(context2, 0x515) == 0 || Read8(context2, 0x270) != 0) {
        if (Read8(context2, 0x514) == 0) goto tailCheck;
      } else {
        if (Read8(context2, 0x514) == 0) {
          if (static_cast<std::int32_t>(Read32(self, 0x1B8)) < 0) goto store;
          visible = false;
          goto store;
        }
      }
      {
        const std::uint8_t state = Read8(self, 0x208);
        if ((state & 0x40u) != 0 && static_cast<std::int8_t>(state) >= 0) {
          visible = false;
          goto store;
        }
      }
    tailCheck:
      if (static_cast<std::int32_t>(Read32(self, 0x1B8)) >= -1) goto store;
      visible = false;
    }
  }

store:
  // LOBYTE(v3) = !v4 (upper EAX bytes are stale in the original)
  ts3::CallGameVirtual<void, 16>(target, visible ? 0 : 1);
}

}  // namespace ts3::renderer