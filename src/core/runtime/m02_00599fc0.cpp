#include "ts3/runtime/m02_00599fc0.h"
#include <intrin.h>
#include <cstring>
namespace ts3::runtime::m02 {
namespace {
// Worker record's intervening fields are opaque to this entry.
struct Worker { Link membership, waiters; uint8_t opaque10[0x1c]; uint32_t mask; Link ready; };
void Unlink(Link* p) { p->prev->next=p->next; p->next->prev=p->prev; }
void Append(Link* head,Link* p) { p->prev=head->prev; p->next=head; head->prev=p; p->prev->next=p; }
void Retain(Job* job) {
    // x86 shifts use only CL's bottom five bits; only bits 2..5 of AL matter.
    if(!((uint32_t(1)<<(job->state&31))&0x3c) && !job->flags[2]) {
        job->flags[2]=1;
        _InterlockedExchangeAdd(reinterpret_cast<volatile long*>(&job->refs),1);
    }
}
uintptr_t Wake(Link* node) {
    Unlink(node); node->next=node->prev=node;
    return CallGameMethod<uintptr_t,0x004E2810>(node+1,uint32_t(1));
}
}
uintptr_t RouteJob(Scheduler* self,Job* job) {
    if(!job->callback) {
        Retain(job);
        if(job->state==5) job->state=4;
        Unlink(&job->links[0]); Append(&self->queues[6],&job->links[0]);
        return CallGameMethod<uintptr_t,0x00599720>(self,job);
    }
    if(++self->state==0) ++self->trailing;
    // The second link pair is a sequence number while the job is queued.
    uint32_t serial[2]={self->state,self->trailing};
    std::memcpy(&job->links[1],serial,sizeof(serial));
    if(job->worker==uintptr_t(-1)) {
        Retain(job);
        Unlink(&job->links[0]);
        CallGameMethod<uintptr_t,0x00599920>(self,&self->queues[5],job);
        job->state=3;
        auto* first=&self->queues[0];
        if(first->prev!=first) {
            auto* p=first->next;
            if(p->prev!=p) Wake(p);
        }
        auto* second=&self->queues[1];
        if(second->prev!=second) {
            auto* p=second->next;
            if(p->prev!=p) return Wake(p);
            return reinterpret_cast<uintptr_t>(p);
        }
        return reinterpret_cast<uintptr_t>(second);
    }
    auto* head=&self->queues[10];
    Worker* worker=nullptr;
    for(auto* p=head->next;p!=head;p=p->next) {
        auto* candidate=reinterpret_cast<Worker*>(p);
        if(candidate->mask & job->worker) { worker=candidate; break; }
    }
    if(!worker) return CallGameMethod<uintptr_t,0x0059A540>(self,job,uint32_t(1));
    Retain(job);
    Unlink(&job->links[0]);
    CallGameMethod<uintptr_t,0x00599920>(self,&worker->ready,job);
    job->state=3;
    auto* wait=&worker->waiters;
    if(wait->prev!=wait) return Wake(wait);
    return reinterpret_cast<uintptr_t>(wait);
}
uintptr_t __fastcall RouteJobEntry(Scheduler* self,void*,Job* job) { return RouteJob(self,job); }
}
