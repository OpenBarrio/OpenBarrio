#include "ts3/roadnetwork/clu_b73620_RoadNetwork.h"

#include <cmath>
#include <cstddef>
#include <cstdint>
#include <cstring>

#include <xmmintrin.h>

#include "ts3/layout.h"

namespace ts3::roadnetwork {
namespace {
template <class T>
T& At(void* p, size_t offset) { return *reinterpret_cast<T*>(static_cast<uint8_t*>(p) + offset); }
template <class T>
const T& At(const void* p, size_t offset) { return *reinterpret_cast<const T*>(static_cast<const uint8_t*>(p) + offset); }

Vec4 operator*(const Vec4& v, float s) { return Vec4{v.x * s, v.y * s, v.z * s, v.w * s}; }
Vec4 operator+(const Vec4& a, const Vec4& b) { return Vec4{a.x + b.x, a.y + b.y, a.z + b.z, a.w + b.w}; }
Vec4 operator-(const Vec4& a, const Vec4& b) { return Vec4{a.x - b.x, a.y - b.y, a.z - b.z, a.w - b.w}; }

Vec4 LoadVec4(const void* p, size_t offset)
{
    Vec4 v;
    std::memcpy(&v, static_cast<const uint8_t*>(p) + offset, sizeof v);
    return v;
}
void StoreVec4(void* p, size_t offset, const Vec4& v)
{
    std::memcpy(static_cast<uint8_t*>(p) + offset, &v, sizeof v);
}

// 0x00B73960 reads the road state out of the game's world-state global. There is no game
// data on the x64 build (which never runs the harness), so abort there.
void* RoadStateFromGameGlobal()
{
#if defined(_M_IX86)
    constexpr uintptr_t kWorldState = 0x011ECBC4;
    void* manager = *reinterpret_cast<void* const*>(kWorldState);
    return At<void*>(manager, 0x5C);
#else
    AbortNotRewritten("the RoadNetwork world state global 0x011ECBC4");
    return nullptr;
#endif
}

// The original converts the tile's x/y with cvtsi2ss (signed) and the width/height with
// fild plus a conditional +2^32 fixup (i.e. read as unsigned when negative); the sum is
// computed in x87 extended precision and rounded once to float, which is modelled with a
// double intermediate (the double-rounding corner cases would need >2^32 magnitudes).
float TileFieldToFloat(uint32_t value)
{
    return static_cast<float>(static_cast<int32_t>(value));
}
float AddTileFields(uint32_t value, float other)
{
    return static_cast<float>(static_cast<double>(value) + static_cast<double>(other));
}

// 0x00B73620's |x| = max(-x, x) (maxps); the sign of a zero result does not survive.
float AbsMaxNeg(float v) { return std::fmax(-v, v); }

// The tangent normalization of 0x00B73620: rsqrtps plus two Newton steps, all four
// lanes identical in the original (x and y are the only ones contributing to len2).
Vec4 NormalizeTangent(const Vec4& d)
{
    const float len2 = d.x * d.x + d.y * d.y;
    const __m128 l2 = _mm_set1_ps(len2);
    const __m128 one = _mm_set1_ps(1.0f);
    const __m128 half = _mm_set1_ps(0.5f);
    __m128 r = _mm_rsqrt_ps(l2);
    r = _mm_add_ps(_mm_mul_ps(_mm_sub_ps(one, _mm_mul_ps(_mm_mul_ps(r, r), l2)), _mm_mul_ps(r, half)), r);
    r = _mm_add_ps(_mm_mul_ps(_mm_sub_ps(one, _mm_mul_ps(_mm_mul_ps(r, r), l2)), _mm_mul_ps(r, half)), r);
    const float s = _mm_cvtss_f32(r);
    return Vec4{d.x * s, d.y * s, d.z * s, d.w * s};
}
}  // namespace

// 0x00B79F90: preserve the original sentinel/range walk and output ordering.
uint32_t* B79F90(const uint32_t* ranges, uint32_t* out)
{
    uint32_t index = 0;
    if (*reinterpret_cast<const uint32_t*>(uintptr_t(ranges[0])) ==
        *reinterpret_cast<const uint32_t*>(uintptr_t(ranges[0]) + 4)) {
        const uint32_t count = ranges[8];
        while (++index != count) {
            const uintptr_t range = uintptr_t(ranges[index]);
            if (*reinterpret_cast<const uint32_t*>(range) != *reinterpret_cast<const uint32_t*>(range + 4))
                break;
        }
        if (index == count) {
            out[0] = static_cast<uint32_t>(reinterpret_cast<uintptr_t>(ranges));
            out[1] = count;
            const uintptr_t last = uintptr_t(ranges[count - 1]);
            out[2] = *reinterpret_cast<const uint32_t*>(last + 4);
            return out;
        }
    }
    out[0] = static_cast<uint32_t>(reinterpret_cast<uintptr_t>(ranges));
    out[1] = index;
    out[2] = *reinterpret_cast<const uint32_t*>(uintptr_t(ranges[index]));
    return out;
}

// 0x00C29EB0: the table base is stored at +0xAC and records are 0x70 bytes.
void* C29EB0(const void* tileTable, int32_t index)
{
    const auto base = At<uintptr_t>(tileTable, 0xAC);
    return reinterpret_cast<void*>(base + uintptr_t(uint32_t(index) * 0x70u));
}

// 0x00B77C50: mark each 16-byte tile record whose integer bounds overlap bounds.
void B77C50(void* state, const float* bounds)
{
    if (!(bounds[0] != bounds[2] && bounds[1] != bounds[3])) return;
    auto* begin = At<uint8_t*>(state, 0x1C);
    auto* end = At<uint8_t*>(state, 0x20);
    const uint32_t count = static_cast<uint32_t>((end - begin) >> 4);
    auto* table = At<void*>(state, 0x0C);
    for (uint32_t i = 0; i < count; ++i) {
        const auto* tile = static_cast<const int32_t*>(C29EB0(table, static_cast<int32_t>(i)));
        const int32_t x = tile[0], y = tile[1], width = tile[2], height = tile[3];
        const float right = static_cast<float>(uint32_t(x) + uint32_t(width));
        const float bottom = static_cast<float>(uint32_t(y) + uint32_t(height));
        if (static_cast<float>(x) <= bounds[2] && bounds[0] <= right &&
            static_cast<float>(y) <= bounds[3] && bounds[1] <= bottom)
            begin[i * 16 + 12] = 1;
    }
}

// 0x00B73620: recursive Bezier subdivision. Records are 64 bytes:
// +0x00 t, +0x10 point, +0x20 normalized tangent, +0x30 running length,
// +0x34 length delta to the previous record, +0x38 the caller's id.
float* B73620(void** records, int32_t* count, const Vec4* p0, const Vec4* p1, const Vec4* p2, const Vec4* p3,
              float tBegin, float tEnd, void* a9, void* a10, const int32_t* id, uint32_t depth, float* totalLength)
{
    constexpr float kMinSpan = 1.0e-4f;
    if (kMinSpan > tEnd - tBegin) return nullptr;  // the original returns the caller's EAX residue
    if (depth >= 0x14u) return reinterpret_cast<float*>(static_cast<uintptr_t>(depth));
    if (static_cast<uint32_t>(*count) > 0x63u) return reinterpret_cast<float*>(static_cast<uintptr_t>(depth));

    const Vec4 q0 = *p0, q1 = *p1, q2 = *p2, q3 = *p3;
    const float mid = (tBegin + tEnd) * 0.5f;
    const float span = tEnd - tBegin;
    const float t = (mid - tBegin) / span;
    const float s = 1.0f - t;

    const Vec4 a = q0 * s + q1 * t;
    const Vec4 b = q1 * s + q2 * t;
    const Vec4 c = q2 * s + q3 * t;
    const Vec4 d = a * s + b * t;
    const Vec4 e = b * s + c * t;
    const Vec4 q = d * s + e * t;

    const Vec4 chord = q3 - q0;
    const float cross1 = (q1.x - q3.x) * chord.y - (q1.y - q3.y) * chord.x;
    const float cross2 = (q2.x - q3.x) * chord.y - (q2.y - q3.y) * chord.x;
    const float err = AbsMaxNeg(cross1) + AbsMaxNeg(cross2);
    const float chordLen2 = chord.y * chord.y + chord.x * chord.x;

    if (!(0.1f * chordLen2 > err * err)) {
        const uint32_t next = depth + 1;
        B73620(records, count, p0, &a, &d, &q, tBegin, mid, a9, a9, id, next, totalLength);
        uint8_t* rec = static_cast<uint8_t*>(*records) + (static_cast<size_t>(static_cast<uint32_t>(*count)) << 6);
        StoreVec4(rec, 0x10, q);
        StoreVec4(rec, 0x20, NormalizeTangent(e - q));
        At<int32_t>(rec, 0x38) = *id;
        At<float>(rec, 0x30) = *totalLength;
        At<float>(rec, 0x00) = mid;
        if (*count != 0) {
            // The original writes the *previous* record's delta here, so the last record of
            // a run keeps whatever its +0x34 held (an EA quirk both sides reproduce).
            const float previous = At<float>(static_cast<const uint8_t*>(*records) +
                                                 (static_cast<size_t>(static_cast<uint32_t>(*count)) << 6) - 0x10,
                                             0);
            At<float>(static_cast<uint8_t*>(*records) +
                          (static_cast<size_t>(static_cast<uint32_t>(*count)) << 6) - 0xC,
                      0) = *totalLength - previous;
        }
        ++*count;
        return B73620(records, count, &q, &e, &c, p3, mid, tEnd, a9, a10, id, next, totalLength);
    }
    *totalLength = static_cast<float>(static_cast<double>(std::sqrt(static_cast<double>(chordLen2))) +
                                      static_cast<double>(*totalLength));
    return totalLength;
}

// 0x00B73960: refresh tile flags, seed the control points and run the segment callback.
int32_t B73960(RoadSegment* self, const Vec4* a2, const Vec4* a3, const Vec4* a4, const Vec4* a5)
{
    if (self->disabled != 0) return 0;  // the original returns the caller's EAX residue
    void* roadState = RoadStateFromGameGlobal();
    B77C50(roadState, self->bounds);

    Vec4 p0 = *a2;
    Vec4 p1 = *a3;
    Vec4 p2 = *a4;
    Vec4 p3 = *a5;
    StoreVec4(self, 0x30, p0);
    StoreVec4(self, 0x40, p1);
    StoreVec4(self, 0x50, p2);
    StoreVec4(self, 0x60, p3);

    constexpr float kNudge = 0.01f;
    if (p0.x == p3.x && p0.y == p3.y) p3.x += kNudge;
    if (p0.x == p1.x && p0.y == p1.y) {
        p1.x += (p3.x - p0.x) * kNudge;
        p1.y += (p3.y - p0.y) * kNudge;
        p1.z += (p3.z - p0.z) * kNudge;
        p1.w += (p3.w - p0.w) * kNudge;
        StoreVec4(self, 0x40, p1);
    }
    if (p2.x == p3.x && p2.y == p3.y) {
        p2.x += (p0.x - p3.x) * kNudge;
        p2.y += (p0.y - p3.y) * kNudge;
        p2.z += (p0.z - p3.z) * kNudge;
        p2.w += (p0.w - p3.w) * kNudge;
        StoreVec4(self, 0x50, p2);
    }
    // The original stores the nudged p3 before the second and third comparisons and the
    // second store can be skipped, so write p3 back here (the first condition's result
    // is the value the later comparisons saw).
    StoreVec4(self, 0x60, p3);

    const int32_t result = CallGameVirtual<int32_t, 17>(self);
    B77C50(roadState, self->bounds);
    return result;
}

// The tile bounds shared by the five 0x00B85C50-family entries: (float)x, (float)y and
// (unsigned)width/height + that float (read back as the box maximums).
struct TileBounds {
    float x, y, maxX, maxY;
};
TileBounds TileBoundsOf(const RoadNetworkContext* self, int32_t tileIndex)
{
    const uint8_t* tile = static_cast<const uint8_t*>(C29EB0(self->tileTable, tileIndex));
    TileBounds b;
    b.x = TileFieldToFloat(At<uint32_t>(tile, 0));
    b.y = TileFieldToFloat(At<uint32_t>(tile, 4));
    b.maxX = AddTileFields(At<uint32_t>(tile, 8), b.x);
    b.maxY = AddTileFields(At<uint32_t>(tile, 12), b.y);
    return b;
}

// The scale/box overlap test of every object loop (movaps compares on the box corners).
bool BoxOverlaps(const RoadNetworkContext* self, const RoadObject* object, const TileBounds& b)
{
    const float scale = self->scale;
    const float box2 = scale * object->box[2];
    const float box0 = scale * object->box[0];
    const float box3 = scale * object->box[3];
    const float box1 = scale * object->box[1];
    return box2 > b.x && b.maxX > box0 && box3 > b.y && b.maxY > box1;
}

// 0x00B85C50
void* B85C50(RoadNetworkContext* self, void* a2, void* a3, int32_t a4, uint16_t a5)
{
    const TileBounds bounds = TileBoundsOf(self, a4);
    const Vec4 v36 = LoadVec4(self->tileBase + static_cast<size_t>(0x250u * a5) + 0x70, 0);

    void* ranges[9];
    ranges[0] = reinterpret_cast<uint8_t*>(self) + 0x54;
    ranges[1] = reinterpret_cast<uint8_t*>(self) + 0x30;
    ranges[8] = reinterpret_cast<void*>(static_cast<uintptr_t>(2));
    uint32_t out[3];
    B79F90(reinterpret_cast<const uint32_t*>(ranges), out);

    uint8_t* current = reinterpret_cast<uint8_t*>(static_cast<uintptr_t>(out[2]));
    uint32_t index = out[1];
    const uint32_t rangeCount = 2;
    // EAX's final dataflow is only observed from the list loops below; the range walk's
    // own values are overwritten by the first list load.
    if (current != At<uint8_t*>(ranges[1], 4)) {
        for (;;) {
            RoadObject* object = *reinterpret_cast<RoadObject**>(current);
            if (!CallGameVirtual<bool, 21>(object) && object->id1A4 == a5 &&
                BoxOverlaps(self, object, bounds)) {
                if (CallGameVirtual<bool, 18>(object)) {
                    Vec4 white{1.0f, 1.0f, 1.0f, 1.0f};
                    CallGameMethod<uint32_t, 0xB83DE0u>(self, a2, a3, a4, object, -1.0f, 1.0f, &v36, &white, 0u,
                                                        0u);
                } else {
                    Vec4 white1{1.0f, 1.0f, 1.0f, 1.0f};
                    Vec4 white2{1.0f, 1.0f, 1.0f, 1.0f};
                    const float thickness = 6.0f;
                    CallGameMethod<uint32_t, 0xB83DE0u>(self, a2, a3, a4, object, -thickness, -(thickness - 2.0f),
                                                        &v36, &white1, 1u, 1u);
                    CallGameMethod<uint32_t, 0xB83DE0u>(self, a2, a3, a4, object, thickness - 2.0f, thickness,
                                                        &v36, &white2, 1u, 0u);
                }
            }
            current = current + 4;
            if (current == At<uint8_t*>(ranges[index], 4)) {
                uint32_t next = index + 1;
                while (next != rangeCount) {
                    uint8_t* range = static_cast<uint8_t*>(ranges[next]);
                    uint8_t* begin = *reinterpret_cast<uint8_t**>(range);
                    current = begin;
                    if (begin != At<uint8_t*>(range, 4)) break;
                    ++next;
                }
                index = next;
            }
            if (current != At<uint8_t*>(ranges[rangeCount - 1], 4)) continue;
            break;
        }
    }

    // 0x00B85C50's visible return is EAX's final dataflow: the last list value it touched
    // (an object, a 0x00B81EE0 result) or, when the lists are empty, the list-0 begin at
    // 0xB85F51 (or its end after a non-empty walk) - so the early path mirrors that load.
    uintptr_t result = reinterpret_cast<uintptr_t>(self->list0);
    uint8_t* objectList = reinterpret_cast<uint8_t*>(self->list0);
    const uint8_t* objectListEnd = reinterpret_cast<const uint8_t*>(self->list0End);
    for (; objectList != objectListEnd; objectList += 4) {
        RoadObject* object = *reinterpret_cast<RoadObject**>(objectList);
        result = reinterpret_cast<uintptr_t>(objectList);
        if (object->idB2 == a5 && BoxOverlaps(self, object, bounds)) {
            Vec4 white{1.0f, 1.0f, 1.0f, 1.0f};
            result = CallGameMethod<uint32_t, 0xB81EE0u>(self, a2, a3, a4, object, 1u, &v36, &white);
            result = CallGameMethod<uint32_t, 0xB81EE0u>(self, a2, a3, a4, object, 4u, &v36, &white);
        }
        result = reinterpret_cast<uintptr_t>(objectList + 4);
    }

    uint8_t* otherList = reinterpret_cast<uint8_t*>(self->list1);
    const uint8_t* otherListEnd = reinterpret_cast<const uint8_t*>(self->list1End);
    for (; otherList != otherListEnd; otherList += 4) {
        RoadObject* object = *reinterpret_cast<RoadObject**>(otherList);
        result = reinterpret_cast<uintptr_t>(object);
        if (object->idB2 == a5 && BoxOverlaps(self, object, bounds)) {
            Vec4 white{1.0f, 1.0f, 1.0f, 1.0f};
            result = CallGameMethod<uint32_t, 0xB81EE0u>(self, a2, a3, a4, object, 7u, &v36, &white);
        }
    }
    return reinterpret_cast<void*>(result);
}

// The shared body of 0x00B86150/0x00B86290/0x00B863F0: walk list +0x78 and hand every
// matching object to 0x00B81EE0 with one constant + one per-tile vector.
void* PlaceListObjects(RoadNetworkContext* self, void* a2, void* a3, int32_t a4, uint16_t a5, uint32_t mode,
                       uintptr_t initialResult, const Vec4& primary, const Vec4& secondary)
{
    const TileBounds bounds = TileBoundsOf(self, a4);
    uintptr_t result = initialResult;
    for (uint8_t* it = reinterpret_cast<uint8_t*>(self->list0); it != reinterpret_cast<const uint8_t*>(self->list0End);
         it += 4) {
        RoadObject* object = *reinterpret_cast<RoadObject**>(it);
        result = reinterpret_cast<uintptr_t>(object);
        if (object->idB2 == a5 && BoxOverlaps(self, object, bounds)) {
            Vec4 second = secondary;
            result = CallGameMethod<uint32_t, 0xB81EE0u>(self, a2, a3, a4, object, mode, &primary, &second);
        }
    }
    return reinterpret_cast<void*>(result);
}

// 0x00B86150
void* B86150(RoadNetworkContext* self, void* a2, void* a3, int32_t a4, uint16_t a5)
{
    const Vec4 v23 = LoadVec4(self->tileBase + static_cast<size_t>(0x250u * a5) + 0x20, 0);
    const Vec4 v22 = LoadVec4(self->tileBase + static_cast<size_t>(0x250u * a5) + 0x70, 0);
    return PlaceListObjects(self, a2, a3, a4, a5, 2u, reinterpret_cast<uintptr_t>(self->tileBase) + 0x250u * a5,
                            v23, v22);
}

// 0x00B86290
void* B86290(RoadNetworkContext* self, void* a2, void* a3, int32_t a4, uint16_t a5)
{
    const Vec4 v21 = LoadVec4(self->tileBase + static_cast<size_t>(0x250u * a5) + 0x20, 0);
    const Vec4 white{1.0f, 1.0f, 1.0f, 1.0f};
    return PlaceListObjects(self, a2, a3, a4, a5, 0u, static_cast<uintptr_t>(0x250u * a5), v21, white);
}

// 0x00B863F0
void* B863F0(RoadNetworkContext* self, void* a2, void* a3, int32_t a4, uint16_t a5)
{
    const Vec4 v23 = LoadVec4(self->tileBase + static_cast<size_t>(0x250u * a5) + 0x20, 0);
    const Vec4 v22 = LoadVec4(self->tileBase + static_cast<size_t>(0x250u * a5) + 0x110, 0);
    return PlaceListObjects(self, a2, a3, a4, a5, 5u, reinterpret_cast<uintptr_t>(self->tileBase) + 0x250u * a5,
                            v23, v22);
}

// 0x00B86530
void* B86530(RoadNetworkContext* self, void* a2, void* a3, int32_t a4, uint16_t a5)
{
    const TileBounds bounds = TileBoundsOf(self, a4);
    const Vec4 v22 = LoadVec4(self->tileBase + static_cast<size_t>(0x250u * a5) + 0x1B0, 0);
    uintptr_t result = static_cast<uintptr_t>(0x250u * a5);
    for (uint8_t* it = reinterpret_cast<uint8_t*>(self->list2); it != reinterpret_cast<const uint8_t*>(self->list2End);
         it += 4) {
        RoadObject* object = *reinterpret_cast<RoadObject**>(it);
        const uint32_t roadId = CallGameVirtual<uint32_t, 22>(object);
        result = roadId;
        if (static_cast<uint16_t>(roadId) == a5 && BoxOverlaps(self, object, bounds)) {
            Vec4 white{1.0f, 1.0f, 1.0f, 1.0f};
            result = CallGameMethod<uint32_t, 0xB81EE0u>(self, a2, a3, a4, object, 7u, &v22, &white);
        }
    }
    return reinterpret_cast<void*>(result);
}

}  // namespace ts3::roadnetwork