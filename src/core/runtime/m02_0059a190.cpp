#include "ts3/runtime/m02_0059a190.h"
#include <intrin.h>
namespace ts3::runtime::m02 {
uintptr_t ScheduleEligibleJob(Scheduler* self, Job* job) {
    CallGameMethod<uintptr_t,0x004E16F0>(&self->mutex,uintptr_t(0x00FD9744));
    if(job->state==1) {
        auto* dependencies=&job->dependencies[0];
        // EA checks only the previous link for emptiness.
        if(dependencies->prev==dependencies) {
            CallGameMethod<uintptr_t,0x00599FC0>(self,job);
            return CallGameMethod<uintptr_t,0x004E17B0>(&self->mutex);
        }
        if(job->flags[2]==0) {
            job->flags[2]=1;
            _InterlockedExchangeAdd(reinterpret_cast<volatile long*>(&job->refs),1);
        }
        auto* node=&job->links[0];
        node->prev->next=node->next;
        node->next->prev=node->prev;
        auto* head=&self->queues[4];
        job->state=2;
        node->prev=head->prev;
        node->next=head;
        head->prev=node;
        node->prev->next=node;
    }
    return CallGameMethod<uintptr_t,0x004E17B0>(&self->mutex);
}
}
