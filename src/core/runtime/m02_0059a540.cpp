#include "ts3/runtime/m02_0059a540.h"
#include "ts3/runtime/m02_J1.h"
#include <windows.h>
#include <cstring>
namespace ts3::runtime::m02 {
namespace {
void Unlink(Link& n) { n.prev->next=n.next; n.next->prev=n.prev; }
void Append(Link& h,Link& n) { n.prev=h.prev; n.next=&h; h.prev=&n; n.prev->next=&n; }
}
void CancelTransition(Scheduler* s,Job* job,uint32_t notify) {
    Link& dependencies=job->dependencies[0];
    while(dependencies.prev!=&dependencies) {
        auto* d=reinterpret_cast<j1::Dependency*>(dependencies.prev);
        Job* owner=d->owner;
        Unlink(d->links[1]);
        Unlink(d->links[0]);
        Append(s->lists[0],d->links[0]);
        j1::Propagate(s,owner);
    }
    const uint32_t oldState=job->state;
    Unlink(job->links[0]);
    Append(s->queues[9],job->links[0]); // scheduler +0x80, cancelled queue
    job->state=7;
    CallGameMethod<void,0x005998b0>(s,job); // cancel outgoing dependencies
    if(static_cast<uint8_t>(notify) && job->callback && oldState!=4 && oldState!=5) {
        CallGameMethod<void,0x004e17b0>(&s->mutex);
        // Reload callback and argument after dropping the mutex, as EA does.
        void* argument;
        std::memcpy(&argument,job->reserved14,sizeof(argument));
        using Callback=void (__cdecl*)(void*,void*,uint32_t);
        reinterpret_cast<Callback>(job->callback)(JobView(job),argument,7);
        CallGameMethod<void,0x004e16f0>(&s->mutex,uintptr_t(0x00fd9744));
    }
    if(job->flags[0]) {
        job->flags[0]=0;
        CallGameMethod<void,0x005993f0>(&s->queues[1]);
    }
    if(job->flags[2]) {
        job->flags[2]=0;
        if(InterlockedDecrement(reinterpret_cast<LONG*>(&job->refs))==0)
            CallGameMethod<void,0x0059a650>(job->scheduler,job);
    }
}
}
