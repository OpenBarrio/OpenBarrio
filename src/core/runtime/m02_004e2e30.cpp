#include "ts3/runtime/m02_004e2e30.h"
#define NOMINMAX
#include <windows.h>

namespace ts3::runtime::m02 {
namespace {
// The original filter returns -1 (CONTINUE_EXECUTION), not EXECUTE_HANDLER.
// Only the debugger notification is protected; callback exceptions must escape.
void DebugName(ThreadRecord* record) {
    ULONG_PTR info[4]={0x1000,reinterpret_cast<ULONG_PTR>(record->name),record->id,0};
    __try {
        RaiseException(0x406d1388,0,4,info);
    } __except(EXCEPTION_CONTINUE_EXECUTION) {
    }
}
}
unsigned __stdcall ThreadCallbackEntry(void* opaque) {
    auto record=static_cast<ThreadRecord*>(opaque);
    using Callback=unsigned (__cdecl*)(void*);
    using Wrapper=unsigned (__cdecl*)(Callback,void*);
    const auto callback=reinterpret_cast<Callback>(record->function);
    void* const argument=record->argument;
    CallGameFunction<void,0x004E1200>(record->tlsHandle,uint32_t(0));
    record->state=1;
    if(record->name[0]!=0) DebugName(record);
    const auto wrapper=reinterpret_cast<Wrapper>(record->wrapper);
    const unsigned result=wrapper ? wrapper(callback,argument) : callback(argument);
    record->result=result;
    CallGameFunction<void,0x004E1200>(static_cast<void*>(nullptr),uint32_t(0));
    record->state=2;
    // JNE immediately follows XADD -1: its ZF describes the NEW value, not ECX.
    if(InterlockedDecrement(reinterpret_cast<volatile LONG*>(&record->refs))==0)
        CallGameFunction<void,0x004E2DA0>(record);
    return result;
}
}
