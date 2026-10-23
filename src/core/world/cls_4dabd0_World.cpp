#include "ts3/world/cls_4dabd0_World.h"

namespace ts3::world {

// 0x004DABD0: destructor. Stores the vtable 0x00FB82F8 (same dword the vtable's slot 0
// lives at) and tail-jumps into the base-class dtor 0x004DAB70 (clu_49d1d0_x, not
// rewritten). The tail jump leaves the base dtor's eax in the caller's eax — MSVC's
// dtor ABI carries no return value, but the big World dtor 0x004A47A0 chains into this
// one by another tail jump, so the value is passed on as-is; the rewrite returns it
// unchanged rather than pretending it does not exist.
int32_t WorldSub::Dtor()
{
    vtable = reinterpret_cast<void* const*>(uintptr_t(0x00FB82F8));
    return CallGameMethod<int32_t, 0x004DAB70>(this);
}

// 0x004DABE0: scalar deleting destructor (ret 4), vtable slot 0. Sets the vtable and
// runs the base dtor itself (a direct call to 0x004DAB70, not through 0x004DABD0), then
// frees this through operator delete (0x004E3F70, clu_4e3f70_Scratch, cdecl, which
// null-checks internally) iff the flag argument's low bit is set (`test byte ptr
// [esp+8], 1` — only the low byte is examined), and returns this in eax.
WorldSub* WorldSub::DeletingDtor(uint32_t flags)
{
    Dtor();
    if (flags & 1u) CallGameFunction<int32_t, 0x004E3F70>(static_cast<void*>(this));
    return this;
}

}  // namespace ts3::world
