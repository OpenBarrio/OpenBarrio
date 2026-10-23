#pragma once

#include <cstddef>
#include <cstdint>

#include "ts3/layout.h"

namespace ts3::speedtree {

// Game records are seven floats: position xyz (+0), tangent xyz (+0xc), time/weight (+0x18).
// `end` points one past the last record, so end[-1] is its weight and end-7 its first field.
// Used by TS3W.exe 0x00921F20 and 0x009228A0.
struct SpeedTreeRecordRange {
    const float* begin;
    const float* end;
};
TS3_X86_OFFSET(SpeedTreeRecordRange, end, 4);
TS3_X86_SIZE(SpeedTreeRecordRange, 8);

// Oscillator state driven by 0x00921D20 / 0x00921990: value at +0, the two mixed amounts at
// +4/+8, the two phases at +0xc/+0x10 and the phase-table cursor at +0x14.
struct SpeedTreeWheel {
    float value;
    float amountA;
    float amountB;
    float phaseA;
    float phaseB;
    uint32_t phaseIndex;
};
TS3_X86_OFFSET(SpeedTreeWheel, amountA, 4);
TS3_X86_OFFSET(SpeedTreeWheel, amountB, 8);
TS3_X86_OFFSET(SpeedTreeWheel, phaseA, 0xC);
TS3_X86_OFFSET(SpeedTreeWheel, phaseB, 0x10);
TS3_X86_OFFSET(SpeedTreeWheel, phaseIndex, 0x14);
TS3_X86_SIZE(SpeedTreeWheel, 0x18);

// Parameters 0x00921D20 / 0x00921990 read: two random ranges, the jitter/step pair, the mix
// blend, the phase table and its wrap mode.
struct SpeedTreeWheelParams {
    float range0[2];       // +0x00
    float range1[2];       // +0x08
    float jitter;          // +0x10
    float step;            // +0x14
    float mix;             // +0x18
    const float* phases;   // +0x1C
    uint32_t phaseCount;   // +0x20
    uint8_t wrapMode;      // +0x24
    uint8_t pad[3];
};
TS3_X86_OFFSET(SpeedTreeWheelParams, range1, 8);
TS3_X86_OFFSET(SpeedTreeWheelParams, jitter, 0x10);
TS3_X86_OFFSET(SpeedTreeWheelParams, step, 0x14);
TS3_X86_OFFSET(SpeedTreeWheelParams, mix, 0x18);
TS3_X86_OFFSET(SpeedTreeWheelParams, phases, 0x1C);
TS3_X86_OFFSET(SpeedTreeWheelParams, phaseCount, 0x20);
TS3_X86_OFFSET(SpeedTreeWheelParams, wrapMode, 0x24);
TS3_X86_SIZE(SpeedTreeWheelParams, 0x28);

// Transform 0x009228A0 applies to the generated point (translate +0x4, scale +0x10 and a
// column-major 3x3 at +0x14). Bit 1 of the byte at +0 selects the rotation.
struct SpeedTreeCurveTransform {
    uint32_t flags;        // +0x00
    float translate[3];    // +0x04
    float scale;           // +0x10
    float matrix[9];       // +0x14, column-major
};
TS3_X86_OFFSET(SpeedTreeCurveTransform, translate, 4);
TS3_X86_OFFSET(SpeedTreeCurveTransform, scale, 0x10);
TS3_X86_OFFSET(SpeedTreeCurveTransform, matrix, 0x14);
TS3_X86_SIZE(SpeedTreeCurveTransform, 0x38);

// TS3W.exe 0x008D9750, __cdecl: clamp to [range[0], range[1]] a random double in [0,1)
// interpolated between them. Reversed/equal ranges collapse as the game's compares do.
double RandomInInterval(const float* range);

// TS3W.exe 0x008D97A0, __cdecl: fills out[0..2] from the (r[0],r[3]), (r[1],r[4]),
// (r[2],r[5]) intervals; returns out.
float* RandomizeIntervals(float* out, const float* ranges);

// TS3W.exe 0x0092C0F0, __cdecl: rejection-samples a point on the torus
// (0.5-sqrt(x^2+y^2))^2*z^2 <= 0.25 in the normalized x/y plane, then maps it into the
// box min/max with `blend` mixing the flat direction and the full 3D vector. Returns out.
float* RandomizeExtent(float* out, const float* box, float blend);

// TS3W.exe 0x00921F20, __cdecl; records are seven floats (28 bytes).
uint8_t CopyNextRecord(float* output, const SpeedTreeRecordRange* records);

// TS3W.exe 0x00921990, __thiscall ret 4: advances the wheel's value/amounts/phases and the
// phase-table cursor. The 0x008CA160 result both drives the phase step and selects its sign.
void StepWheel(SpeedTreeWheel* self, const SpeedTreeWheelParams* params);

// TS3W.exe 0x00921D20, __thiscall ret 0xc: resets the wheel, then either seeds it from
// params (nonzero low byte of `flag`) or runs one StepWheel pass.
void InitWheel(SpeedTreeWheel* self, const SpeedTreeWheelParams* params, void* context, float flag);

// TS3W.exe 0x009228A0, __cdecl: advances out[0] by `delta`, finds the record spanning it,
// writes a Hermite position/tangent into out[3..8] and applies the transform; false when the
// delta leaves the range or the record's weight was already passed.
char InterpolateCurve(float* out, float delta, const SpeedTreeRecordRange* range,
                      const float* prevPosition, const float* prevTangent,
                      const SpeedTreeCurveTransform* transform);

// Prefix view only; the owning game object continues past this property byte.
struct SpeedTreeFlagObject {
    std::byte opaque[0x27C];
    uint8_t flag;

    // TS3W.exe 0x0092C610, __thiscall, ret 0.
    uint8_t GetFlag() const { return flag; }
};
TS3_X86_OFFSET(SpeedTreeFlagObject, flag, 0x27C);

}  // namespace ts3::speedtree

// ABI-compatible hook entry points consumed by the x86 harness. The cdecl entry points above
// are already ABI-exact; the two __thiscall ones need a __fastcall shim (callee pops the stack
// arguments, exactly like the original `ret N`).
void __fastcall SpeedTreeStepWheelHook(ts3::speedtree::SpeedTreeWheel* self, void* edx,
                                       const ts3::speedtree::SpeedTreeWheelParams* params);
void __fastcall SpeedTreeInitWheelHook(ts3::speedtree::SpeedTreeWheel* self, void* edx,
                                       const ts3::speedtree::SpeedTreeWheelParams* params,
                                       void* context, float flag);
uint8_t __cdecl SpeedTreeCopyNextRecord(float* output, const ts3::speedtree::SpeedTreeRecordRange* records);
uint8_t __fastcall SpeedTreeGetFlag(ts3::speedtree::SpeedTreeFlagObject* self, void* edx);