// Unit clu_45ef30_Debug (the project notes): eight TS3W.exe functions the units
// clustering put together. Seven are entry points, all reached from other units; the
// eighth (0x00465330) is a private helper of 0x004662F0. Two object shapes are
// involved, neither of whose class IDA names:
//
//   * the "layout object" returned by 0x005137D0 (unit clu_44ffa0_UI): its methods sit
//     in 0x0045EF30..0x00466C00 and hold a 0x278-byte block at +0xFC, list/container
//     state at +0x14, row bounds at +0x3BC..+0x3C4 and the current row at +0x418;
//   * the outer object that owns two 0x278-byte blocks at +0x2E8 and +0x744 plus the
//     vector at +0x568 (the callers of 0x00541360/0x00542CD0 live in cls_53f080_x).
//
//   LayoutBlock::CopyFrom        0x0045EF30  thiscall, ret 4 (src)         leaf
//   SetLayoutSource              0x00462CE0  thiscall, ret 4 (src)
//   AssignLayoutList             0x00465330  thiscall, ret 4 (other)       private
//   CopyLayoutObject             0x004662F0  thiscall, ret 4 (src)
//   ResetLayoutForRows           0x00466B20  thiscall, ret 0x14 (5 args)
//   MeasureRowsTweaked           0x00541360  thiscall, ret 0x10 (4 args)
//   MeasureRows                  0x00542CD0  thiscall, ret 4 (1 arg)
//   RowBounds                    0x007E5BA0  cdecl, 5 args, returns out
//
// Every unrewritten callee (0x0045A050, 0x00464890, 0x00465DE0, 0x00464BA0,
// 0x00466990, 0x004621F0, 0x00462230, 0x00462240, 0x00463920, 0x004626C0, 0x005137D0,
// 0x008621C0) is called through ts3::CallGameMethod; the two in-unit callees
// (0x0045EF30, 0x00465330) are called directly.
//
// EA quirks kept (they change outputs): 0x0045EF30 is a forward `rep movsd`, so
// overlapping copies with dst > src smear rather than memmove; 0x00462CE0 dereferences
// +0x384 without a null check; 0x00465330/0x00466B20 copy dwords/bytes with the exact
// access widths of the original (movzx byte loads, fld/fstp float 0x374).
#pragma once

#include <cstddef>
#include <cstdint>

namespace ts3::debug {

// The 0x278-byte block 0x0045EF30 copies (this == the block, return == this). The game
// uses it for several embedded blocks (layout object +0xFC, outer object +0x2E8/+0x744).
struct LayoutBlock {
    static constexpr std::size_t kSize = 0x278;
    uint8_t bytes[kSize];

    // 0x0045EF30: forward dword copy of 0x278 bytes from src, returns this. Reads and
    // writes exactly 0x278 bytes; no null check on src (the callers check it).
    void* CopyFrom(const void* src);
};

// 0x00462CE0: copies *src (0x278 bytes) into this+0xFC, stores this+0xFC into
// *(this+0x384) and that value into this+0x3B8; returns the pointer at this+0x384, or 0
// (without touching anything) when src is null. Original does not null-check +0x384.
void* SetLayoutSource(void* self, const void* src);

// 0x00465330: container assignment. Frees this+0x0C through 0x0045A050, re-initialises
// the embedded header at +0x04 (self-pointers), then copies the source tree through
// 0x00464890
// +0x14. Returns this (unchanged when this == other).
void* AssignLayoutList(void* self, const void* other);

// 0x004662F0: copy construction of a layout object (0x37B bytes) from src.
void* CopyLayoutObject(void* self, const void* src);

// 0x00466B20: resets the layout object for a new row block (a2/a3 base and count,
// floats a4/a5 stored at +0x3C0/+0x3C4, +0x3BC = 100000.0f), optionally replacing the
// 0x278-byte source block, then lays the rows out. Returns ([+0x450]-[+0x44C])>>1.
int ResetLayoutForRows(void* self, int a2, int a3, float a4, float a5, const void* a6);

// 0x00541360: like MeasureRows but with the two field overrides at block+0x244/+0x248
// applied to a stack copy of this+0x2E8 (skipped when +0x284==4 && +0x288==3). Writes
// the running total to *total and the last row's item count to *last; returns the
// layout object's current row pointer (0x00462240).
void* MeasureRowsTweaked(void* self, const int32_t* cell, int32_t rows, int32_t* total,
                         int32_t* last);

// 0x00542CD0: measures every layout row: accumulates the row item counts in the vector
// at this+0x568 (pushing the running total), tracks the maximum +0xB8 float in cell[3]
// and adds (float)this[0x560] to cell[4]. Stores the total at this+0x578 and the vector
// size at cell[2]; returns that size.
int32_t MeasureRows(void* self, void* cell);

// 0x007E5BA0: fills out[0] (row start float, item+0xB4) and out[1] (row height: a4+0x274
// when non-zero, else ceil(item+0xBC) - floor(item+0xC0) truncated to int) and returns
// out. When a2 or a4 is null both floats are 0.0f.
float* RowBounds(float* out, int32_t a2, int32_t a3, void* a4, void* obj);

}  // namespace ts3::debug