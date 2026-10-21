#pragma once
#include <cstdint>
#include "ts3/layout.h"

namespace ts3::jobmanager {

// One cell of the game's intrusive doubly-linked list: `next` at +0, `prev` at +4, and a
// header is empty exactly when its `prev` points at itself. Several of the lists below
// keep the cell at a non-zero offset inside their node (0x005998B0 reads the dep-list
// tail as `P` and works on `P-8`), so the cell is named apart from the node that owns it.
struct ListCell {
    ListCell* next;
    ListCell* prev;
};

// Receiver of 0x00599460 (AddRef). The game passes the refcount-holding sub-object at
// `owner+8`, so the interlocked dword is really at `owner+0x30` and at `this+0x28` here.
// Only that dword is ever touched; the prefix documents the rest of the object.
struct RefTarget {
    uint8_t prefix[0x28];
    uint32_t refcount;  // +0x28 lock xadd(+1); 0x0059A990 releases the same dword

    // 0x00599460, __thiscall, ret 0; returns the old count plus one.
    int32_t AddRef();
};
TS3_X86_OFFSET(RefTarget, refcount, 0x28);
TS3_X86_SIZE(RefTarget, 0x2C);

struct JobState;

// A node of a JobState's dependency lists. The same node sits in three lists at three
// cells: `cancelCell` (+0x00) is shared by the owning state's DepsToCancel list (+0x38)
// and by the JobManager free list (+0x94) — 0x005998B0 unlinks the first and reuses the
// cell for the second — while `depCell` (+0x08) is the node's link in the state's dep
// list (+0x40), the list 0x005998B0 drains. `state` is cancelled through the bridged
// 0x0059A540; `obj` is the object the 0x0059A9F0 collector AddRefs and recurses on.
struct DepNode {
    ListCell cancelCell;  // +0x00
    ListCell depCell;     // +0x08
    JobState* obj;        // +0x10
    JobState* state;      // +0x14
};
TS3_X86_OFFSET(DepNode, depCell, 0x08);
TS3_X86_OFFSET(DepNode, obj, 0x10);
TS3_X86_OFFSET(DepNode, state, 0x14);
TS3_X86_SIZE(DepNode, 0x18);

// The object 0x0059A9F0 dispatches on (the `a2` of every function in this unit), and the
// same layout again for the dependency objects it collects: the recursion hands those
// back in as `a2`. Fields the unit only routes to the bridge keep neutral names.
struct JobState {
    ListCell activeLink;  // +0x00 the bridged 0x0059A540 moves the state onto manager +0x80
    uint32_t pad08[2];    // +0x08
    uint32_t callback;    // +0x10 bridged 0x0059A540 calls it unless it is null
    uint32_t callbackArg; // +0x14 second argument of that callback
    uint32_t field18;     // +0x18
    uint32_t field1C;     // +0x1C
    uint32_t field20;     // +0x20 compared with +0x48 by the bridged 0x0059A470
    uint32_t state;       // +0x24 selector of 0x0059A9F0; the bridged 0x0059A540 stores 7
    uint32_t field28;     // +0x28
    uint32_t field2C;     // +0x2C
    uint32_t refcount;    // +0x30 AddRef through the receiver (this+8); 0x0059A800 releases
    uint32_t owner;       // +0x34 handed to 0x0059A650 when the refcount reaches zero
    ListCell cancelList;  // +0x38 DepsToCancel; node cell at node+0x00
    ListCell depList;     // +0x40 drained by 0x005998B0; node cell at node+0x08
    uint32_t field48;     // +0x48 compared with +0x20 by the bridged 0x0059A470
    uint8_t field4C;      // +0x4C cleared by the bridged 0x0059A540
    uint8_t collectFlag;  // +0x4D 0x0059A9F0 collects a node only when this is non-zero
    uint8_t field4E;      // +0x4E cleared by the bridged 0x0059A540
    uint8_t pad4F;        // +0x4F
};
TS3_X86_OFFSET(JobState, callback, 0x10);
TS3_X86_OFFSET(JobState, state, 0x24);
TS3_X86_OFFSET(JobState, refcount, 0x30);
TS3_X86_OFFSET(JobState, cancelList, 0x38);
TS3_X86_OFFSET(JobState, depList, 0x40);
TS3_X86_OFFSET(JobState, collectFlag, 0x4D);
TS3_X86_SIZE(JobState, 0x50);

// The vector 0x008BBD30 pushes into and 0x0059A9F0 keeps one on its own stack. It is the
// game's usual five-word vector: a heap buffer, or the 128-byte inline `bufferBase` (a
// grown vector frees the old `begin` only when it is not that inline buffer) plus the
// allocator hint EA passes to new[].
struct DepVector {
    void* begin;         // +0x00
    void* end;           // +0x04
    void* capacity;      // +0x08
    const char* tag;     // +0x0C hint for EA's new[] ("JobManager/DepsToCancel")
    uint8_t* bufferBase; // +0x10

    // 0x008BBD30, __thiscall, ret 4; returns the slot written, or the grow helper's
    // element count when it had to reallocate.
    void* PushBack(const void* value);
};
TS3_X86_OFFSET(DepVector, end, 0x04);
TS3_X86_OFFSET(DepVector, capacity, 0x08);
TS3_X86_OFFSET(DepVector, tag, 0x0C);
TS3_X86_OFFSET(DepVector, bufferBase, 0x10);
TS3_X86_SIZE(DepVector, 0x14);

// The JobManager itself. This unit only ever uses the free list at +0x94; everything
// else is touched by the functions bridged out of it.
struct JobManager {
    uint8_t pad00[0x80];     // +0x00 includes the lock at +0x08 the bridge functions take
    ListCell activeList;     // +0x80 the bridged 0x0059A540 inserts cancelled states here
    uint32_t treeRoot;       // +0x88 read by the bridged 0x0059A220
    uint32_t pad8C[2];       // +0x8C
    ListCell freeList;       // +0x94 0x005998B0 pushes every drained node here
    ListCell nodeList9C;     // +0x9C used by the bridged 0x0059A220

    // 0x005998B0, __thiscall, ret 4; drains `state`'s dep list into the free list.
    void CancelDepList(JobState* state);
    // 0x0059A9F0, __thiscall, ret 8; the DepsToCancel state machine.
    uint8_t ProcessDeps(JobState* state, uint8_t flag);
};
TS3_X86_OFFSET(JobManager, activeList, 0x80);
TS3_X86_OFFSET(JobManager, freeList, 0x94);
TS3_X86_OFFSET(JobManager, nodeList9C, 0x9C);

// Receiver of 0x006E9540: a one-pointer holder that shares ownership of the value
// through 0x00599460 (rewritten) and the bridged 0x0059A990.
struct RefHolder {
    RefTarget* value;  // +0x00

    // 0x006E9540, __thiscall, ret 4; returns this.
    RefHolder* Assign(RefTarget* newValue);
};
TS3_X86_OFFSET(RefHolder, value, 0x00);
TS3_X86_SIZE(RefHolder, 0x4);

}  // namespace ts3::jobmanager
