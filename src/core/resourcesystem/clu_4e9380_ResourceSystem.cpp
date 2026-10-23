#include "ts3/resourcesystem/clu_4e9380_ResourceSystem.h"

namespace ts3::resourcesystem {
// 0x004E9380: __thiscall, plain ret, EAX = this on the only return path.
ResourceState4e9380* ResourceState4e9380::Construct()
{
    enabled = 0;
    busy = 0;
    nesting = 0;
    CallGameMethod<void, 0x004E17E0>(&lock, nullptr, uint8_t{1});
    interval = 1000;
    active = 1;
    current = nullptr;
    currentFlag = 0;
    // The original memset covers 33 {key, pointer} pairs (0x108 bytes on x86).
    // Assign members individually so pointers remain native-sized on x64.
    for (auto& slot : slots) {
        slot.key = 0;
        slot.resource = nullptr;
    }
    // EA redundantly clears the last pair after memset.
    slots[32].key = 0;
    slots[32].resource = nullptr;
    return this;
}
} // namespace ts3::resourcesystem
