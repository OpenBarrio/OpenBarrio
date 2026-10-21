#pragma once
#include "ts3/runtime/m02_layouts.h"
namespace ts3::runtime::m02 {
// Payload-first list nodes used by the pool (links are at the END of the node).
namespace wait1960 {
struct WorkNode { Work work; WorkNode* next; WorkNode* prev; };
struct ThreadNode { ThreadInfo* info; ThreadNode* next; ThreadNode* prev; };
}
// TS3W.exe 0x004E1960: poll a work identity or pool completion until an absolute deadline; -1 selects aggregate predicates, not an infinite timeout.
int32_t PoolPredicateWait(ThreadPool* pool, uint32_t identity, uint32_t predicate, const uint32_t* deadline);
inline int32_t __fastcall PoolPredicateWaitEntry(ThreadPool* pool, void*, uint32_t identity, uint32_t predicate, const uint32_t* deadline) {
    return PoolPredicateWait(pool,identity,predicate,deadline);
}
}
