// Cluster unit clu_4f3220_Objects (objects): the shared stopwatch's stop and the two
// objects-side entries that drive it. Originals: TS3W.exe 0x004F3220 (150 B),
// 0x0094EEA0 around the 0x0094EEC7 body (209) and 0x009F5590 (185). See the header
// for the layouts and the call contracts; the un-rewritten dependencies (singleton
// getter 0x007692B0, ctor 0x004F35B0, converter 0x004F32C0, virtual slots 74/64/65)
// run through the layout.h bridge.
//
// EA behaviour kept that affects outputs:
//  * the stop's return is the high dword of the elapsed time (of the EXCESS on the
//    big-threshold path) even though no original caller reads it;
//  * the threshold at 0x011CB910 is compared high-dword-first and the low dword is
//    only read on an equal high half (unsigned 64-bit compare overall);
//  * the small path's carry into accum.hi comes from the 32-bit increment of accum.lo,
//    which nothing in between disturbs — a plain u64 += 1 matches;
//  * 0x0094EEA0 re-reads *out1 through the POINTER for the slot-65 gate (out1/out2
//    aliasing is observable), and its disabled path returns the out1 pointer itself;
//  * 0x009F5590 only clears +0x20 on ERROR_INTERNET_CONNECTION_RESET (0x2EFF, 12031).
#include "ts3/objects/clu_4f3220_Objects.h"

#if defined(_M_IX86)
#include <intrin.h>  // __rdtsc
#endif

