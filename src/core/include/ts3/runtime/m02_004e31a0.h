#pragma once
#include "ts3/runtime/m02_threads.h"
namespace ts3::runtime::m02 {
// Native seam: the caller supplies a virtual-callback trampoline and owns its lifetime.
struct VirtualLaunchDispatch {
    void* context;
    ThreadRecord* (*reserve)(void*);
    void (*recycle)(void*,ThreadRecord*);
    uintptr_t (*begin)(void*,unsigned,ThreadEntry,ThreadRecord*,unsigned,unsigned*);
    void (*name)(void*,Thread*,const char*);
    void (*priority)(void*,Thread*,int32_t);
    void (*ideal)(void*,Thread*,int32_t);
    void (*resume)(void*,void*);
    ThreadEntry entry;
    int32_t* defaultCpu;
    uint32_t* cpuCursor;
};
VirtualLaunchDispatch* SetVirtualLaunchDispatch(VirtualLaunchDispatch*);
// TS3W.exe 0x004E31A0: launch a virtual callback suspended, configure it, then resume; preserve partial initialization and wrapping references/CPU cursor.
void* LaunchVirtualThread(Thread*,void* object,void* argument,const ThreadParameters*,void* wrapper);
// TS3W.exe 0x004E31A0: fastcall adapter for the original thiscall entry (callee pops four arguments).
void* __fastcall LaunchVirtualThreadEntry(Thread*,void*,void* object,void* argument,const ThreadParameters*,void* wrapper);
}
