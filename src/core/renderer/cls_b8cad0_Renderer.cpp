// TS3W.exe cls_b8cad0_Renderer: the lot-type dispatch's type-9 (0x00B8CAD0) and type-13
// (0x00B8D210) geometry handlers. See the header for the behaviour map and the kept quirks.
#include "ts3/renderer/cls_b8cad0_Renderer.h"

#include "ts3/layout.h"

#include <intrin.h>

#include <cstdint>
#include <cstring>

namespace ts3::renderer {
namespace {

#if defined(_M_IX86)

int32_t LoadI32(const void* object, int offset)
{
    int32_t value;
    std::memcpy(&value, static_cast<const uint8_t*>(object) + offset, sizeof value);
    return value;
}

float LoadF32(const void* object, int offset)
{
    float value;
    std::memcpy(&value, static_cast<const uint8_t*>(object) + offset, sizeof value);
    return value;
}

uint8_t LoadU8(const void* object, int offset)
{
    return static_cast<const uint8_t*>(object)[offset];
}

float F32(uint32_t bits)
{
    float value;
    std::memcpy(&value, &bits, sizeof value);
    return value;
}

// Both handlers take their float constants out of the mapped image; reading them from the same
// addresses the originals use keeps every bit (including -0.0f) where the EA build put it.
float ConstF(uint32_t address)
{
    return *reinterpret_cast<const float*>(static_cast<uintptr_t>(address));
}

constexpr uint32_t kHalf = 0x00F9A5AC;           // 0.5f
constexpr uint32_t kNegZero = 0x00F988F0;        // -0.0f
constexpr uint32_t kOne = 0x0107A538;            // 1.0f
constexpr uint32_t kQuarter = 0x0106E8A8;        // 0.25f, the rim's inset
constexpr uint32_t kMinusOne = 0x00FAA210;       // -1.0f
constexpr uint32_t kMinusNine = 0x01183C10;      // -0.9f, the weight on every normal emit
constexpr uint32_t kTwoPow32 = 0x00F98940;       // 4294967296.0f
constexpr uint32_t kSqrt2 = 0x0106F8F0;          // 1.41421354f
constexpr uint32_t kTwoSqrt2 = 0x0106F91C;       // 2.8284271f
constexpr uint32_t kMinusRootHalf = 0x01063520;  // -0.70710677f
constexpr uint32_t kRootHalf = 0x0106F8D4;       // 0.70710677f
constexpr uint32_t kNormalOut = 0x0106F8D8;      // 0.57735026f = 1/sqrt(3)
constexpr uint32_t kNormalIn = 0x0106F918;       // -0.57735026f

// addps/mulps pinned to the operand order of the original instruction: MSVC may swap the operands
// of a commutative packed op, which changes the NaN payload it returns (same trap as unit
// clu_6c8680_Debug - see the project notes).
__m128 AddP(__m128 a, __m128 b)
{
    const __m128 sum = _mm_add_ps(a, b);
    const __m128 bothNan = _mm_and_ps(_mm_cmpunord_ps(a, a), _mm_cmpunord_ps(b, b));
    const __m128 quietA = _mm_or_ps(a, _mm_set1_ps(F32(0x00400000u)));
    return _mm_or_ps(_mm_and_ps(bothNan, quietA), _mm_andnot_ps(bothNan, sum));
}

__m128 MulP(__m128 a, __m128 b)
{
    const __m128 product = _mm_mul_ps(a, b);
    const __m128 bothNan = _mm_and_ps(_mm_cmpunord_ps(a, a), _mm_cmpunord_ps(b, b));
    const __m128 quietA = _mm_or_ps(a, _mm_set1_ps(F32(0x00400000u)));
    return _mm_or_ps(_mm_and_ps(bothNan, quietA), _mm_andnot_ps(bothNan, product));
}

// The (world -> uv) map both handlers apply to every emitted vertex. [self+0xc]/[self+0x10] are
// the integer origins (cvtsi2ss in the original), [self+0x24..0x30] the scale and offset pairs;
// all six fields are re-read at every emit, so a callee that moves the renderer mid-loop shows up.
float UvX(void* self, float x)
{
    return (x - static_cast<float>(LoadI32(self, 0x0c))) * LoadF32(self, 0x24) + LoadF32(self, 0x2c);
}

float UvZ(void* self, float z)
{
    return (z - static_cast<float>(LoadI32(self, 0x10))) * LoadF32(self, 0x28) + LoadF32(self, 0x30);
}

// The mesh builder's emit calls (all unrewritten, all __thiscall on the draw object).
// Preserve the discarded conversion's observable x87 exception flags (and traps).
// No control-word change occurs in the binary: FIST obeys the caller's rounding mode.
void DiscardTopFraction(float top)
{
    const float five = ConstF(0x00FBD498);
    float scaled, fraction;
    int integral;
    __asm {
        fld top
        fmul five
        fstp scaled
        fld scaled
        fist integral
        fild integral
        fsubrp st(1), st(0)
        fstp fraction
    }
}

// The generic double bridge would spill ST0 to 64 bits. This ABI-specific bridge keeps
// C03F50's return live through FADD/FSUB, exactly as 00B8D2E2..00B8D313 do.
float BaseHeight(float ay, float centre, float inset)
{
    const uintptr_t target = 0x00C03F50;
    float result;
    __asm {
        push ay
        call target
        add esp, 4
        fadd centre
        fsub inset
        fstp result
    }
    return result;
}

constexpr unsigned kAddNormal = 0x006D5C80;      // (nx, ny, nz, weight)
constexpr unsigned kAddUv = 0x006D5D60;          // (flag, u, v, 0.0f, 0.0f)
constexpr unsigned kAddVertex = 0x006D5E80;      // (x, y, z)
constexpr unsigned kAddSideNormal = 0x006D5CD0;  // (&aligned normal, weight)
constexpr unsigned kBeginQuad = 0x006D4B20;      // (list, a, b, c)
constexpr unsigned kAddIndex = 0x006D4BC0;       // (index)
constexpr unsigned kAddTriangle = 0x006D4A50;    // (i, i, i)

#endif  // _M_IX86

}  // namespace

#if defined(_M_IX86)

// TS3W.exe 0x00B8CAD0. See header. The eight side normals come from the cross product of two
// corner vectors whose y lane is -top; the original normalises them with rsqrtps plus two
// Newton-Raphson steps, transcribed here lane for lane so the emitted bits match exactly.
void RenderLotType9(void* self, void* lot, void* draw)
{
    if (LoadI32(lot, 0x2c) != 9)  // 00B8CAE0: every other lot type is silently skipped
        return;

    const float top = CallGameMethod<float, 0x00BF4E40>(lot);   // 00B8CAF0
    const float sx = LoadF32(lot, 0x84);                        // 00B8CAF9
    const float hy = LoadF32(lot, 0x88) * ConstF(kHalf);        // 00B8CB0A..CB1C
    const uint32_t vertexBase = CallGameMethod<uint32_t, 0x006D0AD0>(draw);     // 00B8CB22
    const uint32_t indexBase = CallGameMethod<uint32_t, 0x006D0B50>(draw, 0u);  // 00B8CB2F
    const uint32_t triangleBase = CallGameMethod<uint32_t, 0x006D0B10>(draw);   // 00B8CB3A
    const float centreY = CallGameVirtual<float, 22>(lot);                      // 00B8CB4A
    DiscardTopFraction(top);  // 00B8CB50..CB7C: dead value, live x87 status.
    const float ay = LoadF32(lot, 0x7c);                        // 00B8CB7F
    const float ax = LoadF32(lot, 0x80);                        // 00B8CBA6

    const float negZero = ConstF(kNegZero);
    const float negTop = negZero - top;            // 00B8CC51
    const float peakY = centreY + top;             // 00B8CC4D
    const float rimY = centreY - ConstF(kQuarter); // 00B8CC45
    const float negHy = negZero - hy;              // 00B8CBB7
    const float negSx = negZero - sx;              // 00B8CBBE
    const float half = ConstF(kHalf);
    const float weight = ConstF(kMinusNine);

    // The corner table at 00B8CBE3..00B8CC33: eight (x, z) pairs once round the octagon.
    const float corners[16] = {hy,    sx,   sx,   hy,   sx,    negHy, hy,    negSx,
                               negHy, negSx, negSx, negHy, negSx, hy,   negHy, sx};
    const __m128 ones = _mm_set1_ps(ConstF(kOne));    // 00B8CC10/CC55: the 1.0f broadcast
    const __m128 halves = _mm_set1_ps(half);          // 00B8CB98/CC59: the 0.5f broadcast

    for (int i = 0; i < 8; ++i) {
        const int next = (i + 1) & 7;  // 00B8CCD2: the counter starts at 1, so this is c & 7
        const float vx = corners[2 * i];
        const float vz = corners[2 * i + 1];
        // The two vectors the original reads back from the stack slots at esp+0x90 and esp+0xa0;
        // lane 3 is a copy of lane 0 and none of the shuffles touches it.
        const __m128 v = _mm_setr_ps(vx, negTop, vz, vx);
        const __m128 u = _mm_setr_ps(corners[2 * next], negTop, corners[2 * next + 1],
                                     corners[2 * next]);
        const __m128 cross = _mm_sub_ps(MulP(_mm_shuffle_ps(u, u, 0x12), _mm_shuffle_ps(v, v, 9)),
                                        MulP(_mm_shuffle_ps(u, u, 9), _mm_shuffle_ps(v, v, 0x12)));
        const __m128 square = MulP(cross, cross);
        const __m128 folded = AddP(AddP(_mm_shuffle_ps(square, square, 2),
                                        _mm_shuffle_ps(square, square, 1)), square);
        const __m128 length2 = _mm_shuffle_ps(folded, folded, 0);
        __m128 inverse = _mm_rsqrt_ps(length2);  // 00B8CD7F
        for (int step = 0; step < 2; ++step) {   // 00B8CD82..00B8CDBD: two identical refinements
            inverse = AddP(MulP(_mm_sub_ps(ones, MulP(MulP(inverse, inverse), length2)),
                                MulP(halves, inverse)), inverse);
        }
        alignas(16) float normal[4];  // 00B8CDC6: the movaps store 006D5CD0 reads back aligned
        _mm_store_ps(normal, MulP(inverse, cross));
        CallGameMethod<void, kAddSideNormal>(draw, normal, weight);

        const float px = vx + ay;
        const float pz = vz + ax;
        CallGameMethod<void, kAddVertex>(draw, px, centreY, pz);
        CallGameMethod<void, kAddUv>(draw, 0, UvX(self, px), UvZ(self, pz), 0.0f, 0.0f);
    }

    // The centre column: the peak, then the rim - and both get the same uv, twice in a row.
    CallGameMethod<void, kAddVertex>(draw, ay, peakY, ax);
    CallGameMethod<void, kAddUv>(draw, 0, UvX(self, ay), UvZ(self, ax), 0.0f, 0.0f);
    CallGameMethod<void, kAddVertex>(draw, ay, rimY, ax);
    CallGameMethod<void, kAddUv>(draw, 0, UvX(self, ay), UvZ(self, ax), 0.0f, 0.0f);
    CallGameMethod<void, kAddNormal>(draw, 0.0f, ConstF(kMinusOne), 0.0f, weight);

    // The rim ring: one vertex per corner, and the midpoint of the corner it wraps to.
    for (int i = 0; i < 8; ++i) {
        const float vx = corners[2 * i];
        const float vz = corners[2 * i + 1];
        const float px = vx + ay;
        const float pz = vz + ax;
        CallGameMethod<void, kAddVertex>(draw, px, rimY, pz);
        CallGameMethod<void, kAddUv>(draw, 0, UvX(self, px), UvZ(self, pz), 0.0f, 0.0f);
        const int next = (i + 1) & 7;
        CallGameMethod<void, kAddNormal>(draw, (corners[2 * next] + vx) * half, 0.0f,
                                        (corners[2 * next + 1] + vz) * half, weight);
    }

    // Two index passes over the octagon: the rim quads (list 8) and the skirt quads (list 9).
    // The counts reach them truncated to 16 bits (movzx), as the original's `lea`s do.
    const uint32_t vertices = static_cast<uint16_t>(vertexBase);
    const uint32_t indices = static_cast<uint16_t>(indexBase);
    for (uint32_t i = 0; i < 8; ++i) {
        const uint32_t next = (i + 1u) & 7u;  // 00B8D0B2..D0BD adds 1 in CL, then masks 7
        CallGameMethod<void, kBeginQuad>(draw, 0, indices + 8u, indices + i, indices + next);
        CallGameMethod<void, kAddIndex>(draw, vertices + 8u);
        CallGameMethod<void, kAddIndex>(draw, vertices + i);
        CallGameMethod<void, kAddIndex>(draw, vertices + next);
        const uint32_t triangle = static_cast<uint16_t>(triangleBase + i);
        CallGameMethod<void, kAddTriangle>(draw, triangle, triangle, triangle);
    }
    const uint32_t skirtTriangle = static_cast<uint16_t>(triangleBase + 8u);
    for (uint32_t i = 0; i < 8; ++i) {
        const uint32_t next = (i + 1u) & 7u;
        const uint32_t skirt = static_cast<uint16_t>(i + 10u);
        CallGameMethod<void, kBeginQuad>(draw, 0, indices + 9u, indices + next + 10u,
                                        indices + skirt);
        CallGameMethod<void, kAddIndex>(draw, vertices + 9u);
        CallGameMethod<void, kAddIndex>(draw, vertices + next + 10u);
        CallGameMethod<void, kAddIndex>(draw, vertices + skirt);
        CallGameMethod<void, kAddTriangle>(draw, skirtTriangle, skirtTriangle, skirtTriangle);
    }

    // And eight strips, which take the untruncated counts and the 16-bit corner ids.
    const uint32_t capTriangle = static_cast<uint16_t>(triangleBase + 9u);
    for (uint32_t i = 0; i < 8; ++i) {
        const uint32_t next = (i + 1u) & 7u;
        CallGameFunction<void, 0x00B894B0>(draw, vertexBase, indexBase, capTriangle, i, i + 10u,
                                          next + 10u, next);
    }
}

// TS3W.exe 0x00B8D210. See header. [lot+0x38] selects between two otherwise identical caps: the
// variant changes which box corners each top vertex interpolates, flips the diagonal offsets and
// reorders the normals and every index list.
int32_t RenderLotType13(void* self, void* lot, void* draw)
{
    const uint32_t vertexBase = CallGameMethod<uint32_t, 0x006D0AD0>(draw);     // 00B8D225
    const uint32_t triangleBase = CallGameMethod<uint32_t, 0x006D0B10>(draw);   // 00B8D230
    const uint32_t indexBase = CallGameMethod<uint32_t, 0x006D0B50>(draw, 0u);  // 00B8D23B
    const int32_t heightHigh = LoadI32(lot, 0xe0);                              // 00B8D247
    const uint8_t variant = LoadU8(lot, 0x38);                                  // 00B8D24D
    const int32_t heightLow = LoadI32(lot, 0xdc);                                // 00B8D254
    CallGameVirtual<void, 4>(lot);  // 00B8D26D: the lot's own vtable slots 4 and 5
    CallGameVirtual<void, 5>(lot);  // 00B8D276

    double height = variant ? static_cast<double>(heightHigh) : static_cast<double>(heightLow);
    if (height < 0.0)  // 00B8D295..D297: a negative count is pushed back into the unsigned range
        height += static_cast<double>(ConstF(kTwoPow32));
    const float halfHeight = static_cast<float>(height * 0.5);  // 00B8D29D..D2AD

    const float ay = LoadF32(lot, 0x3c);              // 00B8D2A3
    const float inset = LoadF32(lot, 0x44);           // 00B8D2BD
    const float reach = halfHeight * ConstF(kSqrt2);  // 00B8D2C0
    const float centreY = CallGameVirtual<float, 22>(lot);  // 00B8D2D4
    const float base = BaseHeight(ay, centreY, inset);
    const float apex = (ay * ConstF(kTwoSqrt2)) * halfHeight + base;  // 00B8D2EB..D31D

    const float half = ConstF(kHalf);
    const float weight = ConstF(kMinusNine);
    const float drop = reach * ConstF(kMinusRootHalf);  // 00B8D364 / 00B8D538
    // The lot's corner box: four (x, z) pairs at the shared height `base`.
    const float nearX = LoadF32(lot, 0x84), nearZ = LoadF32(lot, 0x88);  // 00B8D374 / 00B8D356
    const float midX = LoadF32(lot, 0x8c), midZ = LoadF32(lot, 0x90);    // 00B8D3A2 / 00B8D3C3
    const float backX = LoadF32(lot, 0x7c), backZ = LoadF32(lot, 0x80);  // 00B8D3B8 / 00B8D3D1
    const float frontX = LoadF32(lot, 0x94), frontZ = LoadF32(lot, 0x98);  // 00B8D34E / 00B8D303

    // The six vertices, in the order the original stores them at esp+0x60..esp+0xa4.
    float vertices[18] = {nearX,  base, nearZ,   // the four box corners
                          midX,   base, midZ,
                          backX,  base, backZ,
                          frontX, base, frontZ,
                          0.0f, apex, 0.0f,      // the two top edge points
                          0.0f, apex, 0.0f};
    float normals[12];
    int first, second, mid0, mid1;
    int strip0, strip1, strip2, strip3, quad0, quad1, quad2, quad3;

    const float normalOut = ConstF(kNormalOut);
    const float normalIn = ConstF(kNormalIn);
    if (variant) {
        vertices[12] = (frontX + midX) * half + drop;
        vertices[14] = (frontZ + midZ) * half + drop;
        vertices[15] = (backX + nearX) * half - drop;
        vertices[17] = (backZ + nearZ) * half - drop;
        normals[0] = normalIn;  normals[1] = normalOut; normals[2] = normalIn;
        normals[3] = normalOut; normals[4] = normalOut; normals[5] = normalOut;
        normals[6] = normalOut; normals[7] = normalOut; normals[8] = normalIn;
        normals[9] = normalIn;  normals[10] = normalOut; normals[11] = normalOut;
        first = 0; second = 2; mid0 = 3; mid1 = 1;
        strip0 = 1; strip1 = 0; strip2 = 5; strip3 = 4;
        quad0 = 2; quad1 = 3; quad2 = 4; quad3 = 5;
    } else {
        const float rise = reach * ConstF(kRootHalf);  // 00B8D540
        vertices[12] = (frontX + backX) * half - drop;
        vertices[14] = (frontZ + backZ) * half - rise;
        vertices[15] = (midX + nearX) * half + drop;
        vertices[17] = (midZ + nearZ) * half + rise;
        normals[0] = normalOut; normals[1] = normalIn;  normals[2] = normalOut;
        normals[3] = normalIn;  normals[4] = normalOut; normals[5] = normalOut;
        normals[6] = normalIn;  normals[7] = normalIn;  normals[8] = normalOut;
        normals[9] = normalOut; normals[10] = normalOut; normals[11] = normalOut;
        first = 1; second = 0; mid0 = 2; mid1 = 3;
        strip0 = 0; strip1 = 2; strip2 = 4; strip3 = 5;
        quad0 = 3; quad1 = 1; quad2 = 5; quad3 = 4;
    }

    for (int i = 0; i < 6; ++i)
        CallGameMethod<void, kAddVertex>(draw, vertices[3 * i], vertices[3 * i + 1],
                                        vertices[3 * i + 2]);
    for (int i = 0; i < 6; ++i)
        CallGameMethod<void, kAddUv>(draw, 0, UvX(self, vertices[3 * i]),
                                    UvZ(self, vertices[3 * i + 2]), 0.0f, 0.0f);
    const float negZero = ConstF(kNegZero);
    for (int i = 0; i < 4; ++i) {  // 00B8D770..D806: each normal, then its mirror
        CallGameMethod<void, kAddNormal>(draw, normals[3 * i], normals[3 * i + 1],
                                        normals[3 * i + 2], weight);
        CallGameMethod<void, kAddNormal>(draw, negZero - normals[3 * i], negZero - normals[3 * i + 1],
                                        negZero - normals[3 * i + 2], weight);
    }

    // The four quads. As in the type-9 handler, the index lists get 16-bit counts and 0x00B894B0
    // gets them untruncated.
    const uint32_t vertices16 = static_cast<uint16_t>(vertexBase);
    const uint32_t indices16 = static_cast<uint16_t>(indexBase);
    // All four triangle ids are (uint16) of the count plus the quad number (00B8D879/00B8D8D4/
    // 00B8D94A/00B8D997 each push a movzx); only the 0x00B894B0 strips keep the full 32 bits.
    const uint32_t t0 = static_cast<uint16_t>(triangleBase);
    CallGameMethod<void, kBeginQuad>(draw, 0, indices16 + static_cast<uint32_t>(first),
                                    indices16 + static_cast<uint32_t>(second), indices16 + 5u);
    CallGameMethod<void, kAddIndex>(draw, vertices16 + static_cast<uint32_t>(first));
    CallGameMethod<void, kAddIndex>(draw, vertices16 + static_cast<uint32_t>(second));
    CallGameMethod<void, kAddIndex>(draw, vertices16 + 5u);
    CallGameMethod<void, kAddTriangle>(draw, t0, t0, t0);

    const uint32_t t1 = static_cast<uint16_t>(triangleBase + 1u);
    CallGameMethod<void, kBeginQuad>(draw, 0, indices16 + 5u, indices16 + static_cast<uint32_t>(second),
                                    indices16 + static_cast<uint32_t>(first));
    CallGameMethod<void, kAddIndex>(draw, vertices16 + 5u);
    CallGameMethod<void, kAddIndex>(draw, vertices16 + static_cast<uint32_t>(second));
    CallGameMethod<void, kAddIndex>(draw, vertices16 + static_cast<uint32_t>(first));
    CallGameMethod<void, kAddTriangle>(draw, t1, t1, t1);

    const uint32_t t2 = static_cast<uint16_t>(triangleBase + 2u);
    CallGameMethod<void, kBeginQuad>(draw, 0, indices16 + static_cast<uint32_t>(mid0),
                                    indices16 + static_cast<uint32_t>(mid1), indices16 + 4u);
    CallGameMethod<void, kAddIndex>(draw, vertices16 + static_cast<uint32_t>(mid0));
    CallGameMethod<void, kAddIndex>(draw, vertices16 + static_cast<uint32_t>(mid1));
    CallGameMethod<void, kAddIndex>(draw, vertices16 + 4u);
    CallGameMethod<void, kAddTriangle>(draw, t2, t2, t2);

    const uint32_t t3 = static_cast<uint16_t>(triangleBase + 3u);
    CallGameMethod<void, kBeginQuad>(draw, 0, indices16 + 4u, indices16 + static_cast<uint32_t>(mid1),
                                    indices16 + static_cast<uint32_t>(mid0));
    CallGameMethod<void, kAddIndex>(draw, vertices16 + 4u);
    CallGameMethod<void, kAddIndex>(draw, vertices16 + static_cast<uint32_t>(mid1));
    CallGameMethod<void, kAddIndex>(draw, vertices16 + static_cast<uint32_t>(mid0));
    CallGameMethod<void, kAddTriangle>(draw, t3, t3, t3);

    CallGameFunction<void, 0x00B894B0>(draw, vertexBase, indexBase, triangleBase + 4u,
                                      static_cast<uint32_t>(strip0), static_cast<uint32_t>(strip1),
                                      static_cast<uint32_t>(strip2), static_cast<uint32_t>(strip3));
    CallGameFunction<void, 0x00B894B0>(draw, vertexBase, indexBase, triangleBase + 5u,
                                      static_cast<uint32_t>(strip3), static_cast<uint32_t>(strip2),
                                      static_cast<uint32_t>(strip1), static_cast<uint32_t>(strip0));
    CallGameFunction<void, 0x00B894B0>(draw, vertexBase, indexBase, triangleBase + 6u,
                                      static_cast<uint32_t>(quad0), static_cast<uint32_t>(quad1),
                                      static_cast<uint32_t>(quad2), static_cast<uint32_t>(quad3));
    return CallGameFunction<int32_t, 0x00B894B0>(draw, vertexBase, indexBase, triangleBase + 7u,
                                                 static_cast<uint32_t>(quad3), static_cast<uint32_t>(quad2),
                                                 static_cast<uint32_t>(quad1), static_cast<uint32_t>(quad0));
}

#else  // x64: no game image to bridge into; kept so the unit still compiles.

void RenderLotType9(void*, void*, void*) { NotRewritten("RenderLotType9 game dependencies"); }
int32_t RenderLotType13(void*, void*, void*) { NotRewritten("RenderLotType13 game dependencies"); }

#endif  // _M_IX86

}  // namespace ts3::renderer