namespace ts3::objects {
namespace {

#if defined(_M_IX86)
// The game's imports through their IAT cells: inside the injected game the cells hold
// the resolved WinAPI functions (stdcall as in windows.h); the exehost differential
// tests plant fakes there with fixtures::DwordPatch, so both sides share one clock.
using QpcFn = int32_t(__stdcall*)(void* out);  // BOOL QueryPerformanceCounter(LARGE_INTEGER*)
using InternetReadFn = int32_t(__stdcall*)(uintptr_t handle, void* buffer, uint32_t bytes,
                                           uint32_t* read);  // BOOL InternetReadFile(...)
using LastErrorFn = uint32_t(__stdcall*)(void);             // DWORD GetLastError(void)

constexpr uintptr_t kQpcImport = 0x00F953B8;
constexpr uintptr_t kInternetReadImport = 0x00F95958;
constexpr uintptr_t kLastErrorImport = 0x00F95380;
constexpr uintptr_t kThresholdCell = 0x011CB910;  // qword, engine-wide stop threshold
constexpr uintptr_t kSelectGateCell = 0x011E90F6;  // byte: slot-65 gate

uint64_t ReadClock(bool rdtscMode)
{
    if (rdtscMode)
        return __rdtsc();
    uint64_t now = 0;
    if (const auto qpc = *reinterpret_cast<const QpcFn*>(kQpcImport))
        qpc(&now);
    return now;
}

uint64_t ReadThreshold()
{
    return *reinterpret_cast<const uint64_t*>(kThresholdCell);
}

bool SelectGateOpen()
{
    return *reinterpret_cast<const uint8_t*>(kSelectGateCell) != 0;
}

int32_t InternetRead(uintptr_t handle, void* buffer, uint32_t bytes, uint32_t* read)
{
    const auto fn = *reinterpret_cast<const InternetReadFn*>(kInternetReadImport);
    return fn ? fn(handle, buffer, bytes, read) : 0;
}

uint32_t LastSocketError()
{
    const auto fn = *reinterpret_cast<const LastErrorFn*>(kLastErrorImport);
    return fn ? fn() : 0;
}
#else
// x64: no game image, so no imports and no threshold/gate cells. Every path that would
// touch them is unreachable in a 64-bit game build (the layout.h bridges abort first),
// and the reads fail closed: a null clock, a zero threshold, a closed gate, no read.
uint64_t ReadClock(bool rdtscMode)
{
    (void)rdtscMode;
    return 0;
}
uint64_t ReadThreshold()
{
    return 0;
}
bool SelectGateOpen()
{
    return false;
}
int32_t InternetRead(uintptr_t, void*, uint32_t bytes, uint32_t* read)
{
    (void)bytes;
    if (read)
        *read = 0;
    return 0;
}
uint32_t LastSocketError()
{
    return 0;
}
#endif

}  // namespace

// 0x004F3220: stop the stopwatch (see the header).
uint32_t TS3_OBJECTS_FASTCALL StopwatchStop4F3220(Stopwatch4F3220* self, void* /*edx*/)
{
    if (self->start == 0)
        return 0;
    // Unsigned wrap on both reads matches the original's 64-bit subtraction.
    const uint64_t elapsed = ReadClock(self->mode == 1) - self->start;
    const uint64_t threshold = ReadThreshold();
    if (elapsed <= threshold) {
        self->accum += 1;
        self->start = 0;
        return static_cast<uint32_t>(elapsed >> 32);
    }
    const uint64_t excess = elapsed - threshold;
    self->accum += excess;
    self->start = 0;
    return static_cast<uint32_t>(excess >> 32);
}

// 0x0094EEA0: the objects-selection entry (the 0x0094EEC7 body starts at the
// enabled-flag jump; the frame runs from 0x0094EEA0).
int32_t TS3_OBJECTS_FASTCALL ObjectsSelect94EEA0(SelectObject94EEA0* self, void* /*edx*/,
                                                 int32_t a1, int32_t a2, int32_t* out1,
                                                 int32_t* out2)
{
    if (!self->enabled) {
        *out1 = -1;
        *out2 = -1;
        return static_cast<int32_t>(static_cast<uint32_t>(reinterpret_cast<uintptr_t>(out1)));
    }
    // Order matches the original: singleton getter BEFORE the stopwatch start test.
    void* const obj = CallGameFunction<void*, 0x7692B0>();
    Stopwatch4F3220* const sw = &self->stopWatch;
    if (sw->start == 0)
        sw->start = ReadClock(sw->mode == 1);
    if (!self->skipPrepare)
        CallGameVirtual<void, 74>(obj, a1, a2, 1);
    *out1 = CallGameVirtual<int32_t, 64>(obj, a1, a2);
    *out2 = -1;
    if (*out1 >= 0 && SelectGateOpen())
        *out2 = CallGameVirtual<int32_t, 65>(obj, a1, a2);
    return static_cast<int32_t>(StopwatchStop4F3220(sw, nullptr));
}

// 0x009F5590: the network read with a local stopwatch (see the header).
int32_t TS3_OBJECTS_FASTCALL ObjectsRead9F5590(ReadObject9F5590* self, void* /*edx*/,
                                               int32_t a1, int32_t a2)
{
    if (!self->handle)
        return 0;
    Stopwatch4F3220 local{};
    CallGameMethod<void, 0x4F35B0>(&local, 3, 0);  // zeroes, mode 3, arg2 = 0: no clock
    if (local.start == 0)
        local.start = ReadClock(local.mode == 1);
    uint32_t out = 0;
    const int32_t ok = InternetRead(self->handle, reinterpret_cast<void*>(
                                                     static_cast<uintptr_t>(
                                                         static_cast<uint32_t>(a1))),
                                    static_cast<uint32_t>(a2), &out);
    if (!ok) {
        if (LastSocketError() == 0x2EFF)  // ERROR_INTERNET_CONNECTION_RESET (12031)
            self->timeout20 = 0;
        return 0;
    }
    StopwatchStop4F3220(&local, nullptr);
    const uint64_t elapsed = CallGameMethod<uint64_t, 0x4F32C0>(&local);
    self->elapsed28 = static_cast<uint32_t>(elapsed);
    self->received1C += out;
    self->budget14 -= out;
    self->elapsed2C = static_cast<uint32_t>(elapsed >> 32);
    return static_cast<int32_t>(out);
}

}  // namespace ts3::objects
