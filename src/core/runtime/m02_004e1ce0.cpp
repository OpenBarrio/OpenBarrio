#include "ts3/runtime/m02_004e1ce0.h"
#include <windows.h>
namespace ts3::runtime::m02 {
uintptr_t PoolReapRecord(ThreadPool* pool, ThreadInfo* info) {
    using reap1ce0::Node;
    auto head=reinterpret_cast<Node*>(pool->threads.sentinel.next);
    auto end=reinterpret_cast<Node*>(pool->threads.sentinel.prev);
    auto node=head->prev;
    while(node!=end && node->info!=info) node=node->prev;
    if(node==end) return reinterpret_cast<uintptr_t>(end);
    if(info->thread) {
        auto thread=info->thread;
        CallGameMethod<void,0x004E2E10>(thread);
        CallGameFunction<uintptr_t,0x004E3F70>(thread);
    }
    info->thread=nullptr;
    // The destructor may have changed the list. EA tests emptiness again,
    // but does not search again and does not check the worker's running flag.
    head=reinterpret_cast<Node*>(pool->threads.sentinel.next);
    if(head->prev!=reinterpret_cast<Node*>(pool->threads.sentinel.prev)) {
        node->prev->next=node->next;
        node->next->prev=node->prev;
        --pool->threads.count;
        CallGameFunction<uintptr_t,0x004E3F70>(node);
    }
    auto result=CallGameFunction<uintptr_t,0x004E3F70>(info);
    // xadd's old value is in ECX: EAX remains the allocator result.
    InterlockedExchangeAdd(reinterpret_cast<LONG*>(&pool->live),-1);
    return result;
}
}
