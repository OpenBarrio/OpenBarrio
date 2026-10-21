#include "ts3/runtime/m02_004e2080.h"
#include <windows.h>
namespace ts3::runtime::m02 {
uintptr_t PoolResize(ThreadPool* pool, int32_t desired) {
    using namespace resize2080;
    int32_t difference=static_cast<int32_t>(uint32_t(desired)-uint32_t(pool->live));
#if defined(_M_IX86)
    auto infoName=reinterpret_cast<const char*>(0x00FB8994);
    auto threadName=reinterpret_cast<const char*>(0x00FB8978);
    auto nodeName=reinterpret_cast<const char*>(0x00FB8964);
    auto infinite=reinterpret_cast<const uint32_t*>(0x00FB8960);
    auto entry=reinterpret_cast<void*>(0x004E1DF0);
#else
    const char* infoName="ThreadInfo"; const char* threadName="Thread"; const char* nodeName="ThreadNode";
    const uint32_t never=0xffffffffu; auto infinite=&never;
    void* entry=nullptr; // Only the x86 original bridge can launch image workers.
#endif
    while(difference>0) {
        auto info=CallGameFunction<Info*,0x004E3F90>(sizeof(Info),infoName,0,0,0,0);
        if(info) {
            info->running=0; info->thread=nullptr; info->pool=nullptr; info->stop=0;
            info->payload.object=nullptr; info->payload.function=nullptr; info->payload.argument=nullptr;
        }
        // EA dereferences failed allocations; do not invent rollback or failure returns.
        info->running=0; info->stop=0; info->pool=pool;
        auto thread=CallGameFunction<Thread*,0x004E3F90>(sizeof(Thread),threadName,0,0,0,0);
        info->thread=thread?CallGameMethod<Thread*,0x0060AE20>(thread):nullptr;
        auto node=CallGameFunction<Node*,0x004E3F90>(sizeof(Node),nodeName,0,0,0,0);
        node->info=info;
        auto end=reinterpret_cast<Node*>(pool->threads.sentinel.prev);
        node->previous=end->previous; node->next=end;
        node->previous->next=node; end->previous=node; ++pool->threads.count;
        InterlockedExchangeAdd(reinterpret_cast<LONG*>(&pool->live),1);
        auto wrapper=CallGameFunction<void*,0x004E2980>();
        CallGameMethod<uintptr_t,0x004E2F30>(info->thread,entry,info,&pool->parameters,wrapper);
        --difference;
    }
    if(difference<0) {
        uint32_t remaining=0u-uint32_t(difference);
        Payload stop; stop.object=nullptr; stop.function=nullptr; stop.argument=nullptr;
        // The first payload word is deliberately uninitialized by the image.
        do {
            if(pool->active) {
                CallGameMethod<int,0x004E16F0>(&pool->mutex,infinite);
                if(pool->busy>=pool->live && pool->live<pool->maximum)
                    PoolResize(pool,static_cast<int32_t>(uint32_t(pool->live)+1));
                CallGameMethod<void,0x004E1B80>(&pool->work,&stop);
                CallGameMethod<uintptr_t,0x004E1F30>(pool);
                if(!pool->waiters) CallGameMethod<int,0x004E1590>(&pool->condition,0);
                CallGameMethod<int,0x004E17B0>(&pool->mutex);
            }
        } while(--remaining);
    }
    return CallGameMethod<uintptr_t,0x004E1F30>(pool);
}
}
