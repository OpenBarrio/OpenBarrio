#pragma once
#include "ts3/anim/m05_layouts.h"
namespace ts3::anim::b27 {
// Pointer-bearing fields decoded from the task's formerly opaque +08/+0c.
struct Task {
    void* vtable; int32_t refs; ListLink links;
    uint32_t sourceLow, sourceHigh; int32_t priority, argument;
    LookAtTarget* target; uint32_t flags, active, value2C, value30;
    float gain; uint32_t speed, time;
};
TS3_X86_OFFSET(Task,links,0x08);
TS3_X86_OFFSET(Task,priority,0x18);
TS3_X86_OFFSET(Task,flags,0x24);
TS3_X86_SIZE(Task,0x40);
struct Slot { unsigned char opaque[0x28]; uint32_t kind; };
struct Slots { unsigned char opaque[8]; Slot* entries[10]; };
struct Iterator { Slots* slots; int32_t index; };
struct IndexOwner { unsigned char opaque[0x28]; void* index; };
// TS3W.exe 0x00580120: SetSpeed look-at state; original bit and lifetime semantics retained.
void SetSpeed(Task* p, uint32_t value);
// TS3W.exe 0x00580130: SetAngle34 look-at state; original bit and lifetime semantics retained.
LookAtTarget* SetAngle34(Task* p, float value);
// TS3W.exe 0x00580150: SetAngle38 look-at state; original bit and lifetime semantics retained.
LookAtTarget* SetAngle38(Task* p, float value);
// TS3W.exe 0x00580170: SetGain look-at state; original bit and lifetime semantics retained.
LookAtTarget* SetGain(Task* p, float value);
// TS3W.exe 0x00580190: SetValue40 look-at state; original bit and lifetime semantics retained.
LookAtTarget* SetValue40(Task* p, float value);
// TS3W.exe 0x005801B0: SetValue44 look-at state; original bit and lifetime semantics retained.
LookAtTarget* SetValue44(Task* p, float value);
// TS3W.exe 0x005801D0: SetAngle48 look-at state; original bit and lifetime semantics retained.
LookAtTarget* SetAngle48(Task* p, float value);
// TS3W.exe 0x005801F0: SetTime look-at state; original bit and lifetime semantics retained.
void SetTime(Task* p, uint32_t value);
// TS3W.exe 0x00580200: SetEnabled look-at state; original bit and lifetime semantics retained.
void SetEnabled(Task* p, uint32_t value);
// TS3W.exe 0x00580220: InitIterator look-at state; original bit and lifetime semantics retained.
Iterator* InitIterator(Iterator* p, Slots* slots);
// TS3W.exe 0x00580250: Next look-at state; original bit and lifetime semantics retained.
uint32_t Next(Iterator* p);
// TS3W.exe 0x005803E0: Resolve look-at state; original bit and lifetime semantics retained.
void* Resolve(LookAtTarget* p);
// TS3W.exe 0x00580420: GetIndex look-at state; original bit and lifetime semantics retained.
void* GetIndex(IndexOwner* p);
// TS3W.exe 0x00580AF0: Insert look-at state; original bit and lifetime semantics retained.
uintptr_t Insert(ListLink* p, Task* task);
// TS3W.exe 0x00580DE0: Activate look-at state; original bit and lifetime semantics retained.
uint8_t Activate(Task* p);
#if defined(_M_IX86)
void __fastcall HookSetSpeed(Task* p, void*, uint32_t value);
LookAtTarget* __fastcall HookSetAngle34(Task* p, void*, float value);
LookAtTarget* __fastcall HookSetAngle38(Task* p, void*, float value);
LookAtTarget* __fastcall HookSetGain(Task* p, void*, float value);
LookAtTarget* __fastcall HookSetValue40(Task* p, void*, float value);
LookAtTarget* __fastcall HookSetValue44(Task* p, void*, float value);
LookAtTarget* __fastcall HookSetAngle48(Task* p, void*, float value);
void __fastcall HookSetTime(Task* p, void*, uint32_t value);
void __fastcall HookSetEnabled(Task* p, void*, uint32_t value);
Iterator* __fastcall HookInitIterator(Iterator* p, void*, Slots* slots);
uint32_t __fastcall HookNext(Iterator* p, void*);
void* __fastcall HookResolve(LookAtTarget* p, void*);
void* __fastcall HookGetIndex(IndexOwner* p, void*);
uintptr_t __fastcall HookInsert(ListLink* p, void*, Task* task);
uint8_t __fastcall HookActivate(Task* p, void*);
#endif
}
