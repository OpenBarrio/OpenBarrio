// src/core/misc/cls_bf63d0_x.cpp - declared here.
//
// Diagonal shed-wall locator sampler: the same grid-sampler family as cls_bf4b20_x
// (CreatingNewRoof_3) but on the wider object whose dimensions live at +0xDC/+0xE0, with the
// four corner pairs at +0x7C..+0x98 and a local-to-grid transform around the centre K1.
// EA class `AddDiagonalShedWallLocators`, subsystem `misc/`.
#pragma once

#include <cstddef>
#include <cstdint>

namespace ts3 {
namespace misc {

// Grid sampler + diagonal record emitter (TS3W.exe 0x00BF63D0 et al.).
//
// The IDA declaration in the original disassembly is marked
// "recovered-by-access-width/guess (unverified)" and only reaches +0x48, so the layout below
// follows the unit's own accesses (every offset it touches is pinned by the static_asserts at
// the bottom of the .cpp; IDA's sizeof = 228 = 0xE4 agrees with +0xDC/+0xE0 plus one dword).
//
// The object dispatches Width/Height/Sample/corner through its vtable (0x01072A70 once
// 0x00BFADB0 has run): +0x10 0x00A45BA0, +0x14 0x00A45BB0, +0x28 0x00A46B70, +0x40 0x00BF4B20.
struct AddDiagonalShedWallLocators {
    void* vtable;         // +0x00: 0x01072A70 after 0x00BFADB0, 0x0107396C after the destructor
    void* _header_04[3];  // +0x04: 24-byte vector header (not touched by this unit)
    float _gap_10[8];     // +0x10: not touched by this unit
    uint8_t f_30;         // +0x30: sampler sign flag - 0x00A46B70 negates +0x44 when set
    uint8_t _pad_31[7];   // +0x31: not touched by this unit
    uint8_t f_38;         // +0x38: primary direction flag, set by 0x00BFADB0 from the aspect
    uint8_t f_39;         // +0x39: secondary direction flag
    uint8_t f_3A;         // +0x3A: ascending-rows flag
    uint8_t _pad_3B;      // +0x3B
    float f_3C;           // +0x3C: grid scale (the sampler multiplies each cell by it)
    float f_40;           // +0x40: sampled height addend
    float f_44;           // +0x44: sampled gradient
    void* grid;           // +0x48: float* array of width * height cells, freed by the dtor
    float _gap_4C[3];     // +0x4C: not touched by this unit
    float f_58;           // +0x58: sampled base height
    float _gap_5C[8];     // +0x5C: not touched by this unit
    float k0x;            // +0x7C: corner K0 (x, z)
    float k0z;            // +0x80
    float k1x;            // +0x84: corner K1 = the object centre (the transform's origin)
    float k1z;            // +0x88
    float k2x;            // +0x8C: corner K2
    float k2z;            // +0x90
    float k3x;            // +0x94: corner K3
    float k3z;            // +0x98
    float _gap_9C[16];    // +0x9C: not touched by this unit
    int32_t cellsX;       // +0xDC: vtable +0x10 returns cellsX * 4 + 1
    int32_t cellsY;       // +0xE0: vtable +0x14 returns cellsY * 4 + 1

    // TS3W.exe 0x00BF64C0 (__thiscall, the original instruction sequence, vtable slot +0x44): writes the unit
    // direction triple selected by the three direction flag bytes into out[0..2]; the negative
    // component comes from -0.0f - g, so its sign is the sampler's, not a literal.  Of the four
    // stack arguments only the third is ever read, the first, second and fourth are dead.
    // Returns (out & ~0xFF) | 1: the original instruction sequence leaves the output pointer's upper bytes in EAX (a
    // residue callers ignore, kept because the harness compares registers).  A member, and
    // `out` void*, so that hookgen's thiscall wrapper - which calls
    // `static_cast<Class*>(self)->Method(buf args as void*)` - appends without edits; this is
    // the only entry point of this unit the random runner can drive.
    int DirectionVector(uint32_t a0, uint32_t a1, void* out, uint32_t a3);

    // TS3W.exe 0x00BF63D0 (__thiscall, `ret 8`, vtable slot +0x3C): does the world point
    // (a2, a3) land on a diagonal grid line?  Translates it into grid coordinates through
    // 0x00A46D00, floors 4 * each coordinate the way fld/fist does, samples that cell and
    // compares it against the corner value with a 0.001 tolerance.  0 when they disagree (NaN
    // included), otherwise 12 when the unsigned width >= the unsigned height and 3 when it is
    // smaller - with EAX's upper bytes left holding the height (see the .cpp).
    int PointOnDiagonalGridLine(float a2, float a3);

    // TS3W.exe 0x00BFADB0 (__thiscall, the original instruction sequence): the constructor.  Runs the base
    // constructor with an extra 1, installs the class vtable and sets f_38 from the cell
    // counts: 1 when cellsX > cellsY, 0 when they differ the other way, and for equal counts
    // the byte copied from [*[a0 + 0x2AC] + 0x88] - two loads the original never null-checks.
    AddDiagonalShedWallLocators* Create(void* a0, void* a1, void* a2, void* a3, void* a4);

    // TS3W.exe 0x00BFBDE0 (__thiscall, `ret 4`, vtable slot +0x00): the destructor.  Frees the
    // grid array through the game's null-safe Free, leaves the object on the base vtable
    // 0x0107396C unconditionally (the flag test happens before that store but does not guard
    // it) and frees the object itself when bit 0 of the deleting flag is set.
    AddDiagonalShedWallLocators* Destroy(uint32_t flags);

    // TS3W.exe 0x00BFE0B0 (__thiscall, the original instruction sequence, vtable slot +0x70): walks the grid's two
    // edge strips in strides of 4 (a0's low byte set) or 2 and appends the 6-float diagonal
    // records into the caller's vector, skipping the cells that are not flat against the
    // lowest corner sample and the pairs that both sit on the corner value.  Returns AL = 1
    // with the last width getter's EAX - 1 above it.
    int EmitDiagonalRecords(uint32_t a0, void* out, uint32_t a2);

    // TS3W.exe 0x00BFE800 (__thiscall, `ret 4`, vtable slot +0x74): appends exactly three
    // corner-to-corner records through 0x00BFC780, one per side of the locator's quad, with
    // the endpoints and the height each endpoint carries selected by the direction flags.
    // Returns AL = 1 with the last push-back's EAX (the record's address) above it.
    int EmitCornerRecords(void* out);
};
// The .cpp pins the layout with TS3_X86_SIZE / TS3_X86_OFFSET, which only apply to x86.

}  // namespace misc
}  // namespace ts3
