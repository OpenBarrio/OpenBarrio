#pragma once
#include <cstdint>

namespace ts3::automationutils {
// Shared snapshot request storage (EA 0x011CDBC8 on x86).
int32_t& AutomationSnapshotState();
// TS3W.exe 0x005887E0 and script bridge 0x0073C0F0, respectively.
int32_t __cdecl SetAutomationSnapshot(int32_t value);
int32_t __cdecl AutomationTakeSnapShot(int32_t value);
}  // namespace ts3::automationutils
