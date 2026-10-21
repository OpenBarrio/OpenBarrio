#include "ts3/runtime/m02_004e1c00.h"
#include <intrin.h>
namespace ts3::runtime::m02 {
namespace {
struct ThreadNode { ThreadInfo* info; ThreadNode* next; ThreadNode* prev; };
ThreadNode* First(ThreadPool* p) { return reinterpret_cast<ThreadNode*>(p->threads.sentinel.next)->prev; }
ThreadNode* End(ThreadPool* p) { return reinterpret_cast<ThreadNode*>(p->threads.sentinel.prev); }
}
uint32_t PoolShutdown(ThreadPool* p, uint32_t predicate, const uint32_t* deadline) {
    if(p->active == 0) return 1;
    p->active=0;
    const uint32_t result=CallGameMethod<uint32_t,0x004e1960>(p,UINT32_MAX,predicate,deadline);
#if defined(_M_IX86)
    const auto infinite=reinterpret_cast<const uint32_t*>(0x00fb8960);
#else
    const uint32_t infinity=UINT32_MAX; const auto infinite=&infinity;
#endif
    CallGameMethod<int32_t,0x004e16f0>(&p->mutex,infinite);
    if(predicate == 0) CallGameMethod<void,0x004e18e0>(&p->work);
    for(auto node=First(p);node!=End(p);node=node->prev) node->info->stop=1;
    CallGameMethod<void,0x004e1590>(&p->condition,uint32_t(1));
    CallGameMethod<int32_t,0x004e17b0>(&p->mutex);
    // Unlike the initial predicate wait, this drain is unconditional and unbounded.
    while(First(p)!=End(p)) {
        uint32_t delay=1;
        CallGameFunction<void,0x004e1320>(&delay);
    }
    _InterlockedExchange(reinterpret_cast<volatile long*>(&p->waiters),0);
    return (result & 0xffffff00u) | uint32_t(result == 0);
}
}
