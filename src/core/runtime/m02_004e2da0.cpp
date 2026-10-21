#include "ts3/runtime/m02_004e2da0.h"
#define NOMINMAX
#include <windows.h>
namespace ts3::runtime::m02 {
namespace { thread_local RecordRecycleDispatch* dispatch=nullptr; }
RecordRecycleDispatch* SetRecordRecycleDispatch(RecordRecycleDispatch* value) {
    auto old=dispatch; dispatch=value; return old;
}
void RecycleThreadRecord(ThreadRecord* record) {
    // The null path leaves incoming EAX untouched and the heap path inherits a
    // void allocator's residue: there is no defined C return value.
    ThreadRecord* pool=nullptr;
    uint32_t* occupancy=nullptr;
    if(dispatch) { pool=dispatch->pool; occupancy=dispatch->occupancy; }
#if defined(_M_IX86)
    else { pool=reinterpret_cast<ThreadRecord*>(0x011C8330); occupancy=reinterpret_cast<uint32_t*>(0x011CB540); }
#else
    else { AbortNotRewritten("m02 record recycle requires native dispatch"); }
#endif
    auto address=reinterpret_cast<uintptr_t>(record);
    auto base=reinterpret_cast<uintptr_t>(pool);
    if(address>=base && address<base+128*sizeof(ThreadRecord)) {
        // EA divides the byte displacement by 100, without checking alignment.
        auto index=(address-base)/sizeof(ThreadRecord);
        InterlockedExchange(reinterpret_cast<volatile LONG*>(occupancy+index),0);
        if(record->handle) {
            if(dispatch) dispatch->close(dispatch->context,record->handle);
            else CloseHandle(record->handle);
        }
    } else if(record) {
        if(record->handle) {
            if(dispatch) dispatch->close(dispatch->context,record->handle);
            else CloseHandle(record->handle);
        }
        if(dispatch) dispatch->free(dispatch->context,record);
        else CallGameFunction<void,0x004E3F70>(record);
    }
}
}
