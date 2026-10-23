#pragma once
#include "ts3/layout.h"
#include <cstdint>
namespace ts3::audio::b22 {
struct alignas(16) Point { float x,y,z,w; };
struct Primitive {
    void** vtable; void** secondary; void** interfaceView;
    uint32_t state; uint8_t enabled; uint8_t padding[3]; float gain;
    void* owner; void* resource; uint32_t hash;
    uint32_t tick; int32_t period; union { float depth; uint32_t sampleCount; }; float sampleValue;
    Point* begin; Point* end; Point* capacity;
};
// TS3W.exe 0x0086F800: Construct the spatial primitive, preserving untouched padding.
Primitive* Construct(Primitive*);
// TS3W.exe 0x0086F840: Lazily hash the primitive name.
uint32_t Name(Primitive*);
// TS3W.exe 0x0086F870: Forward the world scalar times the original scale to slot 14.
uintptr_t WorldGain(Primitive*,uintptr_t,uintptr_t);
// TS3W.exe 0x0086F8A0: Construct the world-scalar primitive.
Primitive* ConstructWorld(Primitive*);
// TS3W.exe 0x0086F8E0: Lazily hash the world-scalar name.
uint32_t WorldName(Primitive*);
// TS3W.exe 0x0086F910: Load interval and depth properties and initialize the primitive.
uint8_t LoadDepth(Primitive*);
// TS3W.exe 0x0086F9B0: Periodically compute submerged listener depth; increment wraps before signed division.
uintptr_t UpdateDepth(Primitive*,uintptr_t,uintptr_t);
// TS3W.exe 0x0086FB50: Construct the depth primitive with interval one and depth one.
Primitive* ConstructDepth(Primitive*);
// TS3W.exe 0x0086FBB0: Lazily hash the depth primitive name.
uint32_t DepthName(Primitive*);
// TS3W.exe 0x0086FBE0: Lazily hash the sample primitive name.
uint32_t SampleName(Primitive*);
// TS3W.exe 0x0086FC10: Adjust secondary this and forward to the original deleting destructor.
Primitive* DeleteSecondary(void*,uint32_t);
// TS3W.exe 0x0086FC20: Load interval and sample properties then initialize the primitive.
uint8_t LoadSamples(Primitive*);
// TS3W.exe 0x0086FCD0: Periodically count submerged sample points and divide by unsigned configured count.
uintptr_t UpdateSamples(Primitive*,uintptr_t,uintptr_t);

#if defined(_M_IX86)
inline Primitive* __fastcall HookConstruct(Primitive* p, void*) { return Construct(p); }
inline uint32_t __fastcall HookName(Primitive* p, void*) { return Name(p); }
inline uintptr_t __fastcall HookWorldGain(Primitive* p, void* ,uintptr_t a, uintptr_t b) { return WorldGain(p ,a, b); }
inline Primitive* __fastcall HookConstructWorld(Primitive* p, void*) { return ConstructWorld(p); }
inline uint32_t __fastcall HookWorldName(Primitive* p, void*) { return WorldName(p); }
inline uint8_t __fastcall HookLoadDepth(Primitive* p, void*) { return LoadDepth(p); }
inline uintptr_t __fastcall HookUpdateDepth(Primitive* p, void* ,uintptr_t a, uintptr_t b) { return UpdateDepth(p ,a, b); }
inline Primitive* __fastcall HookConstructDepth(Primitive* p, void*) { return ConstructDepth(p); }
inline uint32_t __fastcall HookDepthName(Primitive* p, void*) { return DepthName(p); }
inline uint32_t __fastcall HookSampleName(Primitive* p, void*) { return SampleName(p); }
inline Primitive* __fastcall HookDeleteSecondary(void* p, void* ,uint32_t a) { return DeleteSecondary(p ,a); }
inline uint8_t __fastcall HookLoadSamples(Primitive* p, void*) { return LoadSamples(p); }
inline uintptr_t __fastcall HookUpdateSamples(Primitive* p, void* ,uintptr_t a, uintptr_t b) { return UpdateSamples(p ,a, b); }
#endif
}
