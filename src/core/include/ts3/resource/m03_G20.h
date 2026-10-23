#pragma once
#include "ts3/layout.h"
#include <cstdint>
namespace ts3::resource::m03::g20 {
// TS3W.exe 0x00731AE0: load configured preloader ranges and explicit keys; preserves low-byte config tests and cleanup return.
uintptr_t __fastcall Configure(void* self,void*);
// TS3W.exe 0x00736420: destroy main resource service members in ownership order, then tail-call manager destruction.
uintptr_t __fastcall Destroy(void* self,void*);
}
