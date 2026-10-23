#include "ts3/usertool/clu_8638c0_UserTool.h"

namespace ts3::usertool {
// 0x008638C0, thiscall, plain ret: EAX is a pointer on every path.
void* UserToolValue::GetValue() const
{
    if (type != 19 && type != 16)
        return CallGameFunction<void*, 0x007935B0>();
    if ((flags & 0x30) != 0)
        return payload.indirect;
    // The original's redundant test ax,ax cannot be zero after either tag match.
    return const_cast<Payload*>(&payload);
}
} // namespace ts3::usertool
