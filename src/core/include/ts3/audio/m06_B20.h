#pragma once
#include "ts3/layout.h"
namespace ts3::audio {
struct b20 {
    void** vtable;
    void* secondary;
    void* reference;
    uint32_t state;
    uint8_t enabled, padding[3];
    float value;
    void* link;
    void* owner;
    uint32_t counter; // name cache in the wind primitive
    uint32_t interval;
    uint32_t parameter;
    float radius;
    struct Map {
        void* unused;
        void** buckets;
        uint32_t bucketCount, size;
        float load, growth;
        uint32_t nextResize;
        const char* label;
    } map;
    // TS3W.exe 0x0086E460: Publish the manager wind value, preserving unsigned conversion rounding.
    uintptr_t Wind(uint32_t, uint32_t);
    // TS3W.exe 0x0086E4A0: Construct the wind primitive, leaving padding untouched.
    b20* InitWind();
    // TS3W.exe 0x0086E4E0: Lazily hash iswind; a zero hash is retried.
    uint32_t NameWind();
    // TS3W.exe 0x0086E510: Adjust the reference interface back to its owner and delete.
    b20* DeleteReference(uint32_t);
    // TS3W.exe 0x0086E520: Release the linked object, restore base tables, and optionally free.
    b20* DeleteBase(uint32_t);
    // TS3W.exe 0x0086E570: Periodically count matching nearby models and publish the count.
    uintptr_t Models(uint32_t, uint32_t);
    // TS3W.exe 0x0086E6E0: Construct the model-map primitive from variant string arguments.
    b20* InitModels(uint32_t, void*);
    // TS3W.exe 0x0086E840: Adjust the secondary interface and delete the model primitive.
    b20* DeleteSecondary(uint32_t);
    // TS3W.exe 0x0086E850: Adjust the reference interface and delete the model primitive.
    b20* DeleteModelReference(uint32_t);
    // TS3W.exe 0x0086E860: Destroy model-map nodes, buckets, and the retained link.
    void DestroyModels();
    // TS3W.exe 0x0086E8D0: Destroy the model primitive and optionally free it; return its original pointer.
    b20* DeleteModels(uint32_t);
    // TS3W.exe 0x0086E8F0: Read configuration and initialize the primitive; preserve stale upper EAX bits.
    uint32_t Configure();
    // TS3W.exe 0x0086E990: Periodically query spatial density and publish it.
    uintptr_t Density(uint32_t, uint32_t);
};
TS3_X86_OFFSET(b20, counter,0x20);
TS3_X86_OFFSET(b20, map,0x30);
TS3_X86_SIZE(b20,0x50);
}
