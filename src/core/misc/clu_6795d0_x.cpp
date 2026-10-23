#include "ts3/misc/clu_6795d0_x.h"
#include <cfloat>
#include <cstring>

namespace ts3::misc::clu6795 {
namespace {
// FLD/FSTP quiet signaling NaNs in the original's copy paths, unlike MOVSS/MOVSD.
float X87Copy(float value)
{
    uint32_t bits;
    std::memcpy(&bits, &value, sizeof bits);
    if ((bits & 0x7f800000u) == 0x7f800000u && (bits & 0x7fffffu)) bits |= 0x400000u;
    std::memcpy(&value, &bits, sizeof bits);
    return value;
}
double X87Copy(double value)
{
    uint64_t bits;
    std::memcpy(&bits, &value, sizeof bits);
    if ((bits & 0x7ff0000000000000ull) == 0x7ff0000000000000ull && (bits & 0xfffffffffffffull)) bits |= 0x8000000000000ull;
    std::memcpy(&value, &bits, sizeof bits);
    return value;
}
}
// 0x0067B280, stdcall, ret 12; the x87 return is a float, not a double.
float __stdcall Nearest(const Point* point, const Points* polygon, Point* output)
{
    float best = FLT_MAX;
    if (polygon->begin == polygon->end) return best;
    const Point* previous = polygon->end - 1;
    for (const Point* current = polygon->begin; current != polygon->end; ++current) {
        float x = previous->x, y = previous->y;
        const float dx = current->x - x, dy = current->y - y;
        const float projection = (point->x - x) * dx + dy * (point->y - y);
        // COMISS zero,projection / JB also enters projection arithmetic for NaN.
        if (!(projection <= 0.0f)) {
            const float length = dx * dx + dy * dy;
            if (projection > length) { x = current->x; y = current->y; }
            else { const float t = projection / length; x += dx * t; y += t * dy; }
        }
        const float px = point->x - x, py = point->y - y;
        const float distance = px * px + py * py;
        if (best > distance) { output->x = x; output->y = y; best = distance; }
        previous = current;
    }
    return best;
}

// 0x0067BA60: unguarded EASTL partition; caller supplies scan sentinels.
Record* __cdecl Partition(Record* first, Record* last, const Record* pivot, Less less)
{
    const Record saved{pivot->x, pivot->y, X87Copy(pivot->payload)};
    for (;;) {
        while (less(first, &saved)) ++first;
        do { --last; } while (less(&saved, last));
        if (first >= last) return first;
        const Record temporary{first->x, first->y, X87Copy(first->payload)};
        *first++ = {X87Copy(last->x), X87Copy(last->y), X87Copy(last->payload)};
        *last = temporary;
    }
}

// 0x0067BF10, stdcall, ret 8. Preserve single-pass use of last accepted vertex.
bool __stdcall Simplify(const Points* input, Points* output)
{
    output->end = output->begin;
    const size_t count = input->begin == input->end ? 0 : input->end - input->begin;
    if (count < 3) return false;
    bool changed = false;
    size_t previous = count - 1;
    for (size_t i = 0; i < count; ++i) {
        const Point* p = input->begin + i;
        const Point& q = input->begin[previous];
        const Point& next = input->begin[i + 1 < count ? i + 1 : 0];
        const float dx = p->x - q.x, dy = p->y - q.y;
        const float nx = next.x - p->x, ny = next.y - p->y;
        const float length = dx * dx + dy * dy;
        bool discard = length < 0.000001f;
        if (!discard) {
            const float dot = nx * dx + ny * dy;
            if (dot < 0.0f) {
                const float nextLength = nx * nx + ny * ny;
                if (nextLength > 0.00001f && (dot * dot) / (nextLength * length) > 0.999f)
                    discard = true;
            }
        }
        if (discard) { changed = true; continue; }
        if (reinterpret_cast<uintptr_t>(output->end) >= reinterpret_cast<uintptr_t>(output->capacity))
            CallGameMethod<void, 0x00405D40>(output, output->end, p);
        else {
            Point* destination = output->end++;
            if (destination) *destination = {X87Copy(p->x), X87Copy(p->y)};
        }
        previous = i;
    }
    return changed;
}

// 0x00681F20, thiscall, ret 8: private chunk of 0x686B30.
Item* __fastcall SetFlag(Holder* self, void*, Item* item, uint8_t enabled)
{
    if (!item) item = self->value;
    if (enabled) item->flags |= 0x10000u;
    else item->flags &= ~0x10000u;
    return item;
}
// 0x00686B30: adjust this then tail-call the private chunk.
Item* __fastcall AdapterSetFlag(Adapter* self, void*, Item* item, uint8_t enabled)
{
    return SetFlag(self->holder, nullptr, item, enabled);
}
// 0x00681FF0: depth-first search; output remains untouched on failure.
bool __fastcall FindOwner(Tree* self, void*, Item* item, Tree** owner)
{
    const size_t count = self->begin == self->end ? 0 : self->end - self->begin;
    if (item->index < count && self->begin[item->index] == item) {
        *owner = self;
        return true;
    }
    void* link = self->prefix[5];
    void* sentinel = &self->prefix[5];
    while (link != sentinel) {
        Tree* child = reinterpret_cast<Tree*>(static_cast<uint8_t*>(link) - 2 * sizeof(void*));
        if (FindOwner(child, nullptr, item, owner)) return true;
        link = child->prefix[2];
    }
    return false;
}
}

