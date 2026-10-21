#include "ts3/runtime/m02_0059a220.h"
namespace ts3::runtime::m02 {
namespace {
struct WaitNode { Link link; Semaphore semaphore; };
void Before(Link* head,Link* node) {
    node->prev=head->prev; node->next=head; head->prev=node; node->prev->next=node;
}
}
uintptr_t __fastcall WaitForJob(Scheduler* s,void*,Job* job) {
    const uintptr_t identity=CallGameFunction<uintptr_t,0x004E1270>();
    DispatchWorker* worker=nullptr;
    auto* workers=&s->queues[10];
    for(auto* p=workers->next;p!=workers;p=p->next) {
        // The identity is at +24 in the worker record, not in its wait link.
        auto* candidate=reinterpret_cast<DispatchWorker*>(p);
        if(candidate->reserved20[1]==identity) { worker=candidate; break; }
    }
    WaitNode* node;
    auto* pool=&s->lists[1];
    if(worker) node=reinterpret_cast<WaitNode*>(&worker->waiting);
    else if(pool->prev==pool) {
#if defined(_M_IX86)
        node=CallGameFunction<WaitNode*,0x004E3F90>(uint32_t(24),reinterpret_cast<const char*>(uintptr_t(0x00FD97EC)),uint32_t(0),uint32_t(0),uint32_t(0),uint32_t(0));
#else
        AbortNotRewritten("JobManager wait-node allocator"); node=nullptr;
#endif
        if(node) {
            CallGameMethod<void,0x004E2920>(&node->semaphore,uint32_t(0));
            node->link.next=node->link.prev=&node->link;
        }
    } else {
        node=reinterpret_cast<WaitNode*>(pool->next);
        node->link.next->prev=pool; pool->next=node->link.next;
    }
    // Volatile loads preserve the original separate observations of asynchronous state.
    auto& state=*reinterpret_cast<volatile uint32_t*>(&job->state);
    uintptr_t result=state;
    while(result!=7 && state!=6 && state!=0 && state!=1) {
        job->flags[0]=1;
        if(!worker || !CallGameMethod<uint8_t,0x00599E60>(s,worker,static_cast<DeadlineTimer*>(nullptr),uint32_t(1),uint32_t(1))) {
            Before(&s->queues[1],&node->link);
            CallGameMethod<void,0x004E17B0>(&s->mutex);
            CallGameMethod<void,0x004E2760>(&node->semaphore,uintptr_t(0x00FD9744));
            CallGameMethod<void,0x004E16F0>(&s->mutex,uintptr_t(0x00FD9744));
        }
        result=state;
    }
    if(!worker) {
        node->link.next=pool->next; node->link.prev=pool;
        pool->next=&node->link; node->link.next->prev=&node->link;
        return reinterpret_cast<uintptr_t>(pool);
    }
    // The final failed terminal check may have loaded ECX/EDX, not EAX.
    return result;
}
}
