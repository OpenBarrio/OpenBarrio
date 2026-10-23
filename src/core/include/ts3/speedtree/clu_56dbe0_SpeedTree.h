// Unit clu_56dbe0_SpeedTree: the game's generic float-vector resize 0x0056DBE0 and the
// SpeedTree sample filter pair 0x0068A650 (private, only reached through 0x0068AA10's
// tail jump). All three are __thiscall with one stack argument (ret 4).
#pragma once

#include <cstdint>

#include "ts3/layout.h"

namespace ts3::speedtree {

// The game's EASTL float vector as laid out by TS3W.exe (x86): begin, end, capacity,
// then a 12-byte allocator block {const char* name, int flags, int id}. 0x0056DBE0
// passes the allocator to 0x0056D7E0, which hands [0] to the game's new[].
struct FloatVector {
    float* begin;
    float* end;
    float* capacity;
    uint32_t allocator[3];
};
TS3_X86_OFFSET(FloatVector, end, 4);
TS3_X86_OFFSET(FloatVector, capacity, 8);
TS3_X86_OFFSET(FloatVector, allocator, 12);
TS3_X86_SIZE(FloatVector, 24);

// The sample-filter object 0x0068A650 / 0x0068AA10 operate on (+0x00 unknown).
struct SpeedTreeFilterObject {
    uint32_t opaque00;    // +0x00
    int32_t sampleCount;  // +0x04 number of samples; the output vector keeps count+1
    uint32_t divisor;     // +0x08 scale divisor; the global 0x1158884 is its numerator
    float* samples;       // +0x0C in-place scaled by 0x0068AA10 before smoothing
    float* samplesEnd;    // +0x10 (end - samples) >> 2 scaled elements
    uint32_t opaque14[2]; // +0x14
    FloatVector output;   // +0x1C smoothed copies; output[count] = output[count - 1]
};
TS3_X86_OFFSET(SpeedTreeFilterObject, sampleCount, 0x04);
TS3_X86_OFFSET(SpeedTreeFilterObject, divisor, 0x08);
TS3_X86_OFFSET(SpeedTreeFilterObject, samples, 0x0C);
TS3_X86_OFFSET(SpeedTreeFilterObject, samplesEnd, 0x10);
TS3_X86_OFFSET(SpeedTreeFilterObject, output, 0x1C);
TS3_X86_SIZE(SpeedTreeFilterObject, 0x34);

// TS3W.exe 0x0056DBE0, __thiscall (ret 4): resizes a 4-byte-element vector (used for
// floats) to `count`; shrinking only moves the end pointer, growing appends zeros
// through the game's 0x0056D7E0 (bridged, not rewritten here). Returns the new end.
void* VectorResize(FloatVector* self, uint32_t count);

// TS3W.exe 0x0068A650, __thiscall (ret 4): resizes output to sampleCount+1 and writes
// smoothed copies of samples. Gate byte 0x1158880 selects a centered window average
// (window = the argument, samples clamped to [0, count-1]) over a plain copy; each
// output value is the float-accumulated sum divided by the window. EA bug kept: with
// sampleCount == 0 the epilogue reads output[-1]. Returns &output[count].
float* SmoothSamples(SpeedTreeFilterObject* self, uint32_t window);

// TS3W.exe 0x0068AA10, __thiscall (ret 4): scales samples[0, (end-samples)/4) in place
// by (double)global 0x1158884 / (double)divisor (both read as unsigned), then runs
// SmoothSamples (the original tail-jumps into it). Returns &output[count].
float* ScaleAndFilterSamples(SpeedTreeFilterObject* self, uint32_t window);

}  // namespace ts3::speedtree

// ABI-compatible hook entry points consumed by the x86 harness.
void* __fastcall SpeedTreeVectorResize(ts3::speedtree::FloatVector* self, void* edx, uint32_t count);
float* __fastcall SpeedTreeFilterSamples(ts3::speedtree::SpeedTreeFilterObject* self, void* edx, uint32_t window);
