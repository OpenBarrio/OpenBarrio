#define WIN32_LEAN_AND_MEAN
#define NOMINMAX
#include <windows.h>

#include "ts3/thread/mutex.h"

namespace ts3::thread {
namespace {

static_assert(sizeof(CRITICAL_SECTION) <= sizeof(Mutex::data), "the critical section fits the mutex storage");

CRITICAL_SECTION* Section(Mutex& m) { return reinterpret_cast<CRITICAL_SECTION*>(m.data); }
HANDLE Handle(const Mutex& m) { return *reinterpret_cast<const HANDLE*>(m.data); }

}  // namespace

int Mutex::Lock(const ThreadTime& timeoutAbsolute)
{
    if (intraProcess) {
        if (timeoutAbsolute == kTimeoutNone) {
            EnterCriticalSection(Section(*this));
            return ++lockCount;
        }
        if (TryEnterCriticalSection(Section(*this)))
            return ++lockCount;
        while (GetTickCount() < timeoutAbsolute) {
            Sleep(1);
            if (TryEnterCriticalSection(Section(*this)))
                return ++lockCount;
        }
        return kResultTimeout;
    }
    DWORD wait = timeoutAbsolute;  // kTimeoutNone is INFINITE, kTimeoutImmediate is 0
    if (timeoutAbsolute != kTimeoutNone && timeoutAbsolute != kTimeoutImmediate) {
        const DWORD now = GetTickCount();
        wait = timeoutAbsolute <= now ? 0 : timeoutAbsolute - now;
    }
    switch (WaitForSingleObject(Handle(*this), wait)) {
    case WAIT_OBJECT_0:
        return ++lockCount;
    case WAIT_TIMEOUT:
        return kResultTimeout;
    default:
        return kResultError;
    }
}

int Mutex::Unlock()
{
    const int remaining = --lockCount;
    if (intraProcess)
        LeaveCriticalSection(Section(*this));
    else
        ReleaseMutex(Handle(*this));
    return remaining;
}

}  // namespace ts3::thread
