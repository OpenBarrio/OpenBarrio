#pragma once
#include <cstdint>
#include "ts3/layout.h"

namespace ts3::scratch {

// Intrusive list node: this unit only ever follows `next` (0x00509890 frees every node).
// The payload the game stores after `next` is not touched by the unit's four functions.
struct UiTextBatchNode {
    UiTextBatchNode* next;
};

// Unnamed class at vtable 0x00FBA3DC, allocator tags "UI/UTFWin/Verts/Parts/TextSpans/
// GlyphInfos" — a UI text batch: an intrusive list of Verts nodes, two game-vector regions
// (Parts, TextSpans) whose begins the dtor frees, a GlyphInfos tag, and an interlocked
// refcount at +0x4C (AddRef 0x00505B70 / Release 0x00509C60, the latter not in this unit).
// Fields the unit only zeroes (listSize, listField14, the end/capacity slots) keep neutral
// names; +0x10/+0x14 are read by 0x00508BC0 (element teardown, not in units.csv — bridged).
// The game object may extend past +0x50; these four functions never touch beyond it.
struct UiTextBatch {
    void* const* vtable;          // +0x00 0x00FBA3DC
    UiTextBatchNode* headNext;    // +0x04 intrusive list head, self-linked by the ctor
    UiTextBatchNode* headPrev;    // +0x08
    const char* vertsTag;         // +0x0C "UI/UTFWin/Verts" (0x00FBA3CC)
    uint32_t listSize;            // +0x10 zeroed; read by 0x00508BC0
    uint32_t listField14;         // +0x14 zeroed; read by 0x00508BC0
    UiTextBatchNode* cursor;      // +0x18 list cursor; ctor sets it to the head
    void* vertsBuffer;            // +0x1C freed by the dtor
    uint32_t partsEnd;            // +0x20 zeroed
    uint32_t partsCapacity;       // +0x24 zeroed
    const char* partsTag;         // +0x28 "UI/UTFWin/Parts" (0x00FBA3BC)
    void* partsBegin;             // +0x2C freed by the dtor
    uint32_t partsEnd2;           // +0x30 zeroed
    uint32_t partsCapacity2;      // +0x34 zeroed
    const char* textSpansTag;     // +0x38 "UI/UTFWin/TextSpans" (0x00FBA3A8)
    void* textSpansBegin;         // +0x3C freed by the dtor
    uint32_t textSpansEnd;        // +0x40 zeroed
    uint32_t textSpansCapacity;   // +0x44 zeroed
    const char* glyphInfosTag;    // +0x48 "UI/UTFWin/GlyphInfos" (0x00FBA390)
    uint32_t refcount;            // +0x4C interlocked; 0 on construct, AddRef/Release

    // 0x00505B70, __thiscall, ret 0; lock xadd(+1) on refcount, returns the new count.
    int32_t AddRef();
    // 0x00506550, __thiscall ctor, ret 0; returns this.
    UiTextBatch* Construct();
    // 0x00509890, __thiscall, ret 0; returns the last free's eax (0 when nothing ran).
    int32_t Dtor();
    // 0x00509C40, __thiscall scalar deleting dtor, ret 4; free(this) when flags & 1.
    UiTextBatch* DeletingDtor(uint32_t flags);
};
TS3_X86_OFFSET(UiTextBatch, headNext, 0x04);
TS3_X86_OFFSET(UiTextBatch, headPrev, 0x08);
TS3_X86_OFFSET(UiTextBatch, vertsTag, 0x0C);
TS3_X86_OFFSET(UiTextBatch, listSize, 0x10);
TS3_X86_OFFSET(UiTextBatch, listField14, 0x14);
TS3_X86_OFFSET(UiTextBatch, cursor, 0x18);
TS3_X86_OFFSET(UiTextBatch, vertsBuffer, 0x1C);
TS3_X86_OFFSET(UiTextBatch, partsTag, 0x28);
TS3_X86_OFFSET(UiTextBatch, partsBegin, 0x2C);
TS3_X86_OFFSET(UiTextBatch, textSpansTag, 0x38);
TS3_X86_OFFSET(UiTextBatch, textSpansBegin, 0x3C);
TS3_X86_OFFSET(UiTextBatch, glyphInfosTag, 0x48);
TS3_X86_OFFSET(UiTextBatch, refcount, 0x4C);
TS3_X86_SIZE(UiTextBatch, 0x50);

}  // namespace ts3::scratch
