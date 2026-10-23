// Rewritten TS3W.exe functions from unit clu_8d9750_SpeedTree (SpeedTree helpers).
// Each function quotes its original address; the x86 ABI shims at the bottom are the
// entry points the harness patches.
#include "ts3/speedtree/clu_8d9750_SpeedTree.h"

#include <cmath>
#include <cstring>

namespace ts3::speedtree {
namespace {

// The game's random source 0x004FA870 (clu_4fa870_SpeedTree) is called with ECX pointing at
// its state object at 0x011DC060 and leaves a double in [0,1) in ST(0).
constexpr uintptr_t kRandomStateAddress =
#if defined(_M_IX86)
    0x011DC060u;
#else
    0u;
#endif

double GameRandom01()
{
    return CallGameMethod<double, 0x004FA870>(reinterpret_cast<void*>(kRandomStateAddress));
}

// 0x008D9750's body, inlined by EA at every call site: double-precision randomised
// interpolation between two float bounds, clamped by two x87 compares.
double RandomBetween(float low, float high)
{
    const double lo = low;
    const double hi = high;
    const double value = (hi - lo) * GameRandom01() + lo;
    if (value >= hi)
        return hi;
    if (lo > value)
        return lo;
    return value;
}

// 6.2831855f lives at 0x0116B630 in TS3W.exe.
constexpr float kTwoPi = 6.2831855f;

// Column-major 3x3 rotate, EA's association: one product pair first, the third added last.
void RotatePoint(float& x, float& y, float& z, const float* m)
{
    const float nx = (m[3] * y + m[6] * z) + m[0] * x;
    const float ny = (m[1] * x + m[4] * y) + m[7] * z;
    const float nz = (m[2] * x + m[5] * y) + m[8] * z;
    x = nx;
    y = ny;
    z = nz;
}

}  // namespace

// TS3W.exe 0x008D9750: random double inside [range[0], range[1]].
double RandomInInterval(const float* range)
{
    return RandomBetween(range[0], range[1]);
}

// TS3W.exe 0x008D97A0: three random components, each stored rounded to float.
float* RandomizeIntervals(float* out, const float* ranges)
{
    out[0] = static_cast<float>(RandomBetween(ranges[0], ranges[3]));
    out[1] = static_cast<float>(RandomBetween(ranges[1], ranges[4]));
    out[2] = static_cast<float>(RandomBetween(ranges[2], ranges[5]));
    return out;
}

// TS3W.exe 0x0092C0F0: rejection-sample the solid torus (sqrt(x^2+y^2)-0.5)^2 + z^2 <= 0.25,
// then map the direction into the box; `blend` morphs the flat x/y direction toward the full
// 3D vector (the z offset always scales with blend).
float* RandomizeExtent(float* out, const float* box, float blend)
{
    float x;
    float y;
    float z;
    double distance;
    for (;;) {
        x = static_cast<float>(2.0 * GameRandom01() - 1.0);
        y = static_cast<float>(2.0 * GameRandom01() - 1.0);
        z = static_cast<float>(2.0 * GameRandom01() - 1.0);
        const double sum = static_cast<double>(y) * y + static_cast<double>(x) * x;
        distance = std::sqrt(sum);
        const double reject = 0.25 - (distance - 0.5) * (distance - 0.5);
        if (!(reject < static_cast<double>(z) * z))
            break;
    }
    const float scale = static_cast<float>((1.0 - blend) / distance + blend);
    out[0] = ((box[3] - box[0]) * (x * scale) + (box[0] + box[3])) * 0.5f;
    out[1] = ((box[4] - box[1]) * (y * scale) + (box[1] + box[4])) * 0.5f;
    out[2] = ((box[5] - box[2]) * (z * blend) + (box[2] + box[5])) * 0.5f;
    return out;
}

// TS3W.exe 0x00921F20: empty range and nonzero begin-record weight return false.
uint8_t CopyNextRecord(float* output, const SpeedTreeRecordRange* records)
{
    // TS3W.exe 0x00921F20: empty range and nonzero begin-record weight return false.
    const float* record = records->begin;
    if (record == records->end || record[6] != 0.0f)
        return 0;

    // The source stride is seven floats; output's six component vectors start at +3.
    output[3] = record[0];
    output[4] = record[1];
    output[5] = record[2];
    output[6] = record[3];
    output[7] = record[4];
    output[8] = record[5];

    const float lastWeight = records->end[-1];
    if (lastWeight > 0.0f)
        output[1] = lastWeight;
    return 1;
}

// TS3W.exe 0x00921990: advance the oscillator by one random amount, jitter both phases from
// +/-jitter (0x008CA160) and walk the phase table with a +/-2pi wrap.
void StepWheel(SpeedTreeWheel* self, const SpeedTreeWheelParams* params)
{
    self->value = static_cast<float>(static_cast<double>(self->value) +
                                     RandomBetween(params->range0[0], params->range0[1]));
    const float amount = static_cast<float>(RandomBetween(params->range1[0], params->range1[1]));
    const float mix = params->mix;
    if (mix >= 0.5f) {
        self->amountB = (1.0f - mix) * amount * 2.0f;
        self->amountA = amount;
    } else {
        self->amountB = amount;
        self->amountA = mix * amount * 2.0f;
    }
    if (mix < 1.0f) {
        const double random = CallGameFunction<double, 0x008CA160>(params->jitter + 1.0e-5f);
        const float advanced = kTwoPi * static_cast<float>(random) + self->phaseA;
        self->phaseA = advanced;
        const float step = params->step * kTwoPi;
        if (random >= 0.0)
            self->phaseA = step + advanced;
        else
            self->phaseA = advanced - step;
    }
    if (mix > 0.0f) {
        const double random = CallGameFunction<double, 0x008CA160>(params->jitter + 1.0e-5f);
        const float advanced = kTwoPi * static_cast<float>(random) + self->phaseB;
        self->phaseB = advanced;
        const float step = params->step * kTwoPi;
        if (random >= 0.0)
            self->phaseB = step + advanced;
        else
            self->phaseB = advanced - step;
    }
    const uint32_t count = params->phaseCount;
    if (count != 0) {
        const uint32_t index = self->phaseIndex;
        if (index < count) {
            const float advanced = params->phases[index] * kTwoPi + self->phaseA;
            self->phaseA = advanced;
            if (advanced > kTwoPi)
                self->phaseA = advanced - kTwoPi;
            const float wrapped = self->phaseA;
            if (-0.0f - kTwoPi > wrapped)
                self->phaseA = wrapped + kTwoPi;
            const uint32_t next = index + 1;
            self->phaseIndex = next;
            if (next >= count) {
                const uint8_t mode = params->wrapMode;
                if (mode == 0) {
                    self->phaseIndex = 0;
                } else {
                    uint32_t last = count;
                    if (mode == 1)
                        --last;
                    self->phaseIndex = last;
                }
            }
        }
    }
}

// TS3W.exe 0x00921D20: hand the two phases to 0x008E47B0, clear the cursor, then seed the
// wheel when the low byte of `flag` is nonzero or run one StepWheel pass when it is zero.
void InitWheel(SpeedTreeWheel* self, const SpeedTreeWheelParams* params, void* context, float flag)
{
    CallGameFunction<void, 0x008E47B0>(context, &self->phaseA, &self->phaseB);
    self->phaseIndex = 0;
    uint8_t flagByte = 0;
    std::memcpy(&flagByte, &flag, sizeof flagByte);
    if (flagByte != 0) {
        self->value = static_cast<float>(RandomBetween(params->range0[0], params->range0[1]));
        const float amount = static_cast<float>(RandomBetween(params->range1[0], params->range1[1]));
        const float mix = params->mix;
        if (mix >= 0.5f) {
            self->amountB = (1.0f - mix) * amount * 2.0f;
            self->amountA = amount;
        } else {
            self->amountB = amount;
            self->amountA = mix * amount * 2.0f;
        }
    } else {
        self->value = 0.0f;
        StepWheel(self, params);
    }
}

// TS3W.exe 0x009228A0: advance out[0], locate the first record whose weight exceeds it and
// blend the previous/current record with a Hermite basis; tangents use the derivative basis.
// A zero record weight means "no end yet" and is replaced by out[1]; a span of at most 1e-6
// skips the write entirely (still returns true).
char InterpolateCurve(float* out, float delta, const SpeedTreeRecordRange* range,
                      const float* prevPosition, const float* prevTangent,
                      const SpeedTreeCurveTransform* transform)
{
    const float time = out[0] + delta;
    const float end = out[1];
    out[0] = time;
    if (time >= end)
        return 0;

    const float* record = range->begin;
    const float* last = range->end - 7;
    while (record < last && time >= record[6])
        record += 7;

    float weight = record[6];
    const float positionX = record[0];
    const float positionY = record[1];
    const float positionZ = record[2];
    const float tangentX = record[3];
    const float tangentY = record[4];
    const float tangentZ = record[5];
    float previousWeight = 0.0f;
    if (weight == 0.0f) {
        weight = end;
    } else if (time > weight) {
        return 0;
    }

    float basePositionX;
    float basePositionY;
    float basePositionZ;
    float baseTangentX;
    float baseTangentY;
    float baseTangentZ;
    if (record <= range->begin) {
        basePositionX = prevPosition[0];
        basePositionY = prevPosition[1];
        basePositionZ = prevPosition[2];
        baseTangentX = prevTangent[0];
        baseTangentY = prevTangent[1];
        baseTangentZ = prevTangent[2];
    } else {
        const float* previous = record - 7;
        previousWeight = previous[6];
        basePositionX = previous[0];
        basePositionY = previous[1];
        basePositionZ = previous[2];
        baseTangentX = previous[3];
        baseTangentY = previous[4];
        baseTangentZ = previous[5];
    }

    const float span = weight - previousWeight;
    if (span > 1.0e-6f) {
        const float u = (time - previousWeight) * (1.0f / span);
        const float u2 = u * u;
        const float u3 = u2 * u;
        const float cubic = u3 - u2;
        const float h01 = u2 - cubic * 2.0f;
        const float h10 = (cubic - u2 + u) * span;
        const float h11 = cubic * span;
        const float slope = u2 * 3.0f - u * 2.0f;
        const float slope2 = u * 2.0f - slope * 2.0f;
        const float tangentScale = (1.0f - h01);
        const float derivative = (1.0f / span) * (-0.0f - slope2);
        const float derivative2 = (1.0f / span) * slope2;
        const float tangentBase = slope - u * 2.0f + 1.0f;
        out[3] = ((tangentScale * basePositionX + h01 * positionX) + h10 * baseTangentX) +
                 h11 * tangentX;
        out[4] = ((tangentScale * basePositionY + h01 * positionY) + h10 * baseTangentY) +
                 h11 * tangentY;
        out[5] = ((tangentScale * basePositionZ + h01 * positionZ) + h10 * baseTangentZ) +
                 h11 * tangentZ;
        out[6] = ((derivative * basePositionX + derivative2 * positionX) +
                  tangentBase * baseTangentX) +
                 slope * tangentX;
        out[7] = ((derivative * basePositionY + derivative2 * positionY) +
                  tangentBase * baseTangentY) +
                 slope * tangentY;
        out[8] = ((derivative * basePositionZ + derivative2 * positionZ) +
                  tangentBase * baseTangentZ) +
                 slope * tangentZ;

        const bool rotate = (transform->flags & 2) != 0;
        if (rotate)
            RotatePoint(out[3], out[4], out[5], transform->matrix);
        const float scale = transform->scale;
        out[3] = out[3] * scale + transform->translate[0];
        out[4] = out[4] * scale + transform->translate[1];
        out[5] = out[5] * scale + transform->translate[2];
        if (rotate)
            RotatePoint(out[6], out[7], out[8], transform->matrix);
        out[6] = out[6] * scale;
        out[7] = out[7] * scale;
        out[8] = out[8] * scale;
    }
    return 1;
}

}  // namespace ts3::speedtree

uint8_t __cdecl SpeedTreeCopyNextRecord(float* output, const ts3::speedtree::SpeedTreeRecordRange* records)
{
    return ts3::speedtree::CopyNextRecord(output, records);
}

uint8_t __fastcall SpeedTreeGetFlag(ts3::speedtree::SpeedTreeFlagObject* self, void*)
{
    return self->GetFlag();
}

void __fastcall SpeedTreeStepWheelHook(ts3::speedtree::SpeedTreeWheel* self, void* edx,
                                       const ts3::speedtree::SpeedTreeWheelParams* params)
{
    (void)edx;
    ts3::speedtree::StepWheel(self, params);
}

void __fastcall SpeedTreeInitWheelHook(ts3::speedtree::SpeedTreeWheel* self, void* edx,
                                       const ts3::speedtree::SpeedTreeWheelParams* params,
                                       void* context, float flag)
{
    (void)edx;
    ts3::speedtree::InitWheel(self, params, context, flag);
}