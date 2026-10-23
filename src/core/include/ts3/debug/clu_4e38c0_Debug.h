// Debug trace helper initializers from TS3W.exe 0x004E38C0..0x00F83E7C.
#pragma once

#include <cstdint>

#include "ts3/layout.h"

namespace ts3::debug {

// 0x004E3C50, __thiscall, returns this. Layout of the TempTraceHelperMap globals.
struct TempTraceHelperMap {
    uint8_t lock[0x34];
    uint32_t field34;
    uint32_t field38;
    uint32_t field3C;
    uint32_t field40;
    uint32_t field44;
    uint32_t field48;
    uint32_t name4C;
    uint8_t flag50;

    TempTraceHelperMap* Construct();
};
TS3_X86_OFFSET(TempTraceHelperMap, field34, 0x34);
TS3_X86_OFFSET(TempTraceHelperMap, name4C, 0x4C);
TS3_X86_SIZE(TempTraceHelperMap, 0x54);

// 0x004E38C0, __cdecl, returns the initialized global's first dword.
uint32_t* InitLogTraceHelpers();

// 0x004E3CB0, __cdecl, returns the TempTraceHelperMap global.
void* InitAssertTraceHelpers();

// 0x00F83E60, __cdecl, returns the game's atexit status.
int InitTempTraceHelperMap();

}  // namespace ts3::debug
