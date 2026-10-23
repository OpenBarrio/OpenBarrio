#include "ts3/lotrenderer/cls_7e3d40_LotRenderer.h"

#include <cstring>

#if defined(_WIN32)
#define WIN32_LEAN_AND_MEAN
#define NOMINMAX
#include <windows.h>
#include <intrin.h>
#endif

namespace ts3::lotrenderer {

// Original 0x007E3D40.
CyclingRenderer* CyclingRenderer::QueryInterface(int32_t interfaceId)
{
    const bool known = interfaceId == 97602185 || interfaceId == 101394490;
    return known ? this : nullptr;
}

// Original 0x007E3D70.
CyclingRenderer* CyclingRenderer::Construct()
{
    vtableAddress = 0x0101B954;
    current = 1;
    // 0x004F35B0: stopwatch init at this+0x10, thiscall, two stack args, ret 8.
    CallGameMethod<void, 0x004F35B0>(&startLow, 5, 1);
    // xchg with a memory operand: atomic store of 0.
    _InterlockedExchange(reinterpret_cast<volatile long*>(&lockWord), 0);
    return this;
}

// Original 0x007E3DA0.
void CyclingRenderer::Update(CyclingRenderTarget* target, void* const* targets, uint32_t count, uint32_t)
{
    uint8_t copy[0x44];
    std::memcpy(copy, target->data, sizeof copy);
    CallGameMethod<void, 0x00516AA0>(target, static_cast<const void*>(copy));
    CallGameMethod<void, 0x0050E990>(target, targets[0], 1);

    // 0x004F33C0: elapsed seconds of the embedded stopwatch (x87 float result).
    const float elapsed = CallGameMethod<float, 0x004F33C0>(&startLow);
    if (elapsed > cycleSeconds) {
        if (++current >= count)
            current = 1;
        // The original zeroes startLow/startHigh here too, but both are always overwritten
        // by the timestamp below; only the pad zeroing is observable.
        pad18 = 0;
        pad1c = 0;
        uint64_t now = 0;
#if defined(_WIN32)
        if (timerMode == 1) {
            now = __rdtsc();
        } else {
            LARGE_INTEGER counter;
#if defined(_M_IX86)
            // 0x00F953B8: the game's QueryPerformanceCounter import slot; the original
            // does the original instruction sequence.
            using QpcFn = BOOL (WINAPI*)(LARGE_INTEGER*);
            const auto qpc = *reinterpret_cast<const QpcFn*>(static_cast<uintptr_t>(0x00F953B8u));
            qpc(&counter);
#else
            QueryPerformanceCounter(&counter);
#endif
            now = static_cast<uint64_t>(counter.QuadPart);
        }
#endif
        startLow = static_cast<uint32_t>(now);
        startHigh = static_cast<uint32_t>(now >> 32);
    }
    for (uint32_t i = 1; i < count; ++i) {
        if (current == i)
            CallGameMethod<void, 0x0050E990>(target, targets[i], 0);
    }
}

}  // namespace ts3::lotrenderer
