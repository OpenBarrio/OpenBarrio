#pragma once
#include "ts3/debug/clu_6c8680_Debug.h"

namespace ts3::routing {
using debug::Vec4;
using debug::DebugLineBuffer;
using debug::DebugLineVertex;
// ECX is DebugLineBuffer* for 6C8900/6C9270 (ret 16). Its checked
// layout is defined in clu_6c8680_Debug.h. Return EAX is scratch-stack
// storage for circles without diameters; the portable API returns nullptr for
// that unstable scratch address. With diameters it returns the final segment.
struct Matrix { Vec4 row[4]; };
TS3_X86_SIZE(Matrix, 64);
struct Sample { Vec4 point, direction, value, unused; };
TS3_X86_OFFSET(Sample, direction, 16);
TS3_X86_OFFSET(Sample, value, 32);
TS3_X86_SIZE(Sample, 64);
DebugLineVertex* DrawCircle(DebugLineBuffer* self, const Matrix& matrix, float radius, uint32_t color, bool diameters);
DebugLineVertex* DrawCircleAt(DebugLineBuffer* self, const Vec4* point, float radius, uint32_t color, bool diameters);
DebugLineVertex* DrawArrow(DebugLineBuffer* self, const Vec4* point, const Vec4* direction, uint32_t color, float length, float width);
// 74C030/74C650: custom LTCG cdecl, vector in XMM4, other args on
// stack; portable API uses a reference. XMM4 is overwritten before use.
// No harness hook for this ABI: hookgen cannot emit an XMM4 entry thunk.
DebugLineVertex* DrawSamples(const Vec4& vector, DebugLineBuffer* self, const Sample* samples, uint32_t count, const Vec4& high, const Vec4& middle, const Vec4& low);
DebugLineVertex* DrawRouting(const Vec4& vector, DebugLineBuffer* self, const Sample* samples, uint32_t count);
}
