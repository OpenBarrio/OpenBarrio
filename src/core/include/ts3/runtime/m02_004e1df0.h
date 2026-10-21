#pragma once
#include "ts3/runtime/m02_threads.h"
namespace ts3::runtime::m02 {
namespace worker1df0 {
// This queue's payload is not the scaffold's generic Work view.
struct Payload { uint32_t id; void* object; void* function; void* argument; };
struct Node { Payload payload; Node* next; Node* prev; };
struct Info { uint8_t running, stop, pad[2]; Thread* thread; ThreadPool* pool; Payload payload; };
TS3_X86_SIZE(Node,24); TS3_X86_SIZE(Info,28);
}
// TS3W.exe 0x004E1DF0: drain work and wait until stopped, reap this worker, and return zero.
int __cdecl PoolWorkerLoop(worker1df0::Info* info);
}
