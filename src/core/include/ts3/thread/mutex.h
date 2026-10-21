// EA::Thread::Mutex (EAThread) as compiled into TS3W.exe 1.67.2: a recursive mutex backed
// by a CRITICAL_SECTION (intra-process, the usual case) or a Win32 mutex handle.
// The x86 layout is the game's, so rewritten code and original code can share objects.
#pragma once

#include <cstdint>

#include "ts3/layout.h"

namespace ts3::thread {

// Absolute time in GetTickCount() milliseconds.
using ThreadTime = uint32_t;
inline constexpr ThreadTime kTimeoutNone = 0xFFFFFFFFu;  // wait forever (the game's constant at 0x00FB24F8)
inline constexpr ThreadTime kTimeoutImmediate = 0;

inline constexpr int kResultError = -1;
inline constexpr int kResultTimeout = -2;

struct Mutex {
    // CRITICAL_SECTION when intraProcess, otherwise the mutex HANDLE in the first word.
    alignas(8) unsigned char data[sizeof(void*) == 4 ? 32 : 40];
    int lockCount;      // 0x20
    bool intraProcess;  // 0x24
    uint32_t unknown28[2];

    // TS3W.exe 0x004E16F0. Returns the new lock count, kResultTimeout or kResultError.
    int Lock(const ThreadTime& timeoutAbsolute = kTimeoutNone);
    // TS3W.exe 0x004E17B0. Returns the remaining lock count.
    int Unlock();
};
TS3_X86_OFFSET(Mutex, lockCount, 0x20);
TS3_X86_OFFSET(Mutex, intraProcess, 0x24);
TS3_X86_SIZE(Mutex, 0x30);

}  // namespace ts3::thread
