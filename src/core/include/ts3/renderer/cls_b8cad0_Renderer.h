// TS3W.exe renderer class unit cls_b8cad0_Renderer (0x00B8CAD0/0x00B8D210).
#pragma once

#include <cstdint>

namespace ts3::renderer {

// TS3W.exe 0x00B8CAD0: __thiscall(this=renderer, lot, draw), ret 8. The lot-type dispatch's
// type-9 handler: returns without emitting unless [lot+0x2c] == 9, then draws an octagonal rim -
// a 16-entry corner table built from [lot+0x84] and [lot+0x88]*0.5f, one normalised side normal
// per corner (the original's _mm_rsqrt_ps plus two Newton-Raphson refinements, lane for lane),
// 8 (position, uv) pairs at height [0x58](lot), a centre pair at y and y-0.25, a (0,-1,0) normal,
// 8 interpolated edge vertices, two index quads per side through 0x006D4B20/0x006D4BC0/0x006D4A50
// and eight 0x00B894B0 strips. EAX is not a contract (the early-out leaves the caller's EAX, the
// full path the final 0x00B894B0 result); the sole dispatch caller at 0x00B95AFF
// ignores EAX (0x00B95B04 reloads ECX), so this returns void.
// EA quirks kept: the index lists get (uint16) of the 0x006D0AD0/0x006D0B50/0x006D0B10 counts
// while 0x00B894B0 gets them untruncated; the corner wrap is ((i+1) as a byte) & 7; the uv
// transform re-reads the renderer's origin/scale/offset fields on every emit; -0.0f-x keeps the
// sign of zero; the dead x87 modf of top*5.0f (00B8CB50..00B8CB7C) is retained
// for its exception flags and caller-controlled rounding; the two centre 0x006D5D60 calls take identical arguments.
void RenderLotType9(void* self, void* lot, void* draw);

// TS3W.exe 0x00B8D210: __thiscall(this=renderer, lot, draw), ret 8. The dispatch's type-13
// handler: an octahedral cap of 6 vertices (the lot's own [0x7c..0x98] corner box at one shared
// height) with 4 normals, emitted as 6 positions, 6 uvs, 8 normals (each mirrored through -0.0f)
// and 4 index quads plus four 0x00B894B0 strips; returns the last 0x00B894B0 result in EAX.
// [lot+0x38] picks the variant: it chooses which of [lot+0xe0]/[lot+0xdc] is the height, and
// swaps the two top vertices, the normals and every index. EA quirks kept: a negative height
// gets +4294967296.0 (so it wraps into range); the four quad triangle ids and both index lists
// take (uint16) of the 0x006D0AD0/0x006D0B50/0x006D0B10 counts while the 0x00B894B0 strips get
// them untruncated; integer height/2 is exact in double before rounding to float; the base
// height 0x00C03F50([lot+0x3c]) + [0x58](lot) - [lot+0x44] stays live in x87 ST0.
int32_t RenderLotType13(void* self, void* lot, void* draw);

}  // namespace ts3::renderer
