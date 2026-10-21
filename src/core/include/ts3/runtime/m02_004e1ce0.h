#pragma once
#include "ts3/runtime/m02_layouts.h"
namespace ts3::runtime::m02 {
namespace reap1ce0 {
struct Node { ThreadInfo* info; Node* next; Node* prev; };
TS3_X86_SIZE(Node,12);
}
// TS3W.exe 0x004E1CE0: remove the specified worker record, preserving sentinel/free EAX and the post-destructor empty-list check.
uintptr_t PoolReapRecord(ThreadPool* pool, ThreadInfo* info);
#if defined(_M_IX86)
inline uintptr_t __fastcall PoolReapRecordEntry(ThreadPool* pool, void*, ThreadInfo* info) { return PoolReapRecord(pool,info); }
#endif
}
