#include "ts3/runtime/m02_004e30a0.h"
#include "ts3/layout.h"
#include <windows.h>
#include <stdexcept>
namespace ts3::runtime::m02 {
namespace {
thread_local VirtualThreadEntryDispatch* dispatch=nullptr;
void Tls(void* value) {
    if(dispatch) dispatch->tls(dispatch->context,value,0);
#if defined(_M_IX86)
    else CallGameFunction<uint32_t,0x004E1200>(value,uint8_t(0));
#else
    else throw std::runtime_error("VirtualThreadEntry needs TLS dispatch");
#endif
}
void DebugName(const char* name,uint32_t id) {
    if(dispatch && dispatch->debugName) { dispatch->debugName(dispatch->context,name,id); return; }
    // The original filter returns -1: resume after RaiseException, not a
    // catch-all around the callback. Four ULONG_PTRs are pointer-sized on x64.
    ULONG_PTR args[4]={0x1000,reinterpret_cast<ULONG_PTR>(name),id,0};
    __try { RaiseException(0x406D1388,0,4,args); }
    __except(EXCEPTION_CONTINUE_EXECUTION) {}
}
void Release(ThreadRecord* record) {
    if(dispatch) dispatch->release(dispatch->context,record);
#if defined(_M_IX86)
    else CallGameFunction<void,0x004E2DA0>(record);
#else
    else throw std::runtime_error("VirtualThreadEntry needs record release dispatch");
#endif
}
}
VirtualThreadEntryDispatch* SetVirtualThreadEntryDispatch(VirtualThreadEntryDispatch* value) {
    auto old=dispatch; dispatch=value; return old;
}
unsigned __stdcall VirtualThreadEntry(void* pointer) {
    auto record=static_cast<ThreadRecord*>(pointer);
    // Capture these BEFORE TLS/debug calls, just as EBX/EDI in the entry.
    auto object=record->function;
    auto argument=record->argument;
    Tls(record->tlsHandle);
    record->state=1;
    if(record->name[0]!=0 && record->handle!=nullptr) DebugName(record->name,record->id);
    uint32_t result;
    if(record->wrapper) {
        using Wrapper=uint32_t(__cdecl*)(void*,void*);
        result=reinterpret_cast<Wrapper>(record->wrapper)(object,argument);
    } else {
        using Method=uint32_t(__fastcall*)(void*,void*,void*);
        auto slots=*static_cast<void***>(object);
        result=reinterpret_cast<Method>(slots[1])(object,nullptr,argument);
    }
    record->result=result;
    Tls(nullptr);
    record->state=2;
    // XADD's ZF describes the NEW value (old == 1), not its EDX output.
    if(InterlockedExchangeAdd(reinterpret_cast<volatile LONG*>(&record->refs),-1)==1) Release(record);
    return result;
}
}
