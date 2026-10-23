#pragma once
#include "ts3/resource/m03_B01.h"
#include "ts3/resource/m03_manager_contracts.h"
namespace ts3::resource::m03::g02 {
struct Manager {
    void* vtable; uint8_t initialized; uint8_t padding[3]; Allocator* allocator;
    manager::FilterSet filters;
    manager::DatabaseVector databases;
    Mutex mutex;
    b01::Hash factories;
    manager::CallbackVector callbacks;
    manager::Controls controls;
    b01::Hash typeNames,keyNames;
    Mutex nameMutex,typeMutex,factoryMutex;
};
struct Record { void* vtable; int32_t refs; void* owner; void* stream; Key key; uint8_t open,owned,pending; };
TS3_X86_OFFSET(Record,key,0x10);
TS3_X86_OFFSET(Record,pending,0x22);
TS3_X86_OFFSET(Manager,filters,0x0c);
TS3_X86_OFFSET(Manager,factories,0x78);
TS3_X86_OFFSET(Manager,keyNames,0x118);
TS3_X86_OFFSET(Manager,factoryMutex,0x1a0);
TS3_X86_SIZE(Manager,0x1d0);
// TS3W.exe 0x004B30B0: destroys manager members in reverse order, returning final tree cleanup EAX.
uintptr_t __fastcall Destroy(Manager*,void*);
// TS3W.exe 0x004B3650: constructs record, zeroes interlocked refs and copies key/low-byte flags; padding untouched.
Record* __fastcall Construct(Record*,void*,void*,void*,const Key*,uint32_t,uint32_t);
}
