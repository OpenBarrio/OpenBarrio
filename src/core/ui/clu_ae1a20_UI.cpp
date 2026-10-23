#include "ts3/ui/clu_ae1a20_UI.h"

namespace ts3::ui {
void** UiSegmentWalk::VisitAE1A20()
{
    // 0x00AE1A20. Unsigned subtraction preserves x86 wraparound without
    // subtracting unrelated C++ pointers. A slot grows with the native pointer.
    void** result = nullptr;
    for (int32_t i = 0; i < count; ++i) {
        const intptr_t bytes = static_cast<intptr_t>(
            reinterpret_cast<uintptr_t>(position) - reinterpret_cast<uintptr_t>(origin));
        // SAR, including negative ragged offsets, rather than truncating division.
        const intptr_t stride = static_cast<intptr_t>(sizeof(void*));
        const intptr_t slots = bytes >= 0 ? bytes / stride : -1 - (-(bytes + 1)) / stride;
        const int32_t index = static_cast<int32_t>(static_cast<uint32_t>(slots) + static_cast<uint32_t>(i));
        const int32_t biased = static_cast<int32_t>(static_cast<uint32_t>(index) + 0x1000000u);
        const int32_t block = biased / 64 - 0x40000;
        const int32_t slot = static_cast<int32_t>(static_cast<uint32_t>(index) - static_cast<uint32_t>(block) * 64u);
        result = blocks[block];
        void* item = result[slot];
        const int32_t ordinal = static_cast<int32_t>(static_cast<uint32_t>(firstIndex) + static_cast<uint32_t>(i));
        // 0x00861C50 is not rewritten: the original currently just executes ret 8.
        CallGameMethod<void, 0x00861C50>(item, int32_t{0}, ordinal);
    }
    return result;
}
}
