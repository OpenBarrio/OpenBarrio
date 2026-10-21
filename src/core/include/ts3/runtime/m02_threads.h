#pragma once
#include "ts3/runtime/m02_layouts.h"
namespace ts3::runtime::m02 {
using ThreadEntry = unsigned (__stdcall*)(void*);
using ThreadVirtualEntry = intptr_t (__fastcall*)(void*,void*,void*);
using BeginThread = uintptr_t (__cdecl*)(void*,unsigned,ThreadEntry,void*,unsigned,unsigned*);
using WaitEntry = uint32_t (__stdcall*)(void*,uint32_t);
using TlsGetEntry = void* (__stdcall*)(uint32_t);
using TlsSetEntry = int (__stdcall*)(uint32_t,void*);
// Opaque boundaries: implementations may use typed CallGame* bridges on x86 only.
struct ThreadBoundary { void* context; uintptr_t (*begin)(void*,ThreadEntry,void*,unsigned,unsigned*); bool (*join)(void*,uintptr_t); };
}
namespace ts3::runtime::m02 {
struct ThreadDispatch {
    void* context;
    void* (*begin)(void*,ThreadEntry,void*,unsigned,unsigned*);
    uint32_t (*resume)(void*,void*);
    bool (*close)(void*,void*);
    void* (*duplicate)(void*,void*);
    uint32_t (*tlsAlloc)(void*);
    bool (*tlsFree)(void*,uint32_t);
    void* (*tlsGet)(void*,uint32_t);
    bool (*tlsSet)(void*,uint32_t,void*);
    void (*enter)(void*,void*);
    void (*leave)(void*,void*);
};
}
