// Unit clu_798e30_Renderer — TS3W.exe 0x00798E30 / 0x00799620 / 0x0079F710 /
// 0x0079F7E0 / 0x0079FAF0 / 0x0079FB20 / 0x007CFCC0 / 0x007CFDD0 / 0x007D01C0 /
// 0x007D2D40. See the header for the object maps.
//
// Unrewritten callees reached through the bridge:
//   0x00799C40 (table lookup), 0x00799540 (container add), 0x0079FA70 / 0x0079F8C0
//   (list insert helpers), 0x004AECE0 (source decoder), 0x007392A0 (apply step).
// The wide hash uses the image's towlower pointer at 0xF95764 on x86 (loaded once per
// call, exactly like the original); x64 builds fall back to the C runtime towlower.
//
// EA behaviour kept / noted:
//  * 0x007CFCC0 truncates towlower's result to 16 bits (the original instruction sequence).
//  * 0x007CFDD0 folds with a logical shift (the original instruction sequence), not an arithmetic one.
//  * 0x0079F7E0 stores at items[count] and then increments; a corrupt list with count
//    already 17 would write past items (the game only ever builds empty lists).
//  * 0x007D2D40 writes the parse result into its own first argument slot (the compiler
//    reused the dead argument as the return spill) and returns stale EAX on the early
//    exits; the byte value is the only observable output, and that value is in AL.

#include <cstddef>
#include <cstdint>
#include <cstring>
#include <cwchar>
#include <cwctype>

#include "ts3/layout.h"
#include "ts3/renderer/clu_798e30_Renderer.h"

