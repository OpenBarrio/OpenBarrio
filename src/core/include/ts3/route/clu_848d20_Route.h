// Cluster unit clu_848d20_Route (subsystem Route): the cost-chooser leaf 0x00848D20
// and the thiscall entry 0x0084AF90 that feeds it from a Route tuner object. Semantics
// from the TS3W.exe 1.67.2 listings (the original addresses are noted on each
// function below).
//
//   0x00848D20  SelectValue848D20  cdecl, 14 arguments, a leaf (no game code runs
//                               inside): walks a ladder of `iter` candidate values,
//                               scores each against four float weights plus a hi/lo
//                               clamp violation, keeps the strict minimum, writes the
//                               winning value back through `inout` (select, then
//                               clamp) and returns the winning iteration index
//   0x0084AF90  RouteTuner::Update84AF90  thiscall, ret 8, arguments (int mode,
//                               char force): resolves the canvas target, bumps its
//                               refcount twice, asks the walker object at +0xC4 for a
//                               64-bit id (vtable slot 4), gets the base value from
//                               0x00849810, runs the chooser above with the tuner's
//                               fields, stores the result when it moved or when
//                               `force` is set, then releases the target
//
// Float globals read live on x86 (each load keeps the exact address the original
// reads; the .data tunables are writable, so the x86 build must not fold them into
// literals): 0x01026FD0 FLT_MAX, 0x0106D6CC 0.01f, 0x0107A538 1.0f, 0x01166494 1.0f
// (W4), 0x01166498 100.0f (W2), 0x011D9270 0.0f (W3), 0x011D9274 0.0f (W1).
//
// EA quirks kept: the denominator select is the original's sign-bit test on
// (0.01f - v47) (negative -> v47, else 0.01f - exact even for -0.0/NaN inputs); the
// hi-violation term is OVERWRITTEN (not max'd) by the lo violation; the best update
// is a strict `>` (comiss/jbe), so equal costs keep the EARLIER iteration; the final
// clamp is select-then-clamp against the ORIGINAL read of *inout; a2 of 848D20 (the
// target pointer at the 84AF90 call) is never read; the array sums walk j = 1..count
// with 4-wide quads plus a tail, mode 2 ignores the cutoff per element while mode 3
// gates each element separately, and the unsigned compares (count > cutoff etc.)
// let a huge count/cutoff run off any buffer exactly like the original does; the
// count/cutoff fields of 84AF90 are read as the low byte of flag (+0x253) and the
// raw dwords at +0x248/+0x24C.
//
// In-unit calls go directly to the rewrite below (84AF90 -> SelectValue848D20);
// the not-yet-rewritten game calls (0x00C93970, vtable slots, 0x00849810) run through
// the ts3 layout.h bridge - x64 builds abort there instead, no game code to call.
#pragma once

#include <cstdint>

#include "ts3/layout.h"

namespace ts3::route {

// TS3W.exe 0x00848D20 - `mode`/`unused` are a1/a2, `num`/`step`/`bonus`/`extra` a3..a6,
// `arrA`/`arrB` the (32-bit) float-array addresses a7/a8, `count`/`cutoff` a9/a10,
// `inout` the a11 float*, `lo`/`hi` a12/a13 and `flag` the low byte of a14.
int32_t __cdecl SelectValue848D20(int32_t mode, int32_t unused, float num, float step,
                                  float bonus, float extra, int32_t arrA, int32_t arrB,
                                  uint32_t count, uint32_t cutoff, float* inout, float lo,
                                  float hi, int8_t flag);

// The x86 object 0x0084AF90 runs on: every offset below is the game's own (asserted
// for x86 in the .cpp), only the head the function touches is modelled (596 bytes).
struct RouteTuner {
    uint8_t pad00[0x14];
    void* canvas;          // +0x14  Canvas*, 0x00C93970 reads its dword at +8
    uint8_t pad18[0x64];   // +0x18..+0x7B
    int32_t field124;      // +0x7C  5th-from-last bridge argument of 0x00849810 (v9)
    uint8_t pad80[4];      // +0x80..+0x83
    int32_t field132;      // +0x84  4th-from-last bridge argument of 0x00849810 (v10)
    uint8_t pad88[0x3C];   // +0x88..+0xC3
    void* walker;          // +0xC4  object whose vtable slot 4 returns the 64-bit id
    uint8_t padC8[0x64];   // +0xC8..+0x12B
    float step;            // +0x12C chooser a4
    float bonus;           // +0x130 chooser a5
    float extra;           // +0x134 chooser a6
    uint8_t pad138[4];     // +0x138..+0x13B
    int32_t arrA;          // +0x13C chooser a7 (32-bit float-array address)
    int32_t arrB;          // +0x140 chooser a8 (32-bit float-array address)
    uint8_t pad144[0x18];  // +0x144..+0x15B
    float lo;              // +0x15C chooser a12
    float hi;              // +0x160 chooser a13
    float chosen;          // +0x164 chooser a11 (the inout value)
    uint8_t pad168[4];     // +0x168..+0x16B
    int32_t lastIndex;     // +0x16C last stored index (a11 of the compare)
    int32_t clear368;      // +0x170 cleared to 0 on a store
    int32_t clear372;      // +0x174 cleared to 0 on a store, subtracted in the compare
    uint8_t pad178[0xD0];  // +0x178..+0x247
    uint32_t cutoff;       // +0x248 chooser a10
    uint32_t count;        // +0x24C chooser a9
    uint8_t pad250[3];     // +0x250..+0x252
    int8_t flag;           // +0x253 chooser a14 (read as a byte)

    // TS3W.exe 0x0084AF90 - `mode` (the a2 argument) goes straight into the chooser,
    // `force` (a3, read as a byte) forces the result store.
    int32_t Update84AF90(int32_t mode, int8_t force);
};

}  // namespace ts3::route
