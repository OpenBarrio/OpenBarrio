#pragma once
#include "ts3/anim/m05_B20.h"
namespace ts3::anim::m105 {
// Serialized relative offsets stay 32-bit on both architectures (not native pointers).
struct ClipView {
    unsigned char prefix[0x10];
    float interval;
    uint32_t reserved, channelCount, reserved1;
    int32_t channelsOffset, valuesOffset;
};
struct Channel {
    int32_t keysOffset;
    uint32_t identifier;
    uint16_t keyCount;
    uint8_t format, component;
};
static_assert(sizeof(Channel)==12);
// TS3W.exe 0x00586880: sample root motion between clamped times, preserving X-in-W translation and SSE quaternion refinements.
bool SampleRoot(const ClipView*, b20::Pose*, float time, float elapsed, bool mirror);
}
