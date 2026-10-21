#include "ts3/runtime/m02_00599720.h"
#include <windows.h>
#include <cstring>
namespace ts3::runtime::m02 {
namespace {
struct Dependency { Link links[2]; Job* source; Job* target; };
void Unlink(Link* l) { l->prev->next=l->next; l->next->prev=l->prev; }
void Append(Link* h,Link* l) { l->prev=h->prev; l->next=h; h->prev=l; l->prev->next=l; }
}
uintptr_t CompleteJob(Scheduler* self,Job* job) {
    if(job->callback) {
        job->state=4;
        CallGameMethod<void,0x4e17b0>(&self->mutex);
        void* context; std::memcpy(&context,job->reserved14,sizeof(context));
        using Callback=uintptr_t(__cdecl*)(void*,void*,uint32_t);
        auto accepted=reinterpret_cast<Callback>(job->callback)(JobView(job),context,4);
        CallGameMethod<void,0x4e16f0>(&self->mutex,reinterpret_cast<void*>(uintptr_t(0xfd9744)));
        if(!(accepted&0xff)) return CallGameMethod<uintptr_t,0x59a540>(self,job,uint32_t(0));
    }
    if(job->state==7) return CallGameMethod<uintptr_t,0x59a540>(self,job,uint32_t(0));
    if(job->dependencies[0].prev!=&job->dependencies[0]) {
        job->links[1]={nullptr,nullptr}; job->state=2;
        Unlink(&job->links[0]); Append(&self->queues[4],&job->links[0]);
        return reinterpret_cast<uintptr_t>(&self->queues[4]);
    }
    if(job->state==5) {
        job->links[1]={nullptr,nullptr};
        return CallGameMethod<uintptr_t,0x599fc0>(self,job);
    }
    job->state=6; Unlink(&job->links[0]); Append(&self->queues[8],&job->links[0]);
    while(job->dependencies[1].prev!=&job->dependencies[1]) {
        auto dep=reinterpret_cast<Dependency*>(reinterpret_cast<uint8_t*>(job->dependencies[1].prev)-sizeof(Link));
        auto target=dep->target;
        Unlink(&dep->links[1]); Unlink(&dep->links[0]); Append(&self->lists[0],&dep->links[0]);
        if(target->dependencies[0].prev==&target->dependencies[0] && target->state==2)
            CallGameMethod<void,0x599fc0>(self,target);
    }
    uintptr_t result=reinterpret_cast<uintptr_t>(&job->dependencies[1]);
    if(job->flags[0]) { job->flags[0]=0; result=CallGameMethod<uintptr_t,0x5993f0>(&self->queues[1]); }
    job->links[1]={nullptr,nullptr}; job->flags[2]=0;
    auto old=InterlockedExchangeAdd(reinterpret_cast<LONG*>(&job->refs),-1);
    if(old==1) return CallGameMethod<uintptr_t,0x59a650>(job->scheduler,job);
    return result;
}
uintptr_t __fastcall CompleteJobEntry(Scheduler* self,void*,Job* job) { return CompleteJob(self,job); }
}
