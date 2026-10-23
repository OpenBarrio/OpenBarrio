#include "ts3/speedtree/clu_f82da0_SpeedTree.h"
#include "ts3/layout.h"

namespace ts3::speedtree {
// 0x00F82DA0, cdecl, no arguments. The constructor owns the opaque global's
// layout; this wrapper never reads or writes its members itself.
int32_t InitializeGlobal()
{
#if defined(_M_IX86)
    CallGameMethod<void, 0x007C4240>(reinterpret_cast<void*>(uintptr_t{0x011D2780}));
    using Callback = void (*)();
    return CallGameFunction<int32_t, 0x00F7E9A1>(
        reinterpret_cast<Callback>(uintptr_t{0x00F92520}));
#else
    AbortNotRewritten("SpeedTree global initializer game dependencies");
    return 0;
#endif
}
}
