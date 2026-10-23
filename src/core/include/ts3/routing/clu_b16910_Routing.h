// Cluster unit clu_b16910_Routing (subsystem Routing): two tiny leaves of the
// route-queue machinery.
//   0x00B16910 RouteManager::GetQueueView  - hands out the address of the route
//              manager's embedded queue (this + 0x1C) through the caller's out
//              parameter (thiscall, `ret 4`, 12 bytes, 5 callers),
//   0x00B17430 RouteQueue::IsEmptyQueue    - the queue-empty predicate `begin == end`
//              (thiscall, plain `ret`, 13 bytes, 2 callers; callers walk the queue
//              with the front/pop pair 0x00B17580 / 0x00B17A60 until it reports
//              empty).
// Both are self-contained leaves: no dependencies. Both are members (not free
// functions) so the harness's thiscall compare wrappers can call them as
// static_cast<Cls*>(self)->Method(...).
#pragma once

#include <cstdint>

#include "ts3/layout.h"

namespace ts3::routing {

// The 12-byte route-queue cursor: 0x00B17430's `this`, and the object 0x00B16910 hands
// out at RouteManager + 0x1C. The observed caller pattern (0x00835300, 0x00AD2CB0):
// read entries[+8] as the sentinel via the +4 base, then walk +0 forward until the
// cursor dword equals the sentinel, popping through 0x00B17A60.
struct RouteQueue {
    uint32_t begin;        // +0: cursor dword; == +8 means empty
    const uint32_t* data;  // +4: entry array base (callers read entries[+8] through it)
    uint32_t end;          // +8: one-past-last index / stop value

    // 0x00B17430 (ENTRY, leaf, 13 bytes, 2 callers 0x00835300 0x00AD2CB0): thiscall,
    // plain `ret`, no arguments.
    //   mov eax, [ecx]; xor edx, edx; cmp eax, [ecx+8]; sete dl; mov al, dl; ret
    // Returns 1 when the queue is empty (begin == end), 0 otherwise. EA quirk kept:
    // the original writes only AL of EAX (which had held *this), so the returned
    // dword's upper 24 bits are the `begin` field; every game caller the original instruction sequence
    // only, but the rewrite reproduces the full dword so EAX is bit-identical to the
    // original.
    uint32_t IsEmptyQueue();
};
TS3_X86_SIZE(RouteQueue, 12);

// The object 0x00B16910 is a method of (its callers hold RouteManager instances, e.g.
// Bridge_RouteManager__RouteManager_ResetSteering at 0x00750F10). This unit only ever
// touches the embedded route queue at +0x1C; the prefix is opaque here.
struct RouteManager {
    uint8_t prefix[0x1C];
    RouteQueue queue;  // +0x1C, handed out by GetQueueView (address only)

    // 0x00B16910 (ENTRY, leaf, 12 bytes, 5 callers 0x00750F10 0x00835300 0x00B28A50
    // 0x00B2C040 0x00B2DAA0): thiscall, `ret 4`.
    //   mov eax, [esp+4]; add ecx, 0x1C; mov [eax], ecx; ret 4
    // *out = &self->queue; returns the out parameter itself (EAX = out). The original
    // never dereferences `this` - it only adds 0x1C to the address - so `self` may be
    // any pointer, and the only memory this function writes is the 4 bytes at `out`.
    void* GetQueueView(void* out);
};
TS3_X86_OFFSET(RouteManager, queue, 0x1C);

}  // namespace ts3::routing