namespace ts3::renderer {
namespace {

constexpr std::uintptr_t kRendererGlobal = 0x011D7530;  // *(void**) = the 0x799C40 `this`
constexpr std::uintptr_t kTowlowerSlot = 0x00F95764;    // wint_t (__cdecl*)(wint_t)
constexpr std::uint32_t kTableMagic = 0x5252544Du;      // 'MTRR'

inline std::uint32_t Read32(const void* base, std::size_t offset) {
  return *reinterpret_cast<const std::uint32_t*>(static_cast<const std::uint8_t*>(base) +
                                                 offset);
}
inline void Write32(void* base, std::size_t offset, std::uint32_t value) {
  *reinterpret_cast<std::uint32_t*>(static_cast<std::uint8_t*>(base) + offset) = value;
}

// towlower through the image's import pointer; the original loads it once before the
// loop and calls it through EBP. x64 builds (no game image) use the CRT's.
inline std::uint16_t FoldToLower(std::uint16_t c, wint_t(__cdecl* fn)(wint_t)) {
#if defined(_M_IX86)
  return static_cast<std::uint16_t>(fn ? fn(static_cast<wint_t>(c)) : c);
#else
  (void)fn;
  return static_cast<std::uint16_t>(std::towlower(static_cast<wint_t>(c)));
#endif
}

}  // namespace

// ---------------------------------------------------------------------------------
// 0x00798E30
// ---------------------------------------------------------------------------------
std::int32_t Holder798E30::GetEntry(std::uint32_t index) {
  const auto* const elements = static_cast<const std::uint8_t*>(this->table);
  const std::uint32_t element =
      *reinterpret_cast<const std::uint32_t*>(elements + (index + 1) * 16);
  if (*reinterpret_cast<const std::uint32_t*>(elements) == kTableMagic) {
    // mov ecx, [0x11d7530]; jmp 0x799c40
    void* const target = *reinterpret_cast<void* const*>(kRendererGlobal);
    return ts3::CallGameMethod<std::int32_t, 0x00799C40>(target, element);
  }
  return static_cast<std::int32_t>(element);
}

// ---------------------------------------------------------------------------------
// 0x00799620
// ---------------------------------------------------------------------------------
std::int32_t Container799620::Add(std::uint32_t a1, void* a2, std::uint32_t a3) {
  // The original re-pushes its own ECX; as a member function `this` is that ECX.
  return ts3::CallGameMethod<std::int32_t, 0x00799540>(this, a1, 2u, a2, a3);
}

// ---------------------------------------------------------------------------------
// 0x0079F710
// ---------------------------------------------------------------------------------
ItemList* ItemList::Init() {
  count = 0;
  std::memset(items, 0, 0x44);
  return this;
}

// ---------------------------------------------------------------------------------
// 0x0079F7E0
// ---------------------------------------------------------------------------------
void ItemList::BuildFrom(const void* src) {
  if (!src) return;
  std::uint32_t remaining = Read32(src, 0x1C);
  const auto* const base = static_cast<const std::uint8_t*>(src);
  const auto* record = *reinterpret_cast<const std::uint8_t* const*>(base + 0x18) + 6;
  if (remaining > 17) remaining = 17;
  if (!remaining) return;
  static const std::uint32_t kOpcodeMap[7] = {0, 1, 6, 3, 4, 2, 5};
  do {
    const std::uint8_t opcode = record[0];
    const std::uint32_t hi = record[1];
    if (opcode <= 6) {
      const std::uint32_t value = kOpcodeMap[opcode] | (hi << 8);
      Write32(this, 4 + static_cast<std::size_t>(count) * 4, value);
      ++count;
    }
    record += 8;
    --remaining;
  } while (remaining);
}

// ---------------------------------------------------------------------------------
// 0x0079FAF0
// ---------------------------------------------------------------------------------
std::int32_t ItemList::Add3(std::uint32_t a1, std::uint32_t a2, std::uint32_t a3) {
  const std::int32_t made = ts3::CallGameMethod<std::int32_t, 0x0079FA70>(this, a2, a3);
  return ts3::CallGameMethod<std::int32_t, 0x0079F8C0>(this, a1, made);
}

// ---------------------------------------------------------------------------------
// 0x0079FB20
// ---------------------------------------------------------------------------------
std::int32_t ItemList::AddFrom(std::uint32_t a1, std::uint32_t a2, std::uint32_t a3) {
  ItemList temporary;  // the original zeroes all 72 bytes before BuildFrom
  temporary.Init();
  temporary.BuildFrom(reinterpret_cast<const void*>(static_cast<std::uintptr_t>(a1)));
  const std::int32_t made = ts3::CallGameMethod<std::int32_t, 0x0079FA70>(this, a2, a3);
  return ts3::CallGameMethod<std::int32_t, 0x0079F8C0>(
      this, reinterpret_cast<std::uintptr_t>(&temporary), made);
}

// ---------------------------------------------------------------------------------
// 0x007CFCC0 / 0x007CFDD0
// ---------------------------------------------------------------------------------
std::int32_t HashWide7CFCC0(const wchar_t* text, std::uint32_t count) {
#if defined(_M_IX86)
  wint_t(__cdecl* const toLower)(wint_t) =
      *reinterpret_cast<wint_t(__cdecl**)(wint_t)>(kTowlowerSlot);
#else
  wint_t(__cdecl* const toLower)(wint_t) = nullptr;
#endif
  std::uint32_t hash = 0x811C9DC5u;  // FNV offset basis
  for (; count; --count, ++text) {
    const std::uint16_t unit = static_cast<std::uint16_t>(*text);
    hash = (hash * 0x01000193u) ^ FoldToLower(unit, toLower);
  }
  return static_cast<std::int32_t>(hash);
}

std::int32_t FoldWideHash7CFDD0(const wchar_t* text) {
  std::uint32_t hash =
      static_cast<std::uint32_t>(HashWide7CFCC0(text, static_cast<std::uint32_t>(std::wcslen(text))));
  return static_cast<std::int32_t>((hash & 0x00FFFFFFu) ^ (hash >> 24));
}

// ---------------------------------------------------------------------------------
// 0x007D01C0
// ---------------------------------------------------------------------------------
std::uint8_t ParserHost7D01C0::ApplyParsed(void* parser, const void* source) {
  auto* const self = reinterpret_cast<std::uint8_t*>(this);
  void* const renderer = *reinterpret_cast<void* const*>(self + 0x48C);
  if (!renderer || !parser) return 0;

  std::uint32_t decoded[4] = {0, 0, 0, 0};
  ts3::CallGameFunction<std::int32_t, 0x004AECE0>(
      static_cast<std::uint32_t>(reinterpret_cast<std::uintptr_t>(decoded)),
      static_cast<std::uint32_t>(reinterpret_cast<std::uintptr_t>(source)), 0x1F886EADu, 0u,
      0u, 0u);

  void* parsed = nullptr;
  const std::uint8_t parseOk = ts3::CallGameVirtual<std::uint8_t, 13>(
      parser, static_cast<std::uint32_t>(reinterpret_cast<std::uintptr_t>(decoded)),
      static_cast<std::uint32_t>(reinterpret_cast<std::uintptr_t>(&parsed)), 2u, 2u, 1u, 0u);
  if (parseOk == 0) {
    if (parsed) ts3::CallGameVirtual<void, 2>(parsed);
    return 0;
  }
  if (!parsed) return 0;

  const std::uint32_t token = ts3::CallGameVirtual<std::uint32_t, 9>(renderer, 0u);
  ts3::CallGameVirtual<void, 10>(renderer, 0u, 0u);
  const std::uint32_t applied = ts3::CallGameVirtual<std::uint32_t, 6>(parsed);
  std::uint8_t result = 0;
  if (applied != 0 &&
      ts3::CallGameFunction<std::uint8_t, 0x007392A0>(renderer, applied) != 0) {
    result = 1;
  }
  ts3::CallGameVirtual<void, 15>(
      parser, static_cast<std::uint32_t>(reinterpret_cast<std::uintptr_t>(parsed)));
  ts3::CallGameVirtual<void, 10>(renderer, token, 0u);
  if (parsed) ts3::CallGameVirtual<void, 2>(parsed);
  return result;
}

// ---------------------------------------------------------------------------------
// 0x007D2D40
// ---------------------------------------------------------------------------------
void FillBlocks7D2D40(std::uint8_t* dst, std::int32_t stride, std::uint8_t value,
                      std::size_t blockSize, std::int32_t rows) {
  if (!blockSize || !rows) return;
  std::int32_t step = stride;
  if (step < 0) {
    dst += static_cast<std::ptrdiff_t>(static_cast<std::int64_t>(step) * (rows - 1));
    step = -step;
  }
  if (static_cast<std::size_t>(step) == blockSize) {
    std::memset(dst, value, blockSize * static_cast<std::size_t>(rows));
    return;
  }
  do {
    std::memset(dst, value, blockSize);
    dst += step;
    --rows;
  } while (rows);
}

}  // namespace ts3::renderer