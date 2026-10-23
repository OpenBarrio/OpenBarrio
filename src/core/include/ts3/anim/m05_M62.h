#pragma once
#include "ts3/layout.h"
namespace ts3::anim::m62 {
// TS3W.exe 0x00571300: unregisters this Scratch type and optionally frees it; returns self even after free.
void* Destroy1300(void* self, uint32_t flags);
// TS3W.exe 0x00571330: unregisters this Scratch type and optionally frees it; returns self even after free.
void* Destroy1330(void* self, uint32_t flags);
// TS3W.exe 0x00571360: unregisters this Scratch type and optionally frees it; returns self even after free.
void* Destroy1360(void* self, uint32_t flags);
// TS3W.exe 0x00571390: unregisters this Scratch type and optionally frees it; returns self even after free.
void* Destroy1390(void* self, uint32_t flags);
// TS3W.exe 0x005713C0: unregisters this Scratch type and optionally frees it; returns self even after free.
void* Destroy13C0(void* self, uint32_t flags);
// TS3W.exe 0x005713F0: unregisters this Scratch type and optionally frees it; returns self even after free.
void* Destroy13F0(void* self, uint32_t flags);
#if defined(_M_IX86)
namespace hooks {
void* __fastcall Destroy1300(void*, void*, uint32_t);
void* __fastcall Destroy1330(void*, void*, uint32_t);
void* __fastcall Destroy1360(void*, void*, uint32_t);
void* __fastcall Destroy1390(void*, void*, uint32_t);
void* __fastcall Destroy13C0(void*, void*, uint32_t);
void* __fastcall Destroy13F0(void*, void*, uint32_t);
}
#endif
}
