// Unit clu_b16910_Routing: TS3W.exe 0x00B16910 / 0x00B17430 (see the header for the
// map). Both are pure address/compare leaves of the route queue - no game calls, no
// globals, portable pointer arithmetic only.
#include "ts3/routing/clu_b16910_Routing.h"

namespace ts3::routing {

void* RouteManager::GetQueueView(void* out)
{
    // 0x00B16910
    // Writes &self->queue through the out parameter and returns the out parameter
    // itself; `this` is never dereferenced, only its address is used.
    *static_cast<RouteQueue**>(out) = &queue;
    return out;
}

uint32_t RouteQueue::IsEmptyQueue()
{
    // 0x00B17430
    //           mov al, dl / ret.
    const uint32_t head = begin;
    const uint32_t eq = (head == end) ? 1u : 0u;
    // EA quirk kept: the original only overwrites AL of EAX (which had held `begin`),
    // so the returned dword is (begin & 0xFFFFFF00) | (begin == end). All four game
    // callers the original instruction sequence only, but the full dword is reproduced so EAX is
    // bit-identical to the original.
    return (head & 0xFFFFFF00u) | eq;
}

}  // namespace ts3::routing
