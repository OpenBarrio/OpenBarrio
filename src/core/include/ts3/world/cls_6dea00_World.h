#pragma once
#include <cstdint>
#include "ts3/layout.h"

namespace ts3::world {

// Embedded sub-object at World +0x08 with base vtable 0x00FE1A5C. 0x006DEA00 initialises
// it (vtable and the dword at its +0x08); the World ctor then replaces the vtable with
// 0x01060994, so only the zeroed dword survives from the helper. Its full extent is not
// observable from this unit (its +0x04 / World +0x0C is never touched anywhere) — no size
// assert here; the World struct pins its placement instead.
struct UiWorldInner {
    void* const* vtable;   // +0x00 (World +0x08) 0x00FE1A5C from 0x006DEA00, final 0x01060994
    uint32_t field04;      // +0x04 (World +0x0C) never touched by this unit
    uint32_t field08;      // +0x08 (World +0x10) zeroed by 0x006DEA00

    // 0x006DEA00, __thiscall, ret 0; returns this.
    UiWorldInner* Construct();
};

// cls_6dea00_World — "World": ctor 0x00A618E0 (caller 0x00B38820), vtable flag exchange
// 0x00A5F9F0, scalar deleting dtor 0x00A61AC0 (whose body 0x00A61190 is NOT a member of
// any unit in units.csv — bridged). Observed extent 0x108 (the ctor writes through
// +0x104); the game object may extend further. Gaps at +0x24..+0x2F, +0x51..+0x57 and
// +0x74..+0x77 are never written by these functions and must stay untouched. The blocks
// at +0x58 and +0x78 mirror each other on a 0x20 stride: {id, count, ?, 1.0f, 2.0f, ?,
// name}; +0x94 starts a third, +0xC8 a fourth (0x011C7E54/0x011C7E56 look like packed
// ids). Purpose of +0x8C is unknown; 0x00A5F9F0 exchanges 1 into it.
struct UiWorld {
    void* const* vtable;         // +0x00 ctor: 0x0107396C first, final 0x01060998 (dtor restores)
    uint32_t field04;            // +0x04 zeroed (xchg — like +0x18/+0x94, likely a shared pointer)
    UiWorldInner inner;          // +0x08
    void* const* vtable14;       // +0x14 ctor: 0x00FE1A5C first, final 0x01060990
    uint32_t field18;            // +0x18 zeroed (xchg)
    int32_t field1C;             // +0x1C = 0
    int32_t field20;             // +0x20 = 0
    uint32_t untouched24[3];     // +0x24..+0x2F never written
    float zero30[4];             // +0x30 16-byte zero block (movaps in the original)
    uint8_t field40;             // +0x40 = 0
    uint8_t field41;             // +0x41 = 0
    uint8_t field42;             // +0x42 = 1
    uint8_t field43[13];         // +0x43..+0x4F = 0 (stored byte by byte)
    uint8_t field50;             // +0x50 = 1
    uint8_t untouched51[7];      // +0x51..+0x57 never written
    uint32_t id58;               // +0x58 = 0x0114C900
    int32_t count5C;             // +0x5C = 1
    int32_t field60;             // +0x60 = 0
    float min64;                 // +0x64 = 1.0f (0x0107A538)
    float max68;                 // +0x68 = 2.0f (0x00F98A58)
    int32_t field6C;             // +0x6C = 0
    const char* name70;          // +0x70 = 0x01060A2C rdata string
    uint32_t untouched74;        // +0x74..+0x77 never written
    uint32_t id78;               // +0x78 = 0x0114C900
    int32_t count7C;             // +0x7C = 1
    int32_t field80;             // +0x80 = 0
    float min84;                 // +0x84 = 1.0f
    float max88;                 // +0x88 = 2.0f
    int32_t flag8C;              // +0x8C = 0; 0x00A5F9F0 exchanges 1 in and returns the old value
    const char* name90;          // +0x90 = 0x01060A08
    uint32_t field94;            // +0x94 zeroed (xchg)
    int32_t field98;             // +0x98 = 0
    int32_t field9C;             // +0x9C = 0
    int32_t fieldA0;             // +0xA0 = 0
    const char* nameA4;          // +0xA4 = 0x010609E4
    int32_t fieldA8[8];          // +0xA8..+0xC4 = 0
    uint32_t fieldC8;            // +0xC8 = 0x011C7E54
    uint32_t fieldCC;            // +0xCC = 0x011C7E54
    uint32_t fieldD0;            // +0xD0 = 0x011C7E56
    const char* nameD4;          // +0xD4 = 0x010609C0
    int32_t fieldD8[12];         // +0xD8..+0x104 = 0

    // 0x00A5F9F0, __thiscall, ret 0xC; atomic exchange of 1 at +0x8C, returns the old
    // value. The three stack arguments are read by nothing.
    int32_t ExchangeFlag1(int32_t a2, int32_t a3, int32_t a4);
    // 0x00A618E0, __thiscall ctor, ret 0; returns this.
    UiWorld* Construct();
    // 0x00A61AC0, __thiscall scalar deleting dtor, ret 4; body 0x00A61190 (bridged),
    // free(this) when flags & 1, returns this.
    UiWorld* DeletingDtor(uint32_t flags);
};
TS3_X86_OFFSET(UiWorld, field04, 0x04);
TS3_X86_OFFSET(UiWorld, inner, 0x08);
TS3_X86_OFFSET(UiWorld, vtable14, 0x14);
TS3_X86_OFFSET(UiWorld, field18, 0x18);
TS3_X86_OFFSET(UiWorld, field1C, 0x1C);
TS3_X86_OFFSET(UiWorld, field20, 0x20);
TS3_X86_OFFSET(UiWorld, untouched24, 0x24);
TS3_X86_OFFSET(UiWorld, zero30, 0x30);
TS3_X86_OFFSET(UiWorld, field40, 0x40);
TS3_X86_OFFSET(UiWorld, field42, 0x42);
TS3_X86_OFFSET(UiWorld, field50, 0x50);
TS3_X86_OFFSET(UiWorld, id58, 0x58);
TS3_X86_OFFSET(UiWorld, count5C, 0x5C);
TS3_X86_OFFSET(UiWorld, min64, 0x64);
TS3_X86_OFFSET(UiWorld, name70, 0x70);
TS3_X86_OFFSET(UiWorld, id78, 0x78);
TS3_X86_OFFSET(UiWorld, flag8C, 0x8C);
TS3_X86_OFFSET(UiWorld, name90, 0x90);
TS3_X86_OFFSET(UiWorld, field94, 0x94);
TS3_X86_OFFSET(UiWorld, nameA4, 0xA4);
TS3_X86_OFFSET(UiWorld, fieldA8, 0xA8);
TS3_X86_OFFSET(UiWorld, fieldC8, 0xC8);
TS3_X86_OFFSET(UiWorld, nameD4, 0xD4);
TS3_X86_OFFSET(UiWorld, fieldD8, 0xD8);
TS3_X86_SIZE(UiWorld, 0x108);

}  // namespace ts3::world
