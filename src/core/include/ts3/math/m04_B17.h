#pragma once
#include "ts3/misc/clu_6795d0_x.h"
#include <xmmintrin.h>
namespace ts3::math::b17 {
using Points=ts3::misc::clu6795::Points;
// TS3W.exe 0x007CD4A0: Absolute triangle area with binary32 intermediate arithmetic.
double Area(const float*,const float*,const float*);
// TS3W.exe 0x007CD750: Point-to-infinite-line distance with x87 double accumulation.
double LineDistance(const float*,const float*,const float*);
// TS3W.exe 0x007CD7E0: Even/odd polygon containment with inclusive Y comparisons.
bool Contains(const float*,const float*,int);
// TS3W.exe 0x007CD970: Segment intersection using the fixed 0.01 determinant threshold.
char Intersect(const float*,const float*,const float*,const float*,float*);
// TS3W.exe 0x007CDAA0: Segment intersection using a caller-provided determinant threshold.
char IntersectEpsilon(const float*,const float*,const float*,const float*,float*,float);
// TS3W.exe 0x007CDFC0: Point-array bounds; ties and unordered comparisons retain the old value.
char Bounds(const Points*,float*);
// TS3W.exe 0x007CE220: Update the nearest segment hit; a farther hit still returns success.
char NearestHit(const float*,const float*,const float*,const float*,char,float*,float*);
// TS3W.exe 0x007CE2B0: Find the nearest rectangle-boundary hit in EA edge order.
bool RectangleHit(const float*,const float*,const float*,const float*,float*);
// TS3W.exe 0x007CE4A0: Project onto an infinite line; near-zero axes leave output unchanged.
char Project(float,float,float,float,float,float,float*);
// TS3W.exe 0x007CE550: Test segment/polygon overlap including endpoint containment.
char SegmentPolygon(const float*,const float*,const float*,unsigned);
// TS3W.exe 0x007CE7A0: Compare cyclic point order; reads EA's mutable lazy squared tolerance.
char CyclicEqual(const Points*,const Points*);
// TS3W.exe 0x007CE8A0: Test polygon overlap after strict bounding-box separation.
bool PolygonsOverlap(const Points*,const float*,const Points*,const float*);
// TS3W.exe 0x009E5560: Oriented SIMD XY triangle expression with explicit runtime scale (011816E0).
float OrientedArea(const __m128*,const __m128*,const __m128*,float);
}
