// src/core/misc/cls_bf4b20_x.cpp - declared here.
//
// Terrain/roof height sampler: a grid of floats sampled by (column, row) through vtable
// accessors, plus the per-cell records it emits.  EA class `CreatingNewRoof_3`, subsystem
// `misc/`.
#pragma once

#include <cstddef>
#include <cstdint>

namespace ts3 {
namespace misc {

// Height/gradient sampler over a width x height grid of floats (TS3W.exe 0x00BF4B20 et al.).
//
// The IDA declaration in the original disassembly is marked
// "recovered-by-access-width/guess (unverified)" and its padding disagrees with the unit's
// own assembly (which reads the grid pointer at +0x48, not +0x4C), so the layout below
// follows the accesses.  The static_asserts at the bottom of the .cpp pin every offset this
// unit touches; they hold in the 32-bit build, where the harness hands real game objects to
// this code.
struct CreatingNewRoof_3 {
    void* vtable;         // +0x00: the game's vtable (0x010729F8 once 0x00BFAD20 has run)
    void* _header_04[3];  // +0x04: 24-byte vector header the record builders append through
    float f_10;           // +0x10: rectangle corner x - compared against f_1C by the ctor
    float f_14;           // +0x14: rectangle corner y - compared against f_20 by the ctor
    float f_18;           // +0x18
    float f_1C;           // +0x1C: opposite rectangle corner x
    float f_20;           // +0x20: opposite rectangle corner y
    float f_24;           // +0x24
    float f_28;           // +0x28
    float f_2C;           // +0x2C
    uint8_t f_30;         // +0x30: selects between f_44 and 1.0f - f_44 in the sampler
    uint8_t _pad_31[7];   // +0x31: not touched by this unit
    uint8_t f_38;         // +0x38: "ramp along rows" / primary direction flag
    uint8_t f_39;         // +0x39: "ascending columns" flag
    uint8_t f_3A;         // +0x3A: "ascending rows" flag
    uint8_t _pad_3B;      // +0x3B
    int f_3C;             // +0x3C: grid scale (the sampler multiplies the cell by it)
    float f_40;           // +0x40: sampled height offset
    float f_44;           // +0x44: sampled gradient scale
    void* grid;           // +0x48: float* array of width * height cells, freed by the dtor
    float _gap_4C[3];     // +0x4C: not touched by this unit
    float f_58;           // +0x58: sampled base height (read by the sampler at 0x00A45EC0)
    float f_5C;           // +0x5C: origin x (0x00BF6280 subtracts it from the world point)
    float f_60;           // +0x60: origin y
    float f_64;           // +0x64
    float f_68;           // +0x68
    float f_6C;           // +0x6C
    float f_70;           // +0x70
    int f_74;             // +0x74: grid width  (what vtable slot +0x10 returns)
    int f_78;             // +0x78: grid height (what vtable slot +0x14 returns)

    // TS3W.exe 0x00BF6360 (__thiscall, the original instruction sequence, vtable slot +0x44): writes the unit
    // direction triple matching the direction flags into out[0..2].  Returns
    // (out & ~0xFF) | 1: the original instruction sequence leaves the output pointer's upper bytes in EAX (a residue
    // callers ignore, kept because the harness compares registers).  Arguments 0, 1 and 3 are
    // dead in the original, kept for ABI.  A member, and `out` void*, so that hookgen's
    // thiscall wrapper - which calls `static_cast<Class*>(self)->Method(buf args as void*)` -
    // appends without edits; this is the only entry point the random runner can drive.
    int DirectionVector(uint32_t a0, uint32_t a1, void* out, uint32_t a3);
    // TS3W.exe 0x00BFD880 (__thiscall, ret 12, slot +0x70): append boundary segments; stride two still samples adjacent cells.
    int EmitCellRecords(int a0, void* out, int a2);
    // TS3W.exe 0x00BFDCA0 (__thiscall, ret 4, slot +0x74): append three corner segments; preserve unordered minimum selection.
    int EmitCornerRecords(void* out);
};
// The .cpp pins the layout with TS3_X86_SIZE / TS3_X86_OFFSET, which only apply to x86.

// --- entry points ---------------------------------------------------------------------

// TS3W.exe 0x00BF4B20 (__thiscall, plain `ret`, vtable slot +0x40): the corner sampler.
// Dispatches on the direction flags and tail-calls the grid sampler (slot +0x28) with the
// matching corner coordinate.  The result reaches the callers on the maths stack, so this is
// declared as returning float.
float CornerSample(CreatingNewRoof_3* self);

// TS3W.exe 0x00BF6040 (__thiscall, plain `ret`, vtable slot +0x20): fills the float grid
// with a linear ramp - per row when f_38 is set, per column otherwise, ascending or descending
// according to f_3A / f_39.  The original's EAX is a vectorised-loop byproduct no caller
// reads, so this is void.
void FillGrid(CreatingNewRoof_3* self);

// TS3W.exe 0x00BF6280 (__thiscall, `ret 8`, vtable slot +0x3C): does the world point (ax, ay)
// land on a grid line?  0 when the sampled cell disagrees with the corner value by 0.001 or
// more; otherwise 12 when width >= height and 3 when width < height, with the register's
// upper bytes left holding the height (see the .cpp).
int PointOnGridLine(CreatingNewRoof_3* self, float ax, float ay);

// TS3W.exe 0x00BFAD20 (__thiscall, the original instruction sequence): the constructor.  Runs the base constructor,
// installs the class vtable and sets f_38 from the rectangle's aspect.
CreatingNewRoof_3* Create(CreatingNewRoof_3* self, void* a0, void* a1, void* a2, void* a3,
                          void* a4);

// TS3W.exe 0x00BFBB40 (__thiscall, `ret 4`, vtable slot +0x00): the destructor.  Frees the
// grid array, leaves the object on the base vtable and frees the object itself when bit 0 of
// the deleting flag is set.
CreatingNewRoof_3* Destroy(CreatingNewRoof_3* self, uint32_t flags);

#if defined(_M_IX86)
// ABI adapters for replace-only hooks (ECX self, ignored EDX, callee-pop stack args).
int __fastcall RoofEmitCellRecordsHook(CreatingNewRoof_3* self, void*, int a0, void* out, int a2);
int __fastcall RoofEmitCornerRecordsHook(CreatingNewRoof_3* self, void*, void* out);
#endif

}  // namespace misc
}  // namespace ts3
