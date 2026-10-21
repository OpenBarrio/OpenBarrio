#include "ts3/runtime/m02_00599bf0.h"
namespace ts3::runtime::m02 {
namespace {
// EA removes the tail and reloads the head after writing the predecessor.
void PopTail(Link* head) {
    head->prev->prev->next=head;
    head->prev=head->prev->prev;
}
}
void FreeSchedulerResources(Scheduler* scheduler) {
    CallGameVirtual<void,2>(scheduler);
    if (scheduler->reservedJob) {
        CallGameVirtual<void,6>(scheduler,scheduler->reservedJob);
        scheduler->reservedJob=nullptr;
    }
    for (unsigned i=2;i<10;++i) {
        auto* head=&scheduler->queues[i];
        while (head->prev!=head) {
            auto* job=reinterpret_cast<SchedulerCleanupJob*>(head->prev);
            PopTail(head);
            job->detached=0;
            if (job->resource) {
                CallGameVirtual<void,1>(job->resource);
                job->resource=nullptr;
            }
            CallGameFunction<void,0x004E3F70>(job);
        }
    }
    auto* dependencies=&scheduler->lists[0];
    while (dependencies->prev!=dependencies) {
        auto* node=dependencies->prev;
        PopTail(dependencies);
        CallGameFunction<void,0x004E3F70>(node);
    }
    auto* workers=&scheduler->lists[1];
    while (workers->prev!=workers) {
        auto* worker=reinterpret_cast<SchedulerCleanupWorker*>(workers->prev);
        PopTail(workers);
        if (worker) {
            CallGameMethod<void,0x004E26E0>(&worker->thread);
            CallGameFunction<void,0x004E3F70>(worker);
        }
    }
}
}
