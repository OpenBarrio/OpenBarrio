// Unit clu_6e7520_CAS: TS3W.exe 0x006E7520 (42 bytes), thiscall ret 4, one stack arg.
// Reference-counted pointer store at +0x24 of a Class_00F9A240 object.
#include "ts3/cas/clu_6e7520_CAS.h"

namespace ts3::cas {

// 0x006E7520:
//   mov edi, [esp+0xc]            ; edi = replacement (the stack arg)
//   if (edi) call edi->vt[0](edi) ; 0x005044C0 AddRef (InterlockedIncrement f_04)
//   ecx = [esi+0x24]              ; old value
//   if (ecx) call ecx->vt[1](ecx) ; 0x00692720 Release (InterlockedDecrement f_04)
//   [esi+0x24] = edi
//   ret 4                         ; eax = result of the last virtual call executed
int32_t CasRefSlot::SetReference(IReference* replacement)
{
    // EA: when both pointers are null no call is made and the original returns the
    // caller's pre-call EAX (undefined); all six callers ignore the return value,
    // so 0 is returned here in that case.
    int32_t result = 0;
    if (replacement)
        result = replacement->AddRef();
    if (value)
        result = value->Release();
    value = replacement;
    return result;
}

}  // namespace ts3::cas
