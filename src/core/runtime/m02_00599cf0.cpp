#include "ts3/runtime/m02_00599cf0.h"
namespace ts3::runtime::m02 {
SchedulerWorker* CreateWorker(Scheduler* self,uint32_t config,bool launch,uint32_t index) {
    auto* worker=CallGameFunction<SchedulerWorker*,0x004E3F90>(uint32_t(sizeof(SchedulerWorker)),uintptr_t(0x00FD987C),0u,0u,0u,0u);
    if(!worker) return nullptr;
    CallGameMethod<void,0x004E2920>(&worker->semaphore,0u);
    worker->pending.next=worker->pending.prev=&worker->pending;
    CallGameMethod<void,0x0060AE20>(&worker->thread);
    for(auto& l:worker->lists) l.next=l.prev=&l;
    worker->scheduler=self;
    worker->config=config;
    worker->stopped=0;
    CallGameMethod<uintptr_t,0x004E16F0>(&self->mutex,uintptr_t(0x00FD9744));
    auto* head=&self->queues[10];
    worker->membership.prev=head->prev;
    worker->membership.next=head;
    head->prev=&worker->membership;
    worker->membership.prev->next=&worker->membership;
    if(launch) {
        char name[64];
        CallGameFunction<int,0x004FBF60>(name,64u,uintptr_t(0x00FD9870),index);
        ThreadParameters parameters;
        CallGameMethod<void,0x004E2960>(&parameters);
        parameters.name=name;
        auto wrapper=CallGameFunction<uintptr_t,0x004E2980>();
        CallGameMethod<uintptr_t,0x004E2F30>(&worker->thread,uintptr_t(0x00599200),worker,&parameters,wrapper);
        CallGameMethod<uintptr_t,0x004E2B70>(&worker->thread,name);
        worker->identity=CallGameMethod<uintptr_t,0x004E2AE0>(&worker->thread);
    } else {
        worker->identity=CallGameFunction<uintptr_t,0x004E1270>();
    }
    CallGameMethod<uintptr_t,0x004E17B0>(&self->mutex);
    return worker;
}
}
