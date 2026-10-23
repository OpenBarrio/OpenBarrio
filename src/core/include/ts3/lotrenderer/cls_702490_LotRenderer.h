// Unit cls_702490_LotRenderer (subsystem LotRenderer): eight TS3W.exe 1.67.2 entries
// over the lot renderer object, two data leaves, and the three 0x93Exxx pumps.
//
//   0x00702490  ApplyEntryColor702490  thiscall, ret 0x0C: (u64 key, LotParams*).
//                               Looks the key up in the renderer's map at +4 (game
//                               find 0x6FDFB0, callee cleans 8) and returns 0 on every
//                               miss path: result == the sentinel at +0x1028, result
//                               == 0, or (result - 8) == 0. On a hit it decodes the
//                               block's color bytes as {r=(rgb>>16)&0xFF,
//                               g=(rgb>>8)&0xFF, b=rgb&0xFF} scaled by the 1/255 float
//                               at 0x1071AE0. Branch polarity: the compare is
//                               the original instruction sequence, so
//                               branch A runs when 0.01f is NOT greater than f0 (i.e.
//                               f0 >= 0.01f, and also for NaN); branch B runs only for
//                               f0 strictly below the threshold.
//                                 branch B: pushes {r,g,b,node1+0x1D0.w} through game
//                               method 0x6FD4D0, then finds in the second map at
//                               +0x10F0 (0x6FE020); if the result equals the end
//                               iterator &+0x10F4 it returns 1 untouched, else writes
//                               {r,g,b,old.w} into the found node's +0x20/+0x30/+0x40.
//                                 branch A: when the block's +4 byte is nonzero, three
//                               floats come out of 0x6FDCC0 (cdecl, callee-cleans 0x10)
//                               and go through 0x6FD4D0 as {v24,v23,v22,node1+0x1D0.w}
//                               (EA quirk: the v24 slot is the old key copy at local
//                               +0x40, whose low byte is zeroed afterwards to form
//                               0x7018B0's third argument - with the +4 flag clear the
//                               stale dword is the key copy's low dword instead);
//                               0x7018B0 then finds in the second map (its out byte at
//                               +1 is the found flag). Three writes are unconditional:
//                               +0x60 {C,C,C,old.w} with C = the float at 0xF98964,
//                               +0x20 {r,g,b,old.w}, +0x50 {f0,f0,f0,old.w}. The
//                               found-flag byte OR the +4 flag selects the flag path
//                               (+0x30 and +0x40 = copy of node1+0x1D0, +0x20 =
//                               {r,g,b,node1+0x1D0.w}) over the blend path (+0x30 =
//                               {old+0x40.xyz, old+0x30.w}). Returns 1 on every write
//                               path - AL only, the upper EAX bits stay stale.
//   0x00702A90  ApplyParams702A90      thiscall, ret 0x0C: (a1, a2, const LotParams*).
//                               Byte +0xF and byte +0xB scale by 1/255 (0x1071AE0);
//                               the +0xF value is clamped into [0,1] with the
//                               original's pointer selects (comiss/ja: the low bound
//                               is taken only when 0.0f > v3 ordered, the high bound
//                               only when sel > 1.0f ordered, so NaN keeps the value),
//                               then calls game method 0x702270 (thiscall, 6 stack
//                               args, callee cleans 0x18) with (a1, a2, clamped, the
//                               +0 float, the +4 byte as a zero-extended dword, the
//                               byte-B float) and returns its result. EA quirk kept
//                               and documented: the original passes the +4 flag as the
//                               low byte of a dword whose upper three bytes are caller
//                               stack garbage; this rewrite passes the zero-extended
//                               byte.
//   0x00702B40  Teardown702B40         thiscall, plain ret: writes the pseudo-vtable
//                               0x00FFA1F4 at +0; takes the lot object from 0x44C470
//                               (a 2-instruction leaf reading the global at 0x11B1378)
//                               and, when it is non-null, registers this renderer
//                               under 67 hashes through vtable slot 11 (byte offset
//                               0x2C) as (this, hash, second) with second = 0xFFFFD8F1
//                               everywhere except hashes 0x0428E0A3 and 0x0428E0A8
//                               which pass 0. Then the element loop guarded by
//                               +0x102C != 0: per element a slot-14 call with a zeroed
//                               16-byte buffer, up to four slot-3 property queries
//                               (0xE92EBE5F / 0x0499DC66 / 0x017F1D88 / 0x938D7230,
//                               the last one twice) feeding strncpy/strcat through
//                               0x4FDD30/0x4FE080 into a local 0x80-byte name buffer,
//                               then slot 1. EA quirk kept: 0x6FDF40's out argument
//                               and the name buffer are written and never read (dead
//                               - kept for call-order fidelity), the dword scan over
//                               the empty container at +8 is unbounded, and a container
//                               that empties while +0x102C stays nonzero dereferences
//                               a null element exactly like the original. Finally the
//                               ten containers at +0x1188 (0x5A3AE0), +0x116C
//                               (0x6A0850), +0x1148 (0xD2BC10), +0x1128 (0x700C60),
//                               +0x110C and +0x10F0 (0x6A0850), +0x1094 (0x82BE80),
//                               +0x1054/+0x1074/+0x1034 (0x6258F0) are cleared; the
//                               seven 2-arg clears then zero their +0xC dword and,
//                               when the +8 count is > 1 unsigned, free the +4 pointer
//                               through 0x4E3F70 (cdecl, callee-cleans 4). Writes
//                               0x00FB2CBC at +0 and returns.
//   0x0070D4E0  IsMatch70D4E0          thiscall, ret 8: (a2, a3) -> nonzero only when
//                               +0x4F8 != 0, +0x1B0 == a2 and +0x1B4 == a3. Sets AL
//                               only (the upper EAX bits stay stale).
//   0x007249D0  StoreFlag7249D0        thiscall, ret 4: stores the argument's low byte
//                               at +8 and returns it (AL only).
//   0x0093E9E0  Pump93E9E0             cdecl, (int32, int32): 0x6FDE10 ->
//                               0x6FE3B0 -> vtable slot 3 with 0x017F1D88 -> 0x6F4A50
//                               -> 0x573300 -> 0x6FC9E0 -> tail 0x6FAC70 (returns its
//                               result). EA quirk kept: NO null check on 0x6FDE10's
//                               result - unlike 0x0093EA40; every other null path
//                               returns 0 (the failed pointer reads as 0 in EAX).
//   0x0093EA40  Pump93EA40             the same chain with the 0x6FDE10 null check.
//   0x0093EAA0  Pump93EAA0             the same chain, but after 0x6F4A50 it tail-jumps
//                               to 0x573320 (returns its result) and never reaches
//                               0x6FC9E0/0x6FAC70; it also tests the slot-3 result
//                               directly instead of parking it in esi.
//
// Layout: LotMatchView/LotFlagView are the two leaves' own views (this_size 0x4FC and
// 16 in the autotest manifest); LotRenderer covers the fields this unit touches - the
// pseudo-vtable at +0, the map head pair at +4/+8, the find sentinel and loop guard at
// +0x1028/+0x102C, and the ten clear containers. LotEntryNode is the map node shape
// 0x702490 reads and writes (quads at +0x20..+0x60 and +0x1D0; the node object starts
// 8 bytes below the slot the find returns).
//
// In-unit calls go directly to the rewrites; every other call runs through the ts3
// layout.h bridge (x64 builds abort there instead - there is no game code to call).
#pragma once

