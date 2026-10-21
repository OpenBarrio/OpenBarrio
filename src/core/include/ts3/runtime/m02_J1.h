#pragma once
#include "ts3/runtime/m02_jobs.h"
namespace ts3::runtime::m02::j1 {
struct Dependency { Link links[2]; Job* owner; Job* target; };
// TS3W.exe 0x00599170: Create the reserved job and jobs selected by the priority mask.
bool Initialize(Scheduler*,uint32_t);
// TS3W.exe 0x005991D0: Stamp the raw job with the current thread identity under the mutex.
uintptr_t Stamp(Scheduler*,Job*);
// TS3W.exe 0x00599220: Read public job state, mapping internal state 5 to 4; null still faults.
uint32_t State(void*);
// TS3W.exe 0x00599290: Store the cancel-propagation byte on a public job; null still faults.
void SetCancel(void*,uint8_t);
// TS3W.exe 0x00599420: Pop a dependency from the free list or allocate 24 bytes.
Dependency* AllocateLink(Scheduler*);
// TS3W.exe 0x00599480: Unlock before setting the externally-held flag and incrementing refs.
uintptr_t RetainExternal(void*);
// TS3W.exe 0x005994F0: Construct scheduler mutex and circular sentinel lists.
Scheduler* Construct(Scheduler*);
// TS3W.exe 0x005995A0: Destroy the mutex, install abstract vtable, optionally free.
Scheduler* Destroy(Scheduler*,uint32_t);
// TS3W.exe 0x00599920: Insert by signed effective priority, preserving EA's reverse scan/tie order.
Link* __stdcall Insert(Link*,Job*);
// TS3W.exe 0x00599970: Construct Service and its embedded scheduler.
JobService* ConstructService(JobService*,uint8_t);
// TS3W.exe 0x00599A10: Dispatch current job with an extended deadline timer; empty path has no return contract.
uintptr_t Update(JobService*,uint32_t,uint32_t);
// TS3W.exe 0x00599AC0: Find descending lower bound using a raw dependency allocation.
Link** LowerRaw(Link**,Link*,Link*,Dependency*);
// TS3W.exe 0x00599B60: Find descending lower bound using the dependency's second-link view.
Link** LowerView(Link**,Link*,Link*,Link*);
// TS3W.exe 0x00599E00: Find the first queued job whose mask intersects the requested mask.
Job* Find(Scheduler*,uint32_t);
// TS3W.exe 0x0059A350: Invoke the scheduler operation for a public job.
uintptr_t Invoke(void*);
// TS3W.exe 0x0059A380: Reposition a dependency after its target priority changes.
bool Reorder(Link*,Link*,const uint8_t*);
// TS3W.exe 0x0059A430: Configure a job under the mutex, preserving unlock's upper return bits.
uintptr_t Configure(Scheduler*,Job*,void*,uint32_t);
// TS3W.exe 0x0059A470: Propagate effective signed priority through dependency lists.
void Propagate(Scheduler*,Job*);
// TS3W.exe 0x0059A500: Operate on a public job under its scheduler mutex.
uintptr_t Operate(void*);
// TS3W.exe 0x0059A800: Decrement raw job refs and recycle on new value zero.
int32_t ReleaseRaw(Job*);
// TS3W.exe 0x0059A820: Set base priority under the mutex and propagate only on change.
uintptr_t SetPriority(void*,int32_t);
// TS3W.exe 0x0059A870: Convert public views and invoke the scheduler dependency operation; first view must be nonnull.
uintptr_t Relate(void*,void*);
// TS3W.exe 0x0059A990: Decrement public job refs and lock only when recycling.
int32_t Release(void*);

// TS3W.exe 0x00599170: ABI entry for Initialize.
bool __fastcall InitializeEntry(Scheduler* s,void*,uint32_t mask);
// TS3W.exe 0x005991D0: ABI entry for Stamp.
uintptr_t __fastcall StampEntry(Scheduler* s,void*,Job* j);
// TS3W.exe 0x00599220: ABI entry for State.
uint32_t __fastcall StateEntry(void* view,void*);
// TS3W.exe 0x00599290: ABI entry for SetCancel.
void __fastcall SetCancelEntry(void* view,void*,uint8_t flag);
// TS3W.exe 0x00599420: ABI entry for AllocateLink.
Dependency* __fastcall AllocateLinkEntry(Scheduler* s,void*);
// TS3W.exe 0x00599480: ABI entry for RetainExternal.
uintptr_t __fastcall RetainExternalEntry(void* view,void*);
// TS3W.exe 0x005994F0: ABI entry for Construct.
Scheduler* __fastcall ConstructEntry(Scheduler* s,void*);
// TS3W.exe 0x005995A0: ABI entry for Destroy.
Scheduler* __fastcall DestroyEntry(Scheduler* s,void*,uint32_t flags);
// TS3W.exe 0x00599970: ABI entry for ConstructService.
JobService* __fastcall ConstructServiceEntry(JobService* s,void*,uint8_t flag);
// TS3W.exe 0x00599A10: ABI entry for Update.
uintptr_t __fastcall UpdateEntry(JobService* s,void*,uint32_t a,uint32_t b);
// TS3W.exe 0x00599E00: ABI entry for Find.
Job* __fastcall FindEntry(Scheduler* s,void*,uint32_t mask);
// TS3W.exe 0x0059A350: ABI entry for Invoke.
uintptr_t __fastcall InvokeEntry(void* view,void*);
// TS3W.exe 0x0059A430: ABI entry for Configure.
uintptr_t __fastcall ConfigureEntry(Scheduler* s,void*,Job* j,void* timer,uint32_t flag);
// TS3W.exe 0x0059A470: ABI entry for Propagate.
void __fastcall PropagateEntry(Scheduler* s,void*,Job* j);
// TS3W.exe 0x0059A500: ABI entry for Operate.
uintptr_t __fastcall OperateEntry(void* view,void*);
// TS3W.exe 0x0059A800: ABI entry for ReleaseRaw.
int32_t __fastcall ReleaseRawEntry(Job* j,void*);
// TS3W.exe 0x0059A820: ABI entry for SetPriority.
uintptr_t __fastcall SetPriorityEntry(void* view,void*,int32_t p);
// TS3W.exe 0x0059A870: ABI entry for Relate.
uintptr_t __fastcall RelateEntry(void* a,void*,void* b);
// TS3W.exe 0x0059A990: ABI entry for Release.
int32_t __fastcall ReleaseEntry(void* view,void*);
}
