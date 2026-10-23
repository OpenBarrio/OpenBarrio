// Unit clu_60e6c0_UserTool (UserTool): the three-dword state reader. Semantics from the
// TS3W.exe 1.67.2 listing (the original address is noted on the function).
#include "ts3/usertool/clu_60e6c0_UserTool.h"

namespace ts3::usertool {

// TS3W.exe 0x0060E6C0 (thiscall, ret 0xC):
//   mov eax, [ecx + 0xE0]     state = this->state
//   mov ecx, [eax + 0x184]    *a2 = state->field184
//   add eax, 0x184
//   mov ecx, [eax + 4]        *a3 = state->field188
//   mov eax, [eax + 0x30]     result = state->field1B4
//   *a4 = result; return result
// No null check on `state` is performed (EA quirk kept: a null state faults).
int32_t UserToolStateRef::ReadStateFields(int32_t* a2, uintptr_t* a3, int32_t* a4)
{
    const UserToolStateBlock* block = static_cast<const UserToolStateBlock*>(state);
    *a2 = block->field184;
    *a3 = block->field188;
    const int32_t result = block->field1B4;
    *a4 = result;
    return result;
}

}  // namespace ts3::usertool
