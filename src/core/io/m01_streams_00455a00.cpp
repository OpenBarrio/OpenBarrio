#include "ts3/io/m01_streams_00455a00.h"

namespace ts3::io::m01 {
// Near twin of FileStream::Release (0x004DB390), with refs at +0x0C.
int32_t __fastcall MemoryRelease(MemoryStream* self, void*)
{
    if (self->refCount > 1)
        return --self->refCount;
    // Slot 0 remains a boundary: dispatch through the object's actual vtable,
    // not a statically selected MemoryStream destructor. It can free self.
    CallGameVirtual<void, 0>(self, 1u);
    return 0;
}
}
