#include "ts3/automationutils/clu_5887e0_AutomationUtils.h"

namespace ts3::automationutils {
namespace {
#if !defined(_M_IX86)
int32_t automationSnapshotValue;
#endif
}

// EA global 0x011CDBC8 must remain visible to unrewritten game consumers on x86.
// The native build owns equivalent storage rather than using an x86 address.
int32_t& AutomationSnapshotState()
{
#if defined(_M_IX86)
    return *reinterpret_cast<int32_t*>(uintptr_t{0x011CDBC8});
#else
    return automationSnapshotValue;
#endif
}

// TS3W.exe 0x005887E0 (__cdecl, ret): store all 32 bits and return them unchanged.
int32_t __cdecl SetAutomationSnapshot(int32_t value)
{
    AutomationSnapshotState() = value;
    return value;
}

// TS3W.exe 0x0073C0F0: script bridge tail jump to 0x005887E0.
int32_t __cdecl AutomationTakeSnapShot(int32_t value)
{
    return SetAutomationSnapshot(value);
}
}  // namespace ts3::automationutils
