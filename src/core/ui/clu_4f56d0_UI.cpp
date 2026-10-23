#include "ts3/ui/clu_4f56d0_UI.h"

namespace ts3::ui {
UiValue4f56d0* UiValue4f56d0::Construct(const void* value)
{
    // 0x004F56D0 (__thiscall, ret 4): clear before calling the initializer,
    // regardless of the previous flags. Its return value is discarded.
    flags = 0;
    size = 0;
#if defined(_M_IX86)
    vtable = reinterpret_cast<void*>(static_cast<uintptr_t>(0x00FB96B4));
#else
    AbortNotRewritten("UiValue4f56d0 game vtable");
#endif
    CallGameMethod<void*, 0x004F55E0>(this, value);
    return this;
}
} // namespace ts3::ui
