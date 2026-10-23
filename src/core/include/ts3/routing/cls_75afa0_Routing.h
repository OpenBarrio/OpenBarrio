#pragma once
#include "ts3/layout.h"

namespace ts3::routing {
struct alignas(16) RoutingPoint {
    float position[4];
    float direction[4];
    float weight;
    int32_t level, metadata[3];
    uint8_t padding[12];
};
TS3_X86_OFFSET(RoutingPoint, weight, 32);
TS3_X86_OFFSET(RoutingPoint, level, 36);
TS3_X86_SIZE(RoutingPoint, 64);
struct RoutingPointVector {
    RoutingPoint *begin, *end, *capacity;
    uint8_t allocator[12];
};
TS3_X86_OFFSET(RoutingPointVector, end, 4);
TS3_X86_OFFSET(RoutingPointVector, capacity, 8);
TS3_X86_SIZE(RoutingPointVector, 24);
struct RoutingSampler {
    uint8_t prefix[44];
    float weight;
    uint8_t unknown48[40];
    int32_t facing;
    float cone, minimum, maximum, unknown104, spacing;
    // 0x0075AFA0, thiscall, ret 44. Output vectors are owned by other units.
    bool SampleSegment(void* context, void* registry, const RoutingPoint* start,
        const RoutingPoint* end, const float* direction, int32_t level,
        void* arg8, void* arg9, void* arg10, void* accepted, void* rejected);
    // 0x0075B610, thiscall, ret 64; final argument is unused by EA.
    uint32_t SampleArc(void* context, void* registry, uint32_t weightByDistance,
        const float* center, float radius, int32_t level, float firstAngle,
        float lastAngle, RoutingPointVector* accepted, void* arg11, void* arg12,
        void* arg13, float* firstPoint, float* lastPoint, void* rejected, void* unused);
};
TS3_X86_OFFSET(RoutingSampler, weight, 44);
TS3_X86_OFFSET(RoutingSampler, facing, 88);
TS3_X86_OFFSET(RoutingSampler, cone, 92);
TS3_X86_OFFSET(RoutingSampler, minimum, 96);
TS3_X86_OFFSET(RoutingSampler, maximum, 100);
TS3_X86_OFFSET(RoutingSampler, spacing, 108);
TS3_X86_SIZE(RoutingSampler, 112);
}
