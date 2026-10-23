#include <cwchar>

#include "ts3/locomotion/clu_58abf0_Locomotion.h"

namespace ts3::locomotion {

// TS3W.exe 0x0058ABF0 (thiscall, plain ret, double in ST):
//
//   push ecx                     ; `this` saved, restored at the end, never used
//   call 0x0058B880              ; reg = global 0x011CDD00 (config registry singleton)
//   test eax, eax / je fallback  ; null registry -> fallback
//   ... push &slot, k2, k1 ...   ; slot is the pushed-`this` stack slot (uninitialized)
//   mov ecx, reg
//   call 0x0058C380              ; thiscall lookup(reg, k1, k2, &slot), ret 0xC
//   test al, al / je fallback    ; low-byte check on the 0/1 result
//   mov edx, [esp]               ; [esp] is the slot after the callee's ret 0xC
//   push 0 / push edx / call wcstod
//   add esp, 8 / pop ecx / ret
//   fallback: fld [esp+0x10]     ; the float argument, widened to double
//             pop ecx / ret
//
// Both dependencies are not rewritten yet, so they run through the bridge (x64 builds
// abort at the first bridge call - there is no game code to call).
double ConfigDoubleLookup::ConfigDouble(const wchar_t* key1, const wchar_t* key2,
                                        float fallback)
{
    // 0x0058B880: the original instruction sequence - the registry singleton getter.
    void* const registry = CallGameFunction<void*, 0x0058B880>();
    if (registry == nullptr)
        return static_cast<double>(fallback);

    // 0x0058C380: thiscall two-level registry lookup; the callee cleans its three
    // stack args (ret 0xC) and, on a hit, writes the value-string pointer into *out.
    wchar_t* value;  // EA: the slot is not initialized before the lookup (it holds
                      // `this`); it is only read when the lookup succeeds.
    // Only AL is defined by the lookup; dirty upper EAX bits must not turn a miss into a hit.
    if (CallGameMethod<uint8_t, 0x0058C380>(registry, key1, key2, &value) != 0)
    {
        // The original calls the game's IAT slot 0x00F954DC (MSVCR80!wcstod), not the
        // host CRT. MSVCR80 parses "inf"/"nan"/"0x10" differently from UCRT (it
        // returns 0 for them), so preserve the exact game call by dereferencing the
        // IAT slot the way the original instruction sequence does. (x86 only in practice:
        // x64 builds abort at the CallGameFunction above, so this line is never
        // reached there.)
        using WcstodFn = double(__cdecl*)(const wchar_t*, wchar_t**);
        const WcstodFn wcstod = *reinterpret_cast<WcstodFn const*>(uintptr_t{0x00F954DC});
        return wcstod(value, nullptr);
    }
    return static_cast<double>(fallback);
}

}  // namespace ts3::locomotion
