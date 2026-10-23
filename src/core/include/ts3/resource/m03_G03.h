#pragma once
#include "ts3/resource/m03_B01.h"
#include "ts3/thread/mutex.h"
namespace ts3::resource::m03::g03 {
struct Tree {
    void *left,*right,*parent;
    uint32_t color; uintptr_t count;
    GameAllocator allocator{};
};
struct Manager {
    void* vtable; uint8_t initialized; uint8_t pad05[3];
    allocator::ICoreAllocator* allocator; uint8_t flag; uint8_t pad0d[3];
    Tree files; b01::Vector databases; thread::Mutex mutex;
    b01::Hash factories; b01::Vector callbacks;
    void* controls[8]; uint16_t types[8]; uint32_t policy; float factor;
    b01::Hash typeNames,keyNames;
    thread::Mutex locks[3];
};
TS3_X86_OFFSET(Manager,callbacks,0xa0);
TS3_X86_OFFSET(Manager,locks,0x140);
TS3_X86_SIZE(Manager,0x1d0);
using Record=b01::Record;
// TS3W.exe 0x004B3200: register/remove callback in descending signed priority order; AL only, early duplicate scan retained.
uint8_t __fastcall Callback(Manager*,void*,uint32_t,void*,int32_t);
// TS3W.exe 0x004B33B0: construct manager containers and four mutexes, preserving untouched padding.
Manager* __fastcall Construct(Manager*,void*,allocator::ICoreAllocator*);
// TS3W.exe 0x004B36B0: close/delete owned pending stream, install base vtable and optionally free; returns self.
Record* __fastcall Destroy(Record*,void*,uint32_t);
}