#include <cstdint>

#include "ts3/layout.h"

namespace ts3::lotrenderer {

// The 16-byte parameter block shared by 0x00702490 and 0x00702A90 (LotParams*).
struct LotParams {
    uint32_t f0Bits;   // +0x0 raw float bits of the "f0" threshold value
    uint8_t flag4;     // +0x4 nonzero flag (0x702490 branch flag, 0x702A90's dword low byte)
    uint8_t pad5[3];   // +0x5
    uint32_t field8;   // +0x8 dword; its top byte (+0xB) feeds 0x702A90's v4 float
    uint32_t rgbC;     // +0xC packed color; top byte (+0xF) feeds 0x702A90's v3 float
};
TS3_X86_OFFSET(LotParams, f0Bits, 0x0);
TS3_X86_OFFSET(LotParams, flag4, 0x4);
TS3_X86_OFFSET(LotParams, field8, 0x8);
TS3_X86_OFFSET(LotParams, rgbC, 0xC);
TS3_X86_SIZE(LotParams, 0x10);

// One 16-byte color quad (the shuffles in 0x00702490 shuffle whole dwords of it).
struct LotQuad {
    float x;
    float y;
    float z;
    float w;
};
TS3_X86_SIZE(LotQuad, 0x10);

// The map node 0x00702490 works on: the find returns slot+8, so the node object the
// rewrite sees starts at (found - 8). Quads are the only observed consumers; the rest
// of the 0x1E0-byte shape is payload.
struct LotEntryNode {
    uint8_t pad00[0x20];
    LotQuad q20;       // +0x20 {r,g,b,w} base / flag-path {r,g,b,node1.w}
    LotQuad q30;       // +0x30 blend result or node1+0x1D0 copy
    LotQuad q40;       // +0x40 branch B {r,g,b,w} / node1+0x1D0 copy
    LotQuad q50;       // +0x50 {f0,f0,f0,w}
    LotQuad q60;       // +0x60 {Flat,Flat,Flat,w}
    uint8_t pad70[0x160];
    LotQuad q1D0;      // +0x1D0 source quad (read-only for this unit)
};
TS3_X86_OFFSET(LotEntryNode, q20, 0x20);
TS3_X86_OFFSET(LotEntryNode, q30, 0x30);
TS3_X86_OFFSET(LotEntryNode, q40, 0x40);
TS3_X86_OFFSET(LotEntryNode, q50, 0x50);
TS3_X86_OFFSET(LotEntryNode, q60, 0x60);
TS3_X86_OFFSET(LotEntryNode, q1D0, 0x1D0);
TS3_X86_SIZE(LotEntryNode, 0x1E0);

// The 16-byte clear-block every cleanup call in 0x00702B40 uses: clear(base, +4, +8),
// then +0xC = 0 and, when the +8 count is > 1 unsigned, free the +4 pointer. The
// 0x6A0850 blocks only pass +0xC.
struct LotContainer {
    uint8_t pad00[4];
    void* f1;        // +4  element/begin pointer (freed through 0x4E3F70)
    uint32_t f2;     // +8  count (unsigned compare against 1)
    uint32_t f3;     // +0xC flag/argument, zeroed after the clear
};
TS3_X86_OFFSET(LotContainer, f1, 0x4);
TS3_X86_OFFSET(LotContainer, f2, 0x8);
TS3_X86_OFFSET(LotContainer, f3, 0xC);
TS3_X86_SIZE(LotContainer, 0x10);

// The lot renderer object as this unit sees it. Offsets above +0x1034 are the ten
// clear containers; +0x1B0/+0x1B4/+0x4F8 (0x0070D4E0) and the +8 byte (0x007249D0)
// belong to the leaves' own views - if those entries share this object their fields
// overlap head08, which is what the original addresses say.
struct LotRenderer {
    void* vtable00;           // +0x00 written 0x00FFA1F4 / 0x00FB2CBC by 0x00702B40
    void* head04;             // +0x04 map head read by the 0x702B40 loop (head = [this+4])
    void* head08;             // +0x08 fallback head slot of the same map (scan from here)
    uint8_t pad0C[0x101C];    // +0x0C .. +0x1027
    void* sentinel1028;       // +0x1028 find-miss sentinel of the +4 map
    uint32_t active102C;      // +0x102C element-loop guard (!= 0 runs the loop)
    uint8_t pad1030[4];       // +0x1030
    LotContainer c1034;       // +0x1034 clear via 0x6258F0
    uint8_t pad1044[0x10];
    LotContainer c1054;       // +0x1054 clear via 0x6258F0
    uint8_t pad1064[0x10];
    LotContainer c1074;       // +0x1074 clear via 0x6258F0
    uint8_t pad1084[0x10];
    LotContainer c1094;       // +0x1094 clear via 0x82BE80
    uint8_t pad10A4[0x4C];    // +0x10A4 .. +0x10EF
    LotContainer c10F0;       // +0x10F0 second map (0x6FE020/0x7018B0), clear via 0x6A0850
    uint8_t pad1100[0xC];
    LotContainer c110C;       // +0x110C clear via 0x6A0850
    uint8_t pad111C[0xC];
    LotContainer c1128;       // +0x1128 clear via 0x700C60
    uint8_t pad1138[0x10];
    LotContainer c1148;       // +0x1148 clear via 0xD2BC10
    uint8_t pad1158[0x14];
    LotContainer c116C;       // +0x116C clear via 0x6A0850
    uint8_t pad117C[0xC];
    LotContainer c1188;       // +0x1188 clear via 0x5A3AE0

