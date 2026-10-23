#pragma once
#include "ts3/misc/clu_6795d0_x.h"
namespace ts3::math::b16 {
using Point = misc::clu6795::Point;
// These short, named-allocator vectors have a 16-byte stride, not Points' 24.
struct Polygon { Point *begin, *end, *capacity; const char* name; };
struct Polygons { Polygon *begin, *end, *capacity; const char* name; };
TS3_X86_SIZE(Polygon,16);
TS3_X86_SIZE(Polygons,16);
// TS3W.exe 0x00676F70: Even/odd horizontal-ray polygon containment.
int __stdcall Contains(const Polygon*, const Point*);
// TS3W.exe 0x006772E0: Test a point against any polygon in a list.
uint8_t __stdcall ContainsAny(const Polygons*, const Point*);
// TS3W.exe 0x0067AE90: Containment including squared distance to the boundary.
uint8_t __stdcall ContainsRadius(const Polygon*, const Point*, float);
// TS3W.exe 0x0067AFB0: Classify consistent turns; small turns do not advance the saved edge.
uint8_t __stdcall ConsistentTurns(const Polygon*);
// TS3W.exe 0x0067B130: Test radius-expanded containment in any polygon.
uint8_t __stdcall ContainsRadiusAny(const Polygons*, const Point*, float);
// TS3W.exe 0x0067B190: Test a segment against a polygon list.
uint8_t __stdcall IntersectsSegment(const Polygons*, const Point*, const Point*);
// TS3W.exe 0x0067B390: Test a box strictly inside a polygon without boundary intersections.
uint8_t __fastcall ContainsBox(void*, void*, const Polygon*, const Point*, const Point*);
// TS3W.exe 0x0067B490: Test box containment against a list of polygons.
uint8_t __fastcall ContainsBoxAny(void*, void*, const Polygons*, const float*);
// TS3W.exe 0x0067B990: Find the closest polygon boundary, returning squared distance.
float __stdcall NearestAny(const Polygons*, const Point*, Point*);
// TS3W.exe 0x0067D920: Test a swept segment, rotating polygon vertices for nonzero radius.
uint8_t __fastcall IntersectsSwept(void*, void*, const Polygons*, const Point*, const Point*, float);
// TS3W.exe 0x007CD2E0: Triangle containment by three division-free ray crossings.
uint8_t __cdecl InTriangle(const Point*, const Point*, const Point*, const Point*);
// TS3W.exe 0x007CD430: Compute the oriented cross product using x87 intermediates.
double __cdecl Cross(const Point*, const Point*, const Point*);
// TS3W.exe 0x007CD460: Half-open point-in-rectangle test (upper boundaries excluded).
int __cdecl InRectangle(const Point*, const Point*, const Point*);
}
