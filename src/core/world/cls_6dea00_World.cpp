#include "ts3/world/cls_6dea00_World.h"

#include <atomic>
#include <cstring>

namespace ts3::world {

namespace {
inline const char* AtString(uintptr_t address)
{
    return reinterpret_cast<const char*>(address);
}
}  // namespace

// 0x006DEA00: inner-object init. Writes the base vtable 0x00FE1A5C and zeroes the dword
// at +0x08, returns this. The World ctor (0x00A618E0) calls it on this+0x08 right after
// zeroing +0x04, then overwrites the inner vtable with the final 0x01060994 — only the
// +0x08 zero survives the call.
UiWorldInner* UiWorldInner::Construct()
{
    vtable = reinterpret_cast<void* const*>(uintptr_t(0x00FE1A5C));
    field08 = 0;
    return this;
}

// 0x00A5F9F0: stores 1 at +0x8C and returns the old value (IDA: _InterlockedExchange(
// this + 35, 1)); the three stack arguments are never read (ret 0xC). The original's
// xchg is implicitly locked on x86, so the rewrite uses a sequentially consistent
// atomic exchange, whose compiled form on x86 is the same locked xchg.
int32_t UiWorld::ExchangeFlag1(int32_t, int32_t, int32_t)
{
    std::atomic_ref<int32_t> flag(flag8C);
    return flag.exchange(1);
}

// 0x00A618E0: World ctor (caller 0x00B38820), __thiscall, ret 0, returns this.
// Two-phase vtable setup (base 0x0107396C / inner-and-+0x14 default 0x00FE1A5C first,
// then the final 0x01060998 / 0x01060994 / 0x01060990), three xchg-cleared pointer-like
// fields (+0x04, +0x18, +0x94), the mirrored spec blocks at +0x58/+0x78 with 1.0f/2.0f
// from the rdata constants 0x0107A538 / 0x00F98A58 and name strings, and the zero tail
// through +0x104. The gaps at +0x24..+0x2F, +0x51..+0x57 and +0x74..+0x77 are never
// written (they keep whatever was in the object). The original builds its zero block on
// the stack for a 16-byte-aligned movaps into +0x30; the rewrite stores the same bytes
// directly.
UiWorld* UiWorld::Construct()
{
    vtable = reinterpret_cast<void* const*>(uintptr_t(0x0107396C));
    field04 = 0;
    inner.Construct();  // 0x006DEA00 on this+0x08
    vtable14 = reinterpret_cast<void* const*>(uintptr_t(0x00FE1A5C));
    vtable = reinterpret_cast<void* const*>(uintptr_t(0x01060998));
    inner.vtable = reinterpret_cast<void* const*>(uintptr_t(0x01060994));
    vtable14 = reinterpret_cast<void* const*>(uintptr_t(0x01060990));
    field18 = 0;
    field1C = 0;
    field20 = 0;
    zero30[0] = 0.0f;
    zero30[1] = 0.0f;
    zero30[2] = 0.0f;
    zero30[3] = 0.0f;
    field42 = 1;
    field50 = 1;
    field40 = 0;
    field41 = 0;
    std::memset(field43, 0, sizeof field43);
    min64 = 1.0f;
    max68 = 2.0f;
    name70 = AtString(0x01060A2C);
    count5C = 1;
    field60 = 0;
    field6C = 0;
    id58 = 0x0114C900;
    min84 = 1.0f;
    max88 = 2.0f;
    name90 = AtString(0x01060A08);
    count7C = 1;
    id78 = 0x0114C900;
    field80 = 0;
    flag8C = 0;
    field94 = 0;
    field98 = 0;
    field9C = 0;
    fieldA0 = 0;
    nameA4 = AtString(0x010609E4);
    for (int32_t& v : fieldA8) v = 0;
    nameD4 = AtString(0x010609C0);
    fieldC8 = 0x011C7E54;
    fieldCC = 0x011C7E54;
    fieldD0 = 0x011C7E56;
    for (int32_t& v : fieldD8) v = 0;
    return this;
}

// 0x00A61AC0: scalar deleting dtor (ret 4). Runs the complete-object dtor 0x00A61190 —
// the dtor body is not a member of any unit in units.csv, so it is bridged (it ends with
// the original instruction sequence, a plain ret 0 thiscall, and its return value is discarded here exactly
// as the original discards it) — then frees this when the flag byte's low bit is set,
// through operator delete (0x004E3F70, clu_4e3f70_Scratch, not rewritten yet), and
// returns this.
UiWorld* UiWorld::DeletingDtor(uint32_t flags)
{
    CallGameMethod<void, 0x00A61190>(this);
    if (flags & 1u) CallGameFunction<int32_t, 0x004E3F70>(static_cast<void*>(this));
    return this;
}

}  // namespace ts3::world
