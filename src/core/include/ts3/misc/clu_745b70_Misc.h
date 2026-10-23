#pragma once
#include "ts3/layout.h"
namespace ts3::misc {
struct TelemetryBuffer { void* begin; void* end; void* capacity; const char* tag; };
struct TelemetryLock { void* debug; int32_t lockCount, recursion; void* thread; void* semaphore; uintptr_t spin; };
struct TelemetryManager {
    TelemetryBuffer queue;
    TelemetryBuffer path;
    TelemetryLock lock;
    uint8_t tail[8];
    // TS3W.exe 0x00892E20: allocate and copy an event queue.
    TelemetryManager* CopyQueue(TelemetryBuffer* source);
    // TS3W.exe 0x00892E80: initialize the queue, path and critical section.
    TelemetryManager* Init();
    // TS3W.exe 0x00893460: convert a wide event and submit; returns final cleanup's EAX.
    uint32_t SubmitText(const void* a, const void* b, const char16_t* text);
};
TS3_X86_SIZE(TelemetryBuffer, 16);
TS3_X86_SIZE(TelemetryLock, 24);
TS3_X86_SIZE(TelemetryManager, 64);
TS3_X86_OFFSET(TelemetryManager, path, 16);
TS3_X86_OFFSET(TelemetryManager, lock, 32);
// TS3W.exe 0x00893590: lazily create the manager and submit a narrow event.
int32_t SubmitEvent(const void* a, const void* b, const char* text);
// TS3W.exe 0x008935E0: lazily create the manager and submit a wide event.
uint32_t SubmitWideEvent(const void* a, const void* b, const char16_t* text);
// TS3W.exe 0x00745B70: obtain text from service slot 22 and submit addresses of the two IDs.
uint32_t ForwardTelemetry(uint32_t a, uint32_t b, uint32_t key);
}
