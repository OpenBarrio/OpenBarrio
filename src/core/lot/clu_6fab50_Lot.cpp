#include "ts3/lot/clu_6fab50_Lot.h"

namespace ts3::lot {
// 0x006FAB50 (__thiscall, ret 8). Notification is owned by another unit.
uintptr_t LotFlags::UpdateFlags(uint32_t mask, uint8_t enable)
{
    uint32_t value = flags & ~mask;
    if (enable != 0)
        value |= mask;
    value &= 0x1f;
    flags = value;
    clearFlags = (value & 1) == 0 ? 1u : 0u;
    setFlags = 0;
    if (value & 2) setFlags = 4;
    if (value & 4) setFlags |= 0x200;
    if ((value & 8) == 0) setFlags |= 0x80;

    // With no owner 0x006FA9F0 leaves EAX untouched: the original returns
    // the newly masked value. Otherwise forward the notification's EAX.
    if (owner == nullptr) {
        CallGameMethod<void, 0x006FA9F0>(this);
        return value;
    }
    return CallGameMethod<uintptr_t, 0x006FA9F0>(this);
}
}
