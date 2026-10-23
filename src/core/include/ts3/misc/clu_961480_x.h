#pragma once
// Cluster unit clu_961480_x (misc subsystem): TS3W.exe 0x00961480, 0x009621E0, 0x00962340,
// 0x00963540, 0x00963580, 0x009636C0, 0x009636E0, 0x00963730, 0x00963750, 0x00963920,
// 0x00963A60, 0x00963AE0, 0x00963B60, 0x00963BD0, 0x00963C10, 0x00963CA0, 0x00963CE0,
// 0x00963D20, 0x00964290, 0x00964330, 0x00967370, 0x00968D20, 0x00969030.
//
// A 4-chain spring/dynamics controller. The object holds four 256-byte chain blocks
// (per-chain parameters, force terms and accumulators), a 224-byte mirror of per-chain
// 7-float old/new tracks at +0x400, a flag byte plus four vectors (current/target/delta)
// at +0x4E0..+0x52F, a pivot vector at +0x600 and a few scattered bytes up to +0x6DF.
// Two function groups: block helpers whose `this` is any 64-byte sub-block of a chain
// (0x00963540/0x009636C0/0x009636E0/0x00963730/0x00963750, plus the chain-base 0x00963920),
// and controller methods whose `this` is the whole state (everything else). The unit is
// self-contained: its only out-of-unit references are the constants at 0xFAA210/0x107A538
// (-1.0f / 1.0f) and the live float global at 0x117DDDC (8.0f in this build, read through
// the game's memory on x86). EA bugs that affect outputs are kept and marked "EA:".
#include <cstdint>

#include "ts3/layout.h"

