// TS3W.exe 0x007EE0D0 / 0x007EE0F0 / 0x007EF650 (subsystem Debug) — see the header for
// the class and vtable map. The router 0x007EF1D0 and the scratch free 0x004E3F70 are
// not rewritten yet and are called through the bridge.
//
// Kept from the original (EA quirks that can affect outputs):
//  - Construct writes only the low byte at +0x08; the other three bytes of that dword
//    survive whatever the object held before;
//  - RouteMessage decides whether to release the router's buffer with a *capacity* span
//    (cap - begin > 1, signed) instead of the used span (end - begin), so a heap buffer
//    the router left empty is freed all the same;
//  - Release's null-`this` test after the atomic decrement is dead code (the xadd above
//    already dereferenced `this`), so the rewrite dispatches destroy without retesting.
#include "ts3/debug/cls_7ee0d0_Debug.h"

#include <intrin.h>

#include "ts3/layout.h"

namespace ts3::debug {
namespace {

constexpr uintptr_t kDebugVtable = 0x0101D964u;
// The router's channel string in the game image ("Debug/UISpy"), stored by address —
// game code may compare the pointer, not just the text.
constexpr uintptr_t kUipyChannel = 0x0101E1BCu;
// The one id the router drops above the small range (the small drops are 0x0C, 0x0D and
// 0x14; everything else, including 0, is forwarded).
constexpr uint32_t kUipySkipId = 0x7893FADBu;

}  // namespace

// TS3W.exe 0x007EE0D0 — thiscall, ret 4, one byte argument, returns this.
DebugObject* DebugObject::Construct(uint8_t modeArg)
{
    mode = modeArg;  // one byte only; +0x09..+0x0B keep whatever they held
    vtable = reinterpret_cast<void*>(kDebugVtable);
    unused04 = 0;
    // The original stores 0 with an `xchg` (an interlocked exchange whose old value it
    // throws away); the atomic RMW keeps that instruction's semantics exactly.
    auto* counter = reinterpret_cast<volatile long*>(&refcount);
    _InterlockedExchange(counter, 0);
    return this;
}

// TS3W.exe 0x007EE0F0 — thiscall, plain ret, returns the new refcount.
int32_t DebugObject::Release()
{
    auto* counter = reinterpret_cast<volatile long*>(&refcount);
    const long remaining = _InterlockedDecrement(counter);
    if (remaining == 0)
        static_cast<const DebugVtable*>(vtable)->destroy(this, 1);
    return remaining;
}

// TS3W.exe 0x007EF650 — thiscall, ret 0xC (kind, extra, msg on the stack, this in ecx).
void DebugObject::RouteMessage(uint32_t kind, void* extra, const UipyMessage* msg)
{
    if (!msg->present)
        return;
    const uint32_t id = msg->id;
    if (id == 0x14u || (id >= 0x0Cu && id <= 0x0Du) || id == kUipySkipId)
        return;

    RouteBuffer local;
    local.begin = local.inlineBuf;
    local.end = local.inlineBuf;
    local.cap = local.inlineBuf + 0x100;
    local.channel = reinterpret_cast<const char*>(kUipyChannel);
    local.inlinePtr = local.inlineBuf;
    local.inlineBuf[0] = 0;

    CallGameMethod<void, 0x007EF1D0>(this, &local, kind, extra, msg);

    // The original re-reads begin/cap/inlinePtr from memory after the call (the router
    // may have swapped the inline buffer for a heap one) and frees on the capacity span,
    // not on how much was written.
    const uintptr_t beginBits = reinterpret_cast<uintptr_t>(local.begin);
    const uintptr_t capBits = reinterpret_cast<uintptr_t>(local.cap);
    if (static_cast<intptr_t>(capBits - beginBits) > 1 && local.begin && local.begin != local.inlinePtr)
        CallGameFunction<void, 0x004E3F70>(local.begin);
}

}  // namespace ts3::debug
