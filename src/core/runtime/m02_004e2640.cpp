#include "ts3/runtime/m02_004e2640.h"
#define NOMINMAX
#include <windows.h>
namespace ts3::runtime::m02 {
namespace {
thread_local CounterUpdateDispatch* dispatch=nullptr;
uint32_t Compare(uint32_t* p,uint32_t desired,uint32_t expected) {
    return dispatch ? dispatch->compareExchange(dispatch->context,p,desired,expected)
        : uint32_t(InterlockedCompareExchange(reinterpret_cast<volatile LONG*>(p),LONG(desired),LONG(expected)));
}
void Add(uint32_t* p,uint32_t amount) {
    if(dispatch) dispatch->exchangeAdd(dispatch->context,p,amount);
    else InterlockedExchangeAdd(reinterpret_cast<volatile LONG*>(p),LONG(amount));
}
}
CounterUpdateDispatch* SetCounterUpdateDispatch(CounterUpdateDispatch* value) {
    auto old=dispatch; dispatch=value; return old;
}
void SemaphoreCounterUpdate(Semaphore* semaphore,int32_t amount) {
    // No defined return value: EAX is untouched on the amount<=0 path.
    if(amount<=0) return;
    uint32_t remainder=uint32_t(amount);
    uint32_t observed=*reinterpret_cast<volatile uint32_t*>(&semaphore->count);
    if(int32_t(observed)<0) {
        for(;;) {
            uint32_t desired=observed+remainder;
            if(int32_t(desired)>0) desired=0;
            uint32_t previous=Compare(&semaphore->count,desired,observed);
            if(previous==observed) {
                remainder+=previous-desired;
                break;
            }
            observed=previous;
            if(int32_t(observed)>=0) break;
        }
    }
    if(int32_t(remainder)>0) Add(&semaphore->maximum,remainder);
}
}
