#pragma once
#include "ts3/layout.h"
namespace ts3::render::m07::m12 {
// TS3W.exe 0x0071DA00: dispatch Swarm messages; preserves AL-only success and ignored-message EAX arithmetic.
uintptr_t __fastcall Dispatch(void* self, void*, uint32_t message, void* data);
// TS3W.exe 0x0071F5A0: resolve and append a shader parameter record; leaves EA's unspecified record fields untouched.
uint8_t __fastcall AddRecord(void* self, void*, uint32_t key, void* data, uint32_t* index);
}
