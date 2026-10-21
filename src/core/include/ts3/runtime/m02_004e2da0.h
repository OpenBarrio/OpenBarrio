#pragma once
#include "ts3/runtime/m02_threads.h"
namespace ts3::runtime::m02 {
struct RecordRecycleDispatch {
    void* context;
    ThreadRecord* pool;
    uint32_t* occupancy;
    void (*close)(void*,void*);
    void (*free)(void*,void*);
};
RecordRecycleDispatch* SetRecordRecycleDispatch(RecordRecycleDispatch* dispatch);
// TS3W.exe 0x004E2DA0: release pool occupancy before closing HANDLE, or close and free a non-pool record; range-only membership and uncleared record bytes are retained.
void RecycleThreadRecord(ThreadRecord* record);
}
