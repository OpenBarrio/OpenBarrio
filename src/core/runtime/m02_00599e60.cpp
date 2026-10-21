#include "ts3/runtime/m02_00599e60.h"
#include <windows.h>
namespace ts3::runtime::m02 {
namespace {
void Unlink(Link* n) { n->prev->next=n->next; n->next->prev=n->prev; }
void Before(Link* h,Link* n) { n->prev=h->prev; n->next=h; h->prev=n; n->prev->next=n; }
int32_t Priority(Link* n) { return static_cast<int32_t>(reinterpret_cast<Job*>(n)->reserved48); }
}
uint8_t DispatchEligible(Scheduler* s,DispatchWorker* w,DeadlineTimer* timer,uint32_t noWait,uint32_t single,const PlatformDispatch* clock) {
    const bool takeGlobal=w->priority<0;
    uint8_t dispatched=0;
    while(!w->stopped) {
        auto* h=&w->ready;
        int32_t priority=h->prev==h?INT32_MIN:Priority(h->next);
        auto* global=&s->queues[5];
        if(takeGlobal && global->prev!=global && Priority(global->next)>priority) {
            auto* n=global->next; Unlink(n);
            // Prepend, not append: only one global head is considered per iteration.
            n->next=h->next; n->prev=h; h->next=n; n->next->prev=n;
        }
        if(h->prev!=h) {
            auto* n=h->next;
            auto* selected=&w->selected->links[0];
            if(n!=selected) { Unlink(n); Before(selected,n); }
            CallGameMethod<void,0x00599720>(s,w->selected);
            dispatched=1;
            if(static_cast<uint8_t>(single)) return dispatched;
            if(timer) {
                int64_t now;
                if(clock) clock->counter(clock->context,&now);
                else QueryPerformanceCounter(reinterpret_cast<LARGE_INTEGER*>(&now));
                // EA subtracts modulo 2^64 and tests the resulting high word's sign.
                if((timer->deadline-static_cast<uint64_t>(now))>>63) return dispatched;
            }
        } else {
            if(static_cast<uint8_t>(noWait)) return dispatched;
            Before(&s->queues[0],&w->waiting);
            CallGameMethod<void,0x004E17B0>(&s->mutex);
            CallGameMethod<void,0x004E2760>(&w->semaphore,uintptr_t(0x00FD9744));
            CallGameMethod<void,0x004E16F0>(&s->mutex,uintptr_t(0x00FD9744));
        }
    }
    return dispatched;
}
uint8_t __fastcall DispatchEligibleEntry(Scheduler* s,void*,DispatchWorker* w,DeadlineTimer* timer,uint32_t noWait,uint32_t single) {
    return DispatchEligible(s,w,timer,noWait,single);
}
}
