#pragma once
#include "ts3/runtime/m02_threads.h"
namespace ts3::runtime::m02 {
struct CounterUpdateDispatch {
    void* context;
    uint32_t (*compareExchange)(void*,uint32_t*,uint32_t,uint32_t);
    uint32_t (*exchangeAdd)(void*,uint32_t*,uint32_t);
};
CounterUpdateDispatch* SetCounterUpdateDispatch(CounterUpdateDispatch* dispatch);
// TS3W.exe 0x004E2640: reconcile a semaphore's negative count by CAS, adding any remainder to its +8 counter; signed tests and wrapping arithmetic are retained.
void SemaphoreCounterUpdate(Semaphore* semaphore,int32_t amount);
inline void __fastcall SemaphoreCounterUpdateEntry(Semaphore* semaphore,void*,int32_t amount) {
    SemaphoreCounterUpdate(semaphore,amount);
}
}