    // 0x00702490: key lookup plus the two-branch color apply. Returns 0/1 (AL only).
    uint8_t ApplyEntryColor702490(uint64_t key, const LotParams* params);

    // 0x00702A90: clamps two parameter bytes and tail-calls game method 0x702270.
    int32_t ApplyParams702A90(uint32_t a1, uint32_t a2, const LotParams* params);

    // 0x00702B40: pseudo-vtable swap, 67 registrations, element loop, container clears.
    void Teardown702B40();
};
TS3_X86_OFFSET(LotRenderer, vtable00, 0x0);
TS3_X86_OFFSET(LotRenderer, head04, 0x4);
TS3_X86_OFFSET(LotRenderer, head08, 0x8);
TS3_X86_OFFSET(LotRenderer, sentinel1028, 0x1028);
TS3_X86_OFFSET(LotRenderer, active102C, 0x102C);
TS3_X86_OFFSET(LotRenderer, c1034, 0x1034);
TS3_X86_OFFSET(LotRenderer, c1054, 0x1054);
TS3_X86_OFFSET(LotRenderer, c1074, 0x1074);
TS3_X86_OFFSET(LotRenderer, c1094, 0x1094);
TS3_X86_OFFSET(LotRenderer, c10F0, 0x10F0);
TS3_X86_OFFSET(LotRenderer, c110C, 0x110C);
TS3_X86_OFFSET(LotRenderer, c1128, 0x1128);
TS3_X86_OFFSET(LotRenderer, c1148, 0x1148);
TS3_X86_OFFSET(LotRenderer, c116C, 0x116C);
TS3_X86_OFFSET(LotRenderer, c1188, 0x1188);

// 0x0070D4E0's view of its object (autotest this_size 0x4FC): three dword reads only.
struct LotMatchView {
    uint8_t pad00[0x1B0];
    uint32_t field1B0;    // +0x1B0 compared against arg 1
    uint32_t field1B4;    // +0x1B4 compared against arg 2
    uint8_t pad1B8[0x340];
    uint32_t enabled4F8;  // +0x4F8 nonzero gate

