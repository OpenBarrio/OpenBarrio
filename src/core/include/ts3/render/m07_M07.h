#pragma once
#include "ts3/render/m07_abi.h"
namespace ts3::render::m07::m07_init {
// Sparse initializer view. All resource/parameter handles are native pointers.
struct System {
    void** vtable;
    uint8_t prefix[8];
    void** secondary;
    uint32_t refs;
    void* parameters[55]; // image +14..EC
    uint8_t highRez, disableInstancing, reservedF2[2];
    void* resources[10]; // image +F4..118 (108/10C are untouched)
    void* techniques[4]; // image +11C..128
    uint8_t opaque12C[0x4C];
    uint8_t shadows, debug, minspec, originalShadows, capability, initialized;
    uint8_t opaque17E[0xCE];
    uint8_t helper[4]; // only its address is used
};
TS3_X86_OFFSET(System,parameters,0x14);
TS3_X86_OFFSET(System,resources,0xF4);
TS3_X86_OFFSET(System,shadows,0x178);
TS3_X86_OFFSET(System,helper,0x24C);
// Address-tagged typed boundary dispatch. a/b/c are pointer-width; numeric
// image data tags are converted to pointers only by the x86 original bridge.
using Dispatch = uintptr_t (*)(uint32_t,void*,uintptr_t,uintptr_t,uintptr_t);
uintptr_t OriginalDispatch(uint32_t,void*,uintptr_t,uintptr_t,uintptr_t);
// TS3W.exe 0x006E05A0: initialize render resources and shader bindings. Keeps AL-only tests, partial initialization and unchecked manager allocation failures.
uint8_t Initialize(System*,Dispatch = OriginalDispatch);

}

namespace ts3::render::m07 {
// Thiscall hook receiver; the sparse base has no native virtual members.
struct M07 : m07_init::System { uint8_t InitializeGame(); };
}
