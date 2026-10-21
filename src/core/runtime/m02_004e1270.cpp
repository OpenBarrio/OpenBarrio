#include "ts3/runtime/m02_004e1270.h"
#include "ts3/layout.h"
#include <windows.h>
namespace ts3::runtime::m02::tls {
namespace {
thread_local Dispatch* dispatch=nullptr;
uint32_t& Index() {
#if defined(_M_IX86)
    return *reinterpret_cast<uint32_t*>(0x114c908);
#else
    static uint32_t index=TLS_OUT_OF_INDEXES; return index;
#endif
}
void* Get(uint32_t i) { return dispatch?dispatch->get(dispatch->context,i):TlsGetValue(i); }
}
Dispatch* SetDispatch(Dispatch* d) { auto old=dispatch; dispatch=d; return old; }
void* CurrentHandle() {
    auto& index=dispatch?*dispatch->index:Index();
    if(index!=TLS_OUT_OF_INDEXES) {
        auto existing=Get(index);
        if(existing) return existing;
    }
    // EA returns this output cell even on DuplicateHandle failure (it is not initialized).
    void* handle;
    bool duplicated;
    if(dispatch) duplicated=dispatch->duplicateCurrent(dispatch->context,&handle);
    else {
        auto thread=GetCurrentThread();
        auto process=GetCurrentProcess();
        duplicated=DuplicateHandle(process,thread,GetCurrentProcess(),&handle,0,TRUE,DUPLICATE_SAME_ACCESS)!=FALSE;
    }
    if(duplicated) {
        if(index==TLS_OUT_OF_INDEXES) {
            index=dispatch?dispatch->allocate(dispatch->context):TlsAlloc();
            if(index==TLS_OUT_OF_INDEXES) return handle;
        }
        if(dispatch) dispatch->sweep(dispatch->context);
#if defined(_M_IX86)
        else CallGameMethod<void,0x004e10e0>(reinterpret_cast<void*>(0x11c7e60),uint32_t(0));
#endif
        void* entry=handle;
        if(!entry) entry=Get(index);
        if(entry) {
            if(dispatch) dispatch->record(dispatch->context,entry,handle!=nullptr);
#if defined(_M_IX86)
            else CallGameMethod<void,0x004e1160>(reinterpret_cast<void*>(0x11c7e60),entry,handle!=nullptr);
#endif
        }
        if(dispatch) dispatch->set(dispatch->context,index,handle);
        else TlsSetValue(index,handle);
    }
    return handle;
}
}
