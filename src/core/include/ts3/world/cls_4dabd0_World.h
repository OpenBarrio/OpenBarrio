#pragma once
#include <cstdint>
#include "ts3/layout.h"

namespace ts3::world {

// cls_4dabd0_World — the embedded World sub-object that the big World class of
// clu_4a47a0_World (0x004A47A0) destroys at +0x204 by tail-jumping into 0x004DABD0 with
// the original instruction sequence. The unit holds only the two destructors; the base class
// (dtor 0x004DAB70, clu_49d1d0_x, not rewritten) owns everything through this+0x228, so
// no member of this class is observable from here. Observed extent 0x22C (largest
// offset 0x004DAB70 touches is +0x228); the real object may be larger — the deleting
// dtor frees through operator delete with no size argument.
struct WorldSub {
    void* const* vtable;  // +0x00 the dtor writes 0x00FB82F8 (slot 0 = deleting dtor)

    // 0x004DABD0, __thiscall, ret 0. Stores the vtable 0x00FB82F8 and tail-jumps into
    // the base dtor 0x004DAB70; the base dtor's eax (its al is a 0/1 success flag, the
    // upper bits are stale) passes through to the caller unchanged, so this rewrite
    // returns it too (0x004A47A0's tail chain propagates it into the big dtor's result).
    int32_t Dtor();

    // 0x004DABE0, __thiscall, ret 4; vtable slot 0 (referenced from 0x00FB82F8). Sets
    // the vtable, runs the base dtor itself (not via 0x004DABD0), frees this through
    // operator delete (0x004E3F70, clu_4e3f70_Scratch, cdecl, null-checked) when the
    // flag argument's low bit is set, and returns this.
    WorldSub* DeletingDtor(uint32_t flags);
};

}  // namespace ts3::world
