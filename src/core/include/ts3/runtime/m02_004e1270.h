#pragma once
#include <cstdint>
namespace ts3::runtime::m02::tls {
struct Dispatch {
    void* context;
    uint32_t* index;
    void* (*get)(void*,uint32_t);
    uint32_t (*allocate)(void*);
    bool (*set)(void*,uint32_t,void*);
    bool (*duplicateCurrent)(void*,void**);
    void (*sweep)(void*);
    void (*record)(void*,void*,bool);
};
Dispatch* SetDispatch(Dispatch*);
// TS3W.exe 0x004E1270: get/create the current TLS thread handle; retains the duplicate on TLS exhaustion and ignores TLS-set failure.
void* CurrentHandle();
}
