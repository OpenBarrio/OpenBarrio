// Unit clu_4fed70_Renderer — TS3W.exe 0x004FED70 / 0x0083A6B0. See the header.
//
// 0x004FED70 (26 bytes):
//   mov eax,[esp+0xc] / mov ecx,[esp+8] / mov edx,[esp+4]
//   push 1 / push eax / push ecx / push edx / call 0x4fea70 / add esp,0x10 / ret
// 0x0083A6B0 (18 bytes):
//   mov eax,[esp+4] / push 0xa / push 0 / push eax / call 0x4fed70 / add esp,0xc / ret
// Both call sites are plain __cdecl (caller cleans) and forward their scalars bit for
// bit. 0x004FEA70 belongs to clu_4f35b0_LotRenderer and is not rewritten, so it goes
// through the bridge.

#include <cstdint>

#include "ts3/layout.h"
#include "ts3/renderer/clu_4fed70_Renderer.h"

namespace ts3::renderer {

std::int32_t Sub4FED70(std::uint32_t a1, std::uint32_t a2, std::uint32_t a3) {
    return ts3::CallGameFunction<std::int32_t, 0x004FEA70>(a1, a2, a3, 1u);
}

std::int32_t Sub83A6B0(std::uint32_t a1) {
    return Sub4FED70(a1, 0u, 10u);
}

}  // namespace ts3::renderer