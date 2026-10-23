// Unit cls_ae0590_LotRenderer (subsystem LotRenderer): six TS3W.exe 1.67.2 entries on
// the lot-renderer object, at 0x00AE0590 / 0x00AFBB40 / 0x00AFBCB0 / 0x00AFCA80 /
// 0x00C552C0 / 0x00C563B0.  The rewrite entry points mirror the original __thiscall
// ABI (this-in-ECX plus stack args) so they can be swapped in by the harness.
//
// Field layout used by this unit (offsets verified against the disassembly in
// the original disassembly - the assembly is the source of truth):
//   +0x24   u32   mode flag; AE0590 tests > 3u
//   +0x13c  i32   threshold compared against signed byte at arg+2 (C563B0)
//   +0x204  ---   embedded "wall scratch" map (ECX for 0xC4CF00/0xC512D0)
//   +0x208  ---   embedded sentinel ("no owner" pointer compared to *node)
//   +0x218  ptr   pointer to a scratch object whose [0x230] is the AA2870 ECX
//   +0x224  i32   width stride for the grid (AFBB40/AFBCB0)
//   +0x230  i32*  begin of sorted int vector "A" (AFBB40/AFBCB0 bit-hit branch)
//   +0x234  i32*  end   of A
//   +0x238  i32*  cap   of A
//   +0x240  u8    reserved 4th arg to 0x6D8BD0 (never read by callee - EA quirk)
//   +0x244  i32*  begin of sorted int vector "B"
//   +0x248  i32*  end   of B
//   +0x24c  i32*  cap   of B
//   +0x254  u8    reserved 4th arg to 0x6D8BD0 in AFBCB0 (also unused)
//   +0x258  f32*  grid of (width*height) Vec2 cells (2 floats per cell, stride 8)
//   +0x270  i32   height stride for the bit-mask index
//   +0x274  u8*   bitmask of "wall present" per cell
//   +0x29c  u32*  pointer to a bucket array of node pointers (AFCA80)
//   +0x2a0  u32   bucket count (AFCA80)
//
// All six functions have differential coverage. The five side-effecting bodies call
// external helpers through the bridge; their fixtures record identical helper calls,
// allocator behavior, grid bytes, vector contents and wall updates.
#pragma once

#include <cstdint>
#include "ts3/layout.h"

namespace ts3 {
namespace lotrenderer {

// 0x00AE0590: pure leaf, reads +0x24 and returns 1/0.  Thiscall ABI.
struct LotRenderer7AE0590 {
    uint8_t pad00[0x24];
    uint32_t mode24;

    // Returns (uint32[+0x24] > 3u) as full EAX (0 or 1).
    // Original ABI: __thiscall, plain ret.
    uint32_t IsEnabledAE0590();
};

// Side-effecting entries use explicit self parameters; harness wrappers supply the
// original thiscall ABI. These layouts document their shared renderer state.
struct LotKeyValue { std::uint32_t col, row; float value; };
struct LotKeyInput { std::uintptr_t opaque[4]; LotKeyValue *begin, *end; };
struct LotKeyVector {
    std::uint32_t *begin, *end, *capacity;
    void* allocator;
    std::uint32_t reserved;
};
struct WallCoordinate { std::uint8_t col, row, level; };
struct WallPair { WallCoordinate from, to; };
struct WallNode { std::uint32_t key; std::int32_t type; WallNode* next; };
struct WallScratch { std::uintptr_t tag; std::uintptr_t sentinel[4]; };
struct WallScratchOwner { std::uint8_t opaque[0x230]; void* service; };
struct LotRendererState {
    LotRenderer7AE0590 leaf;
    std::uint8_t opaque028[0x114];
    std::int32_t threshold;
    std::uint8_t opaque140[0xc4];
    WallScratch scratch;
    WallScratchOwner* owner;
    std::uint8_t opaque21c[8];
    std::uint32_t width;
    std::uint8_t opaque228[8];
    LotKeyVector keysA, keysB;
    struct Cell { float first, second; }* grid;
    std::uint8_t opaque25c[20];
    std::uint32_t height;
    std::uint8_t* mask;
    std::uint8_t opaque278[36];
    WallNode** buckets;
    std::uint32_t bucketCount;
};
TS3_X86_SIZE(LotKeyValue, 12);
TS3_X86_OFFSET(LotKeyInput, begin, 16);
TS3_X86_OFFSET(LotKeyVector, reserved, 16);
TS3_X86_SIZE(WallPair, 6);
TS3_X86_OFFSET(WallNode, next, 8);
TS3_X86_OFFSET(WallScratchOwner, service, 0x230);
TS3_X86_OFFSET(LotRendererState, threshold, 0x13c);
TS3_X86_OFFSET(LotRendererState, scratch, 0x204);
TS3_X86_OFFSET(LotRendererState, owner, 0x218);
TS3_X86_OFFSET(LotRendererState, width, 0x224);
TS3_X86_OFFSET(LotRendererState, keysA, 0x230);
TS3_X86_OFFSET(LotRendererState, keysB, 0x244);
TS3_X86_OFFSET(LotRendererState, grid, 0x258);
TS3_X86_OFFSET(LotRendererState, height, 0x270);
TS3_X86_OFFSET(LotRendererState, mask, 0x274);
TS3_X86_OFFSET(LotRendererState, buckets, 0x29c);
TS3_X86_OFFSET(LotRendererState, bucketCount, 0x2a0);
std::intptr_t LotRenderer_BuildLotKeysAFBB40(void* self, const LotKeyInput* input);
std::intptr_t LotRenderer_InsertOneKeyAFBCB0(void* self, int col, int row, float value);
uint8_t LotRenderer_UpdateWallTypeAFCA80(void* self, uint8_t a2, uint8_t a3);
int LotRenderer_ProcessWallCellC552C0(void* self, void* a2);
int LotRenderer_EnumerateWallKeysC563B0(void* self, const WallCoordinate* coordinate);

}  // namespace lotrenderer
}  // namespace ts3
