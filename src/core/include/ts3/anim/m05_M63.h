#pragma once
#include <cstdint>
namespace ts3::anim::m63 {
// TS3W.exe 0x00571420: Unregister descriptor 87EB0331, optionally free on bit 0, and return this even after free.
void* Delete0(void*, uint32_t flags);
// TS3W.exe 0x00571450: Unregister descriptor 060D998E, optionally free on bit 0, and return this even after free.
void* Delete1(void*, uint32_t flags);
// TS3W.exe 0x00571480: Unregister descriptor 065F6BC2, optionally free on bit 0, and return this even after free.
void* Delete2(void*, uint32_t flags);
// TS3W.exe 0x005714B0: Unregister descriptor 06662B00, optionally free on bit 0, and return this even after free.
void* Delete3(void*, uint32_t flags);
// TS3W.exe 0x005714E0: Unregister descriptor 0B1DC7DC, optionally free on bit 0, and return this even after free.
void* Delete4(void*, uint32_t flags);
// TS3W.exe 0x00571510: Unregister descriptor 0BFF764E, optionally free on bit 0, and return this even after free.
void* Delete5(void*, uint32_t flags);
#if defined(_M_IX86)
void* __fastcall HookDelete0(void*, void*, uint32_t);
void* __fastcall HookDelete1(void*, void*, uint32_t);
void* __fastcall HookDelete2(void*, void*, uint32_t);
void* __fastcall HookDelete3(void*, void*, uint32_t);
void* __fastcall HookDelete4(void*, void*, uint32_t);
void* __fastcall HookDelete5(void*, void*, uint32_t);
#endif
}
