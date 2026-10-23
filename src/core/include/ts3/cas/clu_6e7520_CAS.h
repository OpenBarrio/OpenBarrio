// Unit clu_6e7520_CAS: TS3W.exe 0x006E7520 (42 bytes), thiscall ret 4, one stack arg.
// Reference-counted pointer store at +0x24 of a Class_00F9A240 object: AddRef the
// incoming reference, Release the outgoing one, store the incoming, and return the
// result of the last virtual call executed. Byte-identical to 0x0068ED00 (unit
// clu_68baf0_CAS, ReferenceHolder, field at +0x64) except for the field offset and
// the returned EAX.
#pragma once

#include "ts3/cas/clu_68baf0_CAS.h"
#include "ts3/layout.h"

namespace ts3::cas {

// Partial view of the Class_00F9A240 object (vtable 0x00F9A240, 336 bytes): only the
// +0x24 reference slot is touched by this unit. The stored object is a
// PieHeadLodOverride (vtable 0x00FF7F9C) or TextureEntry (vtable 0x00FF5EE8); both
// vtables have slot 0 = 0x005044C0 (AddRef) and slot 1 = 0x00692720 (Release), i.e.
// the IReference interface.
struct CasRefSlot {
    uint8_t head[0x24];     // +0x00..+0x23 opaque to this unit
    IReference* value;      // +0x24 refcounted object

    // TS3W.exe 0x006E7520, thiscall ret 4 (one stack arg): AddRef the incoming
    // reference, Release the outgoing one, store the incoming. Returns the result
    // of the last virtual call executed. EA: when neither pointer is set no call
    // runs and the original returns the caller's pre-call EAX (undefined); all six
    // callers ignore the return value, so the rewrite returns 0 in that case.
    int32_t SetReference(IReference* replacement);
};
TS3_X86_OFFSET(CasRefSlot, value, 0x24);
TS3_X86_SIZE(CasRefSlot, 0x28);

}  // namespace ts3::cas
