#pragma once
#include "ts3/runtime/m02_layouts.h"
#include "ts3/runtime/m02_T2.h"
namespace ts3::runtime::m02 {
// JobManager scheduler worker record (x86 offsets; built by the worker spawn path,
// chained through the scheduler's worker list). Nodes of "pending" and "drain" are
// Job records chained by links[0]; "queued" nodes are detached as an orphan circular
// chain on shutdown and never freed by this function (EA leak-by-design, kept).
struct Worker {
    Link node;           // +0x00: in the scheduler's worker list (unlink on shutdown)
    Link queued;         // +0x08: jobs handed to the worker; spliced out on shutdown
    t2::Semaphore semaphore; // +0x10: 16-byte wake semaphore (local byte at +0xC)
    Thread thread;       // +0x20: worker thread facade (record* at +0)
    uint32_t pad24[3];   // +0x24: tail of the embedded facade block
    Link pending;        // +0x30: jobs to cancel, cancelled front-first (head.next)
    Link drain;          // +0x38: jobs to wait out, drained back-first (head.prev)
    uint8_t stop;        // +0x40: stop flag forced to 1 before the queue splice
    uint8_t pad41[3];
};
TS3_X86_OFFSET(Worker,node,0); TS3_X86_OFFSET(Worker,queued,8);
TS3_X86_OFFSET(Worker,semaphore,0x10); TS3_X86_OFFSET(Worker,thread,0x20);
TS3_X86_OFFSET(Worker,pending,0x30); TS3_X86_OFFSET(Worker,drain,0x38);
TS3_X86_OFFSET(Worker,stop,0x40);
// TS3W.exe 0x0059A8B0: Under the scheduler mutex cancel the worker's pending jobs
// (front-first through the still-unrewritten 0x0059A540); when the worker thread is
// still running (Poll() == exactly 1), wait out its drain list back-first through the
// unrewritten 0x0059A220, force the stop byte, splice the queued-job list into an
// orphan circular chain, release its semaphore (unrewritten 0x004E2810), then join outside
// the mutex with TimedJoin's EA quirk deadline (the 0xFD9744 tag bytes) and re-lock.
// Finally unlink the worker, release its thread record and semaphore, free it and
// return the free adapter's EAX. TimedJoin/Poll/Release/SemaphoreDestroy are verified
// rewrites and called directly; the four unrewritten leaves go through the bridge.
uintptr_t ShutdownWorker(Scheduler* scheduler,Worker* worker);
// ABI entry for the thiscall (ret 4) scheduler vtable slot 6 operation.
inline uintptr_t __fastcall ShutdownWorkerEntry(Scheduler* scheduler,void*,Worker* worker) {
    return ShutdownWorker(scheduler,worker);
}
}
