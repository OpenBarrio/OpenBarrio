#include "ts3/runtime/m02_0059a8b0.h"
#include "ts3/runtime/m02_004e29a0.h"

namespace ts3::runtime::m02 {
namespace {

// Same mutex ABI the scheduler uses everywhere: Lock takes the debug tag at
// 0xFD9744; Unlock returns the previous lock count. Both are unrewritten leaves.
uintptr_t Lock(Scheduler* s) { return CallGameMethod<uintptr_t,0x4e16f0>(&s->mutex,reinterpret_cast<void*>(uintptr_t(0xfd9744))); }
uintptr_t Unlock(Scheduler* s) { return CallGameMethod<uintptr_t,0x4e17b0>(&s->mutex); }
// 0x0059A540 (job cancel transition under the mutex) and 0x0059A220 (block on a
// job's completion) are still todo singles; the caller ignores both returns.
void Cancel(Scheduler* s,Job* j) { CallGameMethod<void,0x59a540>(s,j,uint32_t(1)); }
void WaitOut(Scheduler* s,Job* j) { CallGameMethod<void,0x59a220>(s,j); }
// 0x004E2810 (semaphore wake/release by count) is unrewritten as well.
void Wake(t2::Semaphore* sem,int32_t count) { CallGameMethod<void,0x4e2810>(sem,count); }

}

uintptr_t ShutdownWorker(Scheduler* scheduler,Worker* worker) {
    Lock(scheduler);
    // Cancel pending jobs front-first; each cancel unlinks its node from the list.
    // List nodes are raw Job records chained by links[0] (not public views).
    while(worker->pending.prev!=&worker->pending)
        Cancel(scheduler,reinterpret_cast<Job*>(worker->pending.next));
    // Drain only while the thread is still running (Poll() == exactly 1); a done or
    // already-joined worker skips the whole drain/join block but is still torn down.
    if(t2::Poll(&worker->thread,nullptr)==uint32_t(1)) {
        // Wait out queued jobs back-first (EA takes head.prev here, head.next above).
        while(worker->drain.prev!=&worker->drain)
            WaitOut(scheduler,reinterpret_cast<Job*>(worker->drain.prev));
        worker->stop=1;
        if(worker->queued.next!=&worker->queued) {
            // Splice the whole list into a standalone circular chain: last->next=first,
            // first->prev=last; the sentinel empties and the jobs stay orphaned (kept).
            Link* first=worker->queued.next;
            Link* last=worker->queued.prev;
            last->next=first;
            first->prev=last;
            worker->queued.next=&worker->queued;
            worker->queued.prev=&worker->queued;
            Wake(&worker->semaphore,1);
        }
        Unlock(scheduler);
        // EA quirk: the "deadline" is the debug-tag bytes at 0xFD9744 dereferenced.
        TimedJoin(&worker->thread,reinterpret_cast<const uint32_t*>(uintptr_t(0xfd9744)),nullptr);
        Lock(scheduler);
    }
    // Unlink the worker from the scheduler's worker list.
    worker->node.prev->next=worker->node.next;
    worker->node.next->prev=worker->node.prev;
    Unlock(scheduler);
    t2::Release(&worker->thread);
    t2::SemaphoreDestroy(&worker->semaphore);
    return CallGameFunction<uintptr_t,0x4e3f70>(worker);
}

}
