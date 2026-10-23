#include "ts3/world/clu_9dc550_World.h"

namespace ts3::world {

// TS3W.exe 0x009DC550 (thiscall, ret 0, result in EAX):
//   lea eax, [ecx + 0x1c]
//   cmp dword ptr [eax], eax
//   setne cl
//   mov al, cl
//   ret
// Compares the 32-bit pointer stored at +0x1c with the field's own address (the
// self-pointer sentinel). Returns 1 when they differ, 0 when the field points at
// itself. Reads one dword; no writes, no callees.
bool WorldSelfRef9dc550::IsRefNotSelf() const
{
    const uint8_t* base = reinterpret_cast<const uint8_t*>(this);
    const uint32_t* field = reinterpret_cast<const uint32_t*>(base + 0x1c);
    const uint32_t fieldAddr = static_cast<uint32_t>(reinterpret_cast<uintptr_t>(field));
    return *field != fieldAddr;
}

}  // namespace ts3::world
