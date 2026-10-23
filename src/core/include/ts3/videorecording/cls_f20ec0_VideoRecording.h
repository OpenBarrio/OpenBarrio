#pragma once

#include "ts3/layout.h"

namespace ts3::videorecording {

// Only the 0x1C-byte prefix accessed by the base initializer is modeled.
// Opaque slots are pointer-sized so potential game pointers are not truncated on x64.
struct RecordingBaseF20EC0 {
    void* const* vtable;
    void* field04;
    void* field08;
    void* untouched0c;
    void* untouched10;
    uint32_t field14;
    uint32_t field18;

    // TS3W.exe 0x00F20EC0: installs the base vtable and clears four fields, returning this; preserves +0x0C/+0x10.
    RecordingBaseF20EC0* Initialize();
};
TS3_X86_OFFSET(RecordingBaseF20EC0, vtable, 0x00);
TS3_X86_OFFSET(RecordingBaseF20EC0, field04, 0x04);
TS3_X86_OFFSET(RecordingBaseF20EC0, field08, 0x08);
TS3_X86_OFFSET(RecordingBaseF20EC0, untouched0c, 0x0C);
TS3_X86_OFFSET(RecordingBaseF20EC0, untouched10, 0x10);
TS3_X86_OFFSET(RecordingBaseF20EC0, field14, 0x14);
TS3_X86_OFFSET(RecordingBaseF20EC0, field18, 0x18);
TS3_X86_SIZE(RecordingBaseF20EC0, 0x1C);

}  // namespace ts3::videorecording
