#pragma once
#include "ts3/layout.h"
#include <cstdint>
namespace ts3::resource::m03::m05 {
struct CacheRegistration { void* vtable; void* database; };
struct StreamAdapter { void* vtable; void* record; void* stream; uint32_t reserved[2]; uint8_t enabled; };
// TS3W.exe 0x0073CF20: start the script cache and validate or replace its metadata; AL always reports success, including startup failures.
uint8_t StartCache(CacheRegistration* self);
// TS3W.exe 0x007D8390: replace a non-memory stream with an owned memory-stream record, disabling adaptation on failure.
void AdaptStream(StreamAdapter* self);
// TS3W.exe 0x00F91910: destroy the locale mutex at process exit.
void DestroyLocaleMutex();
// TS3W.exe 0x00F925D0: destroy the directory mutex at process exit.
void DestroyDirectoryMutex();
#if defined(_M_IX86)
inline uint8_t __fastcall HookStartCache(CacheRegistration* self,void*) { return StartCache(self); }
inline void __fastcall HookAdaptStream(StreamAdapter* self,void*) { AdaptStream(self); }
#endif
}
