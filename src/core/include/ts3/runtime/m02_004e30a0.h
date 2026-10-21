#pragma once
#include "ts3/runtime/m02_threads.h"
namespace ts3::runtime::m02 {
struct VirtualThreadEntryDispatch {
    void* context;
    void (*tls)(void*,void*,uint8_t);
    void (*debugName)(void*,const char*,uint32_t);
    void (*release)(void*,ThreadRecord*);
};
VirtualThreadEntryDispatch* SetVirtualThreadEntryDispatch(VirtualThreadEntryDispatch*);
// TS3W.exe 0x004E30A0: run virtual thread callback with TLS/debug notification, publish result and release on new refs zero.
unsigned __stdcall VirtualThreadEntry(void* record);
}
