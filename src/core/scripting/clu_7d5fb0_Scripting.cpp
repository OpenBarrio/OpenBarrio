// Rewrite for the scripting prototype resource factory constructor.
#include "ts3/scripting/clu_7d5fb0_Scripting.h"

#include "ts3/layout.h"

namespace ts3::scripting {

void* __fastcall CreatePrototypeResourceFactory(void*, void*)
{
#if defined(_M_IX86)
    // 0x007D5FB0 allocates 0x58 bytes through the game allocator, then tail-calls
    // the shared initializer at 0x007D5F20. Keep both operations in the game bridge;
    // the allocator and initializer are not part of this rewritten unit.
    void* const object = CallGameFunction<void*, 0x004E3F90>(
        0x58, "Scripting/PrototypeResourceFactory/CreateResource", 0, 0, 0, 0);
    if (!object)
        return nullptr;
    CallGameMethod<void, 0x007D5F20>(object);
    return object;
#else
    return nullptr;
#endif
}

}  // namespace ts3::scripting
