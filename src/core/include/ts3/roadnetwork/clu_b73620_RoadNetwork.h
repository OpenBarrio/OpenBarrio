#pragma once

#include <cstdint>

#include "ts3/layout.h"

namespace ts3::roadnetwork {

// TS3W.exe 0x00B79F90: find the first non-empty vector range (thiscall, ret 4).
uint32_t* B79F90(const uint32_t* rangeTable, uint32_t* out);

// TS3W.exe 0x00C29EB0: tile record accessor (thiscall, ret 4).
void* C29EB0(const void* tileTable, int32_t index);

// TS3W.exe 0x00B77C50: mark tile records intersecting a rectangle (thiscall, ret 4).
void B77C50(void* roadState, const float* bounds);

// A 16-byte game vector. The original x86 code loads and stores these with movaps, so
// game-owned copies (and every Vec4 this code hands to the game) are 16-byte aligned.
struct alignas(16) Vec4 {
    float x, y, z, w;
};

// The road-segment object 0x00B73960 works on (its `this`). +0x10 is the rectangle
// handed to 0x00B77C50, +0x24 a "disabled" flag and +0x30..+0x60 the four cubic
// control points.
struct RoadSegment {
    void* vtable;          // +0x00
    uint8_t reserved04[0x0C];
    float bounds[4];       // +0x10
    uint8_t reserved20[4];
    uint8_t disabled;      // +0x24
    uint8_t reserved25[0x0B];
    float p0[4];           // +0x30
    float p1[4];           // +0x40
    float p2[4];           // +0x50
    float p3[4];           // +0x60
};
TS3_X86_OFFSET(RoadSegment, bounds, 0x10);
TS3_X86_OFFSET(RoadSegment, disabled, 0x24);
TS3_X86_OFFSET(RoadSegment, p3, 0x60);
TS3_X86_SIZE(RoadSegment, 0x70);

// An entry of the context's object lists (+0x10..+0x1C is its axis-aligned box,
// +0xB2/+0x1A4 are object ids and +0x00 its vtable).
struct RoadObject {
    void* vtable;          // +0x00
    uint8_t reserved04[0x0C];
    float box[4];          // +0x10
    uint8_t reserved20[0x92];
    uint16_t idB2;         // +0xB2
    uint8_t reservedB4[0xF0];
    uint16_t id1A4;        // +0x1A4
};
TS3_X86_OFFSET(RoadObject, box, 0x10);
TS3_X86_OFFSET(RoadObject, idB2, 0xB2);
TS3_X86_OFFSET(RoadObject, id1A4, 0x1A4);

// The `this` of 0x00B85C50/0x00B86150/0x00B86290/0x00B863F0/0x00B86530: the tile
// lookup table at +0x0C, a scale at +0x14, three object lists and two range pairs.
struct RoadNetworkContext {
    uint8_t reserved00[0x0C];
    void* tileTable;       // +0x0C
    uint8_t reserved10[4];
    float scale;           // +0x14
    uint8_t reserved18[0x18];
    void** range0;         // +0x30 begin/end pair also used as list head
    void** range0End;      // +0x34
    uint8_t reserved38[0x1C];
    void** range1;         // +0x54
    void** range1End;      // +0x58
    uint8_t reserved5C[0x1C];
    void** list0;          // +0x78
    void** list0End;       // +0x7C
    uint8_t reserved80[0x1C];
    void** list1;          // +0x9C
    void** list1End;       // +0xA0
    uint8_t reservedA4[0x2C];
    void** list2;          // +0xD0
    void** list2End;       // +0xD4
    uint8_t reservedD8[0x3C];
    uint8_t* tileBase;     // +0x114: array of 0x250-byte per-tile blocks
};
TS3_X86_OFFSET(RoadNetworkContext, tileTable, 0x0C);
TS3_X86_OFFSET(RoadNetworkContext, scale, 0x14);
TS3_X86_OFFSET(RoadNetworkContext, range0, 0x30);
TS3_X86_OFFSET(RoadNetworkContext, range1, 0x54);
TS3_X86_OFFSET(RoadNetworkContext, list0, 0x78);
TS3_X86_OFFSET(RoadNetworkContext, list1, 0x9C);
TS3_X86_OFFSET(RoadNetworkContext, list2, 0xD0);
TS3_X86_OFFSET(RoadNetworkContext, tileBase, 0x114);

// TS3W.exe 0x00B73620: append the cubic Bezier (p0..p3) over [tBegin,tEnd] to the
// 64-byte record array as a flat polyline, subdividing while the curve deviates from
// its chord (stdcall, ret 0x34; ecx is saved but never read). EA quirk: the early
// "span below 1e-4" exit returns the caller's EAX residue, not a value of its own -
// this rewrite returns nullptr there (no caller reads it).
float* B73620(void** records, int32_t* count, const Vec4* p0, const Vec4* p1, const Vec4* p2, const Vec4* p3,
              float tBegin, float tEnd, void* a9, void* a10, const int32_t* id, uint32_t depth,
              float* totalLength);

// TS3W.exe 0x00B73960: seed a road segment's control points, nudge degenerate boxes,
// run the segment vtable slot 17 and refresh the tile flags (thiscall, ret 0x10). The
// road state comes from data 0x011ECBC4 +0x5C (x86 only). EA quirk: when `disabled` is
// nonzero it returns its caller's EAX residue; this rewrite returns 0.
int32_t B73960(RoadSegment* self, const Vec4* a2, const Vec4* a3, const Vec4* a4, const Vec4* a5);

// TS3W.exe 0x00B85C50: place road objects around tile `a4` for road id `a5` - the
// range walk from 0x00B79F90 plus three object lists, one 0x00B83DE0 or 0x00B81EE0
// pass each (thiscall, ret 0x10). Returns EAX's final dataflow value.
void* B85C50(RoadNetworkContext* self, void* a2, void* a3, int32_t a4, uint16_t a5);

// TS3W.exe 0x00B86150: place mode-2 road objects from list +0x78 for tile `a4`
// (thiscall, ret 0x10). Returns EAX's final dataflow value.
void* B86150(RoadNetworkContext* self, void* a2, void* a3, int32_t a4, uint16_t a5);

// TS3W.exe 0x00B86290: place mode-0 road objects from list +0x78 for tile `a4`
// (thiscall, ret 0x10). Returns EAX's final dataflow value.
void* B86290(RoadNetworkContext* self, void* a2, void* a3, int32_t a4, uint16_t a5);

// TS3W.exe 0x00B863F0: place mode-5 road objects from list +0x78 for tile `a4`
// (thiscall, ret 0x10). Returns EAX's final dataflow value.
void* B863F0(RoadNetworkContext* self, void* a2, void* a3, int32_t a4, uint16_t a5);

// TS3W.exe 0x00B86530: place mode-7 road objects from list +0xD0 whose vtable slot 22
// reports road id `a5` (thiscall, ret 0x10). Returns EAX's final dataflow value.
void* B86530(RoadNetworkContext* self, void* a2, void* a3, int32_t a4, uint16_t a5);

// Hook entry points registered as replace patches: the game originals are thiscall, so each
// symbol is a __fastcall function with the reserved second (edx) word and forwards to the
// free rewrite above. `a5` is taken as a full word and truncated, like the originals'
// the original instruction sequence.
inline int32_t __fastcall B73960Hook(RoadSegment* self, void*, const Vec4* a2, const Vec4* a3, const Vec4* a4,
                                     const Vec4* a5)
{
    return B73960(self, a2, a3, a4, a5);
}
inline void* __fastcall B85C50Hook(RoadNetworkContext* self, void*, void* a2, void* a3, int32_t a4, uint32_t a5)
{
    return B85C50(self, a2, a3, a4, static_cast<uint16_t>(a5));
}
inline void* __fastcall B86150Hook(RoadNetworkContext* self, void*, void* a2, void* a3, int32_t a4, uint32_t a5)
{
    return B86150(self, a2, a3, a4, static_cast<uint16_t>(a5));
}
inline void* __fastcall B86290Hook(RoadNetworkContext* self, void*, void* a2, void* a3, int32_t a4, uint32_t a5)
{
    return B86290(self, a2, a3, a4, static_cast<uint16_t>(a5));
}
inline void* __fastcall B863F0Hook(RoadNetworkContext* self, void*, void* a2, void* a3, int32_t a4, uint32_t a5)
{
    return B863F0(self, a2, a3, a4, static_cast<uint16_t>(a5));
}
inline void* __fastcall B86530Hook(RoadNetworkContext* self, void*, void* a2, void* a3, int32_t a4, uint32_t a5)
{
    return B86530(self, a2, a3, a4, static_cast<uint16_t>(a5));
}

}  // namespace ts3::roadnetwork