#include "ts3/runtime/m02_00599620.h"
#include <cstring>
#include <intrin.h>
namespace ts3::runtime::m02 {
uint32_t AllocateJob(Scheduler* self, void** output) {
    CallGameMethod<uintptr_t,0x004E16F0>(&self->mutex,uintptr_t(0x00FD9744));
    auto* freeHead=&self->queues[2];
    Job* job=nullptr;
    // EA tests the tail for emptiness but removes the head.
    if(freeHead->prev!=freeHead) {
        job=reinterpret_cast<Job*>(freeHead->next);
        freeHead->next->next->prev=freeHead;
        freeHead->next=freeHead->next->next;
    } else {
        job=CallGameFunction<Job*,0x004E3F90>(uint32_t(sizeof(Job)),uintptr_t(0x00FD9844),uint32_t(0),uint32_t(0),uint32_t(0),uint32_t(0));
        if(job) {
            _InterlockedExchange(reinterpret_cast<volatile long*>(&job->refs),0);
            for(auto& list:job->dependencies) list.next=list.prev=&list;
            job->flags[2]=0;
            const uint32_t tag=0x00F97F7A, zero=0;
            std::memcpy(job->reserved14+8,&tag,4);
            std::memcpy(job->reserved14+4,&zero,4);
            job->reserved2c=0;
        }
    }
    if(job) {
        _InterlockedExchangeAdd(reinterpret_cast<volatile long*>(&job->refs),1);
        job->links[1].next=nullptr;
        job->links[1].prev=nullptr;
        job->scheduler=self;
        job->callback=nullptr;
        const uint32_t zero=0;
        std::memcpy(job->reserved14+12,&zero,4);
        job->reserved48=0;
        job->worker=uintptr_t(-1);
        job->state=1;
        job->flags[0]=0;
        job->flags[1]=0;
        auto* active=&self->queues[3];
        job->links[0].prev=active->prev;
        job->links[0].next=active;
        active->prev=&job->links[0];
        job->links[0].prev->next=&job->links[0];
    }
    CallGameMethod<uintptr_t,0x004E17B0>(&self->mutex);
    *output=JobView(job);
    return job!=nullptr;
}
}
