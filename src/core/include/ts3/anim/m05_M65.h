#pragma once
#include "ts3/layout.h"
namespace ts3::anim::m65 {
struct Factory { void* vtable; };
// TS3W.exe 0x00571660: Unregister factory 0x067C5CAB, optionally free on bit 0, and return this even after free.
Factory* Delete1660(Factory*,uint32_t);
// TS3W.exe 0x00571690: Unregister factory 0x07A11BA8, optionally free on bit 0, and return this even after free.
Factory* Delete1690(Factory*,uint32_t);
// TS3W.exe 0x005716C0: Unregister factory 0x074A5F9D, optionally free on bit 0, and return this even after free.
Factory* Delete16C0(Factory*,uint32_t);
// TS3W.exe 0x005716F0: Unregister factory 0x0664ED68, optionally free on bit 0, and return this even after free.
Factory* Delete16F0(Factory*,uint32_t);
// TS3W.exe 0x00571720: Unregister factory 0x060D99B1, optionally free on bit 0, and return this even after free.
Factory* Delete1720(Factory*,uint32_t);
// TS3W.exe 0x00571750: Unregister factory 0x060CAEEF, optionally free on bit 0, and return this even after free.
Factory* Delete1750(Factory*,uint32_t);
}
#if defined(_M_IX86)
namespace ts3::anim::m65::hooks {
Factory* __fastcall Delete1660(Factory*,void*,uint32_t);
Factory* __fastcall Delete1690(Factory*,void*,uint32_t);
Factory* __fastcall Delete16C0(Factory*,void*,uint32_t);
Factory* __fastcall Delete16F0(Factory*,void*,uint32_t);
Factory* __fastcall Delete1720(Factory*,void*,uint32_t);
Factory* __fastcall Delete1750(Factory*,void*,uint32_t);
}
#endif
