#pragma once
#include "ts3/runtime/m02_threads.h"
namespace ts3::runtime::m02 {
struct RecordReserveDispatch {
    ThreadRecord* pool;
    uint32_t* occupancy;
    AllocatorDispatch allocator;
};
RecordReserveDispatch* SetRecordReserveDispatch(RecordReserveDispatch*);
// TS3W.exe 0x004E2D30: reserve the first free pooled record or allocate a heap record; pooled bytes and unspecified heap fields remain untouched.
ThreadRecord* __cdecl ReserveThreadRecord();
}
