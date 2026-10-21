#include "ts3/jobmanager/clu_599460_JobManager.h"

#include <atomic>
#include <cstring>

namespace ts3::jobmanager {
namespace {

// ---------------------------------------------------------------------------
// calls into the parts of the JobManager this unit does not own
// ---------------------------------------------------------------------------

// 0x0059A540, __thiscall, ret 8: cancels `state` for real. It drains the state's
// DepsToCancel list into the manager's free list, moves the state onto the manager's
// active list, stores 7 into state->state, recursively drains the state's dep list
// (0x005998B0), optionally invokes state->callback(state+8, state->callbackArg, 7) under
// the manager's lock, and runs the two tail cleanups behind field4C / field4E.
void BridgeCancelState(JobManager* self, JobState* state, uint32_t flag)
{
    CallGameMethod<void, 0x0059A540>(self, state, flag);
}

// 0x0059A220, __thiscall, ret 4: reports the state to whatever the manager keeps at
// +0x88/+0x9C, allocating and locking when those are empty.
void BridgeReportState(JobManager* self, JobState* state)
{
    CallGameMethod<void, 0x0059A220>(self, state);
}

// 0x0059A800, __thiscall, ret 0: Release on state->refcount (+0x30); the destructor runs
// through 0x0059A650 when it reaches zero.
void BridgeReleaseState(JobState* state)
{
    CallGameMethod<void, 0x0059A800>(state);
}

// 0x0059A990, __thiscall, ret 0: Release on a RefTarget, whose refcount sits at +0x28.
void BridgeReleaseTarget(RefTarget* target)
{
    CallGameMethod<void, 0x0059A990>(target);
}

// EA's operator delete[] (0x004E3F70, __cdecl).
void FreeEaArray(void* block)
{
    CallGameFunction<void, 0x004E3F70>(block);
}

// Unlinks `cell` from whichever list holds it: the original writes `*prev = next` then
// `next->prev = prev`, in that order.
void Unlink(ListCell* cell)
{
    ListCell* const prev = cell->prev;
    ListCell* const next = cell->next;
    prev->next = next;
    next->prev = prev;
}

// 0x00599460's receiver is the sub-object at `state+8` (its +0x28 field is the dword at
// state+0x30 that 0x0059A800 decrements), so the AddRef goes through that address.
int32_t AddRefState(JobState* state)
{
    return reinterpret_cast<RefTarget*>(reinterpret_cast<uint8_t*>(state) + 8)->AddRef();
}

// The vector stores 4-byte pointers on x86; keep the load explicit so a 64-bit build
// still reads one slot rather than a native pointer.
JobState* LoadSlot(const void* slot)
{
    uint32_t bits = 0;
    std::memcpy(&bits, slot, sizeof bits);
    return reinterpret_cast<JobState*>(static_cast<uintptr_t>(bits));
}

}  // namespace

// 0x00599460: an atomic increment of the dword at this-8+0x30 that returns the new
// value (the old one plus one). The receiver is the refcount
// sub-object at `owner+8`, so the effective dword is this+0x28 and the caller gets the
// previous count plus one (wrapping mod 2^32, which is why the add stays unsigned). A
// null receiver does not skip the read-modify-write in the original: it lands on absolute
// 0x30 and faults, and this does the same rather than inventing a null check.
int32_t RefTarget::AddRef()
{
    const RefTarget* const self = this;
    uint32_t* const slot =
        self ? &refcount : reinterpret_cast<uint32_t*>(static_cast<uintptr_t>(0x30));
    std::atomic_ref<uint32_t> count(*slot);
    return int32_t(count.fetch_add(1u, std::memory_order_relaxed) + 1u);
}

// 0x006E9540, __thiscall, ret 4. Shares a new value into the holder: when it differs from
// the old one the new value is AddRef'ed (through 0x00599460, rewritten), the pointer is
// stored — the store happens even when the new value is null — and only then is the old
// value released (0x0059A990, bridged). Equal pointers are a no-op that still returns
// this.
RefHolder* RefHolder::Assign(RefTarget* newValue)
{
    RefTarget* const oldValue = value;
    if (newValue != oldValue) {
        if (newValue) newValue->AddRef();
        value = newValue;
        if (oldValue) BridgeReleaseTarget(oldValue);
    }
    return this;
}

// 0x008BBD30, __thiscall, ret 4. The usual vector push_back with one EA quirk: the end
// pointer is advanced before the null check, so a vector whose end is null ends up with
// end == 4 and nothing written. When the end has reached the capacity the grow helper
// 0x00B7B1E0 runs with ecx still holding the vector (the original never reloaded it) and
// its return — the new element count, not the slot — comes back out unchanged.
void* DepVector::PushBack(const void* value)
{
    auto* const slot = static_cast<uint8_t*>(end);
    if (slot >= static_cast<uint8_t*>(capacity))
        return CallGameMethod<void*, 0x00B7B1E0>(this, end, value);
    end = slot + 4;
    if (!slot) return nullptr;
    std::memcpy(slot, value, 4);
    return slot;
}

// 0x005998B0, __thiscall, ret 4. Pops the dep list of `state` (+0x40) from the tail until
// it is empty. For every node it unlinks the node's other cell from the state's
// DepsToCancel list (+0x38), pushes the node onto this+0x94 and cancels the node's state
// through the bridge. An empty list — the common case — returns without touching `this`.
void JobManager::CancelDepList(JobState* state)
{
    ListCell* const header = &state->depList;
    while (header->prev != header) {
        ListCell* const tail = header->prev;  // the DepNode cell at +0x08
        // The original subtracts eight from the tail pointer and dereferences the result
        // unconditionally, so a null tail is a null dereference there; keep the same
        // arithmetic instead of adding a check the game does not have.
        auto* const node =
            reinterpret_cast<DepNode*>(reinterpret_cast<uint8_t*>(tail) - sizeof(ListCell));
        JobState* const dep = node->state;
        Unlink(&node->depCell);     // out of the list being drained
        Unlink(&node->cancelCell);  // out of DepsToCancel
        // Tail insert onto this+0x94, reusing the cell DepsToCancel just gave up.
        ListCell* const freeTail = freeList.prev;
        node->cancelCell.prev = freeTail;
        node->cancelCell.next = &freeList;
        freeList.prev = &node->cancelCell;
        freeTail->next = &node->cancelCell;
        BridgeCancelState(this, dep, 1);
    }
}

// 0x0059A9F0, __thiscall, ret 8. The DepsToCancel state machine, selected by
// state->state: 1..3 collect the state's cancellable dependencies, 4..5 drain it, 6
// returns 0 and 7 returns 1; anything else returns 0. Only the low byte of `flag` is ever
// read (the original re-reads the whole stack slot to pass it on, but the tests inside
// 0x0059A9F0 and 0x0059A220 look at one byte).
uint8_t JobManager::ProcessDeps(JobState* state, uint8_t flag)
{
    const uint32_t arm = state->state - 1u;
    if (arm > 6) return 0;  // state 0 or 8 and up

    if (arm <= 2) {  // states 1, 2, 3
        // A 32-slot vector with a 128-byte inline buffer behind it: the game's is the
        // stack itself, so capacity is the address just past the 32 pointers.
        void* slots[32];
        DepVector deps;
        deps.begin = deps.end = slots;
        deps.capacity = slots + 32;
        deps.tag = "JobManager/DepsToCancel";
        deps.bufferBase = reinterpret_cast<uint8_t*>(slots);

        // Head-first walk of DepsToCancel (+0x38); its node cell is the node itself.
        ListCell* const header = &state->cancelList;
        for (ListCell* cell = header->next; cell != header; cell = cell->next) {
            auto* const node = reinterpret_cast<DepNode*>(cell);
            JobState* const obj = node->obj;
            // EA only collects an object that is flagged and whose +0x40/+0x44 pair
            // still matches — for a list header that is "empty or exactly one entry".
            if (obj->collectFlag && obj->depList.next == obj->depList.prev) {
                deps.PushBack(&obj);
                AddRefState(obj);
            }
        }
        BridgeCancelState(this, state, 1);

        // Back out over what was collected: recurse into each object, then release it.
        while (deps.begin != deps.end) {
            auto* const slot = static_cast<uint8_t*>(deps.end) - 4;
            JobState* const obj = LoadSlot(slot);
            deps.end = slot;
            ProcessDeps(obj, flag);
            BridgeReleaseState(obj);
        }
        // Only a heap buffer is freed; the inline one and a never-grown vector are not.
        if (deps.begin && deps.begin != deps.bufferBase) FreeEaArray(deps.begin);
        return 1;
    }

    if (arm <= 4) {  // states 4 and 5
        CancelDepList(state);  // 0x005998B0
        if (flag) BridgeReportState(this, state);
        return 0;
    }

    return arm == 6 ? 1 : 0;  // state 7 returns 1, state 6 returns 0
}

}  // namespace ts3::jobmanager
