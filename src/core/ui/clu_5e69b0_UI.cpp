#include "ts3/ui/clu_5e69b0_UI.h"

namespace ts3::ui {
// 0x005E69B0, thiscall, ret 24. Helper 0x008B7440 is vector insertion,
// not part of this unit. Its temporary's last eight bytes are uninitialized in EA.
UiAppendRecord* UiRecordList::Append(uint32_t tag, uint8_t flag, uint32_t a,
                                    uint32_t b, uint32_t c, uint32_t d)
{
    UiAppendRecord* slot = end;
    if (reinterpret_cast<uintptr_t>(slot) >= reinterpret_cast<uintptr_t>(capacity)) {
        UiAppendRecord temporary;
        for (auto& value : temporary.values) value = 0;
        CallGameMethod<void, 0x008B7440>(this, slot, &temporary);
    } else {
        end = reinterpret_cast<UiAppendRecord*>(reinterpret_cast<uintptr_t>(slot) + sizeof(UiAppendRecord));
        if (slot) {
            for (auto& value : slot->values) value = 0;
        }
    }
    UiAppendRecord* result = end - 1;
    result->tag = tag;
    result->flag = flag;
    result->values[0] = a;
    result->values[1] = b;
    result->values[2] = c;
    result->values[3] = d;
    return result;
}
} // namespace ts3::ui