namespace ts3::misc::clu6795 {
namespace {
struct Axis { float v[3]; };
float MaxSse(float a, float b) { return a > b ? a : b; }
float MinSse(float a, float b) { return a < b ? a : b; }
float AbsSse(float a) { return MaxSse(-1.0f * a, a); }
float Dot(const Axis& a, const Axis& b)
{
    return (a.v[2] * b.v[2] + a.v[1] * b.v[1]) + a.v[0] * b.v[0];
}
Axis AbsAxis(const Axis& a) { return {{AbsSse(a.v[0]), AbsSse(a.v[1]), AbsSse(a.v[2])}}; }
}
// 0x006795D0: AABB/OBB SAT, B's quaternion expressed in A's frame.
// Inputs: A half-extents, B half-extents, center delta, quaternion (x,y,z,w).
// Preserve EA's containment shortcuts, unnormalized quaternion handling, near-
// parallel axis omission and, especially, its z+y+x dot-product summation order.
bool __cdecl Overlap(const void* extentsA, const void* extentsB, const void* centerDelta, const void* quaternion)
{
    Axis a, b, delta;
    std::memcpy(&a, extentsA, sizeof a);
    std::memcpy(&b, extentsB, sizeof b);
    std::memcpy(&delta, centerDelta, sizeof delta);
    const Axis absoluteDelta = AbsAxis(delta);
    if (!(a.v[0] <= absoluteDelta.v[0]) && !(a.v[1] <= absoluteDelta.v[1]) && !(a.v[2] <= absoluteDelta.v[2])) return true;
    const float minimum = MinSse(MinSse(b.v[0], b.v[1]), b.v[2]);
    if (!(minimum <= absoluteDelta.v[0]) && !(minimum <= absoluteDelta.v[1]) && !(minimum <= absoluteDelta.v[2])) return true;
    float q[4];
    std::memcpy(q, quaternion, sizeof q);
    for (float& component : q) component *= 1.4142135381698608f; // 0xFEF9A0
    const float diagonal[3]{0.5f + (-q[0]) * q[0], 0.5f + (-q[1]) * q[1], 0.5f + (-q[2]) * q[2]};
    const float xy = q[1] * q[0], yz = q[2] * q[1], zx = q[0] * q[2];
    const float wz = q[3] * q[2], wx = q[3] * q[0], wy = q[3] * q[1];
    const Axis axes[3]{
        {{diagonal[1] + diagonal[2], wz + xy, zx - wy}},
        {{xy - wz, diagonal[2] + diagonal[0], wx + yz}},
        {{wy + zx, yz - wx, diagonal[0] + diagonal[1]}}
    };
    bool result = true;
    for (unsigned i = 0; i < 3; ++i) {
        const Axis row{{axes[0].v[i], axes[1].v[i], axes[2].v[i]}};
        const float radius = a.v[i] + Dot(AbsAxis(row), b);
        if (absoluteDelta.v[i] > radius) { result = false; goto finished; }
    }
    for (unsigned j = 0; j < 3; ++j) {
        const float radius = Dot(AbsAxis(axes[j]), a) + b.v[j];
        if (AbsSse(Dot(axes[j], delta)) > radius) { result = false; goto finished; }
    }
    for (unsigned i = 0; i < 3; ++i) {
        for (unsigned j = 0; j < 3; ++j) {
            if (!(0.9998999834060669f > AbsSse(axes[j].v[i]))) continue; // 0xFEF9B0
            Axis cross;
            if (i == 0) cross = {{0.0f, 0.0f - axes[j].v[2], axes[j].v[1]}};
            else if (i == 1) cross = {{axes[j].v[2], 0.0f, 0.0f - axes[j].v[0]}};
            else cross = {{0.0f - axes[j].v[1], axes[j].v[0], 0.0f}};
            float radius = Dot(AbsAxis(cross), a);
            // Original deliberately omits the projection on B's parallel axis.
            for (unsigned k = 0; k < 3; ++k)
                if (k != j) radius += AbsSse(Dot(cross, axes[k])) * b.v[k];
            if (AbsSse(Dot(delta, cross)) > radius) { result = false; goto finished; }
        }
    }
finished:
    return result;
}
}
