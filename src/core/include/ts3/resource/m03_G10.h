#pragma once
#include "ts3/resource/m03_A06.h"
namespace ts3::resource::m03::g10 {
using Context=a06::Context;
// TS3W.exe 0x00726480: materializes a tagged child; clears the tag even on allocation failure. AL is defined.
uint8_t __fastcall Materialize(Context*,void*,uint32_t);
// TS3W.exe 0x007266F0: resolves context resources, reverse-order secondary refs and tagged children. AL is defined.
uint8_t __stdcall Resolve(void*,Context*);
// TS3W.exe 0x00727540: copies a record stream into an owning memory-backed record. AL is defined.
uint8_t __cdecl CopyRecord(void**);
}
