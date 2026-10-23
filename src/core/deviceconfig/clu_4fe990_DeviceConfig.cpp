#include "ts3/deviceconfig/clu_4fe990_DeviceConfig.h"
#include "ts3/layout.h"
namespace ts3::deviceconfig {
// 0x004FE990: The sign flag retains the input's upper 24 bits, an EA partial-byte write.
char16_t* FormatProperty(int32_t value, void* buffer, int32_t radix) {
    const bool negative=value<0 && radix==10;
    const uint32_t sign=(static_cast<uint32_t>(value)&0xFFFFFF00u) | (negative?1u:0u);
    const uint32_t magnitude=negative ? 0u-static_cast<uint32_t>(value) : static_cast<uint32_t>(value);
#if defined(_M_IX86)
    // 0x004FE900 is __usercall: EAX=radix plus four cdecl stack words.
    // The standard bridge has no EAX-argument form. This is only an ABI adapter;
    // all conversion logic remains in the unreconstructed game dependency.
    const uintptr_t target=0x4FE900;
    char16_t* result;
    __asm {
        push sign
        push buffer
        push 0
        push magnitude
        mov eax,radix
        call dword ptr [target]
        add esp,16
        mov result,eax
    }
    return result;
#else
    (void)buffer; (void)magnitude; (void)sign;
    AbortNotRewritten("DeviceConfig EAX-radix conversion dependency"); return nullptr;
#endif
}
// 0x00740F00
uintptr_t GetProperty(uint32_t key, int32_t value) {
    auto strings=CallGameFunction<void*,0x7692B0>();
    auto config=CallGameFunction<void*,0x58B880>();
    auto name=CallGameVirtual<char16_t*,22>(strings,key);
    if (!name || !config) return reinterpret_cast<uintptr_t>(name);
    char16_t buffer[32]{};
#if defined(_M_IX86)
    auto section=reinterpret_cast<const char16_t*>(0xFCC908);
#else
    AbortNotRewritten("DeviceConfig property section");
    const char16_t* section=nullptr;
#endif
    return CallGameMethod<uintptr_t,0x58C9D0>(config,section,name,FormatProperty(value,buffer,10));
}
}
