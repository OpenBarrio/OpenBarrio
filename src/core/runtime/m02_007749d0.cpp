#include "ts3/runtime/m02_007749d0.h"
#include "ts3/runtime/m02_S1.h"
#include "ts3/layout.h"
namespace ts3::runtime::m02 {
uint32_t __cdecl CreateStopwatch(int32_t mode) {
    if (mode > 7) return 0;
#if defined(_M_IX86)
    auto scripting = CallGameFunction<void*,0x007692B0>();
    auto storage = CallGameFunction<StopWatch*,0x004E3F90>(size_t(0x18),
        reinterpret_cast<const char*>(0x0100C490),0,0,0,0);
    auto watch = storage ? s1::Construct(storage,nullptr,static_cast<uint32_t>(mode),0) : nullptr;
    return CallGameVirtual<uint32_t,50>(scripting,watch,uint32_t(0x10));
#else
    NotRewritten("M02 stopwatch factory allocator/scripting bridge");
#endif
}
}
