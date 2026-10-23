// Cluster unit clu_961480_x (misc): a 4-chain spring/dynamics controller.
// Originals: TS3W.exe 0x00961480, 0x009621E0, 0x00962340, 0x00963540, 0x00963580,
// 0x009636C0, 0x009636E0, 0x00963730, 0x00963750, 0x00963920, 0x00963A60, 0x00963AE0,
// 0x00963B60, 0x00963BD0, 0x00963C10, 0x00963CA0, 0x00963CE0, 0x00963D20, 0x00964290,
// 0x00964330, 0x00967370, 0x00968D20, 0x00969030.
//
// Every float operation below follows the original's instruction order lane by lane
// (one rounding per op, no FMA contraction at the project's /fp:precise default). The
// SSE adds and multiplies run through AddP/MulP: with two NaN operands the instruction's
// FIRST source decides the payload that survives, and the compiler is free to swap
// commutative C++ operands - so the order is pinned in software instead. Three originals
// need more spelled out: 0x009636C0 computes in the x87's 80-bit precision (long double
// on x86 MSVC), 0x00968D20's rsqrtps seed is reproduced with the intrinsic so the 12-bit
// approximation comes from the same CPU table, and 0x00963A60's copy loop quiets
// signaling NaNs like the x87 load/store pair it is. The flag stores of
// 0x00963CA0/E0/D20 run before the flag update in the originals too - no observable
// order either way, but the code keeps the original sequence.
#include "ts3/misc/clu_961480_x.h"

#include <cmath>
#include <cstring>

#if defined(_M_IX86) || defined(_M_X64)
#include <xmmintrin.h>
#endif

namespace ts3::detail { float ChainRemapExtended(float, float, float, float); }

