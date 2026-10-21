#pragma once
#include "ts3/runtime/m02_threads.h"
namespace ts3::runtime::m02 {
namespace resize2080 {
struct Payload { uint32_t id; void* object; void* function; void* argument; };
struct Info { uint8_t running, stop, pad[2]; Thread* thread; ThreadPool* pool; Payload payload; };
struct Node { Info* info; Node* previous; Node* next; };
TS3_X86_SIZE(Info,28); TS3_X86_SIZE(Node,12);
}
// TS3W.exe 0x004E2080: grow workers or queue stop payloads, returning the final reaper's EAX; allocation failures remain unchecked.
uintptr_t PoolResize(ThreadPool* pool, int32_t desired);
inline uintptr_t __fastcall PoolResizeEntry(ThreadPool* pool, void*, int32_t desired) {
    return PoolResize(pool,desired);
}
}
