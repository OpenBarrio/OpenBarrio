#pragma once
#include "ts3/layout.h"
namespace ts3::audio {
struct WaterPoints {
    std::byte *begin, *end, *capacityEnd;
    void* allocator;
    // TS3W.exe 0x00870070: allocate aligned sample storage and copy the existing range.
    uintptr_t Allocate(uint32_t count, void* first, void* last);
    // TS3W.exe 0x008700D0: reserve samples; returns begin unchanged or new end after growth.
    uintptr_t Reserve(uint32_t count);
};
struct WaterPrimitive {
    void *vtable, *secondary, *interfaceTable;
    uint32_t state;
    uint8_t active, padding[3];
    float value;
    void *resource, *opaque;
    uint32_t hash, tick;
    int32_t period;
    uint32_t count;
    float radius;
    WaterPoints points;
    uint32_t pondHash;
    // TS3W.exe 0x00870130: initialize water state and reserve ten sample points.
    uintptr_t Construct();
    // TS3W.exe 0x0086FE00: load pond sampling parameters and initialize the base primitive.
    bool LoadPond();
    // TS3W.exe 0x0086FEB0: periodically sample water occupancy and publish its fraction.
    uintptr_t UpdatePond(uint32_t a, uint32_t b);
    // TS3W.exe 0x00870280: replace sample points with two interleaved circular rings.
    uintptr_t Generate(void* center);
    // TS3W.exe 0x008701B0: construct pond state over the water base.
    uintptr_t ConstructPond();
    // TS3W.exe 0x008701E0: lazily hash the pond primitive name.
    uint32_t PondName();
    // TS3W.exe 0x00870220: destroy samples and retained resource, then optionally free self.
    uintptr_t Destroy(uint32_t flags);
};
// TS3W.exe 0x00870210: adjust the third interface to the primary deleting destructor.
uintptr_t DestroyWaterInterface(void* self, uint32_t flags);
struct ScenePrimitive {
    void *vtable, *secondary, *interfaceTable;
    uint32_t state;
    uint8_t active, padding[3];
    float value;
    void *resource, *opaque;
    uint32_t hash;
    // TS3W.exe 0x00870620: initialize the camera-distance primitive, preserving padding.
    uintptr_t Construct();
    // TS3W.exe 0x008704D0: publish camera distance normalized by the selected component range.
    uintptr_t Update(uint32_t a, uint32_t b);
    // TS3W.exe 0x00870660: lazily hash the camera-distance primitive name.
    uint32_t Name();
};
TS3_X86_OFFSET(WaterPrimitive, points, 0x34);
TS3_X86_OFFSET(WaterPrimitive, pondHash, 0x44);
TS3_X86_OFFSET(ScenePrimitive, hash, 0x20);
}
