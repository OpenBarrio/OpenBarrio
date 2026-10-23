#pragma once
#include "ts3/layout.h"

namespace ts3::misc {
struct OverlapItem {
    uint8_t prefix[0x40];
    int32_t left, top, right, bottom;
    uint8_t reserved[7];
    uint8_t marked;
};
TS3_X86_OFFSET(OverlapItem, left, 0x40);
TS3_X86_OFFSET(OverlapItem, top, 0x44);
TS3_X86_OFFSET(OverlapItem, right, 0x48);
TS3_X86_OFFSET(OverlapItem, bottom, 0x4c);
TS3_X86_OFFSET(OverlapItem, marked, 0x57);
TS3_X86_SIZE(OverlapItem, 0x58);
struct OverlapList {
    uint8_t prefix[0xb0];
    OverlapItem** begin;
    OverlapItem** end;
};
TS3_X86_OFFSET(OverlapList, begin, 0xb0);
TS3_X86_OFFSET(OverlapList, end, 0xb4);
TS3_X86_SIZE(OverlapList, 0xb8);
// TS3W.exe 0x00C7E680: mark strict signed rectangle overlaps; keep existing marks and empty-list EAX.
OverlapItem* MarkOverlaps(OverlapList* self, const int32_t* rectangle, OverlapItem* incoming);
#if defined(_M_IX86)
// ABI adapter: ECX=this, one callee-popped argument, incoming EAX retained on empty lists.
OverlapItem* __fastcall MarkOverlapEntry(OverlapList* self, void*, const int32_t* rectangle);
#endif
} // namespace ts3::misc
