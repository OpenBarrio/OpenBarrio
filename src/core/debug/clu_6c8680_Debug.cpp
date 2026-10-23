// Unit clu_6c8680_Debug: TS3W.exe 0x006C8680 / 0x006C87F0 / 0x00814C30 / 0x008592A0 /
// 0x00860DF0 (see the header for the map). The SSE chains of 0x00860DF0 and 0x008592A0
// are transcribed operation-for-operation from the originals so every float lane is
// bit-identical, including NaN payloads: packed add/mul go through AddP/MulP, which
// pin the operand order MSVC is otherwise free to swap for commutative ops (same trap
// as unit clu_961480_x - see the project notes).
#define WIN32_LEAN_AND_MEAN
#define NOMINMAX
#include <windows.h>

#include <cstdint>
#include <cstring>
#include <intrin.h>

#include "ts3/debug/clu_6c8680_Debug.h"
#include "ts3/layout.h"

namespace ts3::debug {
namespace {

// Game globals the two higher drawers read (the originals touch the mapped image
// directly; on x64 every gate then fails closed and the functions early-out).
constexpr uint32_t kSpyGate = 0x11D90B0;          // byte: line-drawing master switch
constexpr uint32_t kShapesSingleton = 0x11D9140;  // shape drawer's container lives here
constexpr uint32_t kShapeOffsetVec = 0x11D93D0;   // xmmword offset {0,0.2,0,0}
constexpr uint32_t kShapeOffsetFlag = 0x11D93E0;  // dword, bit0 = offset initialised

uint32_t GameGlobal32(uint32_t address)
{
#if defined(_M_IX86)
    return *reinterpret_cast<const uint32_t*>(static_cast<uintptr_t>(address));
#else
    (void)address;
    return 0;
#endif
}

uint8_t GameGlobal8(uint32_t address)
{
#if defined(_M_IX86)
    return *reinterpret_cast<const uint8_t*>(static_cast<uintptr_t>(address));
#else
    (void)address;
    return 0;
#endif
}

void* GameGlobalPointer(uint32_t address)
{
#if defined(_M_IX86)
    return *reinterpret_cast<void* const*>(static_cast<uintptr_t>(address));
#else
    (void)address;
    return nullptr;
#endif
}

void GameGlobalSet32(uint32_t address, uint32_t value)
{
#if defined(_M_IX86)
    *reinterpret_cast<uint32_t*>(static_cast<uintptr_t>(address)) = value;
#else
    (void)address;
    (void)value;
#endif
}

__m128 GameGlobalLoad16(uint32_t address)
{
#if defined(_M_IX86)
    return _mm_loadu_ps(reinterpret_cast<const float*>(static_cast<uintptr_t>(address)));
#else
    (void)address;
    return _mm_setzero_ps();
#endif
}

void GameGlobalStore16(uint32_t address, const __m128& value)
{
#if defined(_M_IX86)
    _mm_storeu_ps(reinterpret_cast<float*>(static_cast<uintptr_t>(address)), value);
#else
    (void)address;
    (void)value;
#endif
}

float F32(uint32_t bits)
{
    float f;
    std::memcpy(&f, &bits, sizeof f);
    return f;
}

// addps/mulps with the operand order the original instruction uses: when both lanes
// are NaN the SSE result carries the first operand's payload, so a compiler swap of
// the commutative C++ operands would change bits. These two helpers keep a's
// payload on both-NaN lanes and are plain ops everywhere else.
__m128 AddP(__m128 a, __m128 b)
{
    const __m128 r = _mm_add_ps(a, b);
    const __m128 both = _mm_and_ps(_mm_cmpunord_ps(a, a), _mm_cmpunord_ps(b, b));
    const __m128 quietA = _mm_or_ps(a, _mm_set1_ps(F32(0x00400000u)));
    return _mm_or_ps(_mm_and_ps(both, quietA), _mm_andnot_ps(both, r));
}

__m128 MulP(__m128 a, __m128 b)
{
    const __m128 r = _mm_mul_ps(a, b);
    const __m128 both = _mm_and_ps(_mm_cmpunord_ps(a, a), _mm_cmpunord_ps(b, b));
    const __m128 quietA = _mm_or_ps(a, _mm_set1_ps(F32(0x00400000u)));
    return _mm_or_ps(_mm_and_ps(both, quietA), _mm_andnot_ps(both, r));
}

constexpr uint32_t kColorGray = 0xFFAAAAAAu;  // 0x008592F7 / 0x008593FC pushes
constexpr float kQuadScale = 0.6f;            // 0x008594AA [0xF9D500] = 0x3F19999A

}  // namespace

DebugLineVertex* DebugLineBuffer::AddSegment(const Vec4* a, const Vec4* b, uint32_t colorA,
                                             uint32_t colorB)
{
    DebugLineVertex* pos = end;  // 006C868C
    const int32_t count =
        static_cast<int32_t>(reinterpret_cast<uintptr_t>(end) - reinterpret_cast<uintptr_t>(begin)) >> 4;  // sar
    if (static_cast<uint32_t>(count + 2) > limit)  // 006C8696..9C ja (unsigned)
        return pos;

    alignas(16) DebugLineVertex elemA;  // 006C86A8..E1 {a.xyz, colorA}
    std::memcpy(&elemA, a, 3 * sizeof(float));
    elemA.color = colorA;
    if (reinterpret_cast<uintptr_t>(pos) < reinterpret_cast<uintptr_t>(cap)) {  // 006C86A2/86E5
        end = pos + 1;                                                          // 006C86E9
        if (pos)                                                                // 006C86E7 EA quirk: bump without write
            *pos = elemA;
    } else {
        // 006C870A..12: grow through the game's 0x006D3810 (this, end, &elem).
        pos = static_cast<DebugLineVertex*>(
            ts3::CallGameMethod<void*, 0x6D3810>(this, pos, &elemA));
    }

    pos = end;  // 006C871D re-read after the possible grow
    alignas(16) DebugLineVertex elemB;  // 006C8726..59 {b.xyz, colorB}
    std::memcpy(&elemB, b, 3 * sizeof(float));
    elemB.color = colorB;
    if (reinterpret_cast<uintptr_t>(pos) < reinterpret_cast<uintptr_t>(cap)) {  // 006C8720/875D
        end = pos + 1;                                                          // 006C8764
        if (pos)                                                                // 006C875F EA quirk again
            *pos = elemB;
        return pos;  // 006C8780
    }
    return static_cast<DebugLineVertex*>(
        ts3::CallGameMethod<void*, 0x6D3810>(this, pos, &elemB));  // 006C8787..8F
}

DebugLineVertex* DebugLineBuffer::AddCross(const Vec4* center, float half, uint32_t color)
{
    // Extension vectors as the original builds them (006C880C..006C885F); the w
    // lanes are never stored into an element, only shuffled around harmlessly.
    alignas(16) const Vec4 extX = {half, 0.0f, 0.0f, half};
    alignas(16) const Vec4 extY = {0.0f, half, 0.0f, 0.0f};
    alignas(16) const Vec4 extZ = {0.0f, 0.0f, half, 0.0f};
    const __m128 c = _mm_loadu_ps(&center->x);  // 006C886E (orig movaps)

    alignas(16) Vec4 cpx, cmx, cpy, cmy, cpz, cmz;
    _mm_storeu_ps(&cpx.x, AddP(c, _mm_loadu_ps(&extX.x)));           // 006C887A
    _mm_storeu_ps(&cmx.x, _mm_sub_ps(c, _mm_loadu_ps(&extX.x)));    // 006C887D
    _mm_storeu_ps(&cpy.x, AddP(c, _mm_loadu_ps(&extY.x)));           // 006C88AC
    _mm_storeu_ps(&cmy.x, _mm_sub_ps(c, _mm_loadu_ps(&extY.x)));    // 006C88AF
    _mm_storeu_ps(&cpz.x, AddP(c, _mm_loadu_ps(&extZ.x)));           // 006C88DA
    _mm_storeu_ps(&cmz.x, _mm_sub_ps(c, _mm_loadu_ps(&extZ.x)));    // 006C88DD

    AddSegment(&cmx, &cpx, color, color);  // 006C8891
    AddSegment(&cmy, &cpy, color, color);  // 006C88BF
    return AddSegment(&cmz, &cpz, color, color);  // 006C88ED
}

void DebugLineDrawer::TryAddPair(const Vec4* a, const Vec4* b)
{
    if (!GameGlobal8(kSpyGate))  // 00814C30
        return;
    if (!enabledA || !enabledB || !buffer)  // 00814C3C..52
        return;
    auto* cs = reinterpret_cast<CRITICAL_SECTION*>(lock);  // +0x48, x86 layout
    if (TryEnterCriticalSection(cs)) {                    // 00814C59 via IAT 0xF95218
        buffer->AddSegment(a, b, 0xFFFFFF00u, 0xFFFFFF00u);  // 00814C7A
        LeaveCriticalSection(cs);                            // 00814C80 via IAT 0xF95130
    }
}

DebugLineVertex* DebugShapeDrawer::DrawShapes()
{
    auto* g = GameGlobalPointer(kShapesSingleton);  // 008592A6
    if (!g)
        return reinterpret_cast<DebugLineVertex*>(g);  // 008592B5
    if (!*reinterpret_cast<const uint8_t*>(static_cast<uint8_t*>(g) + 0x100))  // 008592BD
        return reinterpret_cast<DebugLineVertex*>(g);
    auto* buffer =
        *reinterpret_cast<DebugLineBuffer**>(static_cast<uint8_t*>(g) + 0xDC);  // 008592CA
    if (!buffer)
        return reinterpret_cast<DebugLineVertex*>(g);  // 008592D2

    alignas(16) Vec4 orient, worldPos, memberPos;  // [esp+20/70/60]
    CallGameVirtual<void, 9>(transformProxy, &orient);    // 008592D8 (vtable +0x24)
    CallGameVirtual<void, 7>(transformProxy, &worldPos);  // 008592E7 (vtable +0x1C)
    CallGameVirtual<void, 2>(&shape, &memberPos);         // 008592F6 (vtable +0x08)

    const __m128 q = _mm_loadu_ps(&orient.x);      // xmm4
    const __m128 world = _mm_loadu_ps(&worldPos.x);  // xmm3
    const __m128 obj = _mm_loadu_ps(&memberPos.x);   // xmm2
    const __m128 diff = _mm_sub_ps(obj, world);      // 00859315..1B
    const __m128 ex = _mm_set_ps(0.0f, 0.0f, 0.0f, 1.0f);  // {1,0,0,0} [esp+0x10]

    // v = {q3,q2,-q1,0}, basis = 2*(shuf18(v)*shuf9(q) - shuf9(v)*shuf18(q)) + ex
    // (00859343..008593A8) - the quaternion rotation of the x axis.
    const __m128 q9 = _mm_shuffle_ps(q, q, 9);
    const __m128 q18 = _mm_shuffle_ps(q, q, 0x12);
    __m128 v = MulP(_mm_shuffle_ps(ex, ex, 0x12), q9);        // 00859367
    v = _mm_sub_ps(v, MulP(_mm_shuffle_ps(ex, ex, 9), q18));  // 0085936A
    v = AddP(v, MulP(_mm_shuffle_ps(q, q, 0xFF), ex));        // 00859377
    __m128 basis = MulP(_mm_shuffle_ps(v, v, 0x12), q9);              // 00859388
    basis = _mm_sub_ps(basis, MulP(_mm_shuffle_ps(v, v, 9), q18));    // 0085938B
    basis = _mm_add_ps(_mm_mul_ps(basis, _mm_set1_ps(2.0f)), ex);     // 008593A5..A8

    // Lazy one-time offset {0, 0.2, 0, 0} in the global (0085939B..008593E4).
    __m128 offset;
    if (GameGlobal32(kShapeOffsetFlag) & 1u) {
        offset = GameGlobalLoad16(kShapeOffsetVec);  // 008593E4
    } else {
        GameGlobalSet32(kShapeOffsetFlag, GameGlobal32(kShapeOffsetFlag) | 1u);  // 008593B8
        offset = _mm_set_ps(0.0f, 0.0f, F32(0x3E4CCCCD), 0.0f);  // 008593BE..D6 {0,0.2,0,0}
        GameGlobalStore16(kShapeOffsetVec, offset);               // 008593DB
    }

    const __m128 v9 = _mm_sub_ps(world, diff);  // 008593F2
    const Vec4* dirp = reinterpret_cast<const Vec4*>(shape.dir);  // this+0x30

    if (mode == 2) {  // 0x008593EB
        const __m128 s = MulP(_mm_set1_ps(scale), basis);  // 00859407..13
        alignas(16) Vec4 p1, v14, v15, v16;
        __m128 t = AddP(s, obj);                              // 00859419
        t = AddP(t, offset);                                  // 0085941C
        _mm_storeu_ps(&p1.x, t);                              // 0085941F [esp+0x10]
        __m128 a = _mm_sub_ps(obj, s);                        // 00859430
        a = AddP(a, offset);                                  // 00859439
        __m128 b = AddP(v9, s);                               // 00859433
        b = AddP(b, offset);                                  // 0085943C
        __m128 c = _mm_sub_ps(v9, s);                         // 00859436
        c = AddP(c, offset);                                  // 0085943F
        _mm_storeu_ps(&v14.x, a);                             // 00859445
        _mm_storeu_ps(&v15.x, b);                             // 0085944A
        _mm_storeu_ps(&v16.x, c);                             // 0085944F
        // Box corners as four gray segments (00859454..008594A5).
        buffer->AddSegment(&p1, &v14, kColorGray, kColorGray);   // 00859454
        buffer->AddSegment(&p1, &v15, kColorGray, kColorGray);   // 0085946F
        buffer->AddSegment(&v16, &v14, kColorGray, kColorGray);  // 0085948A
        buffer->AddSegment(&v16, &v15, kColorGray, kColorGray);  // 008594A5
        // Four quad outlines (008594AA..00859534).
        DrawQuadOutline(buffer, &p1, dirp, 0xFFFF0000u, kQuadScale, kQuadScale);   // 008594C9
        DrawQuadOutline(buffer, &v14, dirp, 0xFFFF0000u, kQuadScale, kQuadScale);  // 008594EA
        DrawQuadOutline(buffer, &v15, &orient, 0xFF00FFFFu, kQuadScale, kQuadScale);  // 0085950F
        return DrawQuadOutline(buffer, &v16, &orient, 0xFF00FFFFu, kQuadScale, kQuadScale);  // 00859534
    }

    // Cross mode (00859542..00859695): four spokes of length 2*0.15*basis around
    // two centers offset from the corners, then two outlines.
    const __m128 w = AddP(obj, offset);  // 00859542
    const __m128 u = AddP(v9, offset);   // 00859545
    const __m128 h = MulP(_mm_set1_ps(F32(0x3E19999A)), basis);  // 00859548..54 0.15
    alignas(16) Vec4 wv, uv, wph, uph, umh, wmh;
    _mm_storeu_ps(&wv.x, w);                             // 00859574 [esp+0x50]
    _mm_storeu_ps(&uv.x, u);                             // 00859579 [esp+0x20]
    _mm_storeu_ps(&wph.x, AddP(h, w));              // 0085956F..83 (and 008595C3)
    _mm_storeu_ps(&uph.x, AddP(h, u));              // 008595A7 / 00859636
    _mm_storeu_ps(&wmh.x, _mm_sub_ps(w, h));        // 00859566..7E
    _mm_storeu_ps(&umh.x, _mm_sub_ps(u, h));        // 00859603..09
    buffer->AddSegment(&wph, &wmh, kColorGray, kColorGray);  // 0085958B (w+h, w-h)
    buffer->AddSegment(&wph, &uph, kColorGray, kColorGray);  // 008595CE (w+h, u+h)
    buffer->AddSegment(&umh, &wmh, kColorGray, kColorGray);  // 0085960E (u-h, w-h)
    buffer->AddSegment(&umh, &uph, kColorGray, kColorGray);  // 0085964C (u-h, u+h)
    DrawQuadOutline(buffer, &wv, dirp, 0xFFFF0000u, kQuadScale, kQuadScale);    // 00859670
    return DrawQuadOutline(buffer, &uv, &orient, 0xFF00FFFFu, kQuadScale, kQuadScale);  // 00859695
}

DebugLineVertex* DrawQuadOutline(DebugLineBuffer* buffer, const Vec4* point, const Vec4* dir,
                                 uint32_t color, float lenScale, float widScale)
{
    if (!buffer)  // 00860E00: original returns stale eax; no caller passes null
        return nullptr;

    // Orthonormal frame around `dir`: pole {0,0,-1,0} (00860E06..34), then the
    // three shuffled cross products basis1/basis2/basis3 (00860E37..00860F0B).
    const __m128 pole = _mm_sub_ps(_mm_setzero_ps(), _mm_set_ps(0.0f, 1.0f, 0.0f, 0.0f));
    const __m128 d = _mm_loadu_ps(&dir->x);  // 00860E37 (orig movaps)
    const __m128 d_yzxx = _mm_shuffle_ps(d, d, 9);     // {d1,d2,d0,d0}
    const __m128 d_zxxy = _mm_shuffle_ps(d, d, 0x12);  // {d2,d0,d1,d0}
    const __m128 d3 = _mm_shuffle_ps(d, d, 0xFF);      // broadcast d3
    __m128 basis1 = MulP(_mm_shuffle_ps(pole, pole, 0x12), d_yzxx);  // 00860E5C..60
    basis1 = _mm_sub_ps(basis1, MulP(_mm_shuffle_ps(pole, pole, 9), d_zxxy));  // 00860E63
    basis1 = AddP(basis1, MulP(d3, pole));                                 // 00860E6F
    __m128 basis2 = MulP(_mm_shuffle_ps(basis1, basis1, 0x12), d_yzxx);    // 00860E8D..9F
    basis2 = _mm_sub_ps(basis2, MulP(_mm_shuffle_ps(basis1, basis1, 9), d_zxxy));  // 00860EB7
    basis2 = _mm_add_ps(_mm_mul_ps(basis2, _mm_set1_ps(2.0f)), pole);              // 00860EBF..C2

    const __m128 ex = _mm_set_ps(0.0f, 0.0f, 0.0f, 1.0f);  // {1,0,0,0} (00860E91..EAE)
    __m128 u = MulP(_mm_shuffle_ps(ex, ex, 0x12), d_yzxx);              // 00860ED8
    const __m128 u_d3 = MulP(d3, ex);                                   // 00860EDB
    u = _mm_sub_ps(u, MulP(_mm_shuffle_ps(ex, ex, 9), d_zxxy));         // 00860EE1
    u = AddP(u, u_d3);                                                  // 00860EE4
    __m128 basis3 = MulP(_mm_shuffle_ps(u, u, 0x12), d_yzxx);           // 00860EF5
    basis3 = _mm_sub_ps(basis3, MulP(_mm_shuffle_ps(u, u, 9), d_zxxy)); // 00860EFD
    basis3 = _mm_add_ps(_mm_mul_ps(basis3, _mm_set1_ps(2.0f)), ex);     // 00860F08..0B

    // Corners: P1 = point + (len/6)*basis2, P2/P3 = point -/+ (len/2)*basis2,
    // P4/P5 = P3 -/+ (wid/2)*basis3 (00860F0E..00860F92).
    const float sixth = lenScale * F32(0x3E2AAAAB);  // 00860F16 [0xFBB10C]
    const float halfLen = lenScale * 0.5f;           // 00860F29
    const float halfWid = widScale * 0.5f;           // 00860F2D
    const float negHalfWid = -0.0f - halfWid;        // 00860F5B..63 subss from -0.0
    const __m128 p = _mm_loadu_ps(&point->x);        // 00860F31 (orig movaps)
    alignas(16) Vec4 p1, p2, p4, p5;
    _mm_storeu_ps(&p1.x, AddP(MulP(_mm_set1_ps(sixth), basis2), p));  // 00860F22..37
    const __m128 t2 = MulP(_mm_set1_ps(halfLen), basis2);             // 00860F3C..44
    const __m128 p3 = AddP(t2, p);                                    // 00860F50
    _mm_storeu_ps(&p2.x, _mm_sub_ps(p, t2));                          // 00860F47..53
    _mm_storeu_ps(&p4.x, AddP(MulP(_mm_set1_ps(negHalfWid), basis3), p3));  // 00860F72..7C
    _mm_storeu_ps(&p5.x, AddP(MulP(_mm_set1_ps(halfWid), basis3), p3));     // 00860F85..92

    // Rectangle outline: (P2,P4) (P2,P5) (P1,P4) (P1,P5) (00860F97..00860FD0).
    buffer->AddSegment(&p2, &p4, color, color);  // 00860F97
    buffer->AddSegment(&p2, &p5, color, color);  // 00860FAA
    buffer->AddSegment(&p1, &p4, color, color);  // 00860FBD
    return buffer->AddSegment(&p1, &p5, color, color);  // 00860FD0
}

}  // namespace ts3::debug