namespace ts3::misc {
namespace {

// The 56 floats of ChainFillOnes / the 14 param floats of the reset: the original loads
// its 1.0f from 0x107A538 once into xmm and stores it 28 times.
constexpr float kOne = 1.0f;

// a as the first source of an SSE add: with NaN in both operands the first source's
// payload wins (quieted), so a commutative swap by the compiler would show.
float AddP(float a, float b)
{
    if (std::isnan(a) && std::isnan(b)) {
        uint32_t t;
        std::memcpy(&t, &a, sizeof t);
        if ((t & 0x7F800000u) == 0x7F800000u && (t & 0x007FFFFFu) != 0 &&
            (t & 0x00400000u) == 0)
            t |= 0x00400000u;  // signaling NaN: the unit quiets it
        std::memcpy(&a, &t, sizeof t);
        return a;
    }
    return a + b;  // at most one NaN: both encodings give the same bits
}

// The same for mul (0 * inf style invalid results are order-independent; only the
// two-NaN payload choice is not).
float MulP(float a, float b)
{
    if (std::isnan(a) && std::isnan(b)) {
        uint32_t t;
        std::memcpy(&t, &a, sizeof t);
        if ((t & 0x7F800000u) == 0x7F800000u && (t & 0x007FFFFFu) != 0 &&
            (t & 0x00400000u) == 0)
            t |= 0x00400000u;
        std::memcpy(&a, &t, sizeof t);
        return a;
    }
    return a * b;
}

// 0x117DDDC: live float global read by the update (8.0f in TS3W.exe 1.67.2). Other code
// may write it, so the x86 build reads it through the game's memory like the original.
float LiveLengthLimit()
{
#if defined(_M_IX86)
    return *reinterpret_cast<const float*>(0x117DDDC);
#else
    return 8.0f;
#endif
}

// The x87 load/store pair of 0x00963A60's copy loop: a float rounds through the 80-bit
// format unchanged except that a signaling NaN comes back quieted (the x87 sets the
// quiet bit of the mantissa on load).
float X87FloatCopy(float value)
{
    uint32_t bits;
    std::memcpy(&bits, &value, sizeof bits);
    if ((bits & 0x7F800000u) == 0x7F800000u && (bits & 0x007FFFFFu) != 0 &&
        (bits & 0x00400000u) == 0)
        bits |= 0x00400000u;
    std::memcpy(&value, &bits, sizeof value);
    return value;
}

// Zero the force/term vectors of one chain the way 0x00963AE0 does: the eight full
// movaps stores (forceA/forceB, term7/term8, term10/term11, term13/term14) and four
// movss stores that clear only lane 0 of vec60/vec90/vecC0/vecF0 - the .y/.z/.w bytes of
// those four and the gaps at 0x64..0x6F, 0x94..0x9F, 0xC4..0xCF, 0xF4..0xFF stay
// untouched.
void ZeroChainVectors(MotionChain& chain)
{
    const ChainVec4 zero{{0.0f, 0.0f, 0.0f, 0.0f}};
    chain.forceA = zero;
    chain.forceB = zero;
    chain.vec60 = ChainVec4{{0.0f, chain.vec60.v[1], chain.vec60.v[2], chain.vec60.v[3]}};
    chain.term7 = zero;
    chain.term8 = zero;
    chain.vec90 = ChainVec4{{0.0f, chain.vec90.v[1], chain.vec90.v[2], chain.vec90.v[3]}};
    chain.term10 = zero;
    chain.term11 = zero;
    chain.vecC0 = ChainVec4{{0.0f, chain.vecC0.v[1], chain.vecC0.v[2], chain.vecC0.v[3]}};
    chain.term13 = zero;
    chain.term14 = zero;
    chain.vecF0 = ChainVec4{{0.0f, chain.vecF0.v[1], chain.vecF0.v[2], chain.vecF0.v[3]}};
}

}  // namespace

// ---- block helpers ---------------------------------------------------------------------

// 0x00963540
// EA: EAX = idx is set only on the flag=true path (the false path re-reads the argument
// into EDX and leaves EAX inherited); no caller reads it there, so this returns idx.
int ChainSetWeight(void* base, int idx, bool flag)
{
    const float value = flag ? -1.0f : 1.0f;
    std::memcpy(&reinterpret_cast<uint8_t*>(base)[0x1C + 4u * static_cast<unsigned>(idx)],
                &value, sizeof value);
    return idx;
}

// 0x009636C0
// at the x87's 80-bit precision and the caller rounds the result once to float.
float ChainRemap(const void* base, float v)
{
    const auto* f = static_cast<const float*>(base);
#if defined(_M_IX86)
    float result;
    __asm {
        mov eax, f
        fld v
        fsub dword ptr [eax + 0x80]
        fmul dword ptr [eax + 0x88]
        fadd dword ptr [eax + 0x84]
        fstp result
    }
    return result;
#else
    if (std::isfinite(v) && std::isfinite(f[0x80 / 4]) &&
        std::isfinite(f[0x88 / 4]) && std::isfinite(f[0x84 / 4])) {
        return detail::ChainRemapExtended(v, f[0x80 / 4], f[0x88 / 4], f[0x84 / 4]);
    }
    long double r = static_cast<long double>(v) - static_cast<long double>(f[0x80 / 4]);
    r *= static_cast<long double>(f[0x88 / 4]);
    r += static_cast<long double>(f[0x84 / 4]);
    return static_cast<float>(r);
#endif
}

// 0x009636E0: 28 paired movss stores walking [0x00..0x6F] and [0x70..0xDF].
void* ChainFillOnes(void* base)
{
    auto* f = static_cast<float*>(base);
    for (int i = 0; i < 28; ++i) {
        f[i] = kOne;
        f[28 + i] = kOne;
    }
    return base;
}

// 0x00963730
int ChainSetElem(void* base, int row, int col, float value)
{
    const int32_t index = 7 * row + col;
    std::memcpy(&static_cast<float*>(base)[index], &value, sizeof value);
    return row;
}

// 0x00963750: the same store with the row index advanced by 4 first; EAX = row + 4.
int ChainSetElemHigh(void* base, int row, int col, float value)
{
    const int32_t index = 7 * (row + 4) + col;
    std::memcpy(&static_cast<float*>(base)[index], &value, sizeof value);
    return row + 4;
}

// 0x00963920: v3 = {s0, s1, s0, s0} from param[0]/[1]; v4 = {f11*f4, f12*f5, f6, f11*f4}.
// The gate sums (vec90.x + vec60.x, vecF0.x + vecC0.x) select param[2] above zero and
// param[3] at zero-or-NaN (comiss/jbe), then gate*sum lands in the accumulators' x8.
MotionAccum* ChainAccumulate(const MotionChain& chain, MotionAccum* a2, MotionAccum* a3)
{
    const float s0 = chain.param[0];
    const float s1 = chain.param[1];
    const float p0 = MulP(chain.param[11], chain.param[4]);
    const float p1 = MulP(chain.param[12], chain.param[5]);
    const ChainVec4 v3{{s0, s1, s0, s0}};
    const ChainVec4 v4{{p0, p1, chain.param[6], p0}};

    const float gate0 = AddP(chain.vec90.v[0], chain.vec60.v[0]);
    const float sel0 = gate0 > 0.0f ? chain.param[2] : chain.param[3];
    const float gate1 = AddP(chain.vecF0.v[0], chain.vecC0.v[0]);
    const float sel1 = gate1 > 0.0f ? chain.param[2] : chain.param[3];

    // The original's adds put the product/t on the left (addps xmm5, [eax]) - with NaN
    // operands the left source's payload wins, so the order is part of the contract.
    for (int i = 0; i < 4; ++i) {
        const float p = MulP(AddP(chain.term7.v[i], chain.forceA.v[i]), v3.v[i]);
        a2->v0.v[i] = AddP(p, a2->v0.v[i]);
    }
    for (int i = 0; i < 4; ++i) {
        const float p = MulP(AddP(chain.term8.v[i], chain.forceB.v[i]), v4.v[i]);
        a2->v1.v[i] = AddP(p, a2->v1.v[i]);
    }
    const float t0 = MulP(sel0, gate0);
    a2->x8 = AddP(t0, a2->x8);

    for (int i = 0; i < 4; ++i) {
        const float p = MulP(AddP(chain.term13.v[i], chain.term10.v[i]), v3.v[i]);
        a3->v0.v[i] = AddP(p, a3->v0.v[i]);
    }
    for (int i = 0; i < 4; ++i) {
        const float p = MulP(AddP(chain.term14.v[i], chain.term11.v[i]), v4.v[i]);
        a3->v1.v[i] = AddP(p, a3->v1.v[i]);
    }
    const float t1 = MulP(sel1, gate1);
    a3->x8 = AddP(t1, a3->x8);
    return a3;
}

// ---- controller methods -----------------------------------------------------------------

uint8_t SetByte6DF(ChainMotionState& state, uint8_t value)
{
    state.activeByte = value;
    return value;
}

uint8_t SetByte1A1(ChainMotionState& state, uint8_t value)
{
    reinterpret_cast<uint8_t*>(&state)[0x1A1] = value;  // inside chains[1].term10.x
    return value;
}

// 0x009621E0: [0x270] = value; [0x200] = (0 - [+0x50]) * value + [+0x60] (subps from a
// zeroed register, mulps by the broadcast value, addps). The reads are absolute +0x50 /
// +0x60 (chains[0]'s vectors), the writes land in chains[2] - that mix is the original's.
void SetSwingScale(ChainMotionState& state, float value)
{
    const ChainVec4& forceB = state.chains[0].forceB;  // +0x50
    const ChainVec4& vec60 = state.chains[0].vec60;    // +0x60
    ChainVec4 dst;
    for (int i = 0; i < 4; ++i)
        dst.v[i] = AddP(MulP(0.0f - forceB.v[i], value), vec60.v[i]);
    state.chains[2].param[0] = dst.v[0];
    state.chains[2].param[1] = dst.v[1];
    state.chains[2].param[2] = dst.v[2];
    state.chains[2].param[3] = dst.v[3];
    state.chains[2].term7.v[0] = value;  // +0x270
}

// 0x00962340: the flag first, then two aligned 16-byte copies (movaps in the original:
// the game passes aligned pointers, and so does the test).
void SetLimits(ChainMotionState& state, const void* a, const void* b)
{
    state.limitsFlag = 1;
    MotionChain& chain = state.chains[2];
    std::memcpy(&chain.param[8], a, 16);
    std::memcpy(&chain.param[12], b, 16);  // spills into gap38, exactly like the original
}

void* GetParams(ChainMotionState& state)
{
    return &state.mirrorOld[0][0];  // this + 0x400
}

// 0x00963A60
// fld/fstp copies distributing mirrorOld[k] into chains[k].param[0..6].
void* SetTracks(ChainMotionState& state, const void* src)
{
    std::memcpy(&state.mirrorOld[0][0], src, 0xE0);
    for (int k = 0; k < 4; ++k)
        for (int j = 0; j < 7; ++j)
            state.chains[k].param[j] = X87FloatCopy(state.mirrorOld[k][j]);
    return reinterpret_cast<uint8_t*>(&state) + 0x408;
}

// 0x00963AE0: four zero passes + the tail (the movaps quad at 0x4F0..0x52F and the flag
// byte, which the original stores from the loop counter's DL = 0).
void* ClearForces(ChainMotionState& state)
{
    for (int k = 0; k < 4; ++k)
        ZeroChainVectors(state.chains[k]);
    state.cur1 = ChainVec4{{0.0f, 0.0f, 0.0f, 0.0f}};
    state.target = ChainVec4{{0.0f, 0.0f, 0.0f, 0.0f}};
    state.cur2 = ChainVec4{{0.0f, 0.0f, 0.0f, 0.0f}};
    state.delta = ChainVec4{{0.0f, 0.0f, 0.0f, 0.0f}};
    state.flags = 0;
    return reinterpret_cast<uint8_t*>(&state) + 0x490;
}

// 0x00963B60: per chain, (new - old) * t + old over the 7 mirror floats.
void* LerpChains(ChainMotionState& state, float t)
{
    for (int k = 0; k < 4; ++k)
        for (int j = 0; j < 7; ++j) {
            const float oldV = state.mirrorOld[k][j];
            const float newV = state.mirrorNew[k][j];
            state.chains[k].param[j] = AddP(MulP(newV - oldV, t), oldV);
        }
    return reinterpret_cast<uint8_t*>(&state) + 0x4E0;
}

// 0x00963BD0: {lo, hi, lo, lo} built from the two float arguments.
void SetTargetXY(ChainMotionState& state, float a, float b)
{
    state.target = ChainVec4{{a, b, a, a}};
}

// 0x00963C10: two 2D lengths via sqrtps of (v.y^2 + v.x^2); the strict comiss/jbe means
// NaN on either side keeps the stored vector.
void CapDelta(ChainMotionState& state, float a, float b)
{
    const ChainVec4 v{{a, b, a, a}};
    const float len = std::sqrt(AddP(MulP(v.v[1], v.v[1]), MulP(v.v[0], v.v[0])));
    const float oldLen = std::sqrt(AddP(MulP(state.delta.v[1], state.delta.v[1]),
                                        MulP(state.delta.v[0], state.delta.v[0])));
    if (len > oldLen)
        state.delta = v;
}

namespace {

// The shared body of 0x00963CA0/0x00963CE0/0x00963D20: copy target into cur2 and cur1
// (that store order), then set or clear one flag bit (8 / 2 / 4).
void SnapTargets(ChainMotionState& state, bool on, uint8_t bit)
{
    const ChainVec4 v = state.target;
    state.cur2 = v;
    state.cur1 = v;
    if (on)
        state.flags |= bit;
    else
        state.flags = static_cast<uint8_t>(state.flags & ~bit);
}

}  // namespace

void SnapTargetsBit8(ChainMotionState& state, bool on)
{
    SnapTargets(state, on, 8);
}

void SnapTargetsBit2(ChainMotionState& state, bool on)
{
    SnapTargets(state, on, 2);
}

void SnapTargetsBit4(ChainMotionState& state, bool on)
{
    SnapTargets(state, on, 4);
}

// 0x00964290: per chain, the 14 ones and the force/term zero pass; then the mirror fill
// (ecx = this + 0x400) and the full clear, exactly in that order (the clear repeats the
// per-chain zeroing of the first loop, as in the original).
void* ResetChains(ChainMotionState& state)
{
    for (int k = 0; k < 4; ++k) {
        for (int i = 0; i < 14; ++i)
            state.chains[k].param[i] = kOne;
        ZeroChainVectors(state.chains[k]);
    }
    ChainFillOnes(&state.mirrorOld[0][0]);
    ClearForces(state);
    return &state;
}

// 0x00964330: the per-tick update. The impulse gate reads chains[1]: the rate is
// param[2] when the drained difference is positive, param[3] otherwise (NaN -> param[3]).
MotionAccum* UpdateDynamics(ChainMotionState& state, MotionAccum* a2, MotionAccum* a3)
{
    MotionChain& gateChain = state.chains[1];

    const ChainVec4 zero{{0.0f, 0.0f, 0.0f, 0.0f}};
    const MotionAccum zeroAccum{zero, zero, 0.0f};
    *a2 = zeroAccum;
    *a3 = zeroAccum;

    const float diff = gateChain.vecF0.v[0] - gateChain.vecC0.v[0];
    const int selIdx = diff > 0.0f ? 2 : 3;
    const float rate = gateChain.param[selIdx];
    gateChain.vecF0.v[0] = 0.0f;
    gateChain.vecC0.v[0] = 0.0f;
    const float impulse = MulP(rate, diff);

    if (state.flags == 0) {
        state.flags = 0;
        // Four 36-byte blocks of chains[1]: term7/term8/vec90.x, forceA/forceB/vec60.x,
        // term13/term14/vecF0.x, term10/term11/vecC0.x - in the original's order.
        gateChain.term7 = zero;
        gateChain.term8 = zero;
        gateChain.vec90 = ChainVec4{{0.0f, gateChain.vec90.v[1], gateChain.vec90.v[2],
                                     gateChain.vec90.v[3]}};
        gateChain.forceA = zero;
        gateChain.forceB = zero;
        gateChain.vec60 = ChainVec4{{0.0f, gateChain.vec60.v[1], gateChain.vec60.v[2],
                                     gateChain.vec60.v[3]}};
        gateChain.term13 = zero;
        gateChain.term14 = zero;
        gateChain.vecF0 = ChainVec4{{0.0f, gateChain.vecF0.v[1], gateChain.vecF0.v[2],
                                     gateChain.vecF0.v[3]}};
        gateChain.term10 = zero;
        gateChain.term11 = zero;
        gateChain.vecC0 = ChainVec4{{0.0f, gateChain.vecC0.v[1], gateChain.vecC0.v[2],
                                     gateChain.vecC0.v[3]}};
    } else {
        // The original's addps has delta on the left (delta + cur), which decides the
        // NaN payload kept.
        for (int i = 0; i < 4; ++i) {
            state.cur1.v[i] = AddP(state.delta.v[i], state.cur1.v[i]);
            state.cur2.v[i] = AddP(state.delta.v[i], state.cur2.v[i]);
        }
        ChainVec4 a;
        ChainVec4 b;
        for (int i = 0; i < 4; ++i) {
            a.v[i] = state.target.v[i] - state.cur1.v[i];
            b.v[i] = state.target.v[i] - state.cur2.v[i];
        }
        // Both difference vectors go through the original's sign-mixing shuffles before
        // use: {x, y, z, w} -> {x, 0-y, z, w}. The negation is a subtract from a zeroed
        // register, so a NaN lane keeps its payload sign (unary minus would flip it).
        const ChainVec4 procA{{a.v[0], 0.0f - a.v[1], a.v[2], a.v[3]}};
        const ChainVec4 procB{{b.v[0], 0.0f - b.v[1], b.v[2], b.v[3]}};
        // The A excess: length = sqrt(Ay^2 + Ax^2) (the two broadcasts of the squared
        // PROCESSED lanes 1 and 0 - the negation of lane 1 flips a NaN payload's sign);
        // below the live limit it all drops, otherwise the part beyond the limit
        // survives (8/len scales the processed vector down).
        const float len = std::sqrt(AddP(MulP(procA.v[1], procA.v[1]),
                                         MulP(procA.v[0], procA.v[0])));
        state.cur2 = state.target;
        const float limit = LiveLengthLimit();
        ChainVec4 excessA;
        if (len < limit) {
            excessA = zero;
        } else {
            const float k = limit / len;
            for (int i = 0; i < 4; ++i)
                excessA.v[i] = procA.v[i] - MulP(k, procA.v[i]);
        }
        // chains[1].term14 = {t1, t0, old.z, old.w}: the second store's scratch reload
        // takes the other processed lane, so lane 0 keeps -B.y and lane 1 gets +B.x
        // (both behind flag bit 2).
        const float t1 = (state.flags & 2) ? procB.v[1] : 0.0f;
        const float t0 = (state.flags & 2) ? procB.v[0] : 0.0f;
        gateChain.term14 = ChainVec4{{t1, t0, gateChain.term14.v[2], gateChain.term14.v[3]}};
        // chains[1].term11 = {0, 0, old.z, old.w}.
        gateChain.term11 =
            ChainVec4{{0.0f, 0.0f, gateChain.term11.v[2], gateChain.term11.v[3]}};
        // chains[1].term7: the excess y lanes behind bits 4 and 8 - {e0, e1, 0, old.w}
        // with bit 8, {e0, 0, e1, old.w} without it (the original's final shuffle pulls
        // the second store's lane 2 out of the {r3, r3, a1, a1} scratch vector).
        const float e0 = (state.flags & 4) ? excessA.v[0] : 0.0f;
        const float e1 = (state.flags & 4) ? excessA.v[1] : 0.0f;
        if (state.flags & 8)
            gateChain.term7 = ChainVec4{{e0, e1, 0.0f, gateChain.term7.v[3]}};
        else
            gateChain.term7 = ChainVec4{{e0, 0.0f, e1, gateChain.term7.v[3]}};
        // chains[1].forceA = {0, 0, 0, old.w}.
        gateChain.forceA = ChainVec4{{0.0f, 0.0f, 0.0f, gateChain.forceA.v[3]}};
    }

    MotionAccum* result = a3;
    for (int k = 0; k < 4; ++k)
        result = ChainAccumulate(state.chains[k], a2, a3);
    a3->x8 = AddP(a3->x8, impulse);
    state.delta = zero;
    return result;
}

// 0x00967370: two weight stores per chain at chains[k] + 0x40 (block offsets 0x1C + 4*5
// = term7.x and 0x1C + 4*4 = vec60.w). EA: the return is the last helper's EAX, garbage
// inherited from earlier code when a3 is false; this returns 4 in that case.
int SetChainWeights(ChainMotionState& state, bool a2, bool a3)
{
    int result = 0;
    for (int k = 0; k < 4; ++k) {
        ChainSetWeight(&state.chains[k].forceA, 5, a2);
        result = ChainSetWeight(&state.chains[k].forceA, 4, a3);
    }
    return result;
}

// 0x00968D20: the twice-refined rsqrt walk toward the target, with the two 10.0f clamps
// the originals apply through shuffles: each clamp rewrites its own lane only (x, then
// z); the neighbouring lanes pass through untouched.
ChainVec4* StepToward(ChainMotionState& state, ChainVec4* out, const ChainVec4* pos,
                      const ChainVec4* tgt)
{
    ChainVec4 d;
    for (int i = 0; i < 4; ++i)
        d.v[i] = pos->v[i] - tgt->v[i];
    const float x2 = MulP(d.v[0], d.v[0]);
    const float y2 = MulP(d.v[1], d.v[1]);
    const float z2 = MulP(d.v[2], d.v[2]);
    const float len2 = AddP(AddP(z2, y2), x2);  // the original's lane-0 horizontal order
    float r;
#if defined(_M_IX86) || defined(_M_X64)
    {
        // rsqrtps: the 12-bit seed must come from the CPU's table to stay bit-exact.
        const __m128 seed = _mm_rsqrt_ps(_mm_set1_ps(len2));
        r = _mm_cvtss_f32(seed);
    }
#else
    r = 1.0f;
#endif
    const float half1 = MulP(MulP(r, r), len2);
    r = AddP(MulP(1.0f - half1, 0.5f * r), r);
    const float half2 = MulP(MulP(r, r), len2);
    r = AddP(MulP(1.0f - half2, 0.5f * r), r);

    ChainVec4 dir;
    for (int i = 0; i < 4; ++i)
        dir.v[i] = MulP(r, d.v[i]);  // the original's mulps has the refined r first

    // The pivot's y distance, projected onto the direction when the direction's y is
    // not tiny (maxps of +-rdy then comiss: NaN or |rdy| <= 1e-5 keeps d2y as is).
    float d2y = state.pivot.v[1] - pos->v[1];
    const float rdy = dir.v[1];
    const float absRdy = (0.0f - rdy) > rdy ? (0.0f - rdy) : rdy;
    if (absRdy > 1.0e-5f)
        d2y = d2y / rdy;

    ChainVec4 m;
    for (int i = 0; i < 4; ++i)
        m.v[i] = MulP(d2y, dir.v[i]);

    // Clamp 1 on m.x: only lane 0 becomes sign(m.x) * 10.0f (y/z/w pass through).
    const float absMx = (0.0f - m.v[0]) > m.v[0] ? (0.0f - m.v[0]) : m.v[0];
    if (absMx > 10.0f) {
        const float sign = m.v[0] < 0.0f ? -1.0f : (m.v[0] > 0.0f ? 1.0f : 0.0f);
        m.v[0] = MulP(sign, 10.0f);
    }
    // Clamp 2 on m.z: only lane 2 becomes sign(m.z) * 10.0f (lane 3 passes through).
    const float absMz = (0.0f - m.v[2]) > m.v[2] ? (0.0f - m.v[2]) : m.v[2];
    if (absMz > 10.0f) {
        const float sign = m.v[2] < 0.0f ? -1.0f : (m.v[2] > 0.0f ? 1.0f : 0.0f);
        m.v[2] = MulP(sign, 10.0f);
    }

    for (int i = 0; i < 4; ++i)
        out->v[i] = AddP(pos->v[i], m.v[i]);
    return out;
}

}  // namespace ts3::misc
