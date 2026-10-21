#include "ts3/runtime/m02_004e2d30.h"
#define NOMINMAX
#include <windows.h>
#include <stdexcept>
namespace ts3::runtime::m02 {
namespace { thread_local RecordReserveDispatch* dispatch=nullptr; }
RecordReserveDispatch* SetRecordReserveDispatch(RecordReserveDispatch* value) {
    auto old=dispatch; dispatch=value; return old;
}
ThreadRecord* __cdecl ReserveThreadRecord() {
    ThreadRecord* pool;
    uint32_t* occupancy;
    if(dispatch) { pool=dispatch->pool; occupancy=dispatch->occupancy; }
    else {
#if defined(_M_IX86)
        pool=reinterpret_cast<ThreadRecord*>(0x011C8330);
        occupancy=reinterpret_cast<uint32_t*>(0x011CB540);
#else
        throw std::runtime_error("ReserveThreadRecord requires native pool/allocator dispatch");
#endif
    }
    for(unsigned i=0;i<128;++i) {
        if(InterlockedCompareExchange(reinterpret_cast<volatile LONG*>(occupancy+i),1,0)==0)
            return pool+i;
    }
    auto record=static_cast<ThreadRecord*>(dispatch
        ? dispatch->allocator.allocate(dispatch->allocator.context,sizeof(ThreadRecord),alignof(ThreadRecord),0)
        : CallGameFunction<void*,0x005C32D0>(sizeof(ThreadRecord)));
    if(!record) return nullptr;
    record->handle=nullptr;
    record->id=0;
    record->state=0;
    record->result=0;
    record->wrapper=nullptr;
    InterlockedExchange(reinterpret_cast<volatile LONG*>(&record->refs),0);
    return record;
}
}
