// Unit clu_6c8680_Debug (the project notes): the debug line-drawing cluster -
// a growable segment buffer plus the three drawers that append into it. All five
// TS3W.exe functions of the unit are entry points of the rewrite:
//
//   DebugLineBuffer  ::AddSegment     0x006C8680  thiscall, ret 0x10 (2 vec + 2 colors)
//   DebugLineBuffer  ::AddCross       0x006C87F0  thiscall, ret 0xC  (3-axis cross)
//   DebugLineDrawer  ::TryAddPair     0x00814C30  thiscall, ret 8    (gated + locked)
//   DebugShapeDrawer ::DrawShapes     0x008592A0  thiscall, no stack args
//   (free)           DrawQuadOutline 0x00860DF0  cdecl, 6 stack args
//
// Call edges inside the unit: AddCross -> AddSegment, TryAddPair -> AddSegment,
// DrawShapes -> AddSegment + DrawQuadOutline, DrawQuadOutline -> AddSegment.
// Outside the unit AddSegment grows through the game's 0x006D3810 (EA new[]/delete[]
// underneath, patched in the differential tests); DrawShapes reaches its container
// through the global at 0x11D9140 and three virtual calls (slots 9/7/2).
//
// The 0x814C30 and 0x8592A0 `this` types are shown as two structs here; whether the
// game has one class with both field sets is unverified (neither function touches
// the other's fields).
#pragma once

#include <cstdint>

#include "ts3/layout.h"

namespace ts3::debug {

// A raw float4 as the drawers pass them (points, directions, offsets). Lane order
// is x,y,z,w in memory; every SSE op in the originals loads/stores whole xmmwords.
struct Vec4 {
    float x, y, z, w;
};
TS3_X86_SIZE(Vec4, 0x10);

// One buffer element (16 bytes): the point xyz plus a packed ABGR color dword.
struct DebugLineVertex {
    float x, y, z;
    uint32_t color;
};
TS3_X86_SIZE(DebugLineVertex, 0x10);

// The container of 0x006C8680/0x006C87F0 (and of both higher drawers). Three
// pointers into one EA new[] block plus a slot-count limit; +0x0C is read only by
// the grow path 0x006D3810 (heap hint passed to EA's new[] - keep 0 unless the
// owning code has a real value for it).
struct DebugLineBuffer {
    // 0x006C8680: appends the two elements {a.xyz,colorA} and {b.xyz,colorB}.
    // Returns the position of element B (or the grow helper's eax when B grew).
    DebugLineVertex* AddSegment(const Vec4* a, const Vec4* b, uint32_t colorA, uint32_t colorB);
    // 0x006C87F0: three axis-aligned segments of length 2*half through `center`.
    DebugLineVertex* AddCross(const Vec4* center, float half, uint32_t color);

    DebugLineVertex* begin;   // 0x00
    DebugLineVertex* end;     // 0x04
    DebugLineVertex* cap;     // 0x08 (one past the allocated end)
    uint32_t heapHint;        // 0x0C (0x006D3810's new[] name/hint argument)
    uint32_t field10;         // 0x10 (not read by this unit)
    uint32_t limit;           // 0x14 max element count the owner allows
};
TS3_X86_OFFSET(DebugLineBuffer, end, 0x04);
TS3_X86_OFFSET(DebugLineBuffer, cap, 0x08);
TS3_X86_OFFSET(DebugLineBuffer, heapHint, 0x0C);
TS3_X86_OFFSET(DebugLineBuffer, limit, 0x14);
TS3_X86_SIZE(DebugLineBuffer, 0x18);

// 0x00814C30's `this`: the line drawer the game gates on the byte at 0x11D90B0
// and locks with the CRITICAL_SECTION at +0x48 (x86 size 0x18; the x64 layout is
// larger, but this struct only ever backs 32-bit game memory).
struct DebugLineDrawer {
    // 0x00814C30: if every gate is open and TryEnterCriticalSection succeeds,
    // appends one segment colored 0xFFFFFF00 and leaves the lock.
    void TryAddPair(const Vec4* a, const Vec4* b);

    uint8_t head[0x40];        // 0x00
    DebugLineBuffer* buffer;   // 0x40
    uint8_t gap44[4];          // 0x44
    uint8_t lock[0x18];        // 0x48 CRITICAL_SECTION (x86)
    uint8_t gap60[0xE3];       // 0x60..0x143 (fields of the same object, unused here)
    uint8_t enabledA;          // 0x143
    uint8_t enabledB;          // 0x144
};
TS3_X86_OFFSET(DebugLineDrawer, buffer, 0x40);
TS3_X86_OFFSET(DebugLineDrawer, lock, 0x48);
TS3_X86_OFFSET(DebugLineDrawer, enabledA, 0x143);
TS3_X86_OFFSET(DebugLineDrawer, enabledB, 0x144);

// The inline member object at DebugShapeDrawer +0x18 (vtable slot 2 writes a
// float4 out-parameter); its dir[] is what DrawShapes hands DrawQuadOutline.
struct DebugShapeMember {
    void* vptr;         // 0x00
    uint8_t mid[0x14];  // 0x04..0x18
    float dir[4];       // 0x18 (== drawer +0x30)
};
TS3_X86_OFFSET(DebugShapeMember, dir, 0x18);
TS3_X86_SIZE(DebugShapeMember, 0x28);

// 0x008592A0's `this`: reads the container from the global at 0x11D9140 (not from
// itself), draws an oriented box (mode 2) or a cross (any other mode), and draws
// quad outlines through DrawQuadOutline.
struct DebugShapeDrawer {
    // 0x008592A0: all gates/early-outs return the 0x11D9140 pointer as-is; the
    // drawing paths return the last DrawQuadOutline result.
    DebugLineVertex* DrawShapes();

    uint8_t head[0x14];      // 0x00
    void* transformProxy;    // 0x14 (vtable slots 9 and 7 write float4s)
    DebugShapeMember shape;  // 0x18 (vtable slot 2)
    uint8_t gap40[0x80];     // 0x40..0xC0 (includes the +0x40 buffer slot 814C30 uses)
    float scale;             // 0xC0
    uint32_t mode;           // 0xC4 (2 = box, else cross)
};
TS3_X86_OFFSET(DebugShapeDrawer, transformProxy, 0x14);
TS3_X86_OFFSET(DebugShapeDrawer, shape, 0x18);
TS3_X86_OFFSET(DebugShapeDrawer, scale, 0xC0);
TS3_X86_OFFSET(DebugShapeDrawer, mode, 0xC4);

// 0x00860DF0: builds the two axes of `dir` and appends the outline of the quad
// centered on `point` (lenScale along the dir-derived axis, widScale across it).
// Free cdecl, 6 stack args; the original returns its last AddSegment result (and
// returns stale eax when buffer == nullptr - callers never pass null).
DebugLineVertex* DrawQuadOutline(DebugLineBuffer* buffer, const Vec4* point, const Vec4* dir,
                                 uint32_t color, float lenScale, float widScale);

}  // namespace ts3::debug
