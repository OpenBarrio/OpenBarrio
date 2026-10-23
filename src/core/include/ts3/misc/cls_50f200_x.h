// Batch unit cls_50f200_x (subsystem misc): 24 TS3W.exe entry points from one IDA class
// cluster. Three are __cdecl lot-placement helpers over a rectangle (0x0050F200 clips an
// inner rect against an outer one and maps it into a destination rect, 0x0050F580 the same
// with a UV result, 0x0050F720 tiles a rect with cells of a fixed size). The rest are the
// small entry points of two game classes whose vtables sit at 0x0101C0D8 / 0x0101C0C0 (an
// "IconDrawable" record) and 0x00FF42F8 / 0x00FF4518 (the RectangleWindowLight variants),
// plus the identity function 0x00F626F0 that 123 callers use.
//
// The thiscall-shaped entries are declared __fastcall (self in ecx, ignored second
// register) so the harness can swap them for the originals; the original ret N cleanup is
// mirrored by the argument count. Calls into units that are not rewritten yet go through
// ts3::CallGame* (ts3/layout.h).
//
// EA quirks kept (see the .cpp for the exact paths):
//  * 0x007E4E10 returns its out pointer OR 1 in the full EAX (only AL is a real bool).
//  * 0x007E4EA0 returns whatever EAX holds when the object has no +0x1C pointer.
//  * 0x007E4D40/0x007E4FF0 answer the magic interface id 0xEEC58382 without a game call.
//  * 0x006BD7C0/0x006BE020 use the approximate rcpps/rsqrtps Newton steps, not exact math.
#pragma once

#include <cstdint>

#include "ts3/layout.h"

