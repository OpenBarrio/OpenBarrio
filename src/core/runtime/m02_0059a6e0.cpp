#include "ts3/runtime/m02_0059a6e0.h"
#include "ts3/runtime/m02_J1.h"

namespace ts3::runtime::m02 {
namespace {

// Same mutex ABI the scheduler uses everywhere: Lock takes the debug tag at
// 0xFD9744 and cleans its own argument; Unlock returns the previous lock count.
uintptr_t Lock(Scheduler* s) { return CallGameMethod<uintptr_t,0x4e16f0>(&s->mutex,reinterpret_cast<void*>(uintptr_t(0xfd9744))); }
uintptr_t Unlock(Scheduler* s) { return CallGameMethod<uintptr_t,0x4e17b0>(&s->mutex); }
// 0x0059A540 (transition/cancel under the mutex) is not rewritten yet; the
// original ignores its return on both call sites below, so bridge it as void.
void Cancel(Scheduler* s,Job* j) { CallGameMethod<void,0x59a540>(s,j,uint32_t(1)); }

}

uintptr_t Depend(Scheduler* scheduler,Job* target,Job* waiter) {
    Lock(scheduler);
    const uint32_t state=target->state;  // switch value read right after the lock
    if(state>=1&&state<=5&&state!=3) {   // jump table: {1,2,4,5} share the body
        if(waiter->state==uint32_t(7)) {
            Cancel(scheduler,target);
            return Unlock(scheduler);    // early path skips the 4 -> 5 promotion
        }
        if(waiter->state!=uint32_t(6)) {
            j1::Dependency* d=j1::AllocateLink(scheduler);
            if(d) {
                d->owner=waiter;         // +0x10: job that waits on the target
                d->target=target;        // +0x14: awaited job
                Link& h=target->dependencies[0];  // tail-append into the awaited job's list
                d->links[0].next=&h;
                d->links[0].prev=h.prev;
                h.prev->next=&d->links[0];
                h.prev=&d->links[0];
                Link* at=nullptr;                 // sorted insert into the waiter's list
                j1::LowerRaw(&at,waiter->dependencies[1].next,&waiter->dependencies[1],d);
                Link* node=&d->links[1];
                Link* prev=at->prev;
                at->prev=node;
                prev->next=node;
                node->prev=prev;
                node->next=at;
                // EA quirk: the head pointer is re-read AFTER the insert, so the
                // comparison can never succeed for well-formed circular lists.
                if(at==waiter->dependencies[1].next)
                    j1::Propagate(scheduler,target);
            } else {
                Cancel(scheduler,target);         // allocation failure cancels the target
            }
        }
        if(state==uint32_t(4)) target->state=5;   // pending registration -> registered
    }
    return Unlock(scheduler);
}

}
