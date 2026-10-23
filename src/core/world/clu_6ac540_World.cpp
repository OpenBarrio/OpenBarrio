#include "ts3/world/clu_6ac540_World.h"

#include <cstddef>
#include <cstdint>
#include <cstring>

namespace ts3::world {

WorldFragment6ac540* WorldFragment6ac540::CopyFrom(const WorldFragment6ac540* source)
{
    // Original 0x006C2990 uses aligned 16-byte loads/stores for these three blocks.
    std::memcpy(bytes, source->bytes, 0x30);
    std::memcpy(bytes + 0x30, source->bytes + 0x30, sizeof(uint32_t));
    std::memcpy(bytes + 0x40, source->bytes + 0x40, 4 * sizeof(uint32_t));
    return this;
}

bool WorldPredicateState6ac540::IsPending(uint8_t testSecondFlag) const
{
    // Original 0x006B5750 tests the low byte of arg1, then checks +0x3c or +0x38.
    const size_t offset = testSecondFlag ? 0x3c : 0x38;
    uint32_t value;
    std::memcpy(&value, bytes + offset, sizeof value);
    return value == 0;
}

void* WorldState6ac540::Update(int32_t selector)
{
    // Original 0x006C4770 has three distinct paths; retain its selective copies.
    if (selector == -1) {
        std::memcpy(bytes + 0xf0, bytes, 0x34);
        std::memcpy(bytes + 0x120, bytes + 0x30, sizeof(uint32_t));
        std::memcpy(bytes + 0x130, bytes + 0x40, 4 * sizeof(uint32_t));
        return ts3::CallGameMethod<void*, 0x006C3480>(this, 0);
    }
    if (selector == -2) {
        reinterpret_cast<WorldFragment6ac540*>(bytes + 0x50)->CopyFrom(
            reinterpret_cast<const WorldFragment6ac540*>(bytes));
        auto* result = reinterpret_cast<WorldFragment6ac540*>(bytes + 0xa0)->CopyFrom(
            reinterpret_cast<const WorldFragment6ac540*>(bytes + 0xf0));
        const float five = 5.0f;
        const float zero = 0.0f;
        std::memcpy(bytes + 0x140, &five, sizeof five);
        std::memcpy(bytes + 0x144, &zero, sizeof zero);
        return result;
    }
    return ts3::CallGameMethod<void*, 0x006C3480>(this, selector);
}

int32_t InitializeWorldState6ac540()
{
    // Original 0x006AC540 calls the renderer helper with the global world object,
    // then clears the independent byte at 0x011D08C0.
#if defined(_M_IX86)
    auto* globalWorld = reinterpret_cast<WorldState6ac540*>(static_cast<uintptr_t>(0x011D08D0));
    const int32_t result = ts3::CallGameMethod<int32_t, 0x006C4760>(globalWorld);
    *reinterpret_cast<volatile uint8_t*>(static_cast<uintptr_t>(0x011D08C0)) = 0;
    return result;
#else
    ts3::AbortNotRewritten("World global initializer 0x006AC540");
    return 0;
#endif
}

int32_t WorldLightRigState6ac540::SetUpdateLightRigs(uint8_t enabled)
{
    // Original 0x006AFDA0 stores the raw byte at this+0xdd before the renderer call.
    bytes[0xdd] = enabled;
    return ts3::CallGameMethod<int32_t, 0x006AFC70>(this);
}

uint32_t GetWorldObject6ac540()
{
    // Original 0x006E97B0 reads global 0x011D1860 and returns *(world + 0x1c0).
#if defined(_M_IX86)
    const uint32_t world = *reinterpret_cast<volatile uint32_t*>(static_cast<uintptr_t>(0x011D1860));
    return world ? *reinterpret_cast<volatile uint32_t*>(static_cast<uintptr_t>(world + 0x1c0)) : world;
#else
    ts3::AbortNotRewritten("World object getter 0x006E97B0");
    return 0;
#endif
}

void* GetNumDaysUntilFullMoonImpl6ac540(int32_t selector)
{
    // Original 0x00786BD0 delegates null to 0x006C4760; other selectors enter 0x006C4770.
#if defined(_M_IX86)
    auto* globalWorld = reinterpret_cast<WorldState6ac540*>(static_cast<uintptr_t>(0x011D08D0));
    if (selector)
        return globalWorld->Update(selector);
    return ts3::CallGameMethod<void*, 0x006C4760>(globalWorld);
#else
    (void)selector;
    ts3::AbortNotRewritten("World full moon bridge 0x00786BD0");
    return nullptr;
#endif
}

int32_t SetUpdateLightRigsImpl6ac540(uint8_t enabled)
{
    // Original 0x00786C70 obtains the object through 0x006E97B0, then calls 0x006AFDA0.
#if defined(_M_IX86)
    auto* world = reinterpret_cast<WorldLightRigState6ac540*>(static_cast<uintptr_t>(GetWorldObject6ac540()));
    return world->SetUpdateLightRigs(enabled);
#else
    (void)enabled;
    ts3::AbortNotRewritten("World light rig bridge 0x00786C70");
    return 0;
#endif
}

}  // namespace ts3::world
