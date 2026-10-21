#include "ts3/runtime/m02_004e1df0.h"
#include <windows.h>
namespace ts3::runtime::m02 {
int __cdecl PoolWorkerLoop(worker1df0::Info* info) {
    using worker1df0::Node;
    auto pool=info->pool;
    auto mutex=&pool->mutex;
    // The original lock accepts an absolute-time pointer to this static constant.
#if defined(_M_IX86)
    auto infinite=reinterpret_cast<const uint32_t*>(0x00FB8960);
#else
    const uint32_t never=0xffffffffu; auto infinite=&never;
#endif
    CallGameMethod<int,0x004E16F0>(mutex,infinite);
    while(!info->stop) {
        auto head=reinterpret_cast<Node*>(pool->work.sentinel.next);
        auto end=reinterpret_cast<Node*>(pool->work.sentinel.prev);
        if(head->prev!=end) {
            info->payload=head->prev->payload;
            // Re-read the queue after copying, just as the image does.
            auto node=head->prev;
            if(node!=end) {
                head->prev=node->prev;
                node->prev->next=head;
                CallGameFunction<uintptr_t,0x004E3F70>(node);
                --pool->work.count;
            }
            info->running=1;
            InterlockedExchangeAdd(reinterpret_cast<LONG*>(&pool->busy),1);
            CallGameMethod<int,0x004E17B0>(mutex);
            if(info->payload.object) {
                CallGameVirtual<void,1>(info->payload.object,info->payload.argument);
            } else if(info->payload.function) {
                using Callback=void(__cdecl*)(void*);
                reinterpret_cast<Callback>(info->payload.function)(info->payload.argument);
            } else {
                info->stop=1;
            }
            CallGameMethod<int,0x004E16F0>(mutex,infinite);
            InterlockedExchangeAdd(reinterpret_cast<LONG*>(&pool->busy),-1);
            info->running=0;
        } else {
            uint32_t deadline=CallGameFunction<uint32_t,0x004E1330>()+pool->idleTimeout;
            if(CallGameMethod<int,0x004E14C0>(&pool->condition,mutex,&deadline)!=0)
                info->stop=1;
        }
    }
    CallGameMethod<uintptr_t,0x004E1CE0>(pool,info);
    CallGameMethod<int,0x004E17B0>(mutex);
    return 0;
}
}
