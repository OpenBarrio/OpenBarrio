#pragma once
// Cluster unit clu_4f3220_Objects (objects subsystem): TS3W.exe 0x004F3220 (150 B,
// the shared stopwatch's "stop"), 0x0094EEC7 (170, the objects-selection body inside
// the function at 0x0094EEA0 — the callable thiscall frame starts at 0x0094EEA0, 209 B — this entry
// has EDI as `this` and needs the prologue frame) and 0x009F5590 (185, a network read
// driven by a local stopwatch). All three are thiscall with `ret 0` / `ret 0x10` /
// `ret 8` respectively.
//
// The unit's dependencies are NOT rewritten and run through the layout.h bridge:
// 0x007692B0 (the objects singleton getter, returns the pointer stored at 0x011D4BBC, part of
// clu_5a4cb0_CASUtils), the stopwatch constructor 0x004F35B0 and the elapsed converter
// 0x004F32C0 (clu_4f35b0_LotRenderer), and virtual slots 74/64/65 of the object the
// singleton getter returns. Clocks come from the game's own imports through their IAT
// cells (QueryPerformanceCounter 0x00F953B8, InternetReadFile 0x00F95958, GetLastError
// 0x00F95380) or rdtsc; x64 has no game image, so every import/cell read fails closed
// (null clock, closed gate, no read) — those paths are unreachable in a 64-bit game
// build anyway (the bridges abort first).
#include <cstdint>

#include "ts3/layout.h"

#if defined(_MSC_VER) && defined(_M_IX86)
#define TS3_OBJECTS_FASTCALL __fastcall
#else
#define TS3_OBJECTS_FASTCALL
#endif