namespace ts3::misc {

// A 4-lane float vector, accessed lane by lane (the originals use SSE but every lane is
// an independent float op, so plain scalar code reproduces the results bit for bit).
struct ChainVec4 {
    float v[4];
};

// One 256-byte chain block. Field offsets are TS3W.exe offsets within the block; the
// gaps exist in the original object and are left untouched by this unit.
struct MotionChain {
    float param[14];      // +0x00 (reset fills these with 1.0f; +0x20/+0x30 also receive
                          // SetLimits' two owords, +0x00..+0x0F SetSwingScale's vector)
    uint32_t gap38[2];    // +0x38
    ChainVec4 forceA;     // +0x40
    ChainVec4 forceB;     // +0x50
    ChainVec4 vec60;      // +0x60 (only .x is a scalar to the arithmetic: +0x60/+0x64..)
    ChainVec4 term7;      // +0x70
    ChainVec4 term8;      // +0x80 (.x/.y/.z are RemapT's base/offset/scale)
    ChainVec4 vec90;      // +0x90
    ChainVec4 term10;     // +0xA0
    ChainVec4 term11;     // +0xB0
    ChainVec4 vecC0;      // +0xC0
    ChainVec4 term13;     // +0xD0
    ChainVec4 term14;     // +0xE0
    ChainVec4 vecF0;      // +0xF0
};
TS3_X86_SIZE(MotionChain, 0x100);
TS3_X86_OFFSET(MotionChain, forceA, 0x40);
TS3_X86_OFFSET(MotionChain, term7, 0x70);
TS3_X86_OFFSET(MotionChain, term8, 0x80);
TS3_X86_OFFSET(MotionChain, vecF0, 0xF0);

// The 36-byte accumulator the update path zeroes and the chains sum into (a2/a3 of
// 0x00963920/0x00964330). 16-byte aligned in the game (the originals use movaps on it).
struct MotionAccum {
    ChainVec4 v0;   // +0x00
    ChainVec4 v1;   // +0x10
    float x8;       // +0x20
};
TS3_X86_SIZE(MotionAccum, 36);
TS3_X86_OFFSET(MotionAccum, x8, 0x20);

// The controller object as far as this unit reaches (first 0x6E0 bytes of the game's
// object; the game object continues past it, owned by other units).
struct ChainMotionState {
    MotionChain chains[4];          // +0x000
    float mirrorOld[4][7];          // +0x400 (0x00963A60 copies the source here, then
                                    // re-distributes it into the chains' param[0..6])
    float mirrorNew[4][7];          // +0x470 (0x00963B60 lerps old..new into param[0..6])
    uint8_t flags;                  // +0x4E0 (bits 2/4/8 set by 0x00963CA0/E0/D20)
    uint8_t gap4e1[0x4F0 - 0x4E1];  // +0x4E1
    ChainVec4 cur1;                 // +0x4F0
    ChainVec4 target;               // +0x500 (written by SetTargetXY, snapped by the bits)
    ChainVec4 cur2;                 // +0x510
    ChainVec4 delta;                // +0x520 (CapDelta caps it, the update drains it)
    uint8_t gap530[0x600 - 0x530];  // +0x530
    ChainVec4 pivot;                // +0x600 (StepToward's stored point)
    uint8_t gap610[0x6DC - 0x610];  // +0x610
    uint8_t limitsFlag;             // +0x6DC (SetLimits writes 1)
    uint8_t gap6dd[2];              // +0x6DD
    uint8_t activeByte;             // +0x6DF
};
TS3_X86_SIZE(ChainMotionState, 0x6E0);
TS3_X86_OFFSET(ChainMotionState, mirrorOld, 0x400);
TS3_X86_OFFSET(ChainMotionState, flags, 0x4E0);
TS3_X86_OFFSET(ChainMotionState, cur1, 0x4F0);
TS3_X86_OFFSET(ChainMotionState, target, 0x500);
TS3_X86_OFFSET(ChainMotionState, cur2, 0x510);
TS3_X86_OFFSET(ChainMotionState, delta, 0x520);
TS3_X86_OFFSET(ChainMotionState, pivot, 0x600);
TS3_X86_OFFSET(ChainMotionState, limitsFlag, 0x6DC);
TS3_X86_OFFSET(ChainMotionState, activeByte, 0x6DF);

// ---- block helpers (`this` is an untyped sub-block base, like the originals) -----------

// 0x00963540 (__thiscall, ret 8): store -1.0f / 1.0f (the constants at 0xFAA210 /
// 0x107A538) into the float at base + 0x1C + 4*idx. EA: EAX holds idx only on the
// flag=true path and is inherited garbage otherwise; here it returns idx either way
// (no caller reads the false-path value).
int ChainSetWeight(void* base, int idx, bool flag);

// 0x009636C0 (__thiscall, ret 4): (v - base[0x80]) * base[0x88] + base[0x84], computed
// in the x87's 80-bit precision like the original's fld/fsub/fmul/fadd, rounded to float
// once at the return.
float ChainRemap(const void* base, float v);

// 0x009636E0 (__thiscall): fill base[0x00..0xDF] with 1.0f (56 floats). Returns base.
void* ChainFillOnes(void* base);

// 0x00963730 (__thiscall, ret 0xC): block[7*row + col] = value (float bits); returns row.
int ChainSetElem(void* base, int row, int col, float value);

// 0x00963750 (__thiscall, ret 0xC): block[7*(row+4) + col] = value; returns row + 4.
int ChainSetElemHigh(void* base, int row, int col, float value);

// 0x00963920 (__thiscall, ret 8): sum one chain's spring terms into the two accumulators
// (op order lane by lane: add, mul, add; NaN on a gate sum selects param[3], matching the
// original's comiss/jbe). Returns a3.
MotionAccum* ChainAccumulate(const MotionChain& chain, MotionAccum* a2, MotionAccum* a3);

// ---- controller methods (this = ChainMotionState) --------------------------------------

// 0x00961480 (__thiscall, ret 4): store the byte at +0x6DF, return it in AL.
uint8_t SetByte6DF(ChainMotionState& state, uint8_t value);

// 0x00969030 (__thiscall, ret 4): store the byte at +0x1A1, return it in AL.
uint8_t SetByte1A1(ChainMotionState& state, uint8_t value);

// 0x009621E0 (__thiscall, ret 4): chains[2].param[0..3] = -forceB * value + vec60 and
// chains[2].term7.x = value.
void SetSwingScale(ChainMotionState& state, float value);

// 0x00962340 (__thiscall, ret 8): limitsFlag = 1; copy 16 bytes from each argument into
// chains[2].param[8..11] and [12..15] (movaps in the original: the pointers are aligned).
void SetLimits(ChainMotionState& state, const void* a, const void* b);

// 0x00963580 (__thiscall): return the 224-byte mirror base (this + 0x400).
void* GetParams(ChainMotionState& state);

// 0x00963A60 (__thiscall, ret 4): copy 0xE0 bytes from src over the mirror at +0x400
// (raw integer copy, NaN payloads preserved), then re-distribute mirrorOld[k] into
// chains[k].param[0..6] through float loads/stores (x87: signaling NaN payloads get
// quieted, kept here). Returns this + 0x408.
void* SetTracks(ChainMotionState& state, const void* src);

// 0x00963AE0 (__thiscall): zero every chain's force/term vectors and the tail state
// (cur1/target/cur2/delta and the flag byte). Returns this + 0x490.
void* ClearForces(ChainMotionState& state);

// 0x00963B60 (__thiscall, ret 4): chains[k].param[0..6] = old + (new - old) * t over the
// mirror rows. Returns this + 0x4E0.
void* LerpChains(ChainMotionState& state, float t);

// 0x00963BD0 (__thiscall, ret 8): target = {a, b, a, a}.
void SetTargetXY(ChainMotionState& state, float a, float b);

// 0x00963C10 (__thiscall, ret 8): v = {a, b, a, a}; keep it in delta when
// sqrt(v.y^2 + v.x^2) > sqrt(delta.y^2 + delta.x^2) (strict; NaN never replaces).
void CapDelta(ChainMotionState& state, float a, float b);

// 0x00963CA0 / 0x00963CE0 / 0x00963D20 (__thiscall, ret 4): snap cur2 and cur1 to
// target, then set/clear flag bit 8 / 2 / 4.
void SnapTargetsBit8(ChainMotionState& state, bool on);
void SnapTargetsBit2(ChainMotionState& state, bool on);
void SnapTargetsBit4(ChainMotionState& state, bool on);

// 0x00964290 (__thiscall): reset: per chain param[0..13] = 1.0f and the force/term zero
// pass, ChainFillOnes over the mirror, then ClearForces. Returns this.
void* ResetChains(ChainMotionState& state);

// 0x00964330 (__thiscall, ret 8): the per-tick update: drain the +0x1F0/-0x1C0 difference
// into an impulse (gate: chains[1].param[2] vs param[3]), advance cur1/cur2 toward the
// target by delta (keeping only the part of A beyond the live 8.0f global at 0x117DDDC),
// rebuild chains[1]'s gated term vectors from the flag bits, accumulate all four chains
// into the two accumulators, add the impulse to a3->x8 and clear delta. Returns a3.
MotionAccum* UpdateDynamics(ChainMotionState& state, MotionAccum* a2, MotionAccum* a3);

// 0x00967370 (__thiscall, ret 8): ChainSetWeight(chains[k] + 0x40, 5, a2) and
// (..., 4, a3) for all four chains. EA: returns 4 only when a3 is set (the last helper's
// EAX is inherited garbage otherwise).
int SetChainWeights(ChainMotionState& state, bool a2, bool a3);

// 0x00968D20 (__thiscall, ret 0xC): *out = pos + step, where step = d2y/rdy scaled along
// the twice-Newton-Raphson-refined rsqrt direction of pos - target (rsqrtps kept via the
// intrinsic, so the 12-bit seed matches the CPU), clamped: |step.x| > 10 pulls x to
// sign(step.x)*10 and |step.z| > 10 pulls z to sign(step.z)*10 (each clamp touches its
// lane only). Returns out.
ChainVec4* StepToward(ChainMotionState& state, ChainVec4* out, const ChainVec4* pos,
                      const ChainVec4* tgt);

}  // namespace ts3::misc
