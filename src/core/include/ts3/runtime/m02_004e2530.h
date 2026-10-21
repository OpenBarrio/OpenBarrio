#pragma once
#include "ts3/runtime/m02_layouts.h"
namespace ts3::runtime::m02 {
// TS3W.exe 0x004E2530: release a writer or reader and notify waiting writers before readers; ownership is not checked.
int32_t RWRelease(RWLock* lock);
inline int32_t __fastcall RWReleaseEntry(RWLock* lock, void*) { return RWRelease(lock); }
}