namespace ts3::objects {

// 0x004F3220's this object (0x18 bytes; the 0x004F35B0 ctor writes the float at +0x14
// too, so the object cannot be smaller). thiscall with no stack arguments.
struct Stopwatch4F3220 {
    uint64_t start;  // +0x00: clock reading at the last start; 0 = stopped
    uint64_t accum;  // +0x08: accumulated ticks (scaled by 0x004F32C0's conversion)
    uint32_t mode;   // +0x10: 1 = rdtsc, anything else = QueryPerformanceCounter
    float scale;     // +0x14: scale factor written by the 0x004F35B0 ctor
};
TS3_X86_OFFSET(Stopwatch4F3220, start, 0x00);
TS3_X86_OFFSET(Stopwatch4F3220, accum, 0x08);
TS3_X86_OFFSET(Stopwatch4F3220, mode, 0x10);
TS3_X86_OFFSET(Stopwatch4F3220, scale, 0x14);
TS3_X86_SIZE(Stopwatch4F3220, 0x18);

// The this object of 0x0094EEA0. Largest access: the stopwatch's mode dword at +0x58; the
// type is certainly larger in the game (the +0x41 flag byte suggests a 0x48+ header).
// sizeof is 0x60 on both widths (the trailing stopwatch forces 8-byte alignment while
// the game type need not), so assert the offsets the game uses, not the size.
struct SelectObject94EEA0 {
    uint8_t padToEnabled[0x41];
    uint8_t enabled;      // +0x41: 0 = the selector is off (outputs -1, returns out1)
    uint8_t skipPrepare;  // +0x42: nonzero = skip the slot-74 prepare call
    uint8_t padToStopwatch[5];
    Stopwatch4F3220 stopWatch;  // +0x48
};
TS3_X86_OFFSET(SelectObject94EEA0, enabled, 0x41);
TS3_X86_OFFSET(SelectObject94EEA0, skipPrepare, 0x42);
TS3_X86_OFFSET(SelectObject94EEA0, stopWatch, 0x48);

// 0x009F5590's this object (0x30 bytes; largest access: the elapsed-high dword +0x2C).
struct ReadObject9F5590 {
    uint8_t padToHandle[0x0C];
    uint32_t handle;      // +0x0C: HINTERNET; 0 = nothing to read (early return)
    uint8_t padToBudget[4];
    uint32_t budget14;    // +0x14: byte budget; decremented by the bytes read
    uint8_t padToReceived[4];
    uint32_t received1C;  // +0x1C: total bytes read; incremented by the bytes read
    uint8_t timeout20;    // +0x20: cleared when InternetReadFile fails with 0x2EFF
    uint8_t padToElapsed[7];
    uint32_t elapsed28;   // +0x28: converted stopwatch value, low dword
    uint32_t elapsed2C;   // +0x2C: converted stopwatch value, high dword
};
TS3_X86_OFFSET(ReadObject9F5590, handle, 0x0C);
TS3_X86_OFFSET(ReadObject9F5590, budget14, 0x14);
TS3_X86_OFFSET(ReadObject9F5590, received1C, 0x1C);
TS3_X86_OFFSET(ReadObject9F5590, timeout20, 0x20);
TS3_X86_OFFSET(ReadObject9F5590, elapsed28, 0x28);
TS3_X86_OFFSET(ReadObject9F5590, elapsed2C, 0x2C);
TS3_X86_SIZE(ReadObject9F5590, 0x30);

// 0x004F3220 (150 B): stop the stopwatch. Returns 0 and touches nothing when start is
// 0. Otherwise reads the clock (mode 1 = rdtsc, else the game's QueryPerformanceCounter
// import), compares the elapsed time unsigned against the engine-wide threshold qword
// at 0x011CB910 (high dword first, low dword only on an equal high half), adds 1 tick
// to accum when elapsed <= threshold and the elapsed-minus-threshold excess otherwise,
// zeroes start and returns the high half of the elapsed time (of the excess on the big
// path). No original caller reads the return value. Keep the start==0 early-out: it is
// what makes a stop idempotent.
uint32_t TS3_OBJECTS_FASTCALL StopwatchStop4F3220(Stopwatch4F3220* self, void* edx);

// 0x0094EEA0 (the frame around the 0x0094EEC7 body; 209 B): objects selection.
// When +0x41 is 0 both outputs are set to -1 and the out1 POINTER (truncated to 32
// bits) is returned. Otherwise the singleton from 0x007692B0 is asked: virtual slot 74
// (a1, a2, 1) unless +0x42 skips the prepare, then slot 64 (a1, a2) whose result goes
// to *out1; *out2 is set to -1 and slot 65 (a1, a2) fills it only when *out1 >= 0 AND
// the flag byte at 0x011E90F6 is set (the *out1 re-read matters: the original compares
// through the out1 pointer, so out1 == out2 aliasing is observable). The stopwatch at
// +0x48 is started (only if it was stopped, only if it is not already running) around
// the calls, and the stop's result is returned (0 on a fresh stopwatch).
int32_t TS3_OBJECTS_FASTCALL ObjectsSelect94EEA0(SelectObject94EEA0* self, void* edx,
                                                 int32_t a1, int32_t a2, int32_t* out1,
                                                 int32_t* out2);

// 0x009F5590 (185 B): read through the +0x0C Internet handle. Returns 0 untouched when
// the handle is 0. Otherwise constructs a local stopwatch with the game's 0x004F35B0
// ctor (mode 3, arg2 = 0 => no clock start), starts it (rdtsc/QPC as above), zeroes the
// byte counter and calls the game's InternetReadFile import (handle, a1, a2, &out).
// Failure returns 0 and clears +0x20 when GetLastError() is 0x2EFF
// (ERROR_INTERNET_CONNECTION_RESET, 12031). Success stops the stopwatch, stores the 0x004F32C0
// conversion (edx:eax) to +0x28/+0x2C, adds the bytes read to +0x1C, subtracts them
// from +0x14 and returns the bytes read.
int32_t TS3_OBJECTS_FASTCALL ObjectsRead9F5590(ReadObject9F5590* self, void* edx,
                                               int32_t a1, int32_t a2);

}  // namespace ts3::objects
