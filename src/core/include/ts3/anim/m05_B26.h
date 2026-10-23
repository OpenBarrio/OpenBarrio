#pragma once
#include "ts3/anim/m05_layouts.h"
namespace ts3::anim::b26 {
struct Task {
    void* vtable; int32_t refs; ListLink link;
    uint32_t sourceLow, sourceHigh, mode, mask;
    LookAtTarget* target; uint32_t flags; int32_t state;
    float duration, start, progress, gain; void* callback;
};
struct Manager { ListLink list; Task* selected[10]; float time; };
struct Identifier { uint32_t opaque[2], low, high; };
struct Priority { unsigned char opaque[16]; float priority; };
struct PriorityList { Priority** begin; Priority** end; Priority** capacity; };
TS3_X86_OFFSET(Task,link,0x08);
TS3_X86_OFFSET(Task,target,0x20);
TS3_X86_OFFSET(Task,callback,0x3c);
TS3_X86_OFFSET(Manager,time,0x30);
// TS3W.exe 0x00580000: Insert a nonnull item after equal priorities (unordered values do not stop the scan).
uintptr_t __fastcall InsertPriority(PriorityList*, void*, Priority*);
// TS3W.exe 0x00580040: Set or clear the target identifier unless locked.
void __fastcall SetIdentifier(Task*, void*, const Identifier*);
// TS3W.exe 0x00580070: Set target word 28 unless locked.
void __fastcall SetWord(Task*, void*, uint32_t);
// TS3W.exe 0x00580090: Set target first angular bound unless locked.
void __fastcall SetAngle2C(Task*, void*, float);
// TS3W.exe 0x005800B0: Set target second angular bound unless locked.
void __fastcall SetAngle30(Task*, void*, float);
// TS3W.exe 0x005800D0: Toggle target bit zero, testing only the argument low byte.
void __fastcall SetBit0(Task*, void*, uint32_t);
// TS3W.exe 0x00580100: Toggle target bit one, testing only the argument low byte.
void __fastcall SetBit1(Task*, void*, uint32_t);
// TS3W.exe 0x00580580: Resolve target position through optional world and object transforms.
Float4Value* __fastcall Position(void*, void*, Float4Value*);
// TS3W.exe 0x005809D0: Deliver an integer state notification to the optional script callback.
void __fastcall Notify(Task*, void*, int32_t);
// TS3W.exe 0x00580BA0: Rebase every task start time and reset the manager clock.
void* __fastcall Rebase(Manager*, void*);
// TS3W.exe 0x00580BF0: Select up to ten unmasked tasks; preserve the original OR-not-AND selection quirk.
uintptr_t __fastcall Select(Manager*, void*);
// TS3W.exe 0x00580E90: Begin angular interpolation or mark the task complete on missing source data.
void __fastcall Begin(Task*, void*, float);
// TS3W.exe 0x00581050: Enter holding state and notify with event one.
void __fastcall Hold(Task*, void*, float, float);
// TS3W.exe 0x00581270: Advance the look-at state machine, preserving its delayed completion transition.
void __fastcall Advance(Task*, void*, float);
// TS3W.exe 0x005813C0: Advance manager clock, remove completed tasks, select and update survivors.
void __fastcall Update(Manager*, void*, float);
}