    // 0x0070D4E0: three-way match test. Returns the AL-only 0/1 of the original
    // (compare the low byte only - the upper EAX bits are stale in the original).
    uint8_t IsMatch70D4E0(uint32_t a2, uint32_t a3) const;
};
TS3_X86_OFFSET(LotMatchView, field1B0, 0x1B0);
TS3_X86_OFFSET(LotMatchView, field1B4, 0x1B4);
TS3_X86_OFFSET(LotMatchView, enabled4F8, 0x4F8);
TS3_X86_SIZE(LotMatchView, 0x4FC);

// 0x007249D0's view of its object (autotest this_size 16): one byte write at +8.
struct LotFlagView {
    uint8_t pad00[8];
    uint8_t flag8;  // +8 written with the argument's low byte
    uint8_t pad09[7];

    // 0x007249D0: stores value's low byte at +8 and returns it (AL only).
    uint8_t StoreFlag7249D0(uint8_t value);
};
TS3_X86_OFFSET(LotFlagView, flag8, 0x8);
TS3_X86_SIZE(LotFlagView, 0x10);

// The three cdecl pumps (0x0093E9E0 lacks the 0x6FDE10 null check the others have).
int32_t Pump93E9E0(int32_t a1, int32_t a2);
int32_t Pump93EA40(int32_t a1, int32_t a2);
int32_t Pump93EAA0(int32_t a1, int32_t a2);

}  // namespace ts3::lotrenderer
