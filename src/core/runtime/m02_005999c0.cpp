#include "ts3/runtime/m02_005999c0.h"
#include "ts3/runtime/m02_J1.h"
namespace ts3::runtime::m02 {
JobService* DestroyJobService(JobService* self, uint32_t flags) {
#if defined(_M_IX86)
    self->vtable = reinterpret_cast<void**>(0x00FD9810);
    self->secondary = reinterpret_cast<void**>(0x00FD980C);
#endif
    // The verified scheduler destructor performs both scheduler vtable transitions
    // and mutex destruction, without freeing the embedded subobject.
    j1::Destroy(&self->scheduler, 0);
    CallGameMethod<void, 0x00731CF0>(self);
    if (flags & 1) CallGameFunction<void, 0x004E3F70>(self);
    // EA returns this even after the optional deallocation.
    return self;
}
}
