// Verified rewrite of unit clu_56dbe0_SpeedTree. The originals are in TS3W.exe 1.67.2:
//
//   0x0056DBE0  void* __thiscall VectorResize(FloatVector*, uint32 count) [ret 4]
//   0x0068A650  float* __thiscall SmoothSamples(SpeedTreeFilterObject*, uint32 window) [ret 4]
//   0x0068AA10  float* __thiscall ScaleAndFilterSamples(SpeedTreeFilterObject*, uint32 window) [ret 4]
//
// VectorResize is the game's generic 4-byte-element vector resize (27 callers over
// vectors of many types); it never interprets the elements: shrinking only moves the
// end pointer (its memmove call always has count 0 - the original computes the count as
// `end - end`, see the project notes) and growing appends 0.0f through the game's own
// 0x0056D7E0, which is not part of the unit and is bridged. 0x0068A650 is private (its
// only caller is 0x0068AA10's tail jump) and copies or window-averages the samples into
// the output vector; 0x0068AA10 first scales the samples in place by
// (double)global0x1158884 / (double)divisor, both read as unsigned 32-bit.
#include "ts3/speedtree/clu_56dbe0_SpeedTree.h"
#include <cstring>
#include <xmmintrin.h>

namespace ts3::speedtree {
namespace {

// The two globals the sample functions read (absolute addresses in the game image).
constexpr uint32_t kFilterGate = 0x1158880;  // byte: nonzero enables the window average
constexpr uint32_t kScaleNumerator = 0x1158884;  // uint32: the scale's numerator

// The game image is only mapped in the 32-bit builds (exehost / the harness); x64 builds
// see the closed gate and never reach game memory.
uint8_t GameGlobal8(uint32_t address)
{
#if defined(_M_IX86)
    return *reinterpret_cast<const uint8_t*>(static_cast<uintptr_t>(address));
#else
    (void)address;
    return 0;
#endif
}

uint32_t GameGlobal32(uint32_t address)
{
#if defined(_M_IX86)
    return *reinterpret_cast<const uint32_t*>(static_cast<uintptr_t>(address));
#else
    (void)address;
    return 0;
#endif
}

// FLD/FSTP quiets signaling NaNs, unlike an SSE bit-copy.
float X87Copy(float value)
{
#if defined(_M_IX86)
    float result;
    __asm { fld value }
    __asm { fstp result }
    return result;
#else
    uint32_t bits;
    std::memcpy(&bits, &value, 4);
    if ((bits & 0x7f800000u) == 0x7f800000u && (bits & 0x007fffffu))
        bits |= 0x00400000u;
    std::memcpy(&value, &bits, 4);
    return value;
#endif
}

// Explicit ADDSS destination order retains the new sample's NaN payload.
float AddSample(float sample, float sum)
{
#if defined(_M_IX86)
    float result;
    __asm { movss xmm0, sample }
    __asm { addss xmm0, sum }
    __asm { movss result, xmm0 }
    return result;
#else
    return _mm_cvtss_f32(_mm_add_ss(_mm_set_ss(sample), _mm_set_ss(sum)));
#endif
}

float ScaleRatio(uint32_t numerator, uint32_t divisor)
{
#if defined(_M_IX86)
    // Unsigned inputs are exactly representable in double. Keep the division
    // on x87 so the caller's precision control applies before the float store.
    const double n = numerator, d = divisor;
    float result;
    __asm { fld n }
    __asm { fdiv d }
    __asm { fstp result }
    return result;
#else
    return static_cast<float>(static_cast<long double>(numerator) / divisor);
#endif
}

// One windowed sample: the original clamps the running index to [0, sampleCount - 1]
// (signed compares) before every load.
float WindowSample(const float* samples, int32_t index, int32_t sampleCount)
{
    if (index < 0)
        index = 0;
    if (index >= sampleCount)
        index = sampleCount - 1;
    return samples[index];
}

}  // namespace

void* VectorResize(FloatVector* self, uint32_t count)
{
    float* const begin = self->begin;
    float* const end = self->end;
    const int32_t size = static_cast<int32_t>(end - begin);  // (end - begin) >> 2 elements
    if (count <= static_cast<uint32_t>(size)) {
        // Shrink (or same size): only the end pointer moves; the original's memmove of
        // `end - end` bytes always moves nothing and returns the new end.
        self->end = begin + count;
        return self->end;
    }
    // Grow: 0x0056D7E0 appends (count - size) copies of the zero float at `end`.
    uint32_t zero = 0;
    return CallGameMethod<void*, 0x0056D7E0>(self, end, count - static_cast<uint32_t>(size), &zero);
}

float* SmoothSamples(SpeedTreeFilterObject* self, uint32_t window)
{
    FloatVector* const out = &self->output;
    VectorResize(out, static_cast<uint32_t>(self->sampleCount) + 1);

    const float* const samples = self->samples;
    for (uint32_t i = 0; i < static_cast<uint32_t>(self->sampleCount); ++i) {
        float value;
        if (GameGlobal8(kFilterGate) != 0) {
            // Centered window average: sum in the original's exact order - four
            // samples per group for windows >= 4 (signed), then the remainder.
            float sum = 0.0f;
            uint32_t processed = 0;
            int32_t index = static_cast<int32_t>(i) - static_cast<int32_t>(window >> 1);
            if (static_cast<int32_t>(window) >= 4) {
                const uint32_t groups = ((window - 4) >> 2) + 1;
                for (uint32_t g = 0; g < groups; ++g) {
                    sum = AddSample(WindowSample(samples, index, self->sampleCount), sum);
                    sum = AddSample(WindowSample(samples, index + 1, self->sampleCount), sum);
                    sum = AddSample(WindowSample(samples, index + 2, self->sampleCount), sum);
                    sum = AddSample(WindowSample(samples, index + 3, self->sampleCount), sum);
                    index += 4;
                }
                processed = groups * 4;
            }
            while (processed < window) {
                sum = AddSample(WindowSample(samples, index, self->sampleCount), sum);
                ++index;
                ++processed;
            }
            value = static_cast<float>(static_cast<double>(sum) / static_cast<double>(window));
        } else {
            value = X87Copy(self->samples[i]);  // the original reloads the pointer every pass
        }
        out->begin[i] = value;  // likewise reloaded before every store
    }

    // EA bug kept: with sampleCount == 0 this reads one float before the buffer.
    const int32_t count = self->sampleCount;
    out->begin[count] = X87Copy(out->begin[count - 1]);
    return out->begin + count;
}

float* ScaleAndFilterSamples(SpeedTreeFilterObject* self, uint32_t window)
{
    const int32_t count = static_cast<int32_t>(self->samplesEnd - self->samples);
    const float scale =
        ScaleRatio(GameGlobal32(kScaleNumerator), self->divisor);
    uint32_t i = 0;
    if (count >= 4) {
        const uint32_t limit = static_cast<uint32_t>(count - 3);  // 4-wide groups first
        for (; i < limit; i += 4) {
            float* const samples = self->samples;
            samples[i] = samples[i] * scale;
            samples[i + 1] = samples[i + 1] * scale;
            samples[i + 2] = samples[i + 2] * scale;
            samples[i + 3] = samples[i + 3] * scale;
        }
    }
    const uint32_t end = static_cast<uint32_t>(count);  // tail, unsigned compare
    for (; i < end; ++i) {
        float* const samples = self->samples;
        samples[i] = samples[i] * scale;
    }
    return SmoothSamples(self, window);  // the original tail-jumps into 0x0068A650
}

}  // namespace ts3::speedtree

void* __fastcall SpeedTreeVectorResize(ts3::speedtree::FloatVector* self, void*, uint32_t count)
{
    return ts3::speedtree::VectorResize(self, count);
}

float* __fastcall SpeedTreeFilterSamples(ts3::speedtree::SpeedTreeFilterObject* self, void*, uint32_t window)
{
    return ts3::speedtree::ScaleAndFilterSamples(self, window);
}
