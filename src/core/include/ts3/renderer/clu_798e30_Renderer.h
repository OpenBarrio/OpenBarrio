#pragma once
// Unit clu_798e30_Renderer — TS3W.exe 0x00798E30 / 0x00799620 / 0x0079F710 / 0x0079F7E0 /
// 0x0079FAF0 / 0x0079FB20 / 0x007CFCC0 / 0x007CFDD0 / 0x007D01C0 / 0x007D2D40
// (src/core/renderer/clu_798e30_Renderer.cpp).
//
//  * 0x00798E30 indexes a 16-byte-element table through the holder's first pointer and,
//    when the table's magic dword is 'MTRR', forwards the element through the renderer's
//    0x00799C40 lookup (this in ECX = global 0x11D7530).
//  * 0x00799620 forwards its ECX (the container) plus a constant 2 into 0x00799540.
//  * 0x0079F710/0x0079F7E0/0x0079FAF0/0x0079FB20 are the list object's count+17-dword
//    builder family (0x0079F7E0 transcribes the packed records at src+0x18 into the list
//    with a permuted opcode map; the others call the unrewritten 0x0079FA70/0x0079F8C0).
//  * 0x007CFCC0/0x007CFDD0 are the wide-character FNV-1a hash and its folded form; the
//    lowercase step goes through the image's towlower import at 0xF95764.
//  * 0x007D01C0 parses `source` through the parser's virtual slot 13 and applies the
//    parsed object to the renderer at this+0x48C.
//  * 0x007D2D40 is a strided memset (block fill with negative-stride support).

#include <cstddef>
#include <cstdint>

#include "ts3/layout.h"

namespace ts3::renderer {

// ---- 0x00798E30 -------------------------------------------------------------------
// Holder whose first word is the table pointer; elements are 16 bytes, index 0 is the
// magic ('MTRR' = 0x5252544D).
struct Holder798E30 {
  void* table;  // +0x0

  // 0x00798E30 __thiscall, ret 4. Returns element (index+1) as an int; when the table
  // starts with 0x5252544D the element is additionally passed through 0x00799C40.
  std::int32_t GetEntry(std::uint32_t index);
};
TS3_X86_OFFSET(Holder798E30, table, 0x0);
TS3_X86_SIZE(Holder798E30, 4);

// ---- 0x00799620 -------------------------------------------------------------------
// The container `this` is only forwarded (ECX) into 0x00799540; the wrapper never reads
// it, so the layout is not part of this unit.
struct Container799620 {
  // 0x00799620 __thiscall, ret 0xc. 0x00799540(this, a1, 2, a2, a3).
  std::int32_t Add(std::uint32_t a1, void* a2, std::uint32_t a3);
};

// ---- 0x0079F710 .. 0x0079FB20 -----------------------------------------------------
// count followed by up to 17 packed opcode entries (0x44 bytes).
struct ItemList {
  std::int32_t count;        // +0x0
  std::uint32_t items[17];   // +0x4

  // 0x0079F710 __thiscall, ret 0. count = 0, items zeroed; returns `this`.
  ItemList* Init();
  // 0x0079F7E0 __thiscall, ret 4. Appends the packed records at src+0x18 (count at
  // src+0x1C, clamped to 17), remapping opcodes {0,1,2,3,4,5,6} -> {0,1,6,3,4,2,5} and
  // packing the next byte into bits 8..15; records with an opcode > 6 are dropped.
  void BuildFrom(const void* src);
  // 0x0079FAF0 __thiscall, ret 0xc. 0x0079FA70(this, a2, a3) then
  // 0x0079F8C0(this, a1, result); returns the latter.
  std::int32_t Add3(std::uint32_t a1, std::uint32_t a2, std::uint32_t a3);
  // 0x0079FB20 __thiscall, ret 0xc. Builds a temporary list from a1 (like BuildFrom)
  // then 0x0079FA70(this, a2, a3) and 0x0079F8C0(this, &tmp, result).
  std::int32_t AddFrom(std::uint32_t a1, std::uint32_t a2, std::uint32_t a3);
};
TS3_X86_OFFSET(ItemList, count, 0x0);
TS3_X86_OFFSET(ItemList, items, 0x4);
TS3_X86_SIZE(ItemList, 0x48);

// ---- 0x007CFCC0 / 0x007CFDD0 ------------------------------------------------------
// 0x007CFCC0 __cdecl, ret 0: FNV-1a over `count` UTF-16 code units, each folded with
// the image's towlower. 0x007CFDD0 __cdecl, ret 0: the same over the NUL-terminated
// string, then (h >> 24) ^ (h & 0xFFFFFF) (unsigned shift and XOR).
std::int32_t HashWide7CFCC0(const wchar_t* text, std::uint32_t count);
std::int32_t FoldWideHash7CFDD0(const wchar_t* text);

// ---- 0x007D01C0 -------------------------------------------------------------------
// The parser host object; only +0x48C (the renderer being applied to) is read.
struct ParserHost7D01C0 {
  std::uint8_t opaque[0x490];

  // 0x007D01C0 __thiscall, ret 8 (the second stack slot doubles as the argument that
  // held `parser`). Returns the low byte of the 0x007392A0 result (0 when the parse
  // fails, the parsed object is absent, or the apply fails).
  std::uint8_t ApplyParsed(void* parser, const void* source);
};

// ---- 0x007D2D40 -------------------------------------------------------------------
// 0x007D2D40 __cdecl, ret 0. Fills `rows` blocks of `blockSize` bytes at stride
// `stride` with `value`; a negative stride starts at the opposite end (dst +
// stride*(rows-1)) and walks forward. When stride == blockSize one block of
// blockSize*rows bytes is filled at once. Early-outs (blockSize or rows zero) return
// stale EAX in the original; the return is ignored by every caller and typed void here.
void FillBlocks7D2D40(std::uint8_t* dst, std::int32_t stride, std::uint8_t value,
                      std::size_t blockSize, std::int32_t rows);

}  // namespace ts3::renderer