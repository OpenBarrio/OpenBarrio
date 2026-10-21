#pragma once
#include "ts3/runtime/m02_stopwatch.h"
namespace ts3::runtime::m02 {
// TS3W.exe 0x00774CB0: Allocate and publish a StopWatch registration, invoke slot 18 even on failure, and preserve its full EAX; failure leaves the old global intact.
uintptr_t __cdecl RegisterStopWatch(void* scripting);
}
