#include "ts3/sacs/clu_d750a0_SACS.h"

namespace ts3::sacs {
std::byte* Vector16::Reserve(uint32_t count)
{
    // 0x00D750A0
    const auto span = static_cast<intptr_t>(reinterpret_cast<uintptr_t>(capacityEnd) -
                                            reinterpret_cast<uintptr_t>(begin));
    if (count <= static_cast<uintptr_t>(span >> 4))
        return begin;
    auto* replacement = CallGameMethod<std::byte*, 0x00A88710>(this, count, begin, end);
    if (begin)
        CallGameFunction<void, 0x004E3F70>(begin);
    // Read the old fields AFTER the free callback, just as the original does.
    const auto used = static_cast<intptr_t>(reinterpret_cast<uintptr_t>(end) -
                                            reinterpret_cast<uintptr_t>(begin));
    const auto newEnd = reinterpret_cast<std::byte*>(reinterpret_cast<uintptr_t>(replacement) +
                                                    static_cast<uintptr_t>((used >> 4) * 16));
    begin = replacement;
    end = newEnd;
    // EA uses SHL on a 32-bit count (including its wraparound).
    capacityEnd = reinterpret_cast<std::byte*>(reinterpret_cast<uintptr_t>(replacement) + (count << 4));
    return newEnd;
}
}
