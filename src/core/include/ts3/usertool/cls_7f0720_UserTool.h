#pragma once

#include <cstdint>

namespace ts3::usertool {

// TS3W.exe 0x007F0720, 0x007F07F0, and 0x007F0FB0 are __thiscall methods
// with one stack argument (ret 4).
void __fastcall ProcessUserToolNode(void* self, void* edx, uint32_t key);
void __fastcall RemoveUserToolNodes(void* self, void* edx, uint32_t key);
void __fastcall RefreshUserToolNodes(void* self, void* edx, void* object);

// TS3W.exe 0x00EC6060 is the shared implementation entered by the
// 0x00EC6190 vtable thunk; its ECX is the adjusted (base) object pointer.
uint8_t __fastcall ShowIntroScreen(void* self, void* edx);

} // namespace ts3::usertool
