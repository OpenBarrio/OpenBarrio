#include "ts3/videorecording/cls_f20ec0_VideoRecording.h"

namespace ts3::videorecording {

// 0x00F20EC0: __thiscall, ECX = this, plain ret; EAX = this.
RecordingBaseF20EC0* RecordingBaseF20EC0::Initialize()
{
    vtable = reinterpret_cast<void* const*>(uintptr_t{0x0112CC38});
    field04 = nullptr;
    field08 = nullptr;
    // The original deliberately does not initialize +0x0C or +0x10.
    field14 = 0;
    field18 = 0;
    return this;
}

}  // namespace ts3::videorecording
