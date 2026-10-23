// Cluster unit clu_4d8c80_GameUtils (subsystem GameUtils): the engine singleton getter
// 0x004D8C80, the renderer extent read-out 0x0060E6A0, the two script-class registration
// thunks 0x007EAEC0 / 0x007EAF10, the UI launcher-host getter 0x007EFE60 and the launcher
// script bridge 0x00EC1600.
//
// Only 0x004D8C80, 0x0060E6A0 and 0x007EFE60 are leaves; 0x007EAEC0 / 0x007EAF10 call
// through the GameUtils singleton's vtable and 0x00EC1600 drives three singletons
// (0x0050AB70 UIManager, 0x0060E2C0 Renderer, 0x007EFE00 UI manager) plus four vtable
// calls on the launcher object it creates. The unrewritten calls go through the layout.h
// bridge, so the x64 build compiles and traps rather than reading a game address.
#pragma once

#include <cstdint>

#include "ts3/layout.h"

namespace ts3::gameutils {

// 0x004D8C80: __cdecl, the original instruction sequence. The global holds the process-wide
// GameUtils singleton pointer (0x004D8C90 is its setter); null until the engine sets it.
// No null check anywhere here: the callers dereference the result.
void* GetGameUtilsInstance();

// View of the object returned by the renderer singleton getter 0x0060E2C0
// (Class_00FBF760, global 0x11CECC8): the unit reads only the three dwords at +0x2C.
struct RendererExtentSource {
    uint8_t pad00[0x2C];
    int32_t extentA;  // +0x2C
    int32_t extentB;  // +0x30
    int32_t extentC;  // +0x34

    // 0x0060E6A0: __thiscall, the original instruction sequence. Copies +0x2C/+0x30/+0x34 through the three out
    // pointers in ECX's object's field order and returns the +0x34 value (EAX still holds
    // it when the original returns). The original stores unconditionally, so a null out
    // pointer faults exactly as it does there.
    int32_t GetExtent(void* outA, void* outB, void* outC);
};

// View of the object returned by the UI-manager getter 0x007EFE00 (global 0x11D8C44);
// the unit reads only the dword at +0x3CC.
struct LauncherHost {
    uint8_t pad00[0x3CC];
    int32_t launcherId;  // +0x3CC

    // 0x007EFE60: __thiscall, plain `ret`; the dword at +0x3CC.
    int32_t GetLauncherId() const;
};

// 0x007EAEC0 / 0x007EAF10: __cdecl, script-class registration thunks. Both are
// `call 0x4D8C80; mov edx,[eax]; mov ecx,eax; mov eax,[edx+4]; push 0; push <descriptor>;
// call eax; ret` - vtable slot 1 of the singleton with one of the two adjacent descriptor
// table slots at 0x01165908 / 0x0116590C, returning the callee's raw EAX.
void* RegisterScriptClassA();
void* RegisterScriptClassB();

// 0x00EC1600 ("GameUtils_StartLauncherImpl"): __cdecl script bridge, 214 bytes.
// Builds the launcher window 0x034B248B from the UIManager (0x0050AB70), the GameUtils
// singleton and the Renderer singleton, then hands the UI-manager object 0x007EFE00's
// +0x3CC id to vtable slot 66 of the new object and returns that call's EAX; 0 when the
// singleton refuses to create the object or 0x007EFE00 returns null.
uint32_t StartLauncherImpl();

}  // namespace ts3::gameutils