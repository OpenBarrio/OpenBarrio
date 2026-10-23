// Unit clu_417130_Renderer — TS3W.exe 0x00417130. See the header.
//
// 0x00417130:
//   cmp byte [0x11B02A0], 0 / jne done
//   push 0x1146108 / mov byte [0x11B02A0], 1 / call 0x446270 / pop ecx / ret
// The target 0x00446270 is not rewritten, so it goes through the layout.h bridge (x64
// builds fail closed). The guard byte lives in the game image, which is why the code is
// x86-only; the x64 build has no game image to guard.

#include <cstdint>

#include "ts3/layout.h"
#include "ts3/renderer/clu_417130_Renderer.h"

namespace ts3::renderer {
namespace {

constexpr std::uintptr_t kInitGuard = 0x011B02A0;  // byte set by the first call
constexpr std::uintptr_t kInitData = 0x01146108;   // argument of 0x00446270

}  // namespace

std::int32_t Init417130() {
#if defined(_M_IX86)
    if (*reinterpret_cast<const std::uint8_t*>(kInitGuard) == 0) {
        *reinterpret_cast<std::uint8_t*>(kInitGuard) = 1;
        return ts3::CallGameFunction<std::int32_t, 0x00446270>(
            reinterpret_cast<void*>(kInitData));
    }
    // Already initialised: the original returns with EAX untouched (stale). Every caller
    // ignores it; there is nothing portable to return but zero.
    return 0;
#else
    ts3::AbortNotRewritten("clu_417130_Renderer init");
    return 0;
#endif
}

}  // namespace ts3::renderer