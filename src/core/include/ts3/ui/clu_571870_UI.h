// Cluster unit clu_571870_UI (subsystem UI): five TS3W.exe leaves over one paged
// item-list layout. One forwards an interface call through a game vtable, one
// initializes a 48-byte 3x4 float block, and three read counts and five dwords out of
// the paged list through its embedded vectors.
//
//   0x00571870  Forward571870   cdecl: null -> 0, else calls vtable slot 3 of the
//                               argument (thiscall, one stack argument) with the
//                               constant 0x017F1D88 and returns its int
//   0x00575600  Init575600      thiscall, plain ret: writes 48 bytes of `this`
//   0x00577640  CountPages      thiscall: (pagesEnd - pagesBegin) / 2744, signed
//   0x005776A0  CountItems      thiscall, ret 4: item count of one page, 0 when the
//                               page index is out of range
//   0x00577700  GetItemFields   thiscall, ret 0xC: five dwords of one item row,
//                               0 (out untouched) when page or item is out of range
//
// Layout (strides are the x86 game's): the list at +0x90/+0x94 holds a vector of
// 2744-byte pages; each page holds an item vector at +8/+0xC of 272-byte rows; the five
// copied fields of a row sit at +0xE0, +0xE4, +0xF4, +0x108 and +0x104 - note the EA
// order: out[3] takes +0x108 and out[4] takes +0x104 (not monotonic).
//
// EA quirks kept: both bounds checks compare the index against the count UNSIGNED
// (jae), so a negative count (inverted vector) lets every index through; the signed
// span divisions truncate toward zero; GetItemFields sets only AL on both paths (the
// upper EAX bits stay whatever the divisions left); Init575600 reads its 1.0f from the
// float global 0x107A538 and never touches bytes at +0x30 and beyond.
//
// In-unit calls go directly to the rewrites below; Forward571870 runs through the ts3
// layout.h bridge (x64 builds abort there instead - no game code to call).
#pragma once

#include <cstdint>

#include "ts3/layout.h"

namespace ts3::ui {

// The two strides, in bytes (the x86 game's).
constexpr uint32_t kPageStride = 2744;  // 0xAB8
constexpr uint32_t kItemStride = 272;   // 0x110

// One 272-byte item row. Field names are offset-based: only the five copied dwords have
// observed consumers; the rest is payload this unit never reads.
struct UiItemRow {
    uint8_t pad00[0xE0];
    uint32_t dE0;       // +0xE0 copied to out[0]
    uint32_t dE4;       // +0xE4 copied to out[1]
    uint8_t padE8[0x0C];
    uint32_t dF4;       // +0xF4 copied to out[2]
    uint8_t padF8[0x0C];
    uint32_t d104;      // +0x104 copied to out[4]
    uint32_t d108;      // +0x108 copied to out[3]
    uint8_t pad10C[4];  // to the 272-byte stride
};
TS3_X86_OFFSET(UiItemRow, dE0, 0xE0);
TS3_X86_OFFSET(UiItemRow, dE4, 0xE4);
TS3_X86_OFFSET(UiItemRow, dF4, 0xF4);
TS3_X86_OFFSET(UiItemRow, d104, 0x104);
TS3_X86_OFFSET(UiItemRow, d108, 0x108);
TS3_X86_SIZE(UiItemRow, 0x110);

// One 2744-byte page: its item vector at +8/+0xC, payload after.
struct UiPage {
    uint8_t pad00[8];
    UiItemRow* itemsBegin;  // +0x08
    UiItemRow* itemsEnd;    // +0x0C
    uint8_t pad10[kPageStride - 0x10];
};
TS3_X86_OFFSET(UiPage, itemsBegin, 0x08);
TS3_X86_OFFSET(UiPage, itemsEnd, 0x0C);
TS3_X86_SIZE(UiPage, 0xAB8);

// The page vector of the list, embedded at +0x90 of a larger object; this view covers
// just enough of that object for the three readers (the original reads only +0x90/0x94).
struct UiPageList {
    uint8_t pad00[0x90];
    UiPage* pagesBegin;  // +0x90
    UiPage* pagesEnd;    // +0x94

    // 0x00577640: signed (pagesEnd - pagesBegin) / 2744, truncating toward zero.
    int32_t CountPages() const;

    // 0x005776A0 (ret 4): the item count of one page, 0 when the page index fails the
    // unsigned compare against the count.
    int32_t CountItems(uint32_t page) const;

    // 0x00577700 (ret 0xC): writes out[0..4] from the row's +0xE0/+0xE4/+0xF4/+0x108/
    // +0x104 dwords and returns 1; 0 (out untouched) when page or item is out of range.
    uint8_t GetItemFields(uint32_t page, uint32_t item, uint32_t* out) const;
};
TS3_X86_OFFSET(UiPageList, pagesBegin, 0x90);
TS3_X86_OFFSET(UiPageList, pagesEnd, 0x94);
TS3_X86_SIZE(UiPageList, 0x98);

// The 48-byte block 0x00575600 initializes: three 16-byte rows - zeros, {0,0,0,1} and
// {1,1,1,1}. Bytes at +0x30 and beyond are outside the original's stores (the caller's
// object is larger; this type covers exactly what the function writes).
struct UiTransformBlock {
    float v[12];

    // 0x00575600: the constant layout above, with the 1.0f read from the float global
    // 0x107A538 (x86; 1.0f in TS3W.exe 1.67.2).
    void Init575600();
};
TS3_X86_SIZE(UiTransformBlock, 48);

// 0x00571870 (cdecl): null -> 0; otherwise `obj` IS the object whose vtable slot 3 is
// called with the constant 0x017F1D88 (a value pushed, never dereferenced here), and
// the method's int is returned.
int32_t __cdecl Forward571870(void* obj);

}  // namespace ts3::ui
