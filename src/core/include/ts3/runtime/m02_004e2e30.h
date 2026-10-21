#pragma once
#include "ts3/runtime/m02_threads.h"
namespace ts3::runtime::m02 {
// TS3W.exe 0x004E2E30: run a C thread callback with TLS, debugger naming SEH and record release; callback/argument are captured before TLS setup.
unsigned __stdcall ThreadCallbackEntry(void* record);
}
