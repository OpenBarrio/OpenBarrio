// Cluster unit clu_60e6c0_UserTool (subsystem UserTool): one TS3W.exe leaf.
//
//   0x0060E6C0  UserToolStateRef::ReadStateFields  thiscall, ret 0xC: reads the
//                                        pointer at +0xE0 of the owner object and
//                                        copies three dwords of the pointed-to state
//                                        block (+0x184, +0x188, +0x1B4) into the three
//                                        caller-provided outputs; returns the last one.
//
// The owner is the UserTool state object the game keeps at 0x11CEC88 (0x0060E2C0
// returns it); all three callers (0x006945C0, 0x00D0EA00, 0x00EC0640) pass that
// object. The state block's +0x188 dword is used by the callers as an object pointer
// and its +0x1B4 dword is turned into a float (x 0.02) by 0x00D0EA00 and stored at the
// owner's +0x60, but this unit only copies the three dwords.
//
// EA quirk kept: the original dereferences +0xE0 with no null check, so a null state
// pointer faults; the rewrite does the same.
#pragma once

#include <cstdint>

#include "ts3/layout.h"

namespace ts3::usertool {

// The owner object 0x0060E6C0 reads (the game's UserTool state object at 0x11CEC88):
// a view covering exactly the one field this unit touches, the pointer at +0xE0.
struct UserToolStateRef {
    uint8_t pad000[0xE0];
    void* state;  // +0xE0

    // 0x0060E6C0 (ret 0xC): copies the state block's +0x184, +0x188 and +0x1B4 dwords
    // into *a2, *a3 and *a4 and returns the +0x1B4 value.
    int32_t ReadStateFields(int32_t* a2, uintptr_t* a3, int32_t* a4);
};
TS3_X86_OFFSET(UserToolStateRef, state, 0xE0);
TS3_X86_SIZE(UserToolStateRef, 0xE4);

// The state block pointed to by +0xE0: a view covering exactly the three dwords this
// unit reads (the block extends far beyond +0x1B4; 0x0060E800 reads +0x1A4 of it).
struct UserToolStateBlock {
    uint8_t pad000[0x184];
    int32_t field184;  // +0x184 -> *a2
    uintptr_t field188;  // +0x188 -> *a3 (the callers treat it as an object pointer)
    uint8_t pad18C[0x1B4 - 0x18C];
    int32_t field1B4;  // +0x1B4 -> *a4 and the return value
};
TS3_X86_OFFSET(UserToolStateBlock, field184, 0x184);
TS3_X86_OFFSET(UserToolStateBlock, field188, 0x188);
TS3_X86_OFFSET(UserToolStateBlock, field1B4, 0x1B4);
TS3_X86_SIZE(UserToolStateBlock, 0x1B8);

}  // namespace ts3::usertool