namespace ts3::misc {

// One object of the vtable-0x0101C0D8 class. Only the fields this unit touches are named;
// +0x20 and +0x30 are indexable tables (0x007E4DF0/0x007E4E00/0x007E4EA0 use table +0x20
// for flag bit 3 clear and table +0x30 for flag bit 3 set, masking the index with ~8).
struct IconObject {
    void** vtbl;           // +0x00 primary vtable (0x0101C0D8)
    void** vtblSecondary;  // +0x04 secondary interface vtable (0x0101C0C0)
    uint32_t pad08;        // +0x08 unused by this unit
    float f0C;             // +0x0C cell size scale, 0x007E4DD0 stores it as a float
    int32_t f10;           // +0x10 flag word, 0x007E4DA0/0x007E4EA0 read it
    int32_t f14;           // +0x14 horizontal anchor mode, 0x007E4EA0
    int32_t f18;           // +0x18 vertical anchor mode, 0x007E4EA0
    void* f1C;             // +0x1C cell info object (ints at +0x1C/+0x20)
};
TS3_X86_OFFSET(IconObject, vtbl, 0x00);
TS3_X86_OFFSET(IconObject, vtblSecondary, 0x04);
TS3_X86_OFFSET(IconObject, f0C, 0x0C);
TS3_X86_OFFSET(IconObject, f10, 0x10);
TS3_X86_OFFSET(IconObject, f14, 0x14);
TS3_X86_OFFSET(IconObject, f18, 0x18);
TS3_X86_OFFSET(IconObject, f1C, 0x1C);
TS3_X86_SIZE(IconObject, 0x20);

// ---- 0x0050F2xx lot-placement helpers (all __cdecl with the owner as first argument) ----

// TS3W.exe 0x0050F200: clip `inner` against `outer` (through 0x00505BD0) and call the
// object's vtable slot 0x50 (a6 == 0) or 0x5C (a6 != 0) with the clipped rect and the
// mapped destination rect; when the inner rect is already inside outer the slot is called
// directly. Returns the vtable method's result, or `a5` (the outer pointer!) on the size
// early-outs, or 0x00505BD0's result when the clipped rect is degenerate.
int __cdecl ClipRect50F200(void* self, float* inner, float* outer, int a4, float* a5, int a6);

// TS3W.exe 0x0050F580: like 0x0050F200 but always calls slot 0x5C with a UV rect
// ({0,0,1,1} when inner is inside outer, else the clipped/mapped spans).
int __cdecl ClipRect50F580(void* self, float* inner, int a3, int a4, float* outer);

// TS3W.exe 0x0050F720: tile `rect` with cells of size (cellW(a3)*sx, cellH(a3)*sy),
// starting at rect[0]+ox, rect[1]+oy, and call 0x0050EEF0 for each tile. ox/oy are wrapped
// into (-size, size] first; cells smaller than 1.0f in either axis make it a no-op.
void __cdecl TileRect50F720(void* obj, float* rect, void* desc, float sx, float sy, float ox,
                            float oy);

// ---- class 0x0101C0D8 (primary vtable 0x0101C0D8, secondary 0x0101C0C0) ----------------

// TS3W.exe 0x005334C0: vtable slot 15, returns the float at +0x0C (x87 st0 return).
float __fastcall Get5334C0(void* self, void*);

// TS3W.exe 0x007E4D30: secondary vtable slot 5, returns the class-descriptor constant
// 0xBF7BFE30 (-0.9843473434448242f as a dword).
int __fastcall Const7E4D30(void* self, void*);

// TS3W.exe 0x007E4D40: primary vtable slot 3 (secondary slot 3 enters through 0x007E4FF0,
// which subtracts 4 from self first). For the magic id 0xEEC58382 it returns self + 4, or
// self when the adjusted self is 0, or 0 when the adjusted self is 4; any other id
// tail-calls the cls_504d90_x entry 0x00504D90 with the same argument.
void* __fastcall GetInterface7E4D40(void* self, void*, int magic);
void* __fastcall GetInterface7E4FF0(void* self, void*, int magic);

// TS3W.exe 0x007E4DA0: set or clear bits in f10 and forward the result to vtable slot 0x30.
int __fastcall ModifyBits7E4DA0(void* self, void*, int mask, int set);

// TS3W.exe 0x007E4DD0: store a float at +0x0C.
void __fastcall Set0C7E4DD0(void* self, void*, float v);
// TS3W.exe 0x007E4DE0: store an int at +0x18 and return it.
int __fastcall Set18_7E4DE0(void* self, void*, int v);
// TS3W.exe 0x007E4DF0: read the +0x20 table at an unvalidated index.
int __fastcall Get20_7E4DF0(void* self, void*, int index);
// TS3W.exe 0x007E4E00: write the +0x20 table at an unvalidated index, returning the value.
int __fastcall Set20_7E4E00(void* self, void*, int index, int v);
// TS3W.exe 0x007E4E10: when f1C is set, write {f1C[0x1C], f1C[0x20]} as floats to out and
// return `out | 1` in the full EAX (EA quirk: the high bits are the pointer, not a bool);
// otherwise leave out alone and return 0. The last two arguments are unused.
uint32_t __fastcall GetCell7E4E10(void* self, void*, float* out, int a2, int a3);
// TS3W.exe 0x007E4E70: fill a class-registration record {descriptor 0x0101C080, self-4, 1,
// secondary vtable slot 5 result} and return the slot result.
int __fastcall ClassReg7E4E70(void* self, void*, void* rec);
// TS3W.exe 0x007E4EA0: look up a placement record through 0x0050E850 (the first stack
// argument is the lookup object), hand it the table entry selected by *flags (the third
// stack argument; bit 3 picks the table at +0x30 and clears the bit before indexing),
// anchor the rectangle given by the second stack argument through f14/f18 (modes 0 and 1
// are the same: keep the start, extend the end; 2 shrinks the end; 3 centers) and call
// 0x0050EEF0 with (record, anchored vec, cell, original rect, 0). Returns that call's
// result; when f1C is null the original returns whatever EAX held (kept as 0 here).
int __fastcall PlaceClip7E4EA0(void* self, void*, void* lookup, float* rect, uint32_t* flags);
// TS3W.exe 0x007E4FE0: secondary interface destructor (self is the +4 interface; the
// primary object is self-4), flags argue for the deleting operator-delete call.
void* __fastcall DtorSec7E4FE0(void* self, void*, int flags);
// TS3W.exe 0x007E5000: primary destructor, same shape.
void* __fastcall DtorPrimary7E5000(void* self, void*, int flags);
// TS3W.exe 0x00F626F0: vtable slot 8 and 123 direct callers; returns its this pointer.
void* __fastcall IdentityF626F0(void* self, void*);

// ---- class 0x00FF42F8 / 0x00FF4518 (RectangleWindowLight) ----------------------------

// TS3W.exe 0x006BD690: transform the three stored vectors by the 4x4 column matrix at
// `matrix`, store a negated cross product at +0x190, then run vtable slots 0x44 and 0x50
// (slot 0x44 twice). Returns the last slot 0x44 result.
int __fastcall SetTransform6BD690(void* self, void*, const float* matrix);
// TS3W.exe 0x006BD7C0: copy `v` to out with the z lane zeroed for the length; when the
// squared length exceeds +0x1A0, rescale (including the w lane) to length +0x1A4 using an
// approximate reciprocal square root (rcpps + one Newton step). Returns out.
void __fastcall NormalizeToRadius6BD7C0(void* self, void*, float* out, const float* v);
// TS3W.exe 0x006BE020: distance/falloff sample written to `out` as a broadcast float.
// Reads the runtime floats 0x011D0A60 and 0x011D0A68 and the cap at 0x011D1160.
void __fastcall Sample6BE020(void* self, void*, const float* point, const float* dir,
                             float* out);
// TS3W.exe 0x006BFDC0: like 0x006BE020 but with three dot products, a sqrt and a call to
// 0x006BFAF0 (unusual EAX/ECX ABI). Returns 0x006BFAF0's result.
int __fastcall Sample6BFDC0(void* self, void*, const float* point, const float* a1,
                            float* out);
// TS3W.exe 0x006C04F0: RectangleWindowLight_2 constructor (vtable 0x00FF42F8).
void* __fastcall Ctor6C04F0(void* self, void*, void* a2, int a3, int a4);
// TS3W.exe 0x006C1860: RectangleWindowLight_8 constructor (vtable 0x00FF4518).
void* __fastcall Ctor6C1860(void* self, void*, void* a2, int a3, int a4);
// TS3W.exe 0x006C1980: place or sample a light against a point (bit 0x20 of +0x100 picks
// the path); returns the called game method's result.
int __fastcall Func6C1980(void* self, void*, const float* point, void* a1);

}  // namespace ts3::misc