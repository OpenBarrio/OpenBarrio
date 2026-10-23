#pragma once
#include <cstdint>
namespace ts3::anim {
struct m64 {
// TS3W.exe 0x00571540: Unregister this factory, optionally free on bit 0, and return this even after free.
void* Destroy00571540(uint32_t flags);
// TS3W.exe 0x00571570: Unregister this factory, optionally free on bit 0, and return this even after free.
void* Destroy00571570(uint32_t flags);
// TS3W.exe 0x005715A0: Unregister this factory, optionally free on bit 0, and return this even after free.
void* Destroy005715A0(uint32_t flags);
// TS3W.exe 0x005715D0: Unregister this factory, optionally free on bit 0, and return this even after free.
void* Destroy005715D0(uint32_t flags);
// TS3W.exe 0x00571600: Unregister this factory, optionally free on bit 0, and return this even after free.
void* Destroy00571600(uint32_t flags);
// TS3W.exe 0x00571630: Unregister this factory, optionally free on bit 0, and return this even after free.
void* Destroy00571630(uint32_t flags);
};
}
