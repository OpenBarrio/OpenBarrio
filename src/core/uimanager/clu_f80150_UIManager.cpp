#include "ts3/uimanager/clu_f80150_UIManager.h"
#include "ts3/layout.h"

namespace ts3::uimanager {
int InitF80150()
{
#if defined(_M_IX86)
    // 0x00F80150: no arguments, cdecl. The constructor's EAX is discarded;
    // registration is unconditional and its EAX is the initializer's result.
    CallGameMethod<void, 0x0050A520>(reinterpret_cast<void*>(0x011CC9C4));
    using ExitCallback = void(__cdecl*)();
    return CallGameFunction<int, 0x00F7E9A1>(reinterpret_cast<ExitCallback>(0x00F90E70));
#else
    NotRewritten("InitF80150 game singleton and exit registry");
#endif
}
} // namespace ts3::uimanager
