#include "ts3/speedtree/clu_95cf30_SpeedTree.h"
#include "ts3/layout.h"

namespace ts3::speedtree {
// 0x0095CF30: cdecl, two stack arguments, caller cleanup. The constructor
// consumes its own single stack argument (ret 4); forward its EAX unchanged.
void* Construct95cf30(void* storage, void* argument)
{
    if (!storage) return nullptr;
    return CallGameMethod<void*, 0x0095CC40>(storage, argument);
}
} // namespace ts3::speedtree
