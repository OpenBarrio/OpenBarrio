#pragma once
#include "ts3/layout.h"
#include <cstdarg>
namespace ts3::tslhost {
using Wide = char16_t;
struct FormatSink { Wide* data; uint32_t position, capacity; };
TS3_X86_SIZE(FormatSink, 12);
struct Host { void* vtable; unsigned char opaque[316]; void* thread; };
TS3_X86_OFFSET(Host, thread, 0x140);
struct Logger { void* prefix[3]; Wide* logPath; Wide* gamePath; unsigned char opaque[12]; unsigned char mutex[24]; unsigned char tail[24]; int flags; };
TS3_X86_OFFSET(Logger, logPath, 12); TS3_X86_OFFSET(Logger, gamePath, 16);
TS3_X86_OFFSET(Logger, mutex, 32); TS3_X86_OFFSET(Logger, flags, 80);
struct alignas(16) Vec { float x,y,z,w; };
TS3_X86_SIZE(Vec,16);
struct LightRecord { uint32_t hash; unsigned char padding[12]; Vec direction[4], color[4], vertexDirection[4], vertexColor[4]; };
TS3_X86_SIZE(LightRecord,272);
struct LightState { void* prefix[3]; LightRecord* begin; LightRecord* end; LightRecord* capacity; unsigned char opaque[197]; uint8_t enabled; };
TS3_X86_OFFSET(LightState,end,16); TS3_X86_OFFSET(LightState,enabled,221);
struct LightContext { void* reader; LightState* state; };
// TS3W.exe 0x004F2D20: format UTF-16 text with the game's formatter.
int Format(Wide* out, uint32_t capacity, const Wide* format, ...);
// TS3W.exe 0x004FD370: format a va_list and terminate nonnegative results, including truncation.
int VFormat(Wide* out, uint32_t capacity, const Wide* format, va_list args);
// TS3W.exe 0x0060AE00: obtain the logger by subtracting its eight-byte interface prefix.
Logger* GetLogger();
// TS3W.exe 0x0060CB30: initialize launcher logging and the named-pipe listener.
struct HostApi : Host { uint32_t Init(); };
// TS3W.exe 0x006ACFC0: read vectors, optionally normalizing; lane four duplicates x.
void ReadVectors(void* reader, uintptr_t key, const Wide* prefix, Vec* out, int count, uint8_t normalize);
// TS3W.exe 0x006AD100: multiply vectors by per-index intensity values.
int ScaleVectors(void* reader, uintptr_t key, const Wide* prefix, Vec* out, int count);
// TS3W.exe 0x006AF9B0: read and append a four-light record when enabled.
uint8_t ReadLights(uintptr_t key, uintptr_t unused, LightContext* context);
struct LoggerApi : Logger {
// TS3W.exe 0x00898280: set the game log path under the recursive mutex.
int SetGamePath(const Wide* path);
// TS3W.exe 0x008982F0: set logging flags, constructing the game log path when enabled.
int SetFlags(int value);
// TS3W.exe 0x008985B0: set the launcher log path under the recursive mutex.
int SetLogPath(const Wide* path);
};
}
