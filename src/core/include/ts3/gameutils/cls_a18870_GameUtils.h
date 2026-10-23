#pragma once
#include <cstddef>
#include <cstdint>
#include "ts3/layout.h"

namespace ts3::gameutils {

// Opaque x86 GameUtils/HTTP request object. The original accesses fields up to 0x161B7.
struct GameUtils {
    uint8_t bytes[0x161B8];
};
TS3_X86_SIZE(GameUtils, 0x161B8);

// TS3W.exe 0x00A18870 (__cdecl).
int32_t ProcessResponse(GameUtils* self);
// TS3W.exe 0x00A19470 (__thiscall constructor).
int32_t __fastcall Construct(GameUtils* self, void*);
// TS3W.exe 0x00A1A270 (__cdecl).
int32_t ProcessRequest(GameUtils* self, char* request, int32_t flags);

} // namespace ts3::gameutils

