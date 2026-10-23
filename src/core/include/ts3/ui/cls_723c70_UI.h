// Class unit cls_723c70_UI (subsystem UI): the game's "UI/TextGlyphCache" interface
// object (vtable 0x00FFCFE8, class-name string at 0x00FFD00C), TS3W.exe 1.67.2.
//
//   0x00724840  Construct724840  constructor: game vtable at +0, byte 0 at +4, the head
//                               dword of each of the 64 slot records at +0x128..+0x710,
//                               dwords 0 at +0xC3C/+0xC50; returns this
//   0x00723CD0  Acquire723CD0   first-acquire init (uishader, vertex format, four shader
//                               param handles) + refcount at +0xC50; returns AL (1/0)
//   0x00723D80  Release723D80   refcount--; on the drop to 0, virtual slot 2 on the
//                               format object at +0xC30 and field teardown
//   0x00723DC0  QueryIface723DC0 (ret 4) interface-id dword query: this for
//                               0xEE3F516E / 0x00838293 / 0x6ED3E59B, else null
//   0x00724810  Reset724810     conditional 0x007243B0 flush, zero +0x14/+0x18, the
//                               0x0060FCE0
//   0x00723C70  UiFormatHolder::Create723C70 (private helper, this = the holder embedded
//                               at +0xC30): allocate the vertex format through game code
//
// EA quirks kept: Acquire/Release/return paths set only AL (upper EAX bits stay stale);
// Release's the original instruction sequence wraps 0 to -1 and skips cleanup; Construct never
// touches +8/+0x14/+0x18/+0xC30/+0xC40..+0xC4C or the slot payloads; the param-handle
// stores are not monotonic (+0xC4C is written before +0xC48).
//
// In-unit calls go directly to the rewrites below; every game call (0x004E3FB0,
// 0x0061A300, 0x0061A010, 0x0079B840, 0x0079A160, 0x007243B0, 0x0060FCE0, the two game
// vtable slots, and the three pointer globals at 0x011D753C/0x011D7530/0x011CECC8) goes
// through the ts3 layout.h bridge - x64 builds abort there instead.
#pragma once

#include <cstdint>

#include "ts3/layout.h"

namespace ts3::ui {

// One glyph-slot record at +0x128 + i*0x18 (0 <= i < 64): only the leading dword has an
// observed writer (the constructor zeroes it); the remaining 0x14 bytes are payload.
struct UiGlyphSlot {
    uint32_t head;
    uint8_t payload[0x14];
};
TS3_X86_SIZE(UiGlyphSlot, 0x18);

// The format holder embedded at object +0xC30: one method (0x00723C70) that fills
// `format` through the game's allocator and element setter.
struct UiFormatHolder {
    void* format;  // +0x00 == object +0xC30

    // 0x00723C70 (thiscall): game function 0x004E3FB0 (cdecl, 8 args) creates the raw
    // "UI/ImmediateModeEx_TSS/VertexFormat" object, 0x0061A300 wraps it, vtable slot 1
    // of the wrap runs, then 0x0061A010 installs the 5-element table at 0x00FFCEF4.
    // Returns the normalized AL of the last call; `format` is stored before its null
    // check (EA order).
    bool Create723C70();
};
TS3_X86_OFFSET(UiFormatHolder, format, 0x00);

struct UiTextGlyphCache {
    void* vtable;       // +0x00  the game's vtable 0x00FFCFE8 (written by Construct)
    uint8_t flag04;     // +0x04  byte; Reset clears it
    uint8_t pad05[3];
    uint32_t field08;   // +0x08  non-zero: Reset flushes through 0x007243B0 first
    uint8_t pad0C[8];
    uint32_t field14;   // +0x14  zeroed by Reset
    uint32_t field18;   // +0x18  zeroed by Reset
    uint8_t pad1C[0x128 - 0x1C];
    UiGlyphSlot slots[64];  // +0x128, stride 0x18 (last head at +0x710)
    uint8_t pad728[0xC30 - 0x728];
    UiFormatHolder formatHolder;  // +0xC30
    uint8_t padC34[8];
    void* uishader;      // +0xC3C
    void* windata;       // +0xC40  0x0079A160("windata", 80, 2)
    void* texMap;        // +0xC44  0x0079A160("texMap", 1, 5)
    void* texMaskMap;    // +0xC48  0x0079A160("texMaskMap", 1, 5)
    void* texDepthMap;   // +0xC4C  0x0079A160("texDepthMap", 1, 5)
    uint32_t refcount;   // +0xC50

    // 0x00724840: constructor (plain ret), returns this.
    UiTextGlyphCache* Construct724840();

    // 0x00723CD0: first-acquire initialization + refcount increment; returns AL only
    // (1 = ok/steady, 0 = init failed; the original leaves upper EAX stale).
    uint8_t Acquire723CD0();

    // 0x00723D80: refcount--; on the drop to zero, virtual slot 2 on the format object
    // and field teardown. Returns that virtual's int; on every other path the original
    // returns the caller's stale EAX, modelled here as 0.
    int32_t Release723D80();

    // 0x00723DC0 (ret 4): returns this for the three interface-id dwords, else null.
    void* QueryIface723DC0(uint32_t id);

    // 0x00724810: reset; returns whatever the 0x0060FCE0 singleton call returns.
    int32_t Reset724810();
};
TS3_X86_OFFSET(UiTextGlyphCache, vtable, 0x00);
TS3_X86_OFFSET(UiTextGlyphCache, flag04, 0x04);
TS3_X86_OFFSET(UiTextGlyphCache, field08, 0x08);
TS3_X86_OFFSET(UiTextGlyphCache, field14, 0x14);
TS3_X86_OFFSET(UiTextGlyphCache, field18, 0x18);
TS3_X86_OFFSET(UiTextGlyphCache, slots, 0x128);
TS3_X86_OFFSET(UiTextGlyphCache, formatHolder, 0xC30);
TS3_X86_OFFSET(UiTextGlyphCache, uishader, 0xC3C);
TS3_X86_OFFSET(UiTextGlyphCache, windata, 0xC40);
TS3_X86_OFFSET(UiTextGlyphCache, texMap, 0xC44);
TS3_X86_OFFSET(UiTextGlyphCache, texMaskMap, 0xC48);
TS3_X86_OFFSET(UiTextGlyphCache, texDepthMap, 0xC4C);
TS3_X86_OFFSET(UiTextGlyphCache, refcount, 0xC50);
TS3_X86_SIZE(UiTextGlyphCache, 0xC54);

}  // namespace ts3::ui
